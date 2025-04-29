#include "../includes/webserv.hpp"

int	initialize_socket(sockaddr_in *servaddr, t_fd_data *socket_data)
{
	int	server_fd;
	int intopt = 1;

	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
	{
		perror("cannot create socket"); 
		return (-1); 
	}
	
	bzero(servaddr, sizeof(*servaddr));
	bzero(socket_data, sizeof(*socket_data));

	servaddr->sin_family = AF_INET;
	servaddr->sin_port = htons(SERV_PORT);
	//servaddr->sin_addr.s_addr = htonl(INADDR_ANY); // bind to all ports
	//OR 
	servaddr->sin_addr.s_addr = inet_addr("127.0.0.1"); // if to localhost only

	setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT | SO_REUSEADDR, &intopt, sizeof(intopt));
	if (bind(server_fd, (struct sockaddr *)servaddr, sizeof(*servaddr)) < 0)
	{
		perror("cannot bind to socket"); 
		return (-1); 
	}
	if (listen(server_fd, 10) < 0)
	{
		perror("Failed to listen ! ");
		return (-1);
	}
	return (server_fd);
}



int accept_connexion(int server_fd, sockaddr_in *servaddr)
{
	int	my_socket;
	int	addr_len = sizeof(servaddr);

	//here servaddr is the connecting ip
	if ((my_socket = accept(server_fd, (struct sockaddr *)servaddr, (socklen_t*)&addr_len))<0)
	{
		perror("In accept");
		return(0); //exit ?
	}
	return (my_socket);
}

char	find_method_name(std::string request_string)
{
	std::string method;
	int			i;

	method = request_string.substr(0, request_string.find(' '));

	std::string	(method_dictionary[4]) = {"GET", "POST", "DELETE", "PUT"};

	i = -1;
	while(++i < 4)
	{
		if (method == method_dictionary[i])
			break ;
	}

	switch (i)
	{
	case 0:
		return ('G');
	case 1:
		return ('P');
	case 2:
		return ('P');
	case 3:
		return ('U');
	default:
		return ('G');
	}

}

std::ifstream::pos_type filesize(const char *filename)
{
	std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
	return in.tellg(); 
}

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
	std::ostringstream oss;
	std::ifstream::pos_type dataFile;

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
		
		dataFile = filesize(d->requestedFilePath.c_str());
		oss << dataFile;
		d->content_len = atof(oss.str().c_str());

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

	d->content_len = response.length();
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


std::string	openAndDisplayIndex(t_fd_data *d, int *errcode) // to do later
{
	char		buffer[BUFFER_SIZE];
	std::string pathToIndexPage;
	int			bytes_read;
	int			fd;

	
	pathToIndexPage = d->requestedFilePath + "/basePageForIndex.html"; // subject to change ?  --> should NOT be using still .html, or content will be the same no matter the files
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

	d->content_len = response.length();
	memset(buffer, '\0', sizeof(buffer)); // useless ? -> it's not ???
	return (response);
}

void	storeFolderContent(t_fd_data *d, int *errcode)
{
	// std::ostringstream oss;
	struct dirent *pDirent;
    DIR *pDir;

	pDir = opendir (d->requestedFilePath.c_str());
	if (pDir == NULL) 
	{
        printf ("Cannot open directory '%s'\n", d->requestedFilePath.c_str());
		*errcode = FAILEDSYSTEMCALL;
        return ;
    }
	errno = 0;
	while ((pDirent = readdir(pDir)) != NULL) // can fail, check ernoo when null is found, set ernoo to 0 before first call
	{
		std::string fname(pDirent->d_name );
		
		if ((fname == ".") || fname == "..")
			continue;
		d->folderContent.push_back(*pDirent); /// fait une boucle pour check le nom et le type de fichier sale flemmard


    	//printf ("[%s]\n", pDirent->d_name);
		// oss << pDirent->d_name;
		// oss << "\n";
    }
	closedir (pDir);
	if (errno != 0)
	{
		perror("Readdir failed ! "); 
		*errcode = FAILEDSYSTEMCALL;
		return ;
	}
	
}


std::string	buildCurrentIndexPage(t_fd_data *d, int *errcode)
{
	std::ostringstream	oss;
	std::string			pageContent;
	
	storeFolderContent(d, errcode);
	if (*errcode == FAILEDSYSTEMCALL)
		return ("");
	// for (std::vector<dirent>::const_iterator i = d->folderContent.begin(); i != d->folderContent.end(); ++i)   ---> we can store it !
	// 	std::cout << i->d_name << ' ';

	oss << "<html>\n";
	oss << "<head>\n";
	oss << "<title> Index of";
	oss << d->requestedFilePath;
	oss << "</title>\n";
	oss << "</head>\n";
	oss << "<body >\n";
	oss << "<h1> Index of " + d->requestedFilePath + "</h1>\n";
	oss << "<table style=\"width:80%; font-size: 15px\">\n";
	oss << "<thead>\n";
	oss << "<hr>\n";
	oss << "<th style=\"text-align:left\"> File Name </th>\n";
	oss << "<th style=\"text-align:left\"> Last Modification  </th>\n";
	oss << "<th style=\"text-align:left\"> File Size </th>\n";
	oss << "</thead>\n";
	oss << "<tbody>\n";
	for (std::vector<dirent>::const_iterator i = d->folderContent.begin(); i != d->folderContent.end(); ++i)   //---> we can store it !
	{
		oss << "<tr><td> ";
		oss << i->d_name;
		oss << "</td>\n";
		oss << "<td> gougou ? </td>\n"; // last modif
		if (i->d_type == DT_REG) // get file size
		{
			printf("HERE ! (%s)\n", i->d_name);
			std::string m_fpath(i->d_name);
			oss << "<td> ";
			oss << filesize((d->requestedFilePath + "/" + m_fpath).c_str());
			oss << " </td>\n";

		}
		else if (i->d_type == DT_DIR) // no size needed
		{
			oss << "<td> gougougaga ? </td>\n";
		}
		oss << "</tr>\n";
	}
	oss << "</tbody>\n";
	oss << "</table>\n";
	oss << "<hr>\n";
	oss << "</body>\n";
	oss << "</html>\n";

	*errcode = 0;

	pageContent = oss.str().c_str();
	d->content_len = pageContent.length();
	d->folderContent.clear();
	return (pageContent);
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
		response = openAndDisplayIndex(d, errcode);
	else if (objType == IS_DIRECTORY)
		response = buildCurrentIndexPage(d, errcode);
	else if (objType == IS_EXISTINGFILE)
		response = openAndReadFile(d, errcode);
	else
		response = displayErrorPage(d->serverFolder, errcode); // check if error is file not found
	return (response);
}

std::string	defineRequestHeaderResponseCode(int errcode, std::string requestBody, t_fd_data *d)
{
	std::string	responseCode;
	std::ostringstream oss;


	//--------------------------------------------------------//
    //std::cout << "body size is " << requestBody.length() << "\n"; //temporary until ico zorks in binary


	if (requestBody.length() == 0)
	{
		std::cout << "I'm out ! 1.3 sec\n" << std::endl;
		return (""); 
	}
	//--------------------------------------------------------//

	oss << d->content_len;

	switch (errcode)
	{
	case 0:
		responseCode = "HTTP/1.1 200 OK\nContent-Type: text/html\r\nContent-Lenght: ";
		responseCode.append(oss.str());
		responseCode.append("\r\n\r\n\n");
		d->content_len = d->content_len;
		d->content_type = "text/html";
		break;
	
	case 2:
		d->content_len = d->content_len;
		d->content_type = "image/x-icon";
		return(requestBody);

	default:
		responseCode = "HTTP/1.1 200 OK\nContent-Type: text/html\r\nContent-Lenght: ";
		responseCode.append(oss.str());
		responseCode.append("\r\n\r\n\n");
		d->content_len = d->content_len;
		d->content_type = "text/html";
		break;
	}
	responseCode = responseCode + requestBody;
	return (responseCode);
}

int	handle_client_request(int socket, t_fd_data *d)
{
	char buffer[BUFFER_SIZE] = {0}; /// in this case, header size is included in buffer_size = bad ?????
	std::string	requestBody;
	std::string	finalMessage;
	int			errcode;
	ssize_t 	bytesRead;
	
	//Receive the new message : 
	bytesRead = read(socket , buffer, BUFFER_SIZE);
	if (bytesRead < 0)
	{
		perror("Failed to read ! ");
		return (-1);
	}
	requestBody = analyse_request(buffer, d, &errcode); // decide how to interpret the request

	memset(buffer, '\0', sizeof(buffer));
	if (errcode == FAILEDSYSTEMCALL)
	{
		perror("\nAn error occured while trying to open the requested file :(\n\n");
		exit(-1); // to check for leaks later
	}
	//Sending a response :
	finalMessage = defineRequestHeaderResponseCode(errcode, requestBody, d); // when .ico, finalMessage = requestBody

	printf("\033[35m\n#######################\n");
	printf("(%s)\n",finalMessage.c_str() );
	printf(" \nERROR CODE(%d)\n",errcode);
	printf("#######################\033[0m\n");

	if (!finalMessage.empty())
	{
		if (d->content_type == "image/x-icon")
		{
			send(socket , finalMessage.c_str() , finalMessage.length(), 0); // must be content len instead of finaleMessage.lenght, or else header size is added
			send(socket , &d->binaryContent[0] , d->binaryContent.size(), 0);
		}
		else
			send(socket , finalMessage.c_str() , finalMessage.length(), 0);
		std::cout << "message sent from server !\n" << std::endl;
	}
	close(socket);
	return (0);
}

int main(int argc, char **argv)
{

	(void)argv;
	int my_socket;
	int	server_fd;
	struct sockaddr_in servaddr;
	t_fd_data s_data; // to set select	

	server_fd = initialize_socket(&servaddr, &s_data);
	if (server_fd < 0)
	{
		perror("cannot bind to socket");
		return (0);
	}
	FD_ZERO(&s_data.saved_sockets);
	FD_SET(server_fd, &s_data.saved_sockets);

	std::string current_pwd(getcwd(NULL, 0)); // check if null, then exit early
	s_data.max_sckt_fd = server_fd;
	s_data.serverFolder = current_pwd + "/server_files"; //after parsing, to replace
	while(42)
	{
		printf("\n\033[31m++ Waiting for new connection ++\033[0m\n\n");
		s_data.ready_sockets = s_data.saved_sockets;
		if (select(s_data.max_sckt_fd + 1, &s_data.ready_sockets, NULL, NULL, NULL) < 0)
		{
			perror("Select failed ! ");
			return (0);
		}

		for (int i = 0; i <= s_data.max_sckt_fd ; i++)
		{
			if (FD_ISSET(i, &s_data.ready_sockets))
			{
				printf("\n\033[32m========= i = %d =========\033[0m\n\n", i);
				if (i == server_fd) // there is a new connection available on the server socket
				{
					my_socket = accept_connexion(server_fd, &servaddr); // accept the new connection
					FD_SET(my_socket, &s_data.saved_sockets); //add new connection to current set
					printf( "i is %d, server_fd is %d, my_socket is %d\n", i, server_fd, my_socket);
					printf( "request from server_fd : %d\n", my_socket);
					if (my_socket > s_data.max_sckt_fd) // to set the new max
						s_data.max_sckt_fd = my_socket;
				}
				else
				{
					handle_client_request(i, &s_data);
					FD_CLR(i, &s_data.saved_sockets);
				}
			}
		}
		
		my_socket = -1;
	}
	return (argc * 0);
}
