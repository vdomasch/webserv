#include "CGIContent.hpp"
#include "HTTPConfig.hpp"
#include <sys/types.h>
#include <sys/wait.h>

int	stock_childpid(int pid, bool replace);

CGIContent::CGIContent()
{
	this->_cgi_path = "default";
	this->_cgi_env_map.clear();
	this->_argv.clear();
	this->_cgi_env.clear();
	this->_env_storage.clear();
	this->_argv_storage.clear();
	this->_exitcode = 0;
	this->pipe_in[0] = -42;
	this->pipe_in[1] = -42;
	this->pipe_out[0] = -42;
	this->pipe_out[1] = -42;
}

CGIContent::~CGIContent()
{
	this->_cgi_env_map.clear();
}

void 	CGIContent::setEnvCGI(std::string cgi_path, std::string type, std::string len, std::string method, bool& is_php_cgi)
{	
	std::string	script_name_var;
	size_t		pos;

	pos = cgi_path.find("/cgi-bin");
	script_name_var = cgi_path.substr(pos);
	_cgi_path = cgi_path;

	if (method == "GET")
		_cgi_env_map["QUERY_STRING"] = type;
	else
	{
		_cgi_env_map["CONTENT_LENGTH"] = len;
		_cgi_env_map["CONTENT_TYPE"] = type;
	}
	_cgi_env_map["REQUEST_METHOD"] = method;
	_cgi_env_map["SCRIPT_FILENAME"] = cgi_path;
	_cgi_env_map["SCRIPT_NAME"] = script_name_var;
	_cgi_env_map["REDIRECT_STATUS"] = "200";


	for (std::map<std::string, std::string>::iterator it = _cgi_env_map.begin(); it != _cgi_env_map.end(); ++it)
		_env_storage.push_back(it->first + "=" + it->second);

	for (std::vector<std::string>::iterator it = _env_storage.begin(); it != _env_storage.end(); ++it)
		_cgi_env.push_back(const_cast<char*>(it->c_str()));
	_cgi_env.push_back(NULL);


	if (cgi_path.find(".php") != std::string::npos)
	{
		_argv_storage.push_back("/usr/bin/php-cgi");
		is_php_cgi = true;
	}
	else if (cgi_path.find(".py")  != std::string::npos)
		_argv_storage.push_back("/usr/bin/python3");
	_argv_storage.push_back(cgi_path);

	for (std::vector<std::string>::iterator it = _argv_storage.begin(); it != _argv_storage.end(); ++it)
		_argv.push_back(const_cast<char*>(it->c_str()));
	_argv.push_back(NULL);

}

void 	CGIContent::executeCGI(bool &exec_failed)
{	
	if (pipe(this->pipe_in))  //pipe_in[0] is read end of pipe, pipe_in[1] is to write to it 
	{
		std::cerr << "\033[31mError: Pipe failed ... Womp Womp ...\033[0m\n\n" << std::endl;
		this->_exitcode = -1;
		return ;
	}
	if (pipe(this->pipe_out))
	{
		std::cerr << "\033[31mError: Pipe failed ... Womp Womp ...\033[0m\n\n" << std::endl;
		this->_exitcode = -1;
		return ;
	}

	this->cgi_forkfd = fork();
	stock_childpid(this->cgi_forkfd, true);

	if (this->cgi_forkfd == 0)
	{

		int orig_stdout = dup(STDOUT_FILENO);
		int orig_stdin = dup(STDIN_FILENO);

		dup2(pipe_in[0], STDIN_FILENO);
		dup2(pipe_out[1], STDOUT_FILENO);
		close(pipe_in[0]);
		close(pipe_in[1]);
		close(pipe_out[0]);
		close(pipe_out[1]);
		
		// _argv[0] = (char*)"/not/a/real/path";
		this->_exitcode = execve(_argv[0], &_argv[0], &_cgi_env[0]);
		std::cerr << "[child process] Error: Execve failed !\r\n";

		// by this point, the output of the CGI script was written to pipe_out[1] (the write end of the pipe), since it was designated as the STDOUT_FILENO
		// and is waiting to be read using pipe_out[0] (which is the read end of the pipe)
		
		dup2(orig_stdout, STDOUT_FILENO);
		dup2(orig_stdin, STDIN_FILENO);
    	close(orig_stdout);
    	close(orig_stdin);

		g_running = false;
		exec_failed = true;
	}
	else if (this->cgi_forkfd == -1)
	{
		std::cerr << "\033[Error: Fork failed ... Womp Womp ...\033[0m\n\n" << std::endl;
		this->_exitcode = -1;
		return ;
	}
	else
	{
		close(pipe_in[0]);    // Parent doesn't read from pipe_in
    	close(pipe_out[1]);   // Parent doesn't write to pipe_out
	}
}

int	CGIContent::sendCGIBody(std::string body)
{
	size_t total_written = 0;

	while (total_written < body.size()) 
	{
		// write body to pipe_in[1], so that it can be grabbed by pipe_in[0] in the child (dupped as stdin)
		ssize_t written = write(pipe_in[1], body.data() + total_written, body.size() - total_written);
		if (written <= 0) {
			std::cerr << "Error: CGI Write failed !" << std::endl;
			return (-1);
		}
		total_written += written;
	}
	close(pipe_in[1]);	// Signal EOF to child
	return (0);
}


std::string	CGIContent::grabCGIBody(int child_pid, int timeout_sec, int &status)
{
	std::string	result;
	char		buffer[CGI_BUFFERSIZE];
	int			bytes_read = 0;

	// Set pipe to non-blocking mode
	int flags = fcntl(this->pipe_out[0], F_GETFL, 0);
	fcntl(this->pipe_out[0], F_SETFL, flags | O_NONBLOCK);

	time_t start = time(0);
	bool done = false;

	while (!done)
	{
		bytes_read = read(this->pipe_out[0], buffer, CGI_BUFFERSIZE);
		if (bytes_read > 0) {
			result.append(buffer, bytes_read);
		} else if (bytes_read == 0) {
			//pipe closed-->> no more data
			done = true;
			break;
		} else if (bytes_read == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
			// no data right now-> keep looping
		} else {
			std::cerr << "Error: Read error ! \n";
			close(this->pipe_out[0]);
			this->_exitcode = -1;
			return "";
		}

		// is child exited ?
		pid_t ret = waitpid(child_pid, &status, WNOHANG);
		if (ret == child_pid) {
			//yes, read the rest
			continue;
		}

		//timeout
		if (time(0) - start >= timeout_sec) {
			kill(child_pid, SIGKILL);
			stock_childpid(-42, true);
			waitpid(child_pid, &status, 0);
			done = true;
			break;
		}
	}

	close(this->pipe_out[0]);
	return result;
}

// std::string    CGIContent::grabCGIBody()
// {
//     std::string    result;
//     char        buffer[CGI_BUFFERSIZE] = {0};
//     int            bytes_read = 0;
//     int            total_read = 0;


//     while ((bytes_read = read(this->pipe_out[0], buffer, CGI_BUFFERSIZE)) > 0)
//     {
//         result.append(buffer, bytes_read);
//         total_read += bytes_read;
//     }
//     if (bytes_read < 0) {
//         std::cerr << "Error: Read error ! \n";
//         close(this->pipe_out[0]);
//         this->_exitcode = -1;
//         return("");
//     }
    
//     close(this->pipe_out[0]);

//     return (result);
// }

int	CGIContent::get_exitcode()	{ return _exitcode; }