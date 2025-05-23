#include "webserv.hpp"
#include <ctime>
#include <iomanip>

static bool	compareBySize(const orderedFiles& a, const orderedFiles& b)
{
	return a.lowerName < b.lowerName;
}

static void	getRightFileOrder(std::vector<orderedFiles> &sorted, std::vector<dirent> &fileList)
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

static std::ifstream::pos_type	filesize(const char *filename)
{
	std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
	return in.tellg(); 
}

static std::string	displayCorrectFileSize(const char * filename)
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

//static void	sendSizeAndLastChange(t_fd_data *d, std::ostringstream &oss)
//{
//	struct stat					fileinfo;
//	time_t						timestamp;
//	std::vector<orderedFiles>	sorted_names;

//	getRightFileOrder(sorted_names, d->folderContent); //sort all elements by name
//	for (std::vector<orderedFiles>::const_iterator i = sorted_names.begin(); i != sorted_names.end(); ++i)  // loop for folder
//	{
		
//		std::string	m_fpath(i->baseName);
//		std::string	fileHref;

//		if (i->type != DT_DIR)
//			continue ;
//		fileHref = d->requestedFilePath.substr(d->requestedFilePath.find_last_of('/')) + "/"; // a changerrrrrrr
//		if (fileHref == "/server_files/")
//			fileHref = ""; 
//		oss << "<tr><td>\n";
//		oss << "<a class";
//		oss << "=\"icon dir\" href=\"";
//		oss << fileHref + i->baseName;
//		oss << "\">";
//		oss << m_fpath + "/";
//		oss << " ";
//		oss << "</a>\n";
//		oss << "</td>\n";

//		//Handle size
//		oss << "<td> - </td>\n"; // no size needed for folders

//		//handle last file modification
//		stat((d->requestedFilePath + "/" + m_fpath).c_str(), &fileinfo); // check if fails ?
//		timestamp = fileinfo.st_mtime;
//		oss << "<td> ";
//		oss << ctime(&timestamp) ; //might be C ? is it allowed ?
//		oss << " </td>\n";
//		oss << "</tr>\n";
//	}
//	for (std::vector<orderedFiles>::const_iterator i = sorted_names.begin(); i != sorted_names.end(); ++i)  // loop for folder
//	{
		
//		std::string	m_fpath(i->baseName);
//		std::string	fileHref;

//		if (i->type == DT_DIR)
//			continue ;

//		fileHref = d->requestedFilePath.substr(d->requestedFilePath.find_last_of('/')) + "/";
//		if (fileHref == "/server_files/") //temporary fix for when we reach last folder
//			fileHref = ""; 
//		oss << "<tr><td>\n";
//		oss << "<a class";
//		oss << "=\"icon file\" href=\"\n";
//		oss << fileHref + i->baseName;
//		oss << "\">";
//		oss << m_fpath + "/";
//		oss << "</a>\n";
//		oss << "</td>\n";
	
//		//handle size
//		oss << "<td> ";
//		oss << displayCorrectFileSize((d->requestedFilePath + "/" + m_fpath).c_str());
//		oss << " </td>\n";

//		//handle last file modification
//		stat((d->requestedFilePath + "/" + m_fpath).c_str(), &fileinfo); // check if fails ?
//		timestamp = fileinfo.st_mtime;
//		oss << "<td> ";
//		oss << ctime(&timestamp) ; //might be C ? is it allowed ?
//		oss << " </td>\n";
//		oss << "</tr>\n";
//	}
//}

static void	sendSizeAndLastChange(t_fd_data *d, std::ostringstream &oss)
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
		oss << "<tr><td>\n<a class=\"icon dir\" href=\"" + fileHref + i->baseName + "\">" + m_fpath + "/ </a>\n</td>\n";

		//Handle size
		oss << "<td> - </td>\n"; // no size needed for folders

		//handle last file modification
		stat((d->requestedFilePath + "/" + m_fpath).c_str(), &fileinfo); // check if fails ?
		timestamp = fileinfo.st_mtime;
		oss << "<td> " << ctime(&timestamp) /*might be C ? is it allowed ?*/ << " </td>\n</tr>\n";
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
		oss << "<tr><td>\n<a class=\"icon file\" href=\"\n" + fileHref + i->baseName + "\">" + m_fpath + "/</a>\n</td>\n";
	
		//handle size
		oss << "<td> " << displayCorrectFileSize((d->requestedFilePath + "/" + m_fpath).c_str()) << " </td>\n";

		//handle last file modification
		stat((d->requestedFilePath + "/" + m_fpath).c_str(), &fileinfo); // check if fails ?
		timestamp = fileinfo.st_mtime;
		oss << "<td> " << ctime(&timestamp) /*might be C ? is it allowed ?*/ << " </td>\n</tr>\n";
	}
}

static void	storeFolderContent(t_fd_data *d, int *errcode)
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

static void	findParentFolder(std::string &parent, std::string filepath, std::string server_folder)
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

static void	setupHTMLpageStyle(std::ostringstream &oss)
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
	d->content_len = pageContent.length();
	d->folderContent.clear();
	return (pageContent);
}

// Structure pour stocker les infos de fichier
//struct FileEntry {
//	std::string name;
//	bool is_dir;
//	std::string size_str;
//	std::string mtime_str;
//};

//// Formate la taille en chaîne
//std::string format_file_size(off_t size) {
//	std::ostringstream ss;
//	ss << size << " bytes";
//	return ss.str();
//}

//bool compare_file_entries(const FileEntry& a, const FileEntry& b)
//{
//	return a.name < b.name;
//}

//// Lit un dossier et retourne une liste d’entrées
//std::vector<FileEntry> read_directory(const std::string& path) {
//	std::vector<FileEntry> entries;
//	DIR* dir = opendir(path.c_str());
//	if (!dir)
//		throw std::runtime_error("Cannot open directory: " + path);

//	struct dirent* entry;
//	while ((entry = readdir(dir)) != NULL) {
//		std::string name = entry->d_name;
//		if (name == "." || name == "..") continue;

//		std::string full_path = path + "/" + name;
//		struct stat s;
//		if (stat(full_path.c_str(), &s) != 0) continue;

//		FileEntry file;
//		file.name = name;
//		file.is_dir = S_ISDIR(s.st_mode);
//		file.size_str = file.is_dir ? "-" : format_file_size(s.st_size);

//		char time_buf[64];
//		std::strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M", std::localtime(&s.st_mtime));
//		file.mtime_str = time_buf;

//		entries.push_back(file);
//	}
//	closedir(dir);
//	std::sort(entries.begin(), entries.end(), compare_file_entries);
//	return entries;
//}

//// Génère la page HTML d’autoindex
//std::string generate_autoindex_html(const std::string& uri, const std::string& real_path)
//{
//	std::ostringstream html;
//	html << "<!DOCTYPE html>\n<html><head>\n<meta name=\"color-scheme\" content=\"light dark\">\n<title>Index of " << uri << "</title>\n";
//	setupHTMLpageStyle(html);
//	html << "</head><body>\n";
//	html << "<h1>Index of " << uri << "</h1><hr><pre>\n";

//	if (uri != "/")
//		html << "<a href=\"../\">../</a>\n";

//	std::vector<FileEntry> files = read_directory(real_path);
//	for (size_t i = 0; i < files.size(); ++i)
//	{
//		const FileEntry& file = files[i];
//		std::string display_name = file.name + (file.is_dir ? "/" : "");
//		html << "<a href=\"" << display_name << "\">" << display_name << "</a>";
//		html << std::setw(40 - display_name.length()) << ' ';
//		html << file.size_str << "  " << file.mtime_str << "\n";
//	}

//	html << "</pre><hr></body></html>\n";
//	return html.str();
//}