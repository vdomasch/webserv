#ifndef WEBSSERV_HPP
# define WEBSSERV_HPP

# include "HTTPConfig.hpp"
# include "HttpRequest.hpp"
# include "HttpResponse.hpp"
# include "methods_utils.hpp"
# include "struct.hpp"
# include "CGIContent.hpp"

# include <stdio.h>
# include <iostream>//std::cout
# include <sys/socket.h> //socket
# include <strings.h> //bzero
# include <netinet/in.h> // struct addr
# include <arpa/inet.h> // inet_addr
# include <unistd.h> //read & write
# include <string.h> // strlen
# include <fstream> // std::ofstream
# include <sys/stat.h> // stat for file status
# include <sstream> // Content Size
# include <vector> // .ico handling
# include <stdlib.h> // atof
# include <fcntl.h>// open
# include <algorithm> // std::sort
# include <dirent.h> // dirent for opendir
# include <cmath> // floor for size
# include <map> // std::map
# include <string> //string::npos
# include <sys/types.h> //waitpid
# include <sys/wait.h> //waitpid
# include <set>
# include <ctime>


# define SERV_PORT 8080
# define BUFFER_SIZE 100000 /// to change, must be 1024
# define IS_INDEXDIR 60
# define IS_EXISTINGFILE 61
# define IS_DIRECTORY 62
# define FILE_NOT_FOUND 63
# define IS_CGI 64
# define FAILEDSYSTEMCALL -1
# define MISSINGFILE -2
# define ICOHANDELING 2
# define GIFHANDELING 3
# define CSSHANDELING 4
# define PNGHANDELING 5
# define JPGHANDELING 6
# define DEBUG_INDEX_EXISTS 1 // for debug purposes, change between index redirection and auto-index (1 for list)

#define PRINT_DEBUG std::cout << "\n\033[32m[DEBUG] " << __FILE__ << ": " << __LINE__ << " in " << __FUNCTION__ << "\033[0m " << std::endl;
#define PRINT_DEBUG2 std::cout << "\n\033[32m[DEBUG] " << __FILE__ << ": " << __LINE__ << " in " << __PRETTY_FUNCTION__ << "\033[0m " << std::endl;

void	get_request(HTTPConfig &http_config, HttpRequest &req, t_fd_data &fd_data, std::string server_name);
void	post_request(HTTPConfig &http_config, HttpRequest &req, t_fd_data &fd_data, std::string server_name);
void	delete_request(HTTPConfig &http_config, HttpRequest &req, t_fd_data &fd_data, std::string server_name);

template <typename Y, typename T>
Y	convert(const T& from) {
	std::stringstream ss;
	ss << from;
	Y result;
	ss >> result;
	if (ss.fail() || !ss.eof())
		throw std::runtime_error("Conversion failed");
	return result;
}

struct s_socket_data
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
typedef struct s_socket_data	t_socket_data;

class orderedFiles
{
	public :
		std::string		baseName;
		std::string		lowerName;
		unsigned char	type;

	orderedFiles(const std::string& name, const std::string& lowname, unsigned char n_type) : baseName(name), lowerName(lowname), type(n_type) {}
};

#endif
