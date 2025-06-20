#include "webserv.hpp"
#include <ctime>
#include <iomanip>

static bool compareByDirThenName(const orderedFiles& a, const orderedFiles& b)
{
    if (a.type == DT_DIR && b.type != DT_DIR)
        return true;
    if (a.type != DT_DIR && b.type == DT_DIR)
        return false;
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
	std::sort(sorted.begin(), sorted.end(), compareByDirThenName);
}

static std::ifstream::pos_type	filesize(const char *filename)
{
	std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
	if (in.is_open())
		return in.tellg(); 
	return (0);
}

static std::string	displayCorrectFileSize(const char * filename)
{
	std::ifstream::pos_type posSize = filesize(filename);
	float size = static_cast<float>(posSize);
	int i = 0;
	const char* units[] = {" B", " kB", " MB", " GB"};
	while (size >= 1024.0 && i < 3) // 3 is the last index of units
	{
		size /= 1024;
		++i;
	}
	std::ostringstream oss;
	size = std::floor(size * 10 + 0.5f) / 10.0f; // round to one decimal place
	oss << size << units[i];
	return oss.str();
}

static void	sendSizeAndLastChange(t_fd_data *d, std::ostringstream &oss)
{
	struct stat					fileinfo;
	time_t						timestamp;
	std::vector<orderedFiles>	sorted_names;

	getRightFileOrder(sorted_names, d->folderContent);
	for (std::vector<orderedFiles>::const_iterator i = sorted_names.begin(); i != sorted_names.end(); ++i)
	{
		std::string	m_fpath(i->baseName);
		std::string fullPath = d->requestedFilePath + "/" + m_fpath;

		if (i->type == DT_DIR)
		{
			oss << "<tr><td><a class=\"icon dir\" href=\"" << m_fpath << "\">" << m_fpath << "/</a></td>";
			oss << "<td> - </td>";
		}
		else
		{
			oss << "<tr><td><a class=\"icon file\" href=\"" << m_fpath << "\">" << m_fpath << "</a></td>";
			oss << "<td> " << displayCorrectFileSize(fullPath.c_str()) << " </td>";
		}

		if (stat(fullPath.c_str(), &fileinfo) == 0)
		{
			timestamp = fileinfo.st_mtime;
			oss << "<td> " << std::string(ctime(&timestamp)) << " </td></tr>";
		}
		else
			oss << "<td> ? </td></tr>";
	}
}

static void	storeFolderContent(t_fd_data *d, int *errcode)
{
	DIR *pDir;
	pDir = opendir (d->requestedFilePath.c_str());
	if (pDir == NULL) 
		*errcode = FAILEDSYSTEMCALL;

	struct dirent *pDirent;
	errno = 0;
	while ((pDirent = readdir(pDir)) != NULL) // can fail, we check ernoo when null is found,and set ernoo to 0 before first call
	{
		std::string fname(pDirent->d_name );
		
		if ((fname != ".") && fname != "..") // skip previous and current folder objects
			d->folderContent.push_back(*pDirent);
	}
	closedir (pDir);
	if (errno != 0)
		*errcode = FAILEDSYSTEMCALL;
}

std::string getParentHref(const std::string& path)
{
	if (path == "/" || path.empty())
		return "/";

	std::size_t pos = path.find_last_of('/');
	if (pos == std::string::npos || pos == 0)
		return "/";
	return path.substr(0, pos);
}

static void	setupHTMLpageStyle(std::ostringstream &oss)
{
	oss << "<html>\n<head>\n<meta name=\"color-scheme\" content=\"light dark\">\n";
	oss << "<style>body{font-family:monospace;}table{width:100%;}td{padding:4px;}";
	oss << "div {\nmargin-bottom: 10px;\npadding-bottom: 10px; display: block;\n}\n";
	oss << "h1 {\nborder-bottom: 1px solid #c0c0c0;\npadding-bottom: 10px;\nmargin-bottom: 10px;\nwhite-space: nowrap;\n}\n";
	oss << "table {\nborder-collapse: collapse; width:80%; font-size: 15px; text-align:left\n}\n";
	oss << "td.detailsColumn {padding-inline-start: 2em;\ntext-align: end;\nwhite-space: nowrap;\n}\n";
	oss << "a.up {\nbackground : url(\"data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAYAAAAf8/9hAAACM0lEQVR42myTA+w1RxRHz+zftmrbdlTbtq04qRGrCmvbDWp9tq3a7tPcub8mj9XZ3eHOGQdJAHw77/LbZuvnWy+c/CIAd+91CMf3bo+bgcBiBAGIZKXb19/zodsAkFT+3px+ssYfyHTQW5tr05dCOf3xN49KaVX9+2zy1dX4XMk+5JflN5MBPL30oVsvnvEyp+18Nt3ZAErQMSFOfelCFvw0HcUloDayljZkX+MmamTAMTe+d+ltZ+1wEaRAX/MAnkJdcujzZyErIiVSzCEvIiq4O83AG7LAkwsfIgAnbncag82jfPPdd9RQyhPkpNJvKJWQBKlYFmQA315n4YPNjwMAZYy0TgAweedLmLzTJSTLIxkWDaVCVfAbbiKjytgmm+EGpMBYW0WwwbZ7lL8anox/UxekaOW544HO0ANAshxuORT/RG5YSrjlwZ3lM955tlQqbtVMlWIhjwzkAVFB8Q9EAAA3AFJ+DR3DO/Pnd3NPi7H117rAzWjpEs8vfIqsGZpaweOfEAAFJKuM0v6kf2iC5pZ9+fmLSZfWBVaKfLLNOXj6lYY0V2lfyVCIsVzmcRV9Y0fx02eTaEwhl2PDrXcjFdYRAohQmS8QEFLCLKGYA0AeEakhCCFDXqxsE0AQACgAQp5w96o0lAXuNASeDKWIvADiHwigfBINpWKtAXJvCEKWgSJNbRvxf4SmrnKDpvZavePu1K/zu/due1X/6Nj90MBd/J2Cic7WjBp/jUdIuA8AUtd65M+PzXIAAAAASUVORK5CYII=\") left top no-repeat;\n}\n";
	oss << "a.file {\n    background : url(\"data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAIAAACQkWg2AAAABnRSTlMAAAAAAABupgeRAAABEElEQVR42nRRx3HDMBC846AHZ7sP54BmWAyrsP588qnwlhqw/k4v5ZwWxM1hzmGRgV1cYqrRarXoH2w2m6qqiqKIR6cPtzc3xMSML2Te7XZZlnW7Pe/91/dX47WRBHuA9oyGmRknzGDjab1ePzw8bLfb6WRalmW4ip9FDVpYSWZgOp12Oh3nXJ7nxoJSGEciteP9y+fH52q1euv38WosqA6T2gGOT44vry7BEQtJkMAMMpa6JagAMcUfWYa4hkkzAc7fFlSjwqCoOUYAF5RjHZPVCFBOtSBGfgUDji3c3jpibeEMQhIMh8NwshqyRsBJgvF4jMs/YlVR5KhgNpuBLzk0OcUiR3CMhcPaOzsZiAAA/AjmaB3WZIkAAAAASUVORK5CYII=\") left top no-repeat;\n}\n";
	oss << "a.dir {\nbackground : url(\"data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAYAAAAf8/9hAAABt0lEQVR42oxStZoWQRCs2cXdHTLcHZ6EjAwnQWIkJyQlRt4Cd3d3d1n5d7q7ju1zv/q+mh6taQsk8fn29kPDRo87SDMQcNAUJgIQkBjdAoRKdXjm2mOH0AqS+PlkP8sfp0h93iu/PDji9s2FzSSJVg5ykZqWgfGRr9rAAAQiDFoB1OfyESZEB7iAI0lHwLREQBcQQKqo8p+gNUCguwCNAAUQAcFOb0NNGjT+BbUC2YsHZpWLhC6/m0chqIoM1LKbQIIBwlTQE1xAo9QDGDPYf6rkTpPc92gCUYVJAZjhyZltJ95f3zuvLYRGWWCUNkDL2333McBh4kaLlxg+aTmyL7c2xTjkN4Bt7oE3DBP/3SRz65R/bkmBRPGzcRNHYuzMjaj+fdnaFoJUEdTSXfaHbe7XNnMPyqryPcmfY+zURaAB7SHk9cXSH4fQ5rojgCAVIuqCNWgRhLYLhJB4k3iZfIPtnQiCpjAzeBIRXMA6emAqoEbQSoDdGxFUrxS1AYcpaNbBgyQBGJEOnYOeENKR/iAd1npusI4C75/c3539+nbUjOgZV5CkAU27df40lH+agUdIuA/EAgDmZnwZlhDc0wAAAABJRU5ErkJggg==\") left top no-repeat;\n}\n";
	oss << "a.icon {\npadding-inline-start: 1.5em;\ntext-decoration: none;\nuser-select: auto;\n}\n";
	oss << "</style>\n";
}

std::string	buildCurrentIndexPage(t_fd_data *d, std::string path, int *errcode)
{
	std::ostringstream	oss;
	storeFolderContent(d, errcode);
	if (*errcode == FAILEDSYSTEMCALL)
		return ("");

	setupHTMLpageStyle(oss);

	oss << "<title>Index of " + path + "</title>\n</head>\n<body>\n<h1> Index of " + path + "</h1>\n";
	oss << "<div><a class=\"icon up\" href=\"" + getParentHref(path) + "\">[parent directory]</a>\n</div>\n";
	oss << "<table>\n<thead>\n<th> Name </th>\n<th> Size </th>\n<th> Date Modified </th>\n</thead>\n<tbody>\n";

	sendSizeAndLastChange(d, oss); // extract the info about file size and last access
	oss << "</tbody>\n</table>\n</body>\n</html>\n";
	*errcode = 0;
	std::string result = oss.str().c_str();
	d->response_len = result.length();
	d->folderContent.clear();
	return (result);
}