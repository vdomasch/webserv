#include "CGIContent.hpp"
#include "HTTPConfig.hpp"

CGIContent::CGIContent()
{
	// printf("calling default ! \n");

	this->_cgi_path = "default";
	this->_argv = NULL;
	this->_cgi_env = NULL;
	this->_exitcode = -42;
	this->pipe_in[0] = -42;
	this->pipe_in[1] = -42;
	this->pipe_out[0] = -42;
	this->pipe_out[1] = -42;
	//this->_cgi_env_map = NULL; not possible
}

CGIContent::CGIContent(std::string path)
{
	this->_cgi_path = path;
	this->_argv = NULL;
	this->_cgi_env = NULL;
	this->_exitcode = -42;
	this->pipe_in[0] = -42;
	this->pipe_in[1] = -42;
	this->pipe_out[0] = -42;
	this->pipe_out[1] = -42;
}

CGIContent::~CGIContent()
{
	// if (this->_cgi_env)
	// {
	// 	for (int i = 0; this->_cgi_env[i]; i++)
	// 		free(this->_cgi_env[i]);
	// 	free(this->_cgi_env);
	// }
	// if (this->_argv)
	// {
	// 	for (int i = 0; this->_argv[i]; i++)
	// 		free(_argv[i]);
	// 	free(_argv);
	// }
	this->_cgi_env_map.clear();
}

CGIContent::CGIContent(CGIContent const &other)
{
	this->_cgi_path = other._cgi_path;
	this->_argv =  other._argv;
	this->_cgi_env =  other._cgi_env;
	this->_cgi_env_map =  other._cgi_env_map;
	this->_exitcode =  other._exitcode;
}


CGIContent &CGIContent::operator=(CGIContent const &copy)
{
	if (this != &copy)
	{
		this->_cgi_path = copy._cgi_path;
		this->_cgi_env =  copy._cgi_env;
		this->_cgi_env_map =  copy._cgi_env_map;
		this->_exitcode =  copy._exitcode;
		this->_argv =  copy._argv;
	}
	return (*this);
}

void 	CGIContent::setEnvCGI(std::string cgi_path, std::string type, std::string len, std::string method) // string for now, replace by iterator of whatever struct we use
{	
	std::string	script_name_var;
	size_t		pos;


	pos = cgi_path.find("/cgi-bin"); // i guess the name could change ??? to check
	script_name_var = cgi_path.substr(pos);
	// printf("type (%s)\n", type.c_str());
	// printf("len (%s)\n", len.c_str());
	// printf("script_name_var (%s)\n", script_name_var.c_str());
	// printf("method (%s)\n", method.c_str());

	// this->_cgi_env_map["HTTP_COOKIE"] = "default";
	// this->_cgi_env_map["HTTP_USER_AGENT"] = "default";
	// this->_cgi_env_map["PATH_INFO"] = "default";
	// this->_cgi_env_map["REMOTE_ADDR"] = "default";
	// this->_cgi_env_map["REMOTE_HOST"] = "default";
	// this->_cgi_env_map["SERVER_SOFTWARE"] = "AMANIX";
	// this->_cgi_env_map["SERVER_NAME"] = "default";

	this->_cgi_env_map["CONTENT_LENGTH"] = len; // ln in header
	this->_cgi_env_map["CONTENT_TYPE"] = type; // must be of form "multipart/form-data; boundary=----geckoformboundarybd99e35cc2"
	this->_cgi_env_map["QUERY_STRING"] = ""; // to test if i can extract it
	this->_cgi_env_map["REQUEST_METHOD"] = method;   // POST, GET, HEAD ....
	this->_cgi_env_map["SCRIPT_FILENAME"] = cgi_path;	// full path to the CGI script
	this->_cgi_env_map["SCRIPT_NAME"] = script_name_var; // truncated path to cgi


	this->_cgi_env = (char **)calloc(sizeof(char *), this->_cgi_env_map.size() + 1); // is calloc allowed ? no ?

	int i = 0;
	for (std::map<std::string, std::string>::iterator it=this->_cgi_env_map.begin() ; it != this->_cgi_env_map.end(); ++it)
	{
		// std::cout << it->first << " = " << it->second << "\n";
		this->_cgi_env[i] = strdup((it->first + "=" + it->second).c_str());
		i++;
	}
	// printf("\n\n\n");
	this->_cgi_env[i] = NULL;
	// for (int i = 0; this->_cgi_env[i]; i++)
	// 	printf("%s \n",this->_cgi_env[i]);

	this->_cgi_path = cgi_path; // for debug, will be taken from config file


	this->_argv = (char **)malloc(sizeof(char *) * 3);
	this->_argv[0] = strdup("/usr/bin/python3");
	this->_argv[1] = strdup(this->_cgi_path.c_str());
	this->_argv[2] = NULL;
	// std::cout << "\033[31m|-----###---" << this->_argv[0] << "---------------|\033[0m\n\n" << std::endl;
	// std::cout << "\033[31m|------###--" << this->_argv[1] << "---------------|\033[0m\n\n" << std::endl;
}

void 	CGIContent::executeCGI()
{	
	if (pipe(this->pipe_in))  //pipe_in[0] is read end of pipe, pipe_in[1] is to write to it 
	{
		std::cout << "\033[31mPipe failed ... Womp Womp ...\033[0m\n\n" << std::endl;
		this->_exitcode = -1; // to change
		return ;
	}
	if (pipe(this->pipe_out))
	{
		std::cout << "\033[31mPipe failed ... Womp Womp ...\033[0m\n\n" << std::endl;
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
		
		this->_exitcode = execve(this->_argv[0], this->_argv, this->_cgi_env);
		
		std::cerr << "EXECVE FAILED !\r\n";

		// by this point, the output of the CGI script was written to pipe_out[1] (the write end of the pipe), since it was designated as the STDOUT_FILENO
		// and is waiting to be read using pipe_out[0] (which is the read end of the pipe)
		
		exit(this->_exitcode); // if fails
	}
	else if (this->cgi_forkfd == -1)
	{
		std::cout << "\033[Fork failed ... Womp Womp ...\033[0m\n\n" << std::endl;
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
			perror("write failed !");
			return (-1);
			
		}
		total_written += written;
		printf("[%lu]", total_written);
	}
	close(pipe_in[1]);	// Signal EOF to child
	return (0);
}


std::string 	CGIContent::grabCGIBody(int	&bodySize)
{
	std::string	result;
	char		buffer[CGI_BUFFERSIZE];
	int			bytes_read = 0;
	int			total_read = 0;


	while ((bytes_read = read(this->pipe_out[0], buffer, CGI_BUFFERSIZE)) > 0)
	{
		// printf("CGI READ : %d\n", bytes_read);
		result.append(buffer, bytes_read);  //might be an issue with binary data ?
		total_read += bytes_read;
	}
	if (bytes_read < 0) {
		std::cerr << "Read error ! \n";
		close(this->pipe_out[0]);
	}
	
	close(this->pipe_out[0]);
	memset(buffer, 0, sizeof(buffer));
	printf("\ntotal read %d\n", total_read);
	printf("total len %lu\n", result.length());
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

