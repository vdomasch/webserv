#ifndef WEBSSERV_HPP
# define WEBSSERV_HPP

# include "HTTPConfig.hpp"
# include "HttpRequest.hpp"

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
#include <dirent.h> // dirent for opendir
#include <fcntl.h>// open

#define SERV_PORT 8080
#define BUFFER_SIZE 5000 /// to change, must be 1024
#define IS_INDEXDIR 60
#define IS_EXISTINGFILE 61
#define IS_DIRECTORY 62
#define FAILEDSYSTEMCALL -1
#define MISSINGFILE -2
#define ICOHANDELING 2

typedef struct s_fd_data
{
	fd_set		ready_sockets;
	fd_set		saved_sockets; // current sockets
	std::string	serverFolder; // contains the files to display (index.html etc etc ...)
	std::string	requestedFilePath; // obtained after analyse_request, is the splitted version of the GET of POST request to isolate the file name, is used to determine the size of file for Content-Lenght
	int			max_fd;

	std::string			content_type; // only for .ico for the moment
	unsigned int		content_len;
	std::vector<char> 	binaryContent;
	std::vector<dirent>	folderContent;
}	t_fd_data;

void	get_request(HttpRequest &, std::map<std::string, ServerConfig> &);
void	post_request(HttpRequest &, std::map<std::string, ServerConfig> &);
void	delete_request(HttpRequest &, std::map<std::string, ServerConfig> &);

std::string	analyse_request(char buffer[BUFFER_SIZE], t_fd_data *d, int *errcode);

std::string create_header(const std::string &status, const std::string &content_type, const std::string &content_length, const std::string &connection);

std::string tostr(const int value);

#endif
