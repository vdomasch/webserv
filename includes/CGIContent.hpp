
#ifndef CGICONTENT_HPP
# define CGICONTENT_HPP
 
#include <stdio.h>
#include <iostream>//std::cout
#include <sys/socket.h> //socket
#include <strings.h> //bzero
#include <netinet/in.h> // struct addr
#include <arpa/inet.h> // inet_addr
#include <unistd.h> //read & write
#include <string.h> // strlen
#include <fstream> // std::ofstream
#include <sys/stat.h> // stat for file status
#include <sstream> // Content Size
#include <vector> // .ico handling
#include <stdlib.h> // atof
#include <algorithm> // std::sort
#include <dirent.h> // dirent for opendir
#include <cmath> // floor for size
#include <fcntl.h>// open
#include <map> // std::map

#define CGI_BUFFERSIZE 10000

class CGIContent
{
	private :
		char**								_argv;
		char**								_cgi_env;
		int									_exitcode;
		std::string							_cgi_path;
		std::map<std::string, std::string>	_cgi_env_map;

		
		public :

		int	pipe_in[2]; // Pass data from parent (server) to the child (CGI script), [0] is to read , [1] is to write
		int	pipe_out[2]; // Pass data from child (CGI script) back to the parent (server)
		int									cgi_forkfd;


	CGIContent();
	CGIContent(std::string path);
	~CGIContent();
	CGIContent(CGIContent const &other);
	CGIContent &operator=(CGIContent const &rhs);

	void			setEnvCGI(std::string cgi_path, std::string type, std::string len, std::string method);
	void 			executeCGI();
	std::string		grabCGIBody(int	&bodySize);
	int				sendCGIBody(std::string body);
	//getters et setters
};

#endif