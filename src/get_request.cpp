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

void	get_request(HTTPConfig &http_config, HttpRequest &req, std::map<std::string, ServerConfig> &server_list, t_fd_data &fd_data, std::string server_name)
{
	int errcode = 0;

	std::string target = normalize_path(req.get_target());
	//std::cout << "Target: " << target << std::endl;
	//std::cout << "Server name: " << server_name << std::endl;

	//std::cout << "is_error_request: " << req._is_error_request << std::endl;
	std::map<std::string, ServerConfig>::iterator it_serv;
	if ((it_serv = server_list.find(server_name)) == server_list.end())
	{
		server_name = server_name.substr(server_name.find(':') + 1, server_name.size());
		if ((it_serv = server_list.find(server_name)) == server_list.end())
		{
			std::cerr << "Server not found: " << server_name << std::endl;
			build_response(req, 404, "Not Found", "text/html", displayErrorPage("404", "Page Not Found", "", http_config, req, server_list, fd_data, server_name, req._is_error_request), false);
			return;
		}
	}

	ServerConfig &server = it_serv->second;
	std::string location_name;
	std::string root;
	std::map<std::string, LocationConfig>::iterator it_loc;
	bool autoindex = server.get_autoindex();
	try
	{
		location_name = server.get_matching_location(target, autoindex);
		std::map<std::string, LocationConfig>& location_list = server.get_location_list();
		it_loc = location_list.find(location_name);
		if (it_loc != location_list.end())
			root = it_loc->second.get_root();
		else
			throw std::runtime_error("Location not found");
	}
	catch (std::exception &e)
	{
		std::cerr << "Error getting matching location: " << e.what() << std::endl;
		std::string err;

		build_response(req, 404, "Not Found", "text/html", displayErrorPage("404", "Page Not Found", find_error_page("404", NULL, server, http_config), http_config, req, server_list, fd_data, server_name, req._is_error_request), false);
		return;
	}
	if (root == "")
	{
		std::cerr << "Error: Root directory not set for location: " << location_name << std::endl;
		build_response(req, 500, "Internal Server Error", "text/html", displayErrorPage("500", "Internal Server Error", find_error_page("500", NULL, server, http_config), http_config, req, server_list, fd_data, server_name, req._is_error_request), false);
		return;
	}

	if (check_object_type(root, &errcode) != IS_DIRECTORY)
	{
		std::cerr << "Error: Root directory does not exist or is not a directory: " << root << std::endl;
		build_response(req, 500, "Internal Server Error", "text/html", displayErrorPage("500", "Internal Server Error", find_error_page("500", NULL, server, http_config), http_config, req, server_list, fd_data, server_name, req._is_error_request), false);
		return;
	}

	if (check_allowed_methods(server, it_loc->second, "GET") == false)
	{
		build_response(req, 405, "Method Not Allowed", "text/html", displayErrorPage("405", "Method Not Allowed", find_error_page("405", NULL, server, http_config), http_config, req, server_list, fd_data, server_name, true), false);
		return;
	}

	std::string path_no_index = root + remove_prefix(target, location_name); // Supprimer le préfixe location du target
	std::string file_path = try_index_file(path_no_index, it_loc->second.get_index()); // Si le target finit par '/', on essaie un fichier index

	//std::cout << "File path: " << file_path << std::endl;
	if (check_object_type(file_path, &errcode) != IS_EXISTINGFILE)
	{
		if (!autoindex)
		{
			std::cerr << "Forbidden request: " << file_path << std::endl;
			build_response(req, 403, "Forbidden", "text/html", displayErrorPage("403", "Forbidden Request", find_error_page("403", NULL, server, http_config), http_config, req, server_list, fd_data, server_name, req._is_error_request), false);
			return;
		}
		if (autoindex && check_object_type(path_no_index, &errcode) == IS_DIRECTORY)
		{
			fd_data.requestedFilePath = path_no_index;
			fd_data.serverFolder = server.get_map_server()["root"];
			fd_data.content_len = 0;
			fd_data.folderContent.clear();
			//std::cout << "Directory found, generating index page" << std::endl;
			std::string body = buildCurrentIndexPage(&fd_data, req.get_target(), &errcode);
			build_response(req, 200, "OK", "text/html", body, false);
			return;
		}
		std::cerr << "File not found: " << file_path << std::endl;
		build_response(req, 404, "Not Found", "text/html", displayErrorPage("404", "Page Not Found", find_error_page("404", NULL, server, http_config), http_config, req, server_list, fd_data, server_name, req._is_error_request), false);
		return;
	}

	std::ifstream file(file_path.c_str(), std::ios::binary);
	if (!file.is_open())
	{
		std::cerr << "Error opening file: " << file_path << std::endl;
		build_response(req, 404, "Not Found", "text/html", displayErrorPage("404", "Page Not Found", find_error_page("404", NULL, server, http_config), http_config, req, server_list, fd_data, server_name, req._is_error_request), false);
		return;
	}
	//std::cout << "File found: " << file_path << std::endl;
	std::ostringstream content;
	content << file.rdbuf();
	std::string body = content.str();
	std::string type = get_content_type(file_path);
	//std::cout << "Keep-Alive: " << req.getKeepAlive() << std::endl;
	build_response(req, 200, "OK", type, body, req.getKeepAlive());
}
