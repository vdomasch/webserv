#ifndef STRUCT_HPP
# define STRUCT_HPP

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <dirent.h>
#include <sys/types.h>

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