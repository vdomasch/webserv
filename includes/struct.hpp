#ifndef STRUCT_HPP
# define STRUCT_HPP

#include "CGIContent.hpp"

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <dirent.h>
#include <sys/types.h>

typedef struct s_fd_data
{
	int			max_fd;

	fd_set		ready_readsockets;
	fd_set		ready_writesockets;
	fd_set		saved_readsockets;
	fd_set		saved_writesockets;
	std::string	requestedFilePath;	// obtained after analyse_request, is the splitted version of the GET of POST request to isolate the file name, is used to determine the size of file for Content-Lenght

	std::string			Content_Type;		// right now, is used for POST requests (var is passed to env)
	std::string			Content_Length;		// right now, is used for POST requests (var is passed to env)
	std::string			QueryString;		// Used when a GET request is called on a CGI script (var is passed to env)
	std::vector<char> 	binaryContent;
	std::vector<dirent>	folderContent;
	CGIContent			cg;

}	t_fd_data;

typedef struct s_request_state
{
	bool		header_complete;
	bool		ready_to_process;
	size_t		content_length;
	int			bytesRead;
	std::string	request;
	std::string	body;
	int			errcode;
}	t_request_state;

#endif