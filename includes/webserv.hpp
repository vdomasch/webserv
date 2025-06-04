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
#include <string> //string::npos
#include <sys/types.h> //waitpid
#include <sys/wait.h> //waitpid

#include "CGIContent.hpp"


#define SERV_PORT 8080
#define BUFFER_SIZE 100000 /// to change, must be 1024
#define IS_INDEXDIR 60
#define IS_EXISTINGFILE 61
#define IS_DIRECTORY 62
#define IS_CGI 63
#define FAILEDSYSTEMCALL -1
#define MISSINGFILE -2
#define ICOHANDELING 2
#define GIFHANDELING 3
#define CSSHANDELING 4
#define PNGHANDELING 5
#define JPGHANDELING 6
#define DEBUG_INDEX_EXISTS 1 // for debug purposes, change between index redirection and auto-index (1 for list)

struct s_fd_data
{
	fd_set		ready_readsockets;
	fd_set		ready_writesockets;
	fd_set		saved_readsockets; // current sockets
	fd_set		saved_writesockets; // current sockets
	std::string	serverFolder; // contains the files to display (index.html etc etc ...)
	std::string	requestedFilePath; // obtained after analyse_request, is the splitted version of the GET of POST request to isolate the file name, is used to determine the size of file for Content-Lenght
	std::string	method_name; // the name of the method used and truncated from the original request
	int			max_sckt_fd;

	std::string			Content_Type; // right now, is used for POST requests
	std::string			Content_Length; // right now, is used for POST requests
	unsigned int		response_len; // Was previously content_len, is used only for the response header right now.
	bool				is_binaryContent; //used for the final send, to know is we need to send binaryContent;
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