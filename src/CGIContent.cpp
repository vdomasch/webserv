#include "CGIContent.hpp"
#include "HTTPConfig.hpp"

CGIContent::CGIContent()
{
	this->_cgi_path = "default";
	this->_cgi_env_map.clear();
	this->_argv.clear();
	this->_cgi_env.clear();
	this->_env_storage.clear();
	this->_argv_storage.clear();
	this->_exitcode = -42;
	this->pipe_in[0] = -42;
	this->pipe_in[1] = -42;
	this->pipe_out[0] = -42;
	this->pipe_out[1] = -42;
}

CGIContent::~CGIContent()
{
	this->_cgi_env_map.clear();
}

void 	CGIContent::setEnvCGI(std::string cgi_path, std::string type, std::string len, std::string method, bool& is_php_cgi) // string for now, replace by iterator of whatever struct we use
{	
	std::string	script_name_var;
	size_t		pos;

	pos = cgi_path.find("/cgi-bin"); // i guess the name could change ??? to check
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

void 	CGIContent::executeCGI()
{	
	if (pipe(this->pipe_in))  //pipe_in[0] is read end of pipe, pipe_in[1] is to write to it 
	{
		std::cerr << "\033[31mPipe failed ... Womp Womp ...\033[0m\n\n" << std::endl;
		this->_exitcode = -1; // to change
		return ;
	}
	if (pipe(this->pipe_out))
	{
		std::cerr << "\033[31mPipe failed ... Womp Womp ...\033[0m\n\n" << std::endl;
		this->_exitcode = -1; // to change
		return ;
	}

	this->cgi_forkfd = fork();
	
	if (this->cgi_forkfd == 0)
	{
		dup2(pipe_in[0], STDIN_FILENO);
		dup2(pipe_out[1], STDOUT_FILENO);
		close(pipe_in[0]);
		close(pipe_in[1]);
		close(pipe_out[0]);
		close(pipe_out[1]);
		
		this->_exitcode = execve(_argv[0], &_argv[0], &_cgi_env[0]);
		std::cerr << "EXECVE FAILED !\r\n";

		// by this point, the output of the CGI script was written to pipe_out[1] (the write end of the pipe), since it was designated as the STDOUT_FILENO
		// and is waiting to be read using pipe_out[0] (which is the read end of the pipe)
		
		exit(this->_exitcode); // if fails
	}
	else if (this->cgi_forkfd == -1)
	{
		std::cerr << "\033[Fork failed ... Womp Womp ...\033[0m\n\n" << std::endl;
		this->_exitcode = -1; // to change
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
			std::cerr << "CGI Write failed !" << std::endl;
			return (-1);
		}
		total_written += written;
	}
	close(pipe_in[1]);	// Signal EOF to child
	return (0);
}


std::string 	CGIContent::grabCGIBody(int	&bodySize)
{
	std::string	result;
	char		buffer[CGI_BUFFERSIZE] = {0};
	int			bytes_read = 0;
	int			total_read = 0;


	while ((bytes_read = read(this->pipe_out[0], buffer, CGI_BUFFERSIZE)) > 0)
	{
		result.append(buffer, bytes_read);
		total_read += bytes_read;
	}
	if (bytes_read < 0) {
		std::cerr << "Read error ! \n";
		close(this->pipe_out[0]);
	}
	
	close(this->pipe_out[0]);
	bodySize = result.length();

	return (result);
}





// PARENT                         CHILD
// +---------------------+         +---------------------+
// |                     |         |                     |
// |  pipe_in[1] (write) |-------->|  STDIN_FILENO (0)   |
// |                     |         |   (from pipe_in[0]) |
// |                     |         |                     |
// |  pipe_out[0] (read) |<--------|  STDOUT_FILENO (1)  |
// |                     |         |   (to pipe_out[1])  |
// +---------------------+         +---------------------+

// Direction:
// - Parent writes to pipe_in[1] ---> Child reads from pipe_in[0] (via dup2'd STDIN)
// - Child writes to pipe_out[1] ---> Parent reads from pipe_out[0] (via dup2'd STDOUT)

