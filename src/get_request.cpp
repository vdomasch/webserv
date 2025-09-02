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

std::string	try_index_file(std::string path, HttpRequest& req, ServerConfig &server)
{
	std::string index = server.get_map_server().find("index")->second;

	std::map<std::string, LocationConfig>::iterator it_loc = server.get_location_list().find(req._location_name);
	if (it_loc != server.get_location_list().end())
	{
		std::map<std::string, std::string> &map = it_loc->second.get_map_location();
		if (map.find("index") != map.end())
			index = map.find("index")->second;
	}

	int errcode = 0;
	if (path.empty())
		return "";
	if (index.empty())
		return path;
	if (check_object_type(path, &errcode) == IS_DIRECTORY)
	{
		if (index.empty())
			return path;
		else if (path.at(path.size() - 1) != '/')
			path += '/'; // Ensure path ends with a slash for directory
		path += index; // Append index file to directory path
	}
	return path;
}

std::string	buildCurrentIndexPage(t_fd_data *d, std::string path, int *errcode);

void	get_request(HTTPConfig &http_config, HttpRequest &req, t_fd_data &fd_data)
{
	int errcode = 0;

	std::string target = req.get_target();
	ServerConfig &server = find_current_server(http_config, req._server_name);

	std::map<std::string, std::string>& map_server = server.get_map_server();
	std::map<std::string, std::string>::iterator it = map_server.find("return");
	if (it != map_server.end())
		build_response(req, it->second.substr(0, 3), "", req.getKeepAlive());

	std::string root = req._location_root;
	std::string error_code = validate_request_context(req._location_name, root, errcode, server, "GET");
	if (!error_code.empty())
		return (build_response(req, error_code, displayErrorPage(error_code, http_config, req, fd_data), req.getKeepAlive()));
	if (req._location_name == "/cgi-bin/" && (target.find(".py") != std::string::npos || target.find(".php") != std::string::npos))
	{
		fd_data.QueryString = req.get_query_string();
		std::string body;

		body = handleCGI(req, fd_data, &errcode);
		if (body.empty())
		{
			if (errcode == 500)
			{
				std::cerr << "Error: System call failed " << target << std::endl;
				return (build_response(req, "500", displayErrorPage("500", http_config, req, fd_data), req.getKeepAlive()));
			}
		}
		return (build_response(req, "200", body, req.getKeepAlive()));
	}
	else if (req._location_name == "/cgi-bin/")
		req._autoindex = false;
	else if (target.find("cgi-bin/") != std::string::npos)
	{
		std::cerr << "Error: No CGI location" << std::endl;
		return (build_response(req, "404", displayErrorPage("404", http_config, req, fd_data), req.getKeepAlive()));
	}
	std::string path_no_index = root + target;
	std::string file_path = try_index_file(path_no_index, req, server);
	
	if (check_object_type(file_path, &errcode) != IS_EXISTINGFILE)
	{
		if (!req._autoindex)
		{
			std::cerr << "Error: Forbidden request: " << file_path << std::endl;
			return (build_response(req, "403", displayErrorPage("403", http_config, req, fd_data), req.getKeepAlive()));
		}
		if (req._autoindex && check_object_type(path_no_index, &errcode) == IS_DIRECTORY)
		{
			fd_data.requestedFilePath = path_no_index;
			fd_data.folderContent.clear();
			std::string body = buildCurrentIndexPage(&fd_data, req.get_target(), &errcode);
			if (body.empty())
			{
				std::cerr << "Error: Failed to build index page for: " << file_path << std::endl;
				return (build_response(req, "500", displayErrorPage("500", http_config, req, fd_data), req.getKeepAlive()));
			}
			return (build_response(req, "200", body, req.getKeepAlive()));
		}
		std::cerr << "Error: File not found: " << file_path << std::endl;
		return (build_response(req, "404", displayErrorPage("404", http_config, req, fd_data), req.getKeepAlive()));
	}

	std::ifstream file(file_path.c_str(), std::ios::binary);
	if (!file.is_open())
	{
		std::cerr << "Error: Opening file: " << file_path << std::endl;
		return (build_response(req, "404", displayErrorPage("404", http_config, req, fd_data), req.getKeepAlive()));
	}
	std::ostringstream content;
	content << file.rdbuf();
	std::string body = content.str();
	req.set_content_type(get_content_type(file_path));
	build_response(req, "200", body, req.getKeepAlive());
}
