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

std::string	try_index_file(std::string &path, const std::string &index)
{
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

	std::string root = req._location_root;
	std::string error_code = validate_request_context(req._location_name, root, errcode, server, "GET");
	if (!error_code.empty())
		return (build_response(req, error_code, displayErrorPage(error_code, http_config, req, fd_data), false));

	if (req._location_name == "/cgi-bin/")
	{
		fd_data.QueryString = req.get_query_string();
		std::string body;

		body = handleCGI(req, fd_data, &errcode);
		build_response(req, "200", body, req.getKeepAlive());
		return ;
	}

	std::string path_no_index = root + /*remove_prefix(target, req._location_name)*/ target; // Supprimer le préfixe location du target
	std::string file_path = try_index_file(path_no_index, server.get_location_list().find(req._location_name)->second.get_index()); // Si le target finit par '/', on essaie un fichier index
	

	if (check_object_type(file_path, &errcode) != IS_EXISTINGFILE)
	{
		if (!req._autoindex)
		{
			std::cerr << "Error: Forbidden request: " << file_path << std::endl;
			return (build_response(req, "403", displayErrorPage("403", http_config, req, fd_data), false));
		}
		if (req._autoindex && check_object_type(path_no_index, &errcode) == IS_DIRECTORY)
		{
			fd_data.requestedFilePath = path_no_index;
			fd_data.serverFolder = server.get_map_server()["root"];
			fd_data.response_len = 0;
			fd_data.folderContent.clear();
			std::string body = buildCurrentIndexPage(&fd_data, req.get_target(), &errcode);
			if (body.empty())
			{
				std::cerr << "Error: Failed to build index page for: " << file_path << std::endl;
				return (build_response(req, "500", displayErrorPage("500", http_config, req, fd_data), false));
			}
			return (build_response(req, "200", body, false));
		}
		std::cerr << "Error: File not found: " << file_path << std::endl;
		return (build_response(req, "404", displayErrorPage("404", http_config, req, fd_data), false));
	}

	std::ifstream file(file_path.c_str(), std::ios::binary);
	if (!file.is_open())
	{
		std::cerr << "Error: opening file: " << file_path << std::endl;
		return (build_response(req, "404", displayErrorPage("404", http_config, req, fd_data), false));
	}
	std::ostringstream content;
	content << file.rdbuf();
	std::string body = content.str();
	req.set_content_type(get_content_type(file_path));
	build_response(req, "200", body, req.getKeepAlive());
}
