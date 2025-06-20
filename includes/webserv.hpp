#ifndef WEBSSERV_HPP
# define WEBSSERV_HPP

# include "HTTPConfig.hpp"
# include "HttpRequest.hpp"
# include "HttpResponse.hpp"
# include "methods_utils.hpp"
# include "struct.hpp"
# include "CGIContent.hpp"
# include "utils.hpp"

# include <iostream>
# include <sys/socket.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <fstream>
# include <sys/stat.h>
# include <sstream>
# include <vector>
# include <algorithm>
# include <dirent.h>
# include <cmath>
# include <map>
# include <string>
# include <sys/types.h>
# include <sys/wait.h>
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

# define TIMEOUT_SEC 5
# define SELECT_TIMEOUT_USEC 500000
# define SELECT_TIMEOUT_SEC 0

#define PRINT_DEBUG std::cout << "\n\033[32m[DEBUG] " << __FILE__ << ": " << __LINE__ << " in " << __FUNCTION__ << "\033[0m " << std::endl;
#define PRINT_DEBUG2 std::cout << "\n\033[32m[DEBUG] " << __FILE__ << ": " << __LINE__ << " in " << __PRETTY_FUNCTION__ << "\033[0m " << std::endl;

void	get_request(HTTPConfig &http_config, HttpRequest &req, t_fd_data &fd_data);
void	post_request(HTTPConfig &http_config, HttpRequest &req, t_fd_data &fd_data);
void	delete_request(HTTPConfig &http_config, HttpRequest &req, t_fd_data &fd_data);

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
	fd_set		saved_readsockets;	// current sockets
	fd_set		saved_writesockets; // current sockets
	std::string	serverFolder;		// contains the files to display (index.html etc etc ...)
	std::string	requestedFilePath;	// obtained after analyse_request, is the splitted version of the GET of POST request to isolate the file name, is used to determine the size of file for Content-Lenght
	std::string	method_name;		// the name of the method used and truncated from the original request
	int			max_sckt_fd;

	std::string			Content_Type;		// right now, is used for POST requests
	std::string			Content_Length;		// right now, is used for POST requests
	unsigned int		response_len;		// Was previously content_len, is used only for the response header right now.
	bool				is_binaryContent;	//used for the final send, to know is we need to send binaryContent;
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
