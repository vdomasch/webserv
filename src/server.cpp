#include "../includes/webserv.hpp"

void	init_base_datastruct(t_socket_data *socket_data)
{
	socket_data->serverFolder = "";
	socket_data->requestedFilePath = "";
	socket_data->max_sckt_fd = -1;
	socket_data->response_len = 0;
	socket_data->Content_Length = "default";
	socket_data->Content_Type = "default";
	socket_data->is_binaryContent = false;
	FD_ZERO(&socket_data->ready_readsockets);
	FD_ZERO(&socket_data->ready_writesockets);
	FD_ZERO(&socket_data->saved_readsockets);
	FD_ZERO(&socket_data->saved_writesockets);
}

int	initialize_socket(sockaddr_in *servaddr, t_socket_data *socket_data)
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

	memset(buffer, '\0', sizeof(buffer));
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


std::string	handleBinaryFiles(t_socket_data *d)
{
	std::ifstream binfile(d->requestedFilePath.c_str(), std::ios::binary);
	std::ostringstream oss;
	std::ifstream::pos_type dataFile;

	if (!binfile.is_open()) 
		std::cerr << "Could not open binary file.." << std::endl; // handle more errors
	else 
	{
		binfile.seekg(0, std::ios::end);
		size_t file_size = binfile.tellg();
		binfile.seekg(0, std::ios::beg);
		std::vector<char> buffer2(file_size);
		binfile.read(&buffer2[0], file_size);
		binfile.close();
		
		std::ostringstream response;
		response << "Content-Length: " << file_size << "\r\n\r\n";
		d->binaryContent = buffer2;
		
		dataFile = filesize(d->requestedFilePath.c_str());
		oss << dataFile;
		//d->response_len = atof(oss.str().c_str()); // pretty much useless now ?
		return(response.str().c_str());
	}
	return ("errorstring"); // to handle, doesn´t happens unless the file can't be opened
	
}

bool	indexFileExists(t_socket_data *d, int debug)
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

void		identifyFileExtension(std::string filename, int *errcode)
{
	int len = filename.length();
	if (len < 4)
	{
		errcode = 0;
		return ;
	}
	std::string	extension = filename.substr(len - 4, len - 1);
	std::string	(extension_dictionary[4]) = {".ico", ".gif", ".png", ".jpg"};
	int type = -1;
	while(++type < 4)
	{
		if (extension == extension_dictionary[type])
			break ;
	}

	switch (type)
	{
	case 0:
		*errcode = ICOHANDELING;
		break;
	case 1:
		*errcode = GIFHANDELING;
		break;
	case 2:
		*errcode = PNGHANDELING;
		break;
	case 3:
		*errcode = JPGHANDELING;
		break;
	
	default:
		*errcode = 0;
		break;
	}
}

std::string	openAndReadFile(t_socket_data *d, int *errcode)
{
	char			buffer[BUFFER_SIZE];
	int				bytes_read;
	int				fd;
	unsigned int	len;

	memset(buffer, '\0', sizeof(buffer));
	len = d->requestedFilePath.length();
	identifyFileExtension(d->requestedFilePath, errcode);
	if (*errcode && *errcode != CSSHANDELING) // if image, recover the binary data
		return (handleBinaryFiles(d));
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
	if (len >= 4 && (d->requestedFilePath.substr(len - 4, len - 1) == ".css"))
		*errcode = CSSHANDELING;
	else
		*errcode = 0;
	close(fd);
	std::string response(buffer);
	d->response_len = response.length();
	memset(buffer, '\0', sizeof(buffer)); // useless ? -> it's not ???
	return (response);
}

int	checkObjectType(std::string filename, t_socket_data *d, int *errcode)
{
	struct stat fileinfo;  
	std::string pathToCheck;
	std::string	fileContent;
	int			statErr = 0;
	

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

	statErr = stat (pathToCheck.c_str(), &fileinfo);
    if (statErr == 0) // then file exists --> to secure better, check requestedFilePath too
		printf("\033[34mFound it --> \033[0m");
	else
	{
		printf("\033[31mFile wasn't found ! (%d) Setting error code appropriately !\n\033[0m\n", errno);
		perror("ERRNO SAYS : ");
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

void	storeFolderContent(t_socket_data *d, int *errcode)
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


	// IMPORTANT ! server_files is to be replaced with the folder provided in config file !!!!!!!
	//
	if (filepath == server_folder || (filepath + "/server_files" == server_folder))
	{
		// printf("\033[31m\nSPECIAL CASE!\nSAME SERVERFOLDER DETECTED ! 🗣 🗣 🗣\033[0m\n");
		parent = "/";
		return ;
	}

	std::string	extendedServerFolder = server_folder + "/";
	parent = filepath.substr(extendedServerFolder.find_last_of("/"));
	printf("\033[31m\nSpecial parent is (%s) !\033[0m\n\n", parent.c_str());
	std::size_t pos = parent.find_last_of('/');
	if (pos == 0)
	{
		parent = "/";
		return ;
	}

	if (pos != std::string::npos)
		parent = parent.substr(0, pos);
	printf("\033[31mfinal is (%s) !\033[0m\n", parent.c_str());

	// OLD METHOD
	// std::size_t pos = filepath.find_last_of('/');   //check if it fails i guess ?
	// parent = filepath.substr(0, pos);
	// printf("\033[31mParent is (%s) !\033[0m\n", parent.c_str());
	// pos = parent.find_last_of('/');
	// parent = parent.substr(pos);


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



std::string	handleCGI(t_socket_data *d, int *errcode)  ////////////////////////////////////////////////////////
{
	std::string	CGIBody;
	int			CGI_body_size;

	printf("beep beep boop ... i'm CGI ... \n\n");
	d->cg.setEnvCGI(d->requestedFilePath, d->Content_Type, d->Content_Length, d->method_name);
	d->cg.executeCGI();
	d->cg.sendCGIBody(&d->binaryContent);
	CGIBody = d->cg.grabCGIBody(CGI_body_size); // errcode si fail read ?

	//test, avoid zombie i guess ?
	int status = 0;
	waitpid(d->cg.cgi_forkfd, &status, 0);
	if(WEXITSTATUS(status) != 0)
	{
		printf("Ptit flop\n\n");
		return ("emptyerror");
	}

	d->response_len = CGI_body_size;
	*errcode = 0;
	return CGIBody;
}

void	sendSizeAndLastChange(t_socket_data *d, std::ostringstream &oss)
{
	struct stat					fileinfo;
	time_t						timestamp;
	std::vector<orderedFiles>	sorted_names;

	getRightFileOrder(sorted_names, d->folderContent); //sort all elements by name
	for (std::vector<orderedFiles>::const_iterator i = sorted_names.begin(); i != sorted_names.end(); ++i)  // loop for folder
	{
		
		std::string	m_fpath(i->baseName);
		std::string	fileHref;
		std::string	extendedServerFolder;

		if (i->type != DT_DIR) // if not a dir, leave
			continue ;

		extendedServerFolder = d->serverFolder + "/";
		fileHref = d->requestedFilePath.substr(extendedServerFolder.find_last_of("/")) + "/";

		if (fileHref == "/server_files/") // i don´t think it happens anymore
			fileHref = "";
		// In any case, server_files is to be replaced with actual folder name from config file

		oss << "<tr><td>";
		oss << "<a class";
		oss << "=\"icon dir\" href=\"";
		oss << fileHref + i->baseName;
		oss << "\">";
		oss << m_fpath + "/";
		oss << " ";
		oss << "</a>";
		oss << "</td>\n";

		//Handle size
		oss << "<td>-</td>\n"; // no size needed for folders

		//handle last file modification
		stat((d->requestedFilePath + "/" + m_fpath).c_str(), &fileinfo); // check if fails ?
		timestamp = fileinfo.st_mtime;
		oss << "<td>";
		oss << ctime(&timestamp); //might be C ? is it allowed ?
		oss << "</td>\n";
		oss << "</tr>\n";
	}
	for (std::vector<orderedFiles>::const_iterator i = sorted_names.begin(); i != sorted_names.end(); ++i)  // loop for folder
	{
		
		std::string	m_fpath(i->baseName);
		std::string	fileHref;
		std::string	extendedServerFolder;
		
		if (i->type == DT_DIR) //if not a file, leave
		continue ;
		
		// printf("request if(%s)\n", d->requestedFilePath.c_str());
		extendedServerFolder = d->serverFolder + "/";
		fileHref = d->requestedFilePath.substr(extendedServerFolder.find_last_of("/")) + "/";
		// printf("\n\nHref is : (%s)\n\n", fileHref.c_str());
		
		//  EDIT : FIXED I THINK
		//  (OLD) 
		//	fileHref = d->requestedFilePath.substr(d->requestedFilePath.find_last_of('/')) + "/" is wrong.
		//  This is not a correct method, since /home/lchapard/Documents/Webserv/server_files/assets/css_files/myfile.css
		//  will give a href of /css_files/myfile.css, which when searched by the server in the next loop, will give
		//  /home/lchapard/Documents/Webserv/server_files  +  /css_files/myfile.css, with the asset folder missing,
		//  preventing us from finding the file


		if (fileHref == "/server_files/") //temporary fix for when we reach last folder
			fileHref = "";
		// server_files is to be replaced with actual folder name


		oss << "<tr><td>";
		oss << "<a class";
		oss << "=\"icon file\" href=\"";
		oss << fileHref + i->baseName;
		oss << "\">";
		oss << m_fpath + "/";
		oss << "</a>";
		oss << "</td>\n";
	
		//handle size
		oss << "<td>";
		oss << displayCorrectFileSize((d->requestedFilePath + "/" + m_fpath).c_str());
		oss << "</td>\n";

		//handle last file modification
		stat((d->requestedFilePath + "/" + m_fpath).c_str(), &fileinfo); // check if fails ?
		timestamp = fileinfo.st_mtime;
		oss << "<td>";
		oss << ctime(&timestamp) ; //might be C ? is it allowed ?
		oss << "</td>\n";
		oss << "</tr>\n";
	}
}

void	setupHTMLpageStyle(std::ostringstream &oss)
{
	oss << "<!DOCTYPE html>\n";
	oss << "<html>\n<head>\n<meta name=\"color-scheme\" content=\"light dark\" charset=\"UTF-8\">\n";
	oss << "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n";
	oss << "<link rel=\"stylesheet\" href=\"/assets/css_files/autoindex.css\">\n";
}

std::string	buildCurrentIndexPage(t_socket_data *d, int *errcode)
{
	std::ostringstream	oss;
	std::string			pageContent;
	std::string			parentFolder;
	
	storeFolderContent(d, errcode);
	findParentFolder(parentFolder, d->requestedFilePath, d->serverFolder);
	if (*errcode == FAILEDSYSTEMCALL)
		return (""); // to handle better ??

	setupHTMLpageStyle(oss); // contains the <style> of the html 
	oss << "<title> Index of ";
	oss << d->requestedFilePath + "/";
	oss << "</title>\n</head>\n<body >\n";
	oss << "<h1> Index of " + d->requestedFilePath + "/" "</h1>\n";

	oss << "<div id=\"parentDirLinkBox\" style=\"display: block;\">\n";
	oss << "<a id=\"parentDirLink\" class=\"icon up\" href=\"";
	oss << parentFolder;
	oss << "\">\n";
	oss << "<span id=\"parentDirText\">[parent directory]</span>\n";
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
	d->response_len = pageContent.length();
	d->folderContent.clear();
	return (pageContent);
}

std::string	isolateFileAttributes(std::string request, t_socket_data *d) // temporary just to make cleaner, will be deleted and replaced
{
	size_t		filename_start;
	size_t		filename_end;
	size_t		header_end;
	std::string requested_file;
	std::string c_type;
	std::string c_len;
	std::string first_line;
	std::string curated_body; // temporary, should idealy be able to hold binary
	
	first_line = request.substr(0, request.find('\n'));
	filename_start = first_line.find_first_of(' ');
	filename_end = first_line.find_last_of(' ');
	requested_file = first_line.substr(filename_start + 1, filename_end - filename_start - 1);
	

	// std::cout << "REQUEST AT THE START IS " << request.length() << "\n";
	printf("Full request :\n");
	printf("\033[34m------------------------------------\n");
	printf("(%s)\n",request.c_str() );
	printf("------------------------------------\n");
	printf("\033[32m------------------------------------\n");
	printf("Requested : (%s)\n",requested_file.c_str() );
	printf("------------------------------------\033[0m\n");


	d->method_name = first_line.substr(0, filename_start);
	header_end = request.find("\r\n\r\n");
	if (header_end == std::string::npos)
		return ("[isolateFileName] Header is invalid, \\r\\n\\r\\n was not found in request."); // handle better ?
	else if (d->method_name == "POST")
	{
		c_type = request.substr(request.find("Content-Type:") + 14); // + 14 is to skip "Content-Type: " and to only grab the type
		d->Content_Type = c_type.substr(0, c_type.find("\r\n"));
		c_len = request.substr(request.find("Content-Length:") + 16); // same thing
		d->Content_Length = c_len.substr(0, c_len.find("\r\n"));

		curated_body = request.substr(header_end + 4);
		// std::cout << "CURATED LEN IS " << curated_body.length() << "\n";
		printf("\033[33m------------------------------------\n");
		printf("POST body : (%s)\n",curated_body.c_str() );
		printf("------------------------------------\033[0m\n");
		std::vector<char> binary_body(curated_body.begin(), curated_body.end());
		d->binaryContent = binary_body;
	}
	return (requested_file);
}

std::string	analyse_request(char buffer[BUFFER_SIZE], t_socket_data *d, int *errcode, int bytes_read)
{
	std::string request(buffer, bytes_read);
	
	std::string requested_file;
	std::string response;
	char		objType;
	
	std::cout << "BUFF SIZE ISSSSSSSSSSSSSSSSSSS" << request.size() << "\n";
	requested_file = isolateFileAttributes(request, d);
	objType = checkObjectType(requested_file, d, errcode); // to check if we're looking at a folder or a file

	if (objType == IS_DIRECTORY || objType == IS_INDEXDIR) // NEW : uses actual index.html if exists, else list the content of the folder
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

std::string	defineRequestHeaderResponseCode(int errcode, std::string requestBody, t_socket_data *d)
{
	std::string	responseCode;
	std::string	headerStart;
	std::ostringstream oss;

	if (requestBody.length() == 0) // specific case that i think doesn´t happens anymore ? --> it did when testing the CGI since we return empty string for now
	{
		std::cout << "I'm out ! 1.3 sec\n" << std::endl;
		return (""); 
	}
	oss << d->response_len;
	switch (errcode)
	{
		case ICOHANDELING:
			headerStart = "HTTP/1.1 200 OK\r\nContent-Type: image/x-icon\r\n";
			headerStart.append(requestBody);
			d->is_binaryContent = true;
			return(headerStart);
	
		case GIFHANDELING:
			headerStart = "HTTP/1.1 200 OK\r\nContent-Type: image/gif\r\n";
			headerStart.append(requestBody);
			d->is_binaryContent = true;
			return(headerStart);
	
		case PNGHANDELING:
			headerStart = "HTTP/1.1 200 OK\r\nContent-Type: image/png\r\n";
			headerStart.append(requestBody);
			d->is_binaryContent = true;
			return(headerStart);
	
		case JPGHANDELING:
			headerStart = "HTTP/1.1 200 OK\r\nContent-Type: image/jpeg\r\n";
			headerStart.append(requestBody);
			d->is_binaryContent = true;
			return(headerStart);
	
		case CSSHANDELING:
			responseCode = "HTTP/1.1 200 OK\nContent-Type: text/css\r\nContent-Length: ";
			responseCode.append(oss.str());
			responseCode.append("\r\n\r\n\n");
			d->is_binaryContent = false;
			break;
	
		default:
			responseCode = "HTTP/1.1 200 OK\nContent-Type: text/html\r\nContent-Length: ";
			responseCode.append(oss.str());
			responseCode.append("\r\n\r\n\n");
			d->is_binaryContent = false;
			break;
	}
	responseCode = responseCode + requestBody;
	return (responseCode);
}

int	handle_client_request(int socket, t_socket_data *d)
{
	char buffer[BUFFER_SIZE] = {0}; /// in this case, header size is included in buffer_size = bad ?? --> i might be stupid
	std::string	requestBody;
	std::string	finalMessage;
	int			errcode;
	ssize_t 	bytesRead;
	
	//Receive the new message :

	memset(buffer, '\0', sizeof(buffer));
	bytesRead = read(socket , buffer, BUFFER_SIZE);

	if (bytesRead < 0)
	{
		perror("Failed to read ! ");
		return (-1);
	}
	else if (bytesRead == 0) //can happen with multiple fast request in a row
	{
		printf("Read 0 bytes ... Closing..\n");
		close(socket);
		return (0);
	}
	printf("(%lu)\n",bytesRead );
	requestBody = analyse_request(buffer, d, &errcode, bytesRead); // decide how to interpret the request
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
		if (d->is_binaryContent == true)
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
	t_socket_data s_data; // to set select	

	server_fd = initialize_socket(&servaddr, &s_data);
	if (server_fd < 0)
	{
		perror("cannot bind to socket");
		return (0);
	}
	FD_SET(server_fd, &s_data.saved_readsockets);

	char	*char_pwd = getcwd(NULL, 0);
	std::string current_pwd(char_pwd); // to check if null, then exit early + getcwd result should be freed
	s_data.max_sckt_fd = server_fd;
	s_data.serverFolder = current_pwd + "/server_files"; //after parsing, to replace
	free(char_pwd);

	while(42)
	{
		printf("\n\033[31m++ Waiting for new connection ++\033[0m\n\n");
		s_data.ready_readsockets = s_data.saved_readsockets;
		if (select(s_data.max_sckt_fd + 1, &s_data.ready_readsockets, NULL, NULL, NULL) < 0) // must also check write
		{
			perror("Select failed ! ");
			return (0);
		}

		for (int i = 0; i <= s_data.max_sckt_fd ; i++)
		{
			if (FD_ISSET(i, &s_data.ready_readsockets))
			{
				printf("\n\033[32m========= i = %d =========\033[0m\n\n", i);
				if (i == server_fd) // there is a new connection available on the server socket
				{
					my_socket = accept_connexion(server_fd, &servaddr); // accept the new connection
					FD_SET(my_socket, &s_data.saved_readsockets); //add new connection to current set
					printf( "i is %d, server_fd is %d, my_socket is %d\n", i, server_fd, my_socket);
					printf( "request from server_fd : %d\n", my_socket);
					if (my_socket > s_data.max_sckt_fd) // to set the new max
						s_data.max_sckt_fd = my_socket;
				}
				else
				{
					handle_client_request(i, &s_data);
					FD_CLR(i, &s_data.saved_readsockets);
				}
			}
		}
		my_socket = -1;
	}
	return (argc * 0);
}
