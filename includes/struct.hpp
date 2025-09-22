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
	std::string	requestedFilePath;	

	std::string			Content_Type;
	std::string			Content_Length;
	std::string			QueryString;
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