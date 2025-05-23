#include "webserv.hpp"
#include "HttpResponse.hpp"
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

std::string	buildCurrentIndexPage(t_fd_data *d, int *errcode);

void build_response(HttpRequest &req, int status_code, const std::string &status_msg, const std::string &content_type, const std::string &body, bool close_connection)
{
	HttpResponse res;
	res.set_status(status_code, status_msg);
	res.set_body(body);
	res.add_header("Content-Type", content_type);
	if (close_connection)
		res.add_header("Connection", "close");
	else
		res.add_header("Connection", "keep-alive");
	try { res.add_header("Content-Length", convert<std::string>(body.size())); }
	catch (std::exception &e) { std::cerr << "Error converting size: " << e.what() << std::endl; }
	req.set_response(res.generate_response());
}

std::string displayErrorPage(const std::string& error_uri, HTTPConfig& http_config, HttpRequest& req, std::map<std::string, ServerConfig>& server_list, t_fd_data& fd_data, const std::string& server_name, bool is_error_request)
{
    if (error_uri.empty() || is_error_request)
	{
		return "<html><body><h1>404 Page Not Found</h1></body></html>";
	}

	req.set_target(error_uri);
	req._is_error_request = true;

	get_request(http_config, req, server_list, fd_data, server_name);

	if (req.get_response().empty())
	{
		std::cerr << "Error: No response from server" << std::endl;
		return "<html><body><h1>500 Internal Server Error</h1></body></html>";
	}
	else if (req.get_response().find("\r\n\r\n") != std::string::npos)
		return req.get_response().substr(req.get_response().find("\r\n\r\n") + 4);
	return req.get_response();
}

//std::string generate_autoindex_html(const std::string& uri, const std::string& real_path);

std::string find_error_page(const std::string& code, LocationConfig* loc, ServerConfig& serv, HTTPConfig& http)
{
	// Vérifie Location (si fourni)
	if (loc)
	{
		std::map<std::string, std::string> map = loc->get_map_location();
		std::map<std::string, std::string>::iterator it = map.find(code);
		if (it != map.end())
		{
			std::string res = it->second;
			return res;
		}
	}

	// Vérifie Server
	std::map<std::string, std::string> map = serv.get_map_server();
	std::map<std::string, std::string>::iterator it = map.find(code);
	if (it != map.end())
		return it->second;

	// Vérifie HTTP global
	map = http.get_http_map();
	it = map.find(code);
	if (it != http.get_http_map().end())
		return it->second;

	return ""; // Pas trouvé
}

void	get_request(HTTPConfig &http_config, HttpRequest &req, std::map<std::string, ServerConfig> &server_list, t_fd_data &fd_data, std::string server_name)
{
	int errcode = 0;

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
	std::string location_name;
	std::string root;
	std::map<std::string, LocationConfig>::iterator it_loc;
	bool autoindex = server.get_autoindex();
	try
	{
		location_name = server.get_matching_location(target, autoindex);
		std::map<std::string, LocationConfig> location_list = server.get_location_list();
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

		build_response(req, 404, "Not Found", "text/html", displayErrorPage(find_error_page("404", NULL, server, http_config), http_config, req, server_list, fd_data, server_name, req._is_error_request), false);
		return;
	}

	std::string path_no_index = root + remove_prefix(target, location_name); // Supprimer le préfixe location du target
	std::string file_path = try_index_file(path_no_index, it_loc->second.get_index()); // Si le target finit par '/', on essaie un fichier index

	std::cout << "File path: " << file_path << std::endl;

	//HttpResponse res;

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
			//std::string body = generate_autoindex_html(target, path_no_index);
			build_response(req, 200, "OK", "text/html", body, false);
			return;
		}
		std::cerr << "File not found: " << file_path << std::endl;
		build_response(req, 404, "Not Found", "text/html", displayErrorPage(find_error_page("404", NULL, server, http_config), http_config, req, server_list, fd_data, server_name, req._is_error_request), false);
		return;
	}

	std::ifstream file(file_path.c_str(), std::ios::binary);
	if (!file.is_open())
	{
		std::cerr << "Error opening file: " << file_path << std::endl;
		build_response(req, 404, "Not Found", "text/html", displayErrorPage(find_error_page("404", NULL, server, http_config), http_config, req, server_list, fd_data, server_name, req._is_error_request), false);
		return;
	}
	std::cout << "File found: " << file_path << std::endl;
	std::ostringstream content;
	content << file.rdbuf();
	std::string body = content.str();
	std::string type = get_content_type(file_path);

	build_response(req, 200, "OK", type, body, req.getKeepAlive());
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