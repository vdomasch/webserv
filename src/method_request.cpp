#include "webserv.hpp"
#include "HttpResponse.hpp"
#include <sys/types.h>
#include <sys/socket.h>

///////////////////////////////////////////////////
//std::string	handleIcoFile(t_fd_data *d)
//{
//	// printf("\033[31m DETECTED A ISO REQUEST 🗣 🗣 🗣 🗣 🗣\n %s \n\n \033[0m",d->requestedFilePath.c_str());
//	std::ifstream binfile(d->requestedFilePath.c_str(), std::ios::binary);
//	std::ostringstream oss;
//	std::ifstream::pos_type dataFile;

//	if (!binfile.is_open()) 
//		std::cerr << "Could not open .ico file" << std::endl; // handle more errors
//	else 
//	{
//		binfile.seekg(0, std::ios::end);
//		size_t file_size = binfile.tellg();
//		binfile.seekg(0, std::ios::beg);
//		std::vector<char> buffer2(file_size);
//		binfile.read(&buffer2[0], file_size);
//		binfile.close();
		
//		std::ostringstream response;
//		response << "HTTP/1.1 200 OK\r\n"
//		<< "Content-Type: image/x-icon\r\n"
//		<< "Content-Length: " << file_size << "\r\n"
//		<< "\r\n";

//		d->binaryContent = buffer2;
		
//		dataFile = filesize(d->requestedFilePath.c_str());
//		oss << dataFile;
//		d->content_len = atof(oss.str().c_str());

//		return(response.str().c_str());
//	}

//	return ("errorstring"); // to handle, doesn´t happens unless the file can't be opened
	
//}

//bool	indexFileExists(t_fd_data *d, int debug)
//{
//	struct stat fileinfo;


//	if (debug) // check DEBUG_INDEX_EXISTS in webserv.hpp
//		return (false);

//	if (stat ((d->serverFolder + "/index.html").c_str(), &fileinfo) == 0) // does NOT have to be named index, to be replaced with the name of the index file parsed
//	{
//		d->requestedFilePath = d->serverFolder + "/index.html";
//		return (true);
//	}
//	else
//		return (false);
//}

//std::string	openAndReadFile(t_fd_data *d, int *errcode)
//{
//	char			buffer[BUFFER_SIZE];
//	int				bytes_read;
//	int				fd;
//	unsigned int	len;

//	len = d->requestedFilePath.length();
//	if (len >= 4 && (d->requestedFilePath.substr(len - 4, len - 1) == ".ico")) //ugly hardcoding just to test the ico case
//	{
//		*errcode = ICOHANDELING;
//		return (handleIcoFile(d));
//	}

//	fd = open(d->requestedFilePath.c_str(), O_RDONLY);	
//	if (fd < 0)
//	{
//		*errcode = FAILEDSYSTEMCALL;
//		return ("void"); // to handle better
//	}
//	bytes_read = read(fd, buffer, BUFFER_SIZE);
//	if (bytes_read < 0)
//	{
//		*errcode = FAILEDSYSTEMCALL;
//		close(fd);
//		return ("void"); //handle better
//	}
//	*errcode = 0;
//	close(fd);
//	std::string response(buffer);
//	d->content_len = response.length();
//	memset(buffer, '\0', sizeof(buffer)); // useless ? -> it's not ???
//	return (response);
//}

///////////////////////////////////////////////////

//bool search_file(const std::string& path, HttpRequest& req, const ServerConfig& server)
//{
//	static_cast<void>(server);
//	std::cout << "File found" << std::endl;
//	std::ifstream file(path.c_str(), std::ios::in);
//	if (!file)
//		return false;

//	std::ostringstream ss;
//	ss << file.rdbuf();
//	file.close();

//	static_cast<void>(req);
//	if (req.getKeepAlive())
//		req.set_response(create_header("200 OK", "text/html", tostr(ss.str().size()), "keep-alive") + ss.str());
//	else
//		req.set_response(create_header("200 OK", "text/html", tostr(ss.str().size()), "close") + ss.str());
//	return true;
//}

std::ifstream::pos_type	filesize(const char *filename)
{
	std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
	return in.tellg(); 
}

std::string displayErrorPage(std::string serverFolder, int *errcode)
{
	char buffer[BUFFER_SIZE] = {0};
	std::string pathToErrPage = serverFolder + "/error_404.html";

	int fd = open(pathToErrPage.c_str(), O_RDONLY);
	if (fd < 0)
	{
		*errcode = FAILEDSYSTEMCALL;
		return "";
	}
	int bytes_read = read(fd, buffer, BUFFER_SIZE);
	close(fd);
	if (bytes_read < 0)
	{
		*errcode = FAILEDSYSTEMCALL;
		return "";
	}
	*errcode = 0;
	return std::string(buffer, bytes_read);
}

std::string	get_content_type(const std::string& path)
{
	size_t dot = path.find_last_of('.');
	if (dot == std::string::npos) return "text/plain";

	std::string ext = path.substr(dot);
	if (ext == ".html") return "text/html";
	if (ext == ".css") return "text/css";
	if (ext == ".js") return "application/javascript";
	if (ext == ".png") return "image/png";
	if (ext == ".jpg" || ext == ".jpeg") return "image/jpeg";
	if (ext == ".ico") return "image/x-icon";
	return "application/octet-stream";
}

int	check_object_type(std::string& path, int *errcode)
{
	struct stat fileinfo;  

    if (stat (path.c_str(), &fileinfo) != 0) // then file exists --> to secure better, check requestedFilePath too
	{
		*errcode = MISSINGFILE;
		return (MISSINGFILE);
	}
	if (S_ISDIR(fileinfo.st_mode))
        return IS_DIRECTORY;
    else if (S_ISREG(fileinfo.st_mode))
        return IS_EXISTINGFILE;
	else
		return FILE_NOT_FOUND;
}

std::string	remove_prefix(std::string target, const std::string prefix)
{
	if (target.find(prefix) == 0)
		target.erase(0, prefix.length());
	return target;
}

std::string	try_index_file(const std::string &path, const std::string &index)
{
	if (path.empty() || path.at(path.size() - 1) != '/')
		return path;
	if (!index.empty())
		return path + index;
	return path;
}

bool	compareBySize(const orderedFiles& a, const orderedFiles& b)
{
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
	d->content_len = pageContent.length();
	d->folderContent.clear();
	return (pageContent);
}

void	get_request(HttpRequest &req, std::map<std::string, ServerConfig> &server_list, t_fd_data &fd_data, std::string server_name)
{
	static_cast<void>(fd_data);

	std::string target = req.get_target();
	std::cout << "Server name: " << server_name << std::endl;

	std::map<std::string, ServerConfig>::iterator it_serv;
	if ((it_serv = server_list.find(server_name)) == server_list.end())
	{
		server_name = server_name.substr(server_name.find(':') + 1, server_name.size());
		if ((it_serv = server_list.find(server_name)) == server_list.end())
		{
			std::cerr << "Server not found: " << server_name << std::endl;
			req.set_response("HTTP/1.1 404 Not Found\r\n\r\n");
			return;
		}
	}

	ServerConfig &server = it_serv->second;
	std::string location;
	std::string root;
	std::map<std::string, LocationConfig>::iterator it_loc;
	bool autoindex = server.get_autoindex();
	try
	{
		location = server.get_matching_location(target, autoindex);
		std::map<std::string, LocationConfig> location_list = server.get_location_list();
		it_loc = location_list.find(location);
		if (it_loc != location_list.end())
			root = it_loc->second.get_root();
		else
			throw std::runtime_error("Location not found");
	}
	catch (std::exception &e)
	{
		std::cerr << "Error getting matching location: " << e.what() << std::endl;
		req.set_response("HTTP/1.1 404 Not Found\r\n\r\n");
		return;
	}

	std::string path_no_index = root + remove_prefix(target, location); // Supprimer le préfixe location du target
	std::string file_path = try_index_file(path_no_index, it_loc->second.get_index()); // Si le target finit par '/', on essaie un fichier index

	std::cout << "File path: " << file_path << std::endl;

	int errcode = 0;
	HttpResponse res;

	if (check_object_type(file_path, &errcode) != IS_EXISTINGFILE)
	{
		if (autoindex && check_object_type(path_no_index, &errcode) == IS_DIRECTORY)
		{
			fd_data.requestedFilePath = path_no_index;
			fd_data.serverFolder = server.get_map_server()["root"];
			fd_data.content_len = 0;
			fd_data.folderContent.clear();
			std::cout << "Directory found, generating index page" << std::endl;
			std::string body = buildCurrentIndexPage(&fd_data, &errcode);
			res.set_status(200, "OK");
			res.set_body(body);
			res.add_header("Content-Type", "text/html");
			try { res.add_header("Content-Length", convert<std::string>(body.size())); }
			catch (std::exception &e) { std::cerr << "Error converting size: " << e.what() << std::endl; }
			req.set_response(res.generate_response());
			PRINT_DEBUG2
			return;
		}
		std::cerr << "File not found: " << file_path << std::endl;
		std::string body = displayErrorPage(root, &errcode);
		res.set_status(404, "Not Found");
		res.set_body(body);
		res.add_header("Content-Type", "text/html");
		try { res.add_header("Content-Length", convert<std::string>(body.size())); }
		catch (std::exception &e) { std::cerr << "Error converting size: " << e.what() << std::endl; }
		res.add_header("Connection", "close");
		req.set_response(res.generate_response());
		return;
	}

	std::ifstream file(file_path.c_str(), std::ios::binary);
	if (!file.is_open())
	{
		std::cerr << "Error opening file: " << file_path << std::endl;
		res.set_status(404, "Not Found");
		res.set_body("File not found");
		res.add_header("Content-Type", "text/plain");
		try { res.add_header("Content-Length", convert<std::string>(res.get_body().size())); }
		catch (std::exception &e) { std::cerr << "Error converting size: " << e.what() << std::endl; }
		req.set_response(res.generate_response());
		return;
	}
	std::cout << "File found: " << file_path << std::endl;
	std::ostringstream content;
	content << file.rdbuf();
	std::string body = content.str();
	std::string type = get_content_type(file_path);

	res.set_status(200, "OK");
	res.set_body(body);
	res.add_header("Content-Type", type);
	try { res.add_header("Content-Length", convert<std::string>(body.size())); }
	catch (std::exception &e) { std::cerr << "Error converting size: " << e.what() << std::endl; }
	req.set_response(res.generate_response());
}

void	post_request(HttpRequest &req, std::map<std::string, ServerConfig> &server_list, t_fd_data &d, std::string response)
{
	static_cast<void>(server_list);
	static_cast<void>(d);

	std::cout << "POST request received" << std::endl;
	std::cout << req << std::endl;

	response = create_header("200 OK", "text/plain", "16", "keep-alive");
}

void	delete_request(HttpRequest &req, std::map<std::string, ServerConfig> &server_list, t_fd_data &d, std::string response)
{
	static_cast<void>(server_list);
	static_cast<void>(d);
	static_cast<void>(response);
	
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