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

std::string	normalize_path(const std::string &path)
{
	std::string normalized = path;
	if (normalized.empty() || normalized == "/")
		return "/";
	if (normalized[0] != '/')
		normalized = "/" + normalized;
	if (normalized.at(normalized.size() - 1) == '/')
		normalized.erase(normalized.size() - 1, 1);
	return normalized;
}

void	build_response(HttpRequest &req, int status_code, const std::string &status_msg, const std::string &content_type, const std::string &body, bool keep_alive_connection)
{
	HttpResponse res;
	res.set_status(status_code, status_msg);
	res.set_body(body);
	res.add_header("Content-Type", content_type);
	if (keep_alive_connection)
		res.add_header("Connection", "keep-alive");
	else
		res.add_header("Connection", "close");
	try { res.add_header("Content-Length", convert<std::string>(body.size())); }
	catch (std::exception &e) { std::cerr << "Error converting size: " << e.what() << std::endl; }
	req.set_response(res.generate_response());
}

std::string	displayErrorPage(const std::string code, const std::string message, const std::string& error_uri, HTTPConfig& http_config, HttpRequest& req, std::map<std::string, ServerConfig>& server_list, t_fd_data& fd_data, const std::string& server_name, bool is_error_request)
{
    if (error_uri.empty() || is_error_request)
	{
		std::cerr << "Error: No error page URI provided or request is an error request" << std::endl;
		return "<html><body><h1>" + code + " " +  message + "</h1></body></html>";
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

std::string	find_error_page(const std::string& code, LocationConfig* loc, ServerConfig& serv, HTTPConfig& http)
{
	// Vérifie Location (si fourni)
	if (loc)
	{
		std::map<std::string, std::string>& map = loc->get_map_location();
		std::map<std::string, std::string>::iterator it = map.find(code);
		if (it != map.end())
		{
			std::string res = it->second;
			return res;
		}
	}


	// Vérifie Server
	std::map<std::string, std::string>& map = serv.get_map_server();
	std::map<std::string, std::string>::iterator it = map.find(code);
	if (it != map.end())
		return it->second;

	// Vérifie HTTP global
	map = http.get_http_map();
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
	if (location.get_map_location().count(method) == 0)
	{
		if (server.get_map_server().count(method) == 0)
		{
			std::cerr << "Error: method not allowed." << std::endl;
			return false;
		}
		return true;
	}
	return true;
}