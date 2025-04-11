#ifndef WEBSERV_HPP
# define WEBSERV_HPP

# include <stdio.h>
# include <iostream>//std::cout
# include <sys/socket.h> //socket
# include <netinet/in.h> // struct addr
# include <arpa/inet.h> // inet_addr
# include <fstream> // std::ofstream
# include <sstream>
# include <cstring>


# include "recieve_msg.hpp"

# define SERV_PORT 8080
# define BUFFER_SIZE 1024

void		parse_request(t_browser_request & , char * );

std::string	itostr(int );
std::string	generate_http_response(std::string , std::string , t_browser_request &);


#endif