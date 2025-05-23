/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdomasch <vdomasch@student.42lyon.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/25 10:01:15 by lchapard          #+#    #+#             */
/*   Updated: 2025/03/11 11:22:32 by vdomasch         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef WEBSERVER_HPP
# define WEBSERVER_HPP

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

#include "CGIContent.hpp"


#define SERV_PORT 8080
#define BUFFER_SIZE 5000 /// to change, must be 1024
#define IS_INDEXDIR 60
#define IS_EXISTINGFILE 61
#define IS_DIRECTORY 62
#define IS_CGI 63
#define FAILEDSYSTEMCALL -1
#define MISSINGFILE -2
#define ICOHANDELING 2
#define GIFHANDELING 3
#define DEBUG_INDEX_EXISTS 1 // for debug purposes, change between index redirection and auto-index (1 for list)

struct s_fd_data
{
	int			serverSocketFd; // temporary (i think ?), only to test out the cgi part
	fd_set		ready_sockets;
	fd_set		saved_sockets; // current sockets
	std::string	serverFolder; // contains the files to display (index.html etc etc ...)
	std::string	requestedFilePath; // obtained after analyse_request, is the splitted version of the GET of POST request to isolate the file name, is used to determine the size of file for Content-Lenght
	int			max_sckt_fd;

	std::string			content_type; // only for .ico for the moment
	unsigned int		content_len;
	std::vector<char> 	binaryContent;
	std::vector<dirent>	folderContent;
	CGIContent			cg;
};
typedef struct s_fd_data	t_fd_data;

class orderedFiles
{
	public :
		std::string		baseName;
		std::string		lowerName;
		unsigned char	type;

	orderedFiles(const std::string& name, const std::string& lowname, unsigned char n_type) : baseName(name), lowerName(lowname), type(n_type) {}
};


#endif