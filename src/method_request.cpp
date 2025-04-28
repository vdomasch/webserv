#include "webserv.hpp"
#include <sys/types.h>
#include <sys/socket.h>

std::string	displayErrorPage(std::string serverFolder, int *errcode)
{
	char		buffer[BUFFER_SIZE];
	std::string current_pwd(getcwd(NULL, 0));
	std::string pathToErrPage;
	int			bytes_read;
	int			fd;


	pathToErrPage = serverFolder + "/error_404.html"; // subject to change ? 

	fd = open(pathToErrPage.c_str(), O_RDONLY);	// the error page can still crash ???? 
	if (fd < 0)
	{
		*errcode = FAILEDSYSTEMCALL;
		return ("void");
	}
	bytes_read = read(fd, buffer, BUFFER_SIZE);
	if (bytes_read < 0)
	{
		*errcode = FAILEDSYSTEMCALL;
		close(fd);
		return ("void");
	}
	*errcode = 0;
	close(fd);
	std::string response(buffer);
	memset(buffer, '\0', sizeof(buffer)); // useless ? -> it's not ???
	return (response);
}

std::string	handleIcoFile(t_fd_data *d)
{
	// printf("\033[31m DETECTED A ISO REQUEST 🗣 🗣 🗣 🗣 🗣\n %s \n\n \033[0m",d->requestedFilePath.c_str());
	std::ifstream binfile(d->requestedFilePath.c_str(), std::ios::binary);
	// std::ifstream binfile("reset", std::ios::binary);
	if (!binfile.is_open()) 
		std::cerr << "Could not open .ico file" << std::endl; // handle more errors
	else 
	{
		binfile.seekg(0, std::ios::end);
		size_t file_size = binfile.tellg();
		binfile.seekg(0, std::ios::beg);
		
		std::vector<char> buffer2(file_size);
		binfile.read(&buffer2[0], file_size);

		// for (std::vector<char>::const_iterator i = buffer2.begin(); i != buffer2.end(); ++i)
    	// 	std::cout << *i << ' ';

		binfile.close();
		
		std::ostringstream response;
		response << "HTTP/1.1 200 OK\r\n"
		<< "Content-Type: image/x-icon\r\n"
		<< "Content-Length: " << file_size << "\r\n"
		<< "\r\n";

		d->binaryContent = buffer2;
		return(response.str().c_str());
		// printf("\033[31m  gougou %lu\n", d->binaryContent.size());
		// printf("\033[31m gaga %lu\n", buffer2.size());

	}

	return ("errorstring"); // to handle, doesn´t happens unless the file can't be opened
	
}

std::string	openAndReadFile(t_fd_data *d, int *errcode)
{
	char			buffer[BUFFER_SIZE];
	int				bytes_read;
	int				fd;
	unsigned int	len;


	len = d->requestedFilePath.length();
	// printf("\033[31m HEY ! 🗣 \n I AM %s WITH A LEN OF %u\n\n", file.substr(len - 4, len - 1).c_str(), len);
	if (len >= 4 && (d->requestedFilePath.substr(len - 4, len - 1) == ".ico")) //ugly hardcoding just to test the ico case
	{
		// printf("\033[34m####Setting error code to 2 ! Seems to be an ico file !\n\033[0m\n");
		*errcode = ICOHANDELING; // move into func bellow
		return (handleIcoFile(d));
	}

	fd = open(d->requestedFilePath.c_str(), O_RDONLY);	
	if (fd < 0)
	{
		*errcode = FAILEDSYSTEMCALL;
		return ("void"); // to handle better
	}
	bytes_read = read(fd, buffer, BUFFER_SIZE);
	if (bytes_read < 0)
	{
		*errcode = FAILEDSYSTEMCALL;
		close(fd);
		return ("void"); //handle better
	}
	// printf("\033[36m->Bytes read : (%d)\n\033[0m\n", bytes_read);
	*errcode = 0;
	
	close(fd);
	std::string response(buffer);
	memset(buffer, '\0', sizeof(buffer)); // useless ? -> it's not ???
	return (response);
}

int	checkObjectType(std::string filename, t_fd_data *d, int *errcode)
{
	struct stat fileinfo;  
	std::string pathToCheck;
	std::string	fileContent;
	

	if (filename == "/") // then redirect to index  --> to check ??
	{
		d->requestedFilePath = d->serverFolder;
		printf("Got it ! the folder is : (%s)\n\n", d->requestedFilePath.c_str());
		return (IS_INDEXDIR);
	}
	pathToCheck = d->serverFolder + filename; // we need to check if len > 0 before ? 
	printf("Path to check is : (%s)\n\n", pathToCheck.c_str());

    if (stat (pathToCheck.c_str(), &fileinfo) == 0) // then file exists --> to secure better, check requestedFilePath too
		printf("\033[34mFound it --> \033[0m");
	else
	{
		printf("\033[31mFile wasn't found ! Setting error code appropriately !\n\033[0m\n");
		*errcode = MISSINGFILE;
		return (MISSINGFILE);
	}
	d->requestedFilePath = pathToCheck;
	switch (fileinfo.st_mode & S_IFMT) 
	{
		case S_IFDIR: 
			printf("\033[34mis a dir !\n\033[0m");
			return (IS_DIRECTORY);
		case S_IFREG:
			printf("\033[34mis a file !\n\033[0m");
			return (IS_EXISTINGFILE);
		default: return (IS_EXISTINGFILE);
	}
}


std::string	openAndDisplayIndex(std::string file, int *errcode) // to do later
{
	char		buffer[BUFFER_SIZE];
	std::string pathToIndexPage;
	int			bytes_read;
	int			fd;

	
	pathToIndexPage = file + "/basePageForIndex.html"; // subject to change ?  --> should NOT be using still .html, or content will be the same no matter the files
	fd = open(pathToIndexPage.c_str(), O_RDONLY);	// the error page can still crash ???? 
	if (fd < 0)
	{
		*errcode = FAILEDSYSTEMCALL;
		return ("void");
	}
	bytes_read = read(fd, buffer, BUFFER_SIZE);
	if (bytes_read < 0)
	{
		*errcode = FAILEDSYSTEMCALL;
		close(fd);
		return ("void");
	}
	*errcode = 0;
	close(fd);
	std::string response(buffer);
	memset(buffer, '\0', sizeof(buffer)); // useless ? -> it's not ???
	return (response);
}


std::string	buildCurrentIndexPage(std::string file, int *errcode)
{
	std::ostringstream oss;
	struct dirent *pDirent;
    DIR *pDir;
	(void)errcode;

	pDir = opendir (file.c_str());
	if (pDir == NULL) 
	{
        printf ("Cannot open directory '%s'\n", file.c_str());
		*errcode = FAILEDSYSTEMCALL;
        return ("");
    }
	errno = 0;
	while ((pDirent = readdir(pDir)) != NULL) // can fail, check ernoo when null is found, set ernoo to 0 before first call
	{
		std::string fname(pDirent->d_name );
		if ((fname == ".") || fname == "..")
			continue;
    	printf ("[%s]\n", pDirent->d_name);
		oss << pDirent->d_name;
		oss << "\n";
    }
	if (errno != 0)
	{
		perror("Readdir failed ! "); 
		closedir (pDir);
		*errcode = FAILEDSYSTEMCALL;
		return ("");
	}

   
	return (oss.str().c_str());
}

std::string	analyse_request(char buffer[BUFFER_SIZE], t_fd_data *d, int *errcode)
{
	std::string request(buffer);
	std::string first_line;
	std::string requested_file;
	std::string response;
	char		objType;
	size_t		filename_start;
	size_t		filename_end;

	first_line = request.substr(0, request.find('\n')); // doesn´t work if curl
	filename_start = first_line.find_first_of(' ');
	filename_end = first_line.find_last_of(' ');
	requested_file = first_line.substr(filename_start + 1, filename_end - filename_start - 1);

	printf("\033[34m------------------------------------\n");
	printf(" Requested : (%s)\n",requested_file.c_str() );
	printf("------------------------------------\033[0m\n");

	objType = checkObjectType(requested_file, d, errcode); // is it a file ? or a folder ?

	if (objType == IS_INDEXDIR)
		response = openAndDisplayIndex(d->requestedFilePath, errcode);
	else if (objType == IS_DIRECTORY)
		response = buildCurrentIndexPage(d->requestedFilePath, errcode);
	else if (objType == IS_EXISTINGFILE)
		response = openAndReadFile(d, errcode);
	else
		response = displayErrorPage(d->serverFolder, errcode); // check if error is file not found
	return (response);
}

std::ifstream::pos_type filesize(const char *filename)
{
    std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
    return in.tellg(); 
}

std::string	defineRequestHeaderResponseCode(int errcode, std::string requestBody, t_fd_data *d)
{
	std::string	responseCode;
	std::ifstream::pos_type dataFile;
	std::ostringstream oss;


	//--------------------------------------------------------//
    //std::cout << "body size is " << requestBody.length() << "\n"; //temporary until ico zorks in binary


	if (requestBody.length() == 0)
	{
		std::cout << "I'm out ! 1.3 sec\n" << std::endl;
		return (""); 
	}
	//--------------------------------------------------------//

	dataFile = filesize(d->requestedFilePath.c_str()); // doesn´t work on folders
	oss << dataFile;

	switch (errcode)
	{
	case 0:
		responseCode = "HTTP/1.1 200 OK\nContent-Type: text/html\r\nContent-Lenght: ";
		responseCode.append(oss.str());
		responseCode.append("\r\n\r\n\n");
		d->content_len = atof(oss.str().c_str()); // ugly but ok
		d->content_type = "text/html";
		break;
	
	case 2:
		d->content_len = atof(oss.str().c_str());
		d->content_type = "image/x-icon";
		return(requestBody);

	default:
		responseCode = "HTTP/1.1 200 OK\nContent-Type: text/html\r\nContent-Lenght: ";
		responseCode.append(oss.str());
		responseCode.append("\r\n\r\n\n");
		d->content_len = atof(oss.str().c_str());
		d->content_type = "text/html";
		break;
	}
	responseCode = responseCode + requestBody;
	return (responseCode);
}




void	get_request(HttpRequest &req, std::map<std::string, ServerConfig> &server_list)
{
	static_cast<void>(server_list);

	std::cout << "GET request received" << std::endl;
	//std::cout << req << std::endl;

	if (req.getPath().find("favicon.ico") != std::string::npos)
	{
		std::cout << "Favicon request received" << std::endl;
		return;
	}
	else if (!req.getPath().empty())
	{
		std::cout << "Path Request Recieved" << std::endl;
		//std::cout << "Path: " << req.getPath() << std::endl;
		//std::cout << "Host: " << req.getHost() << std::endl;
		//std::cout << "Host: " << req.getHost().substr(0, req.getHost().find(':')) << std::endl;
		//std::cout << "Port: " << req.getPort() << std::endl;

		std::string server_name(req.getHost().substr(0, req.getHost().find(':')));

		std::string key(req.getPort() + ":" + server_name);

		if (req.getHost().find(':') != std::string::npos)		// DEMANDER BEN SI 8080:server_name -> peut changer en server_name:8080
		{
			if (server_list.find(key) != server_list.end())
			{
				std::cout << "Server found 1" << std::endl;
				ServerConfig server = server_list[key];

				if (req.getPath().rfind('/') != 0)
				{
					std::vector<LocationConfig> locations = server.get_location_list();
					for (std::vector<LocationConfig>::iterator it = locations.begin(); it != locations.end(); ++it)
					{
						if (it.base()->get_map_location().find("path") != it.base()->get_map_location().end())
						{
							std::string location_path = it.base()->get_map_location()["path"];

							std::cout << "Location path: " << location_path << std::endl;
							std::cout << "Request path: " << req.getPath() << std::endl;

							std::string path = req.getPath().substr(0, req.getPath().rfind('/'));
							std::cout << "Path: " << path << std::endl;

							if (path == location_path)
							{
								std::cout << "Location found" << std::endl;
								if (server.get_map_server()["root"].at(server.get_map_server()["root"].size() - 1) == '/' && req.getPath().at(0) == '/')
									req.setPath(req.getPath().substr(1));
								std::string complete_path = server.get_map_server()["root"] + req.getPath();
								std::cout << "Complete path: " << complete_path << std::endl;

								std::ifstream file;
								file.open(complete_path.c_str(), std::ios::in);
								if (file)
								{
									std::cout << "File found" << std::endl;
									std::ostringstream ss;
									ss << file.rdbuf();

									if (req.getKeepAlive())
										req.setResponse(create_header("200 OK", "text/html", tostr(file.width()), "keep-alive") + ss.str());
									else
										req.setResponse(create_header("200 OK", "text/html", tostr(file.width()), "close") + ss.str());

									file.close();
									return ;
								}
								else
								{
									std::cout << "File not found" << std::endl;
									return;
								}
								return;
							}
							else
							{
								std::cout << "Location not found" << std::endl;
								return;
							}
						}
					}
				}
				else
				{
					std::cout << "Path is root" << std::endl;
					std::ifstream file;
					file.open(req.getPath().c_str(), std::ios::in);
					if (file)
					{
						std::cout << "File found" << std::endl;
						std::ostringstream ss;
						ss << file.rdbuf();
						if (req.getKeepAlive())
							req.setResponse(create_header("200 OK", "text/html", tostr(file.width()), "keep-alive") + ss.str());
						else
							req.setResponse(create_header("200 OK", "text/html", tostr(file.width()), "close") + ss.str());

						file.close();
						return ;
					}
					else
					{
						std::cout << "File not found, using default path" << std::endl;
						std::string default_path = server.get_map_server()["root"] + server.get_map_server()["index"];
						std::cout << "Default path: " << default_path << std::endl;
						file.open(default_path.c_str(), std::ios::in);
						std::cout << "File found" << std::endl;
						std::ostringstream ss;
						ss << file.rdbuf();
						if (req.getKeepAlive())
							req.setResponse(create_header("200 OK", "text/html", tostr(file.width()), "keep-alive") + ss.str());
						else
							req.setResponse(create_header("200 OK", "text/html", tostr(file.width()), "close") + ss.str());

						file.close();
						return;
					}
					return;
				}
			}
			else if (server_list.find(req.getPort()) != server_list.end())
			{
				std::cout << "Server found 2" << std::endl;
				return;
			}
			else
			{
				std::cout << "Server not found" << std::endl;
				return;
			}
		}

	}
	else
	{
		std::cout << "Request error" << std::endl;
		return;
	}
}

void	post_request(HttpRequest &req, std::map<std::string, ServerConfig> &server_list)
{
	static_cast<void>(server_list);

	std::cout << "POST request received" << std::endl;
	std::cout << req << std::endl;

	std::string response = create_header("200 OK", "text/plain", "16", "keep-alive");

}

void	delete_request(HttpRequest &req, std::map<std::string, ServerConfig> &server_list)
{
	static_cast<void>(server_list);
	
	std::cout << "DELETE request received" << std::endl;
	std::cout << req << std::endl;


}

std::string create_header(const std::string &status, const std::string &content_type, const std::string &content_length, const std::string &connection)
{
	std::string header = "HTTP/1.1 " + status + "\r\n";
	header += "Content-Type: " + content_type + "\r\n";
	header += "Content-Length: " + content_length + "\r\n";
	header += "Connection: " + connection + "\r\n\r\n";
	return header;
}