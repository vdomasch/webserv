#include "../includes/webserv.hpp"

void	init_base_datastruct(t_fd_data *socket_data)
{
	socket_data->serverFolder = "";
	socket_data->requestedFilePath = "";
	socket_data->max_sckt_fd = -1;
	socket_data->content_type = "";
	socket_data->content_len = 0;
	FD_ZERO(&socket_data->saved_sockets);
	FD_ZERO(&socket_data->ready_sockets);
}

int	initialize_socket(sockaddr_in *servaddr, t_fd_data *socket_data)
{
	int	server_fd;
	int intopt = 1;

	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
	{
		perror("cannot create socket"); 
		return (-1); 
	}
	
	bzero(servaddr, sizeof(*servaddr)); // might be false idk
	init_base_datastruct(socket_data);

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

std::ifstream::pos_type filesize(const char *filename)
{
	std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
	return in.tellg(); 
}

std::string	displayErrorPage(std::string serverFolder, int *errcode)
{
	char		buffer[BUFFER_SIZE] = {0};
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

std::string	handleGIF(t_fd_data *d) // UGLY AS FUCK, TO DELETE SOON
{
	std::ifstream binfile(d->requestedFilePath.c_str(), std::ios::binary);
	std::ostringstream oss;
	std::ifstream::pos_type dataFile;

	if (!binfile.is_open()) 
		std::cerr << "Could not open .gif file" << std::endl; // handle more errors
	else 
	{
		binfile.seekg(0, std::ios::end);
		size_t file_size = binfile.tellg();
		binfile.seekg(0, std::ios::beg);
		std::vector<char> buffer2(file_size);
		binfile.read(&buffer2[0], file_size);
		binfile.close();
		
		std::ostringstream response;
		response << "HTTP/1.1 200 OK\r\n"
		<< "Content-Type: image/gif\r\n"
		<< "Content-Length: " << file_size << "\r\n"
		<< "\r\n";

		d->binaryContent = buffer2;
		
		dataFile = filesize(d->requestedFilePath.c_str());
		oss << dataFile;
		d->content_len = atof(oss.str().c_str());

		return(response.str().c_str());
	}

	return ("errorstring"); // to handle, doesn´t happens unless the file can't be opened
	
}


std::string	handleIcoFile(t_fd_data *d)
{
	// printf("\033[31m DETECTED A ISO REQUEST 🗣 🗣 🗣 🗣 🗣\n %s \n\n \033[0m",d->requestedFilePath.c_str());
	std::ifstream binfile(d->requestedFilePath.c_str(), std::ios::binary);
	std::ostringstream oss;
	std::ifstream::pos_type dataFile;

	if (!binfile.is_open()) 
		std::cerr << "Could not open .ico file" << std::endl; // handle more errors
	else 
	{
		binfile.seekg(0, std::ios::end);
		size_t file_size = binfile.tellg();
		binfile.seekg(0, std::ios::beg);
		std::vector<char> buffer2(file_size);
		binfile.read(&buffer2[0], file_size);
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
	}

	return ("errorstring"); // to handle, doesn´t happens unless the file can't be opened
	
}

bool	indexFileExists(t_fd_data *d, int debug)
{
	struct stat fileinfo;


	if (debug) // check DEBUG_INDEX_EXISTS in webserv.hpp
		return (false);

	if (stat ((d->serverFolder + "/index.html").c_str(), &fileinfo) == 0) // does NOT have to be named index, to be replaced with the name of the index file parsed
	{
		d->requestedFilePath = d->serverFolder + "/index.html";
		return (true);
	}
	else
		return (false);
}

std::string	openAndReadFile(t_fd_data *d, int *errcode)
{
	char			buffer[BUFFER_SIZE];
	int				bytes_read;
	int				fd;
	unsigned int	len;

	len = d->requestedFilePath.length();
	if (len >= 4 && (d->requestedFilePath.substr(len - 4, len - 1) == ".ico")) //ugly hardcoding just to test the ico case
	{
		*errcode = ICOHANDELING;
		return (handleIcoFile(d));
	}

	//horrible,just here to test
	if (len >= 4 && (d->requestedFilePath.substr(len - 4, len - 1) == ".gif")) //ugly hardcoding just to test the ico case
	{
		*errcode = GIFHANDELING;
		return (handleGIF(d));
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
	

	if (filename == "/") // special case, must act like the index page was asked
	{
		d->requestedFilePath = d->serverFolder;
		return (IS_INDEXDIR);
	}
	if (filename.find("cgi-bin") != std::string::npos) // is meant for CGI
	{
		char	*char_pwd = getcwd(NULL, 0);
		std::string current_pwd(char_pwd); // ugly, to change asap, will do for now though
		d->requestedFilePath = current_pwd + filename;
		free(char_pwd);
		return(IS_CGI);
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


void	storeFolderContent(t_fd_data *d, int *errcode)
{
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
	while ((pDirent = readdir(pDir)) != NULL) // can fail, we check ernoo when null is found,and set ernoo to 0 before first call
	{
		std::string fname(pDirent->d_name );
		
		if ((fname == ".") || fname == "..") // skip previous and current folder objects
			continue;
		d->folderContent.push_back(*pDirent);
    }
	closedir (pDir);
	if (errno != 0)
	{
		perror("Readdir failed ! "); 
		*errcode = FAILEDSYSTEMCALL;
		return ;
	}
	
}


void	findParentFolder(std::string &parent, std::string filepath, std::string server_folder)
{
	if (filepath == server_folder || (filepath + "/server_files" == server_folder))
	{
		// printf("\033[31m\nSPECIAL CASE!\nSAME SERVERFOLDER DETECTED ! 🗣 🗣 🗣\033[0m\n");
		parent = "/";
		return ;
	}
	std::size_t pos = filepath.find_last_of('/');   //check if fails i guess ?
	parent = filepath.substr(0, pos);
	pos = parent.find_last_of('/');
	parent = parent.substr(pos);
	printf("\033[31mfinal is (%s) !\033[0m\n", parent.c_str());
	if (parent == "/server_files")
		parent = "/";
}

bool compareBySize(const orderedFiles& a, const orderedFiles& b) {
	return a.lowerName < b.lowerName;
}

void	getRightFileOrder(std::vector<orderedFiles> &sorted, std::vector<dirent> &fileList)
{
	for (std::vector<dirent>::const_iterator i = fileList.begin(); i != fileList.end(); ++i)
	{
		std::string	filename(i->d_name);
		std::transform(filename.begin(), filename.end(), filename.begin(), ::tolower);
		sorted.push_back(orderedFiles(i->d_name, filename, i->d_type));
	}

	std::sort(sorted.begin(), sorted.end(), compareBySize);
	// for (std::vector<orderedFiles>::iterator j = sorted.begin(); j != sorted.end(); ++j)   //---> we can store it !
	// 	std::cout << j->baseName << std::endl;
}

std::string	displayCorrectFileSize(const char * filename)
{
	std::ostringstream		oss;
	std::ifstream::pos_type	posSize;
	float					size;
	int						i = 0;

	posSize = filesize(filename); //check if fail i guess ?
	oss << posSize;
	size = atof(oss.str().c_str());
	
	std::string dico[4] = {" B"," kB"," MB", " GB"};
	while (size > 1024)
	{
		size /= 1024;
		i++;
	}
	if (i > 3)
		i = 3;
	size = floor(size * 10 + 0.5f) / 10.0f;
	oss.str("");
	oss.clear();
	oss << size;
	return (oss.str() + dico[i]);
}



std::string	handleCGI(t_fd_data *d, int *errcode)
{
	std::string	CGIBody;

	printf("beep beep boop ... i'm CGI ... \n\n");

	//to delete
	d->cg.testfd = d->serverSocketFd;
	//
	d->cg.setEnvCGI(d->requestedFilePath);
	d->cg.executeCGI();

	CGIBody = d->cg.grabCGIBody(); // errcode si fail read ?
	*errcode = 0;
	return CGIBody;
}

void	sendSizeAndLastChange(t_fd_data *d, std::ostringstream &oss)
{
	struct stat					fileinfo;
	time_t						timestamp;
	std::vector<orderedFiles>	sorted_names;

	getRightFileOrder(sorted_names, d->folderContent); //sort all elements by name
	for (std::vector<orderedFiles>::const_iterator i = sorted_names.begin(); i != sorted_names.end(); ++i)  // loop for folder
	{
		
		std::string	m_fpath(i->baseName);
		std::string	fileHref;

		if (i->type != DT_DIR)
			continue ;
		fileHref = d->requestedFilePath.substr(d->requestedFilePath.find_last_of('/')) + "/"; // a changerrrrrrr
		if (fileHref == "/server_files/")
			fileHref = ""; 
		oss << "<tr><td>\n";
		oss << "<a class";
		oss << "=\"icon dir\" href=\"";
		oss << fileHref + i->baseName;
		oss << "\">";
		oss << m_fpath + "/";
		oss << " ";
		oss << "</a>\n";
		oss << "</td>\n";

		//Handle size
		oss << "<td> - </td>\n"; // no size needed for folders

		//handle last file modification
		stat((d->requestedFilePath + "/" + m_fpath).c_str(), &fileinfo); // check if fails ?
		timestamp = fileinfo.st_mtime;
		oss << "<td> ";
		oss << ctime(&timestamp) ; //might be C ? is it allowed ?
		oss << " </td>\n";
		oss << "</tr>\n";
	}
	for (std::vector<orderedFiles>::const_iterator i = sorted_names.begin(); i != sorted_names.end(); ++i)  // loop for folder
	{
		
		std::string	m_fpath(i->baseName);
		std::string	fileHref;

		if (i->type == DT_DIR)
			continue ;

		fileHref = d->requestedFilePath.substr(d->requestedFilePath.find_last_of('/')) + "/";
		if (fileHref == "/server_files/") //temporary fix for when we reach last folder
			fileHref = ""; 
		oss << "<tr><td>\n";
		oss << "<a class";
		oss << "=\"icon file\" href=\"\n";
		oss << fileHref + i->baseName;
		oss << "\">";
		oss << m_fpath + "/";
		oss << "</a>\n";
		oss << "</td>\n";
	
		//handle size
		oss << "<td> ";
		oss << displayCorrectFileSize((d->requestedFilePath + "/" + m_fpath).c_str());
		oss << " </td>\n";

		//handle last file modification
		stat((d->requestedFilePath + "/" + m_fpath).c_str(), &fileinfo); // check if fails ?
		timestamp = fileinfo.st_mtime;
		oss << "<td> ";
		oss << ctime(&timestamp) ; //might be C ? is it allowed ?
		oss << " </td>\n";
		oss << "</tr>\n";
	}
}

void	setupHTMLpageStyle(std::ostringstream &oss)
{
	oss << "<html>\n<head>\n<meta name=\"color-scheme\" content=\"light dark\">\n";
	oss << "<style>\n";
	oss << "#parentDirLinkBox {\nmargin-bottom: 10px;\npadding-bottom: 10px;\n}\n";
	oss << "h1 {\nborder-bottom: 1px solid #c0c0c0;\npadding-bottom: 10px;\nmargin-bottom: 10px;\nwhite-space: nowrap;\n}\n";
	oss << "table {\nborder-collapse: collapse;\n}\n";
	oss << "td.detailsColumn {padding-inline-start: 2em;\ntext-align: end;\nwhite-space: nowrap;\n}\n";
	oss << "a.up {\nbackground : url(\"data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAYAAAAf8/9hAAACM0lEQVR42myTA+w1RxRHz+zftmrbdlTbtq04qRGrCmvbDWp9tq3a7tPcub8mj9XZ3eHOGQdJAHw77/LbZuvnWy+c/CIAd+91CMf3bo+bgcBiBAGIZKXb19/zodsAkFT+3px+ssYfyHTQW5tr05dCOf3xN49KaVX9+2zy1dX4XMk+5JflN5MBPL30oVsvnvEyp+18Nt3ZAErQMSFOfelCFvw0HcUloDayljZkX+MmamTAMTe+d+ltZ+1wEaRAX/MAnkJdcujzZyErIiVSzCEvIiq4O83AG7LAkwsfIgAnbncag82jfPPdd9RQyhPkpNJvKJWQBKlYFmQA315n4YPNjwMAZYy0TgAweedLmLzTJSTLIxkWDaVCVfAbbiKjytgmm+EGpMBYW0WwwbZ7lL8anox/UxekaOW544HO0ANAshxuORT/RG5YSrjlwZ3lM955tlQqbtVMlWIhjwzkAVFB8Q9EAAA3AFJ+DR3DO/Pnd3NPi7H117rAzWjpEs8vfIqsGZpaweOfEAAFJKuM0v6kf2iC5pZ9+fmLSZfWBVaKfLLNOXj6lYY0V2lfyVCIsVzmcRV9Y0fx02eTaEwhl2PDrXcjFdYRAohQmS8QEFLCLKGYA0AeEakhCCFDXqxsE0AQACgAQp5w96o0lAXuNASeDKWIvADiHwigfBINpWKtAXJvCEKWgSJNbRvxf4SmrnKDpvZavePu1K/zu/due1X/6Nj90MBd/J2Cic7WjBp/jUdIuA8AUtd65M+PzXIAAAAASUVORK5CYII=\") left top no-repeat;\n}\n";
	oss << "a.file {\n    background : url(\"data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAIAAACQkWg2AAAABnRSTlMAAAAAAABupgeRAAABEElEQVR42nRRx3HDMBC846AHZ7sP54BmWAyrsP588qnwlhqw/k4v5ZwWxM1hzmGRgV1cYqrRarXoH2w2m6qqiqKIR6cPtzc3xMSML2Te7XZZlnW7Pe/91/dX47WRBHuA9oyGmRknzGDjab1ePzw8bLfb6WRalmW4ip9FDVpYSWZgOp12Oh3nXJ7nxoJSGEciteP9y+fH52q1euv38WosqA6T2gGOT44vry7BEQtJkMAMMpa6JagAMcUfWYa4hkkzAc7fFlSjwqCoOUYAF5RjHZPVCFBOtSBGfgUDji3c3jpibeEMQhIMh8NwshqyRsBJgvF4jMs/YlVR5KhgNpuBLzk0OcUiR3CMhcPaOzsZiAAA/AjmaB3WZIkAAAAASUVORK5CYII=\") left top no-repeat;\n}\n";
	oss << "a.dir {\nbackground : url(\"data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAYAAAAf8/9hAAABt0lEQVR42oxStZoWQRCs2cXdHTLcHZ6EjAwnQWIkJyQlRt4Cd3d3d1n5d7q7ju1zv/q+mh6taQsk8fn29kPDRo87SDMQcNAUJgIQkBjdAoRKdXjm2mOH0AqS+PlkP8sfp0h93iu/PDji9s2FzSSJVg5ykZqWgfGRr9rAAAQiDFoB1OfyESZEB7iAI0lHwLREQBcQQKqo8p+gNUCguwCNAAUQAcFOb0NNGjT+BbUC2YsHZpWLhC6/m0chqIoM1LKbQIIBwlTQE1xAo9QDGDPYf6rkTpPc92gCUYVJAZjhyZltJ95f3zuvLYRGWWCUNkDL2333McBh4kaLlxg+aTmyL7c2xTjkN4Bt7oE3DBP/3SRz65R/bkmBRPGzcRNHYuzMjaj+fdnaFoJUEdTSXfaHbe7XNnMPyqryPcmfY+zURaAB7SHk9cXSH4fQ5rojgCAVIuqCNWgRhLYLhJB4k3iZfIPtnQiCpjAzeBIRXMA6emAqoEbQSoDdGxFUrxS1AYcpaNbBgyQBGJEOnYOeENKR/iAd1npusI4C75/c3539+nbUjOgZV5CkAU27df40lH+agUdIuA/EAgDmZnwZlhDc0wAAAABJRU5ErkJggg==\") left top no-repeat;\n}\n";
	oss << "a.icon {\npadding-inline-start: 1.5em;\ntext-decoration: none;\nuser-select: auto;\n}\n";
	oss << "</style>\n";
}

std::string	buildCurrentIndexPage(t_fd_data *d, int *errcode)
{
	std::ostringstream	oss;
	std::string			pageContent;
	std::string			parentFolder;
	
	storeFolderContent(d, errcode);
	findParentFolder(parentFolder, d->requestedFilePath, d->serverFolder);
	if (*errcode == FAILEDSYSTEMCALL)
		return (""); // to handle better ??

	setupHTMLpageStyle(oss); // contains the <style> of the html 
	oss << "<title> Index of";
	oss << d->requestedFilePath + "/";
	oss << "</title>\n</head>\n<body >\n";
	oss << "<h1> Index of " + d->requestedFilePath + "/" "</h1>\n";

	oss << "<div id=\"parentDirLinkBox\" style=\"display: block;\">";
	oss << "<a id=\"parentDirLink\" class=\"icon up\" href=\"";
	oss << parentFolder;
	oss << "\">";
	oss << "<span id=\"parentDirText\">[parent directory]</span>";
    oss << "</a>\n</div>\n";

	oss << "<table style=\"width:80%; font-size: 15px\">\n<thead>\n";
	oss << "<th style=\"text-align:left\"> Name </th>\n";
	oss << "<th style=\"text-align:left\"> Size </th>\n";
	oss << "<th style=\"text-align:left\"> Date Modified  </th>\n</thead>\n<tbody>\n";
	sendSizeAndLastChange(d, oss); // extract the info about file size and last access
	oss << "</tbody>\n</table>\n</body>\n</html>\n";
	*errcode = 0;
	pageContent = oss.str().c_str();
	oss.clear();
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

	first_line = request.substr(0, request.find('\n'));
	filename_start = first_line.find_first_of(' ');
	filename_end = first_line.find_last_of(' ');
	requested_file = first_line.substr(filename_start + 1, filename_end - filename_start - 1);

	printf("\033[34m------------------------------------\n");
	printf("(%s)\n",request.c_str() );
	printf("\033[32m------------------------------------\n");
	printf("Requested : (%s)\n",requested_file.c_str() );
	printf("------------------------------------\033[0m\n");

	objType = checkObjectType(requested_file, d, errcode); // to check if we're looking at a folder or a file

	if (objType == IS_DIRECTORY || objType == IS_INDEXDIR) // NEW : use actual index.html if exists, else list the content of the folder
	{
		if (indexFileExists(d ,DEBUG_INDEX_EXISTS)) // and index file was found, redirecting	
			response = openAndReadFile(d, errcode);
		else
			response = buildCurrentIndexPage(d, errcode); // no index found, listing files instead (if auto-index on)
	}
	else if (objType == IS_EXISTINGFILE)
		response = openAndReadFile(d, errcode);
	else if (objType == IS_CGI)
		response = handleCGI(d, errcode);
	else
		response = displayErrorPage(d->serverFolder, errcode); // check if error is file not found
	return (response);
}

std::string	defineRequestHeaderResponseCode(int errcode, std::string requestBody, t_fd_data *d)
{
	std::string	responseCode;
	std::ostringstream oss;

	if (requestBody.length() == 0) // specific case that i think doesn´t happens anymore ? --> it did when testing the CGI since we return empty string for now
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

	case 3: //gifs i guess ? temporary 
		d->content_len = d->content_len;
		d->content_type = "image/gif";
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
	d->serverSocketFd = socket;
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
		if (d->content_type == "image/x-icon" || d->content_type == "image/gif" )
		{
			send(socket , finalMessage.c_str() , finalMessage.length(), 0);
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


	// s_data.cg.setEnvCGI("oui");  //only here to test out map doesn't segfault

	server_fd = initialize_socket(&servaddr, &s_data);
	if (server_fd < 0)
	{
		perror("cannot bind to socket");
		return (0);
	}
	FD_SET(server_fd, &s_data.saved_sockets);

	char	*char_pwd = getcwd(NULL, 0);
	std::string current_pwd(char_pwd); // check if null, then exit early + getcwd result should be freed
	s_data.max_sckt_fd = server_fd;
	s_data.serverFolder = current_pwd + "/server_files"; //after parsing, to replace
	free(char_pwd);

	while(42)
	{
		printf("\n\033[31m++ Waiting for new connection ++\033[0m\n\n");
		s_data.ready_sockets = s_data.saved_sockets;
		if (select(s_data.max_sckt_fd + 1, &s_data.ready_sockets, NULL, NULL, NULL) < 0) // must also check write
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
