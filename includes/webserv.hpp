#ifndef WEBSSERV_HPP
# define WEBSSERV_HPP

# include "HTTPConfig.hpp"
# include "HttpRequest.hpp"
# include "struct.hpp"

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
#define BUFFER_SIZE 2048 /// to change, must be 1024
#define IS_INDEXDIR 60
#define IS_EXISTINGFILE 61
#define IS_DIRECTORY 62
#define FAILEDSYSTEMCALL -1
#define MISSINGFILE -2
#define ICOHANDELING 2
#define PRINT_DEBUG std::cout << "\n\033[32m[DEBUG] " << __FILE__ << ":" << __LINE__ << in << __FUNCTION__ << ":\033[0m " << std::endl;
#define PRINT_DEBUG2 std::cout << "\n\033[32m[DEBUG] " << __FILE__ << ":" << __LINE__ << in << __PRETTY_FUNCTION__ << ":\033[0m " << std::endl;

void	get_request(HttpRequest &, std::map<std::string, ServerConfig> &);
void	post_request(HttpRequest &, std::map<std::string, ServerConfig> &);
void	delete_request(HttpRequest &, std::map<std::string, ServerConfig> &);

std::string	analyse_request(char buffer[BUFFER_SIZE], t_fd_data *d, int *errcode);

std::string create_header(const std::string &status, const std::string &content_type, const std::string &content_length, const std::string &connection);

template <typename T, typename Y>
void	convert(const T& from, Y& to)
{
    std::stringstream ss;
	ss << from;
    ss >> to;
    if (!ss.fail() && ss.eof())
		throw std::runtime_error("Conversion failed");
}

#endif
