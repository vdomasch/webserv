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
# define BUFFER_SIZE 100000
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

class orderedFiles
{
	public :
		std::string		baseName;
		std::string		lowerName;
		unsigned char	type;

	orderedFiles(const std::string& name, const std::string& lowname, unsigned char n_type) : baseName(name), lowerName(lowname), type(n_type) {}
};

#endif
