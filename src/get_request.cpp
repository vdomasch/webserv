#include "webserv.hpp"
#include <sys/types.h>
#include <sys/socket.h>

void	get_request(HTTPConfig &http_config, HttpRequest &req, std::map<std::string, ServerConfig> &server_list, t_fd_data &fd_data, std::string server_name);

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

std::string	try_index_file(const std::string &path, const std::string &index)
{
	if (path.empty())
		return "";
	if (!index.empty() && path.at(path.size() - 1) == '/')
		return path + index;
	return path;
}

std::string	buildCurrentIndexPage(t_fd_data *d, std::string path, int *errcode);

std::string find_location_name(const std::string &target, ServerConfig &server, std::string &root, bool& autoindex)
{
	std::map<std::string, LocationConfig>::iterator it_loc;
	std::string location_name = server.get_matching_location(target, autoindex);
	std::map<std::string, LocationConfig>& location_list = server.get_location_list();
	it_loc = location_list.find(location_name);
	if (it_loc != location_list.end())
		root = it_loc->second.get_root();
	else
		return "";
	return location_name;
}

void	get_request(HTTPConfig &http_config, HttpRequest &req, t_fd_data &fd_data, std::string server_name)
{
	int errcode = 0;

	std::string target = normalize_path(req.get_target());

	ServerConfig &server = find_current_server(http_config, server_name);
	bool autoindex = server.get_autoindex();


	std::string root("");
	std::string location_name = find_location_name(target, server, root, autoindex);
	if (location_name.empty())
	{
		std::cerr << "Error: No matching location found for target: " << target << std::endl;
		build_response(req, "404", displayErrorPage("404", location_name, http_config, req, fd_data, server_name), false);
		return;
	}
	if (root.empty())
	{
		std::cerr << "Error: Root directory not set for location: " << location_name << std::endl;
		build_response(req, "500", displayErrorPage("500", location_name, http_config, req, fd_data, server_name), false);
		return;
	}

	if (check_object_type(root, &errcode) != IS_DIRECTORY)
	{
		std::cerr << "Error: Root directory does not exist or is not a directory: " << root << std::endl;
		build_response(req, "500", displayErrorPage("500", location_name, http_config, req, fd_data, server_name), false);
		return;
	}

	if (check_allowed_methods(server, server.get_location_list().find(location_name)->second, "GET") == false)
	{
		std::cerr << "Error: Method GET not allowed for location: " << location_name << std::endl;
		build_response(req, "405", displayErrorPage("405", location_name, http_config, req, fd_data, server_name), false);
		return;
	}

	if (location_name == "/cgi-bin/")
	{
		PRINT_DEBUG2
		//std::string	c_type = request.substr(request.find("Content-Type:") + 14); // + 14 is to skip "Content-Type: " and to only grab the type
		//d->Content_Type = c_type.substr(0, c_type.find("\r\n"));
		//std::string	c_len = request.substr(request.find("Content-Length:") + 16); // same thing
		//d->Content_Length = c_len.substr(0, c_len.find("\r\n"));
		fd_data.Content_Type = req.get_header("Content-Type"); // Assurez-vous que le Content-Type est présent
		fd_data.Content_Length = req.get_header("Content-Length"); // Assurez-vous que le Content-Length est présent
		handleCGI(fd_data, &errcode);
	}

	std::string path_no_index = root + remove_prefix(target, location_name); // Supprimer le préfixe location du target
	std::string file_path = try_index_file(path_no_index, server.get_location_list().find(location_name)->second.get_index()); // Si le target finit par '/', on essaie un fichier index

	if (check_object_type(file_path, &errcode) != IS_EXISTINGFILE)
	{
		if (!autoindex)
		{
			std::cerr << "Forbidden request: " << file_path << std::endl;
			build_response(req, "403", displayErrorPage("403", location_name, http_config, req, fd_data, server_name), false);
			return;
		}
		if (autoindex && check_object_type(path_no_index, &errcode) == IS_DIRECTORY)
		{
			fd_data.requestedFilePath = path_no_index;
			fd_data.serverFolder = server.get_map_server()["root"];
			fd_data.response_len = 0;
			//fd_data.content_len = 0;
			fd_data.folderContent.clear();
			std::string body = buildCurrentIndexPage(&fd_data, req.get_target(), &errcode);
			build_response(req, "200", body, false);
			return;
		}
		std::cerr << "File not found: " << file_path << std::endl;
		build_response(req, "404", displayErrorPage("404", location_name, http_config, req, fd_data, server_name), false);
		return;
	}

	std::ifstream file(file_path.c_str(), std::ios::binary);
	if (!file.is_open())
	{
		std::cerr << "Error opening file: " << file_path << std::endl;
		build_response(req, "404", displayErrorPage("404", location_name, http_config, req, fd_data, server_name), false);
		return;
	}
	std::ostringstream content;
	content << file.rdbuf();
	std::string body = content.str();
	req.set_content_type(get_content_type(file_path));
	build_response(req, "200", body, req.getKeepAlive());
}
