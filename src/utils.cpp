#include "utils.hpp"

bool	is_error(int errcode)
{
	return (errcode >= 400 && errcode < 600);
}

static std::map<std::string, std::string> make_status_messages()
{
	std::map<std::string, std::string> m;
	m["200"] = "OK";
	m["201"] = "Created";
	m["204"] = "No Content";
	m["301"] = "Moved Permanently";
	m["302"] = "Found";
	m["303"] = "See Other";
	m["304"] = "Not Modified";
	m["400"] = "Bad Request";
	m["403"] = "Forbidden";
	m["404"] = "Not Found";
	m["405"] = "Method Not Allowed";
	m["408"] = "Request Timeout";
	m["413"] = "Content Too Large";
	m["500"] = "Internal Server Error";
	m["505"] = "HTTP Version Not Supported";
	return m;
}

std::string	message_status(const std::string &status)
{
	static const std::map<std::string, std::string> statusMessages = make_status_messages();

	std::map<std::string, std::string>::const_iterator it = statusMessages.find(status);
	if (it != statusMessages.end())
		return it->second;
	else
		return "Unknown Status";
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
	std::string location_name = server.get_matching_location("/" + target, autoindex);
	std::map<std::string, LocationConfig>& location_list = server.get_location_list();
	it_loc = location_list.find(location_name);
	if (it_loc != location_list.end())
		root = it_loc->second.get_root();
	else
		return "";
	return location_name;
}

void	build_response(HttpRequest &req, const std::string &status_code, const std::string &body, bool keep_alive_connection)
{
	HttpResponse res;
	res.set_status(status_code, message_status(status_code));
	res.set_body(body);
	if (!req._is_php_cgi)
		res.add_header("Content-Type", req.get_content_type());

	if (keep_alive_connection)
		res.add_header("Connection", "keep-alive");
	else
		res.add_header("Connection", "close");
	try {
		size_t content_len = body.size();
		if (req._is_php_cgi)
			content_len -= body.find("\r\n\r\n") + 4;
		res.add_header("Content-Length", convert<std::string>(content_len));
	}
	catch (std::exception &e) { std::cerr << "Error: Converting size failed" << std::endl; }
	
	try { req.set_status_code(convert<int>(status_code)); }
	catch (std::exception &e) { std::cerr << "Error: Converting status code" << std::endl; }
	req.set_response(res.generate_response(req._is_php_cgi));
}

std::string build_html_body(std::string code)
{
	std::string body = "<html>";
	body += "	<head>";
	body += "		<meta charset=\"UTF-8\">";
	body += "		<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">";
	body += "		<link rel=\"icon\" type=\"image/x-icon\" href=\"/icons/favicon.ico\">";
	body += "		<link rel=\"stylesheet\" href=\"/assets/css_files/styles.css\">";
	body += "		<title>" + code + " " + message_status(code) + "</title>";
	body += "	</head>";
	body += "	<body><section class=\"block\"><h1>" + code + " " + message_status(code) + "</h1>";
	body += "	<p>Webserv encountered an error while processing your request.</p>";
	body += "	<p>Please check the server configuration or contact the administrator.</p>";
	body += "	</section></body>";
	body += "</html>";
	return body;
}

std::string	displayErrorPage(const std::string& code, HTTPConfig& http_config, HttpRequest& req, t_fd_data& fd_data)
{
	if (req._location_name.empty() || req._server_name.empty())
	{
		std::cerr << "Error: Location or server name is empty" << std::endl;
		return build_html_body(code);
	}
	std::string error_uri = find_error_page(code, req._location_name, req._server_name, http_config);
	if (error_uri.empty() || req._is_error_request)
		return build_html_body(code);
	req.set_target(error_uri);
	req._is_error_request = true;
	req._location_name = find_location_name_and_set_root(error_uri, req._server, req._location_root, req._autoindex);

	get_request(http_config, req, fd_data);

	if (req.get_response().empty())
	{
		std::cerr << "Error: No response from server" << std::endl;
		return build_html_body("500");
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

	std::map<std::string, std::string> map = http.get_http_map();
	it = map.find(code);
	if (it != map.end())
		return it->second;

	return "";
}
