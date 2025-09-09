
#ifndef CGICONTENT_HPP
# define CGICONTENT_HPP
 
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <fstream>
#include <sys/stat.h>
#include <sstream>
#include <vector>
#include <algorithm>
#include <dirent.h>
#include <cmath>
#include <fcntl.h>
#include <map>

#define CGI_BUFFERSIZE 10000

class CGIContent
{
	private :
		std::vector<char*>					_argv;
		std::vector<char*>					_cgi_env;
		int									_exitcode;
		std::string							_cgi_path;
		std::map<std::string, std::string>	_cgi_env_map;
		std::vector<std::string>			_env_storage;
		std::vector<std::string>			_argv_storage;


		
	public :

		int	pipe_in[2]; // Pass data from parent (server) to the child (CGI script), [0] is to read , [1] is to write
		int	pipe_out[2]; // Pass data from child (CGI script) back to the parent (server)
		int	cgi_forkfd;

		CGIContent();
		~CGIContent();

		void			setEnvCGI(std::string cgi_path, std::string type, std::string len, std::string method, bool& is_php_cgi);
		void			executeCGI(bool &exec_failed);
		std::string		grabCGIBody(int child_pid, int timeout_sec, int &status);
		int				sendCGIBody(std::string body);
		int				get_exitcode();

};

#endif