#include "webserv.hpp"

std::string	handleCGI(t_fd_data &d, int *errcode)
{
	std::string	CGIBody;
	int			CGI_body_size;

	printf("beep beep boop ... i'm CGI ... \n\n");
	d.cg.setEnvCGI(d.requestedFilePath, d.Content_Type, d.Content_Length, d.method_name);
	d.cg.executeCGI();
	d.cg.sendCGIBody(&d.binaryContent);
	CGIBody = d.cg.grabCGIBody(CGI_body_size); // errcode si fail read ?

	//test, avoid zombie i guess ?
	int status = 0;
	waitpid(d.cg.cgi_forkfd, &status, 0);
	if(WEXITSTATUS(status) != 0)
	{
		printf("Ptit flop\n\n");
		return ("emptyerror");
	}

	d.response_len = CGI_body_size;
	*errcode = 0;
	return CGIBody;
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
	if (prefix.empty())
		return target;
	if (target.at(target.size() - 1) != '/')
	{
		if (target.find(prefix.substr(0, prefix.size() - 1)) == 0)
			target.erase(0, prefix.length());
		return target;
	}
	if (target.find(prefix) == 0)
		target.erase(0, prefix.length());
	return target;
}

std::string	normalize_path(const std::string &path)
{
	std::string normalized = path;
	std::cout << "Normalizing path: " << normalized << std::endl;
	if (normalized.empty() || normalized == "/")
		return "/";
	if (normalized[0] != '/')
		normalized = "/" + normalized;
	if (normalized.at(normalized.size() - 1) == '/')
		normalized.erase(normalized.size() - 1, 1);
	return normalized;
}

void	build_response(HttpRequest &req, const std::string &status_code, const std::string &body, bool keep_alive_connection)
{
	HttpResponse res;
	res.set_status(status_code, message_status(status_code));
	res.set_body(body);
	res.add_header("Content-Type", req.get_content_type());
	if (keep_alive_connection)
		res.add_header("Connection", "keep-alive");
	else
		res.add_header("Connection", "close");
	try { res.add_header("Content-Length", convert<std::string>(body.size())); }
	catch (std::exception &e) { std::cerr << "Error converting size: " << e.what() << std::endl; }
	req.set_response(res.generate_response());
	req.is_finished();
}

std::string	displayErrorPage(const std::string& code, const std::string& location_name, HTTPConfig& http_config, HttpRequest& req, t_fd_data& fd_data, const std::string& server_name)
{
	std::string error_uri = find_error_page(code, location_name, server_name, http_config);
    if (error_uri.empty() || req._is_error_request)
	{
		std::cerr << "Error: Error Page Not Found" << std::endl;
		return "<html><body><h1>" + code + " " +  message_status(code) + "</h1></body></html>";
	}

	req.set_target(error_uri);
	req._is_error_request = true;

	get_request(http_config, req, fd_data, server_name);

	if (req.get_response().empty())
	{
		std::cerr << "Error: No response from server" << std::endl;
		return "<html><body><h1>500 Internal Server Error</h1></body></html>";
	}
	else if (req.get_response().find("\r\n\r\n") != std::string::npos)
		return req.get_response().substr(req.get_response().find("\r\n\r\n") + 4);
	return req.get_response();
}

std::string	find_error_page(const std::string& code, const std::string& location_name, const std::string& server_name, HTTPConfig& http)
{
	std::map<std::string, std::string>::iterator it;

	std::map<std::string, ServerConfig>& server_list = http.get_server_list();
	std::map<std::string, ServerConfig>::iterator it_serv = server_list.find(server_name);
	// Vérifie Location (si fourni)
	if (!location_name.empty())
	{
		std::map<std::string, LocationConfig>::iterator it_loc = it_serv->second.get_location_list().find(location_name);
		if (it_loc != it_serv->second.get_location_list().end())
		{
			std::map<std::string, std::string>& map = it_loc->second.get_map_location();
			it = map.find(code);
			if (it != map.end())
				return it->second;
		}
	}

	// Vérifie Server
	if (it_serv == server_list.end())
	{
		std::string server_default_name = server_name.substr(server_name.find(':') + 1);
		it_serv = server_list.find(server_default_name);
		if (it_serv != server_list.end())
		{
			std::map<std::string, std::string>& map = it_serv->second.get_map_server();
			it = map.find(code);
			if (it != map.end())
				return it->second;
		}
	}
	else
	{
		std::map<std::string, std::string>& map = it_serv->second.get_map_server();
		std::map<std::string, std::string>::iterator it = map.find(code);
		if (it != map.end())
			return it->second;
	}

	// Vérifie HTTP global
	std::map<std::string, std::string> map = http.get_http_map();
	it = map.find(code);
	if (it != map.end())
		return it->second;

	return ""; // Pas trouvé
}

std::string	create_header(const std::string &status, const std::string &content_type, const std::string &content_length, const std::string &connection)
{
	std::string header = "HTTP/1.1 " + status + "\r\n";
	header += "Content-Type: " + content_type + "\r\n";
	header += "Content-Length: " + content_length + "\r\n";
	header += "Connection: " + connection + "\r\n\r\n";
	return header;
}

bool	check_allowed_methods(ServerConfig &server, LocationConfig &location, const std::string &method)
{
	std::map<std::string, std::string>& location_map = location.get_map_location();
	if (location_map.count(method))
		return true;
	else if (!location_map.count("allow_methods"))
	{
		std::map<std::string, std::string>& server_map = server.get_map_server();
		if (server_map.count(method))
			return true;
	}
	return false;	
}

ServerConfig&	find_current_server(HTTPConfig& http_config, std::string &server_name)
{
	std::map<std::string, ServerConfig>& server_list = http_config.get_server_list();
	std::map<std::string, ServerConfig>::iterator it = server_list.find(server_name);
	if (it == server_list.end())
	{
		server_name = server_name.substr(server_name.find(':') + 1);
		it = server_list.find(server_name);
		if (it == server_list.end())
			throw std::runtime_error("Server not found: " + server_name);
	}
	return it->second;
}

std::string find_location_name_and_set_root(const std::string &target, ServerConfig &server, std::string &root, bool& autoindex)
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

std::string	validate_request_context(std::string &location_name, std::string &root, int &errcode, ServerConfig &server, const std::string &method)
{
	if (location_name.empty())
	{
		std::cerr << "Error: No matching location found for target." << std::endl;
		return "500";
	}
	if (root.empty())
	{
		std::cerr << "Error: Root directory not set for location: " << location_name << std::endl;
		return "500";
	}

	if (check_object_type(root, &errcode) != IS_DIRECTORY)
	{
		std::cerr << "Error: Root directory does not exist or is not a directory: " << root << std::endl;
		return "500";
	}

	if (check_allowed_methods(server, server.get_location_list().find(location_name)->second, method) == false)
	{
		std::cerr << "Error: Method GET not allowed for location: " << location_name << std::endl;
		return "405";
	}
	return "";
}

std::string	message_status(const std::string &status)
{
	if (status == "200")
		return "OK";
	else if (status == "201")
		return "Created";
	else if (status == "204")
		return "No Content";
	else if (status == "400")
		return "Bad Request";
	else if (status == "403")
		return "Forbidden";
	else if (status == "404")
		return "Not Found";
	else if (status == "405")
		return "Method Not Allowed";
	else if (status == "500")
		return "Internal Server Error";
	else
		return "Unknown Status";
}