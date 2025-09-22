#include "utils.hpp"

bool	is_error(int errcode)
{
	return (errcode >= 400 && errcode < 600);
}

std::map<std::string, std::string> make_status_messages()
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
	m["502"] = "Bad Gateway";
	m["504"] = "Gateway Timeout";
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

ServerConfig&	find_current_server(HTTPConfig& http_config, HttpRequest& req)
{
	//std::map<std::string, std::vector<ServerConfig> >& server_list = http_config.get_server_list();
	//std::vector<ServerConfig>& servers = server_list[req._port];


	//ServerConfig best_match;
	//size_t number_of_matches = 0;
	//bool ip_match = false;
	//for (std::vector<ServerConfig>::iterator it = servers.begin(); it != servers.end(); ++it)
	//{
	//	if (it->get_server_name() == req._server_name)
	//	{
	//		if (number_of_matches != 0)
	//		{
	//			if (ip_match == true && it->get_host_ip() == req._ip)
	//				throw std::runtime_error("Multiple servers match the request with same server name and IP: " + req._server_name + " at " + req._ip);
	//			else if (it->get_host_ip() == req._ip)
	//			{
	//				best_match = *it;
	//				ip_match = true;
	//			}
	//		}
	//		else
	//			best_match = *it;
	//		number_of_matches++;
	//	}
	//}
	//return best_match;

	 // Find the list of all servers configured for the request's port
    std::map<std::string, std::vector<ServerConfig> >& port_servers = http_config.get_server_list();
    std::vector<ServerConfig>& servers = port_servers[req._port]; // Convert port to string for map key

    // Priority 1: Search for an exact match based on server_name and IP
    for (std::vector<ServerConfig>::iterator it = servers.begin(); it != servers.end(); ++it)
    {
        std::string config_ip = it->get_host_ip();

        if (it->get_server_name() == req.get_header("Host") && config_ip == req._ip)
        {
            return *it; // Return reference to the object in the vector
        }
    }
    
    // Priority 2: Wildcard IP match (0.0.0.0)
    for (std::vector<ServerConfig>::iterator it = servers.begin(); it != servers.end(); ++it)
    {
        if (it->get_server_name() == req.get_header("Host") && it->get_host_ip() == "0.0.0.0")
        {
            return *it; // Return reference to the object in the vector
        }
    }

    // Priority 3: Fallback to the default server (the first one)
    if (!servers.empty())
    {
        return servers.front(); // Return reference to the first object in the vector
    }

    // No server found for this port, throw an exception
    throw std::runtime_error("No server configured for the requested port.");
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
	// if (status_code == "")

	if (!req._is_php_cgi)
		res.add_header("Content-Type", req.get_content_type());

	if (keep_alive_connection)
		res.add_header("Connection", "keep-alive");
	else
		res.add_header("Connection", "close");
	if (req.get_is_redirection())
		res.add_header("Location", req.get_redirection());
	else
	{
		try {
			size_t content_len = body.size();
			if (req._is_php_cgi)
				content_len -= body.find("\r\n\r\n") + 4;
			res.add_header("Content-Length", convert<std::string>(content_len));
		}
		catch (std::exception &e) { std::cerr << "Error: Converting size failed" << std::endl; }
	}

	try { req.set_status_code(convert<int>(status_code)); }
	catch (std::exception &e) { std::cerr << "Error: Converting status code" << std::endl; }
	req.set_response(res.generate_response(req._is_php_cgi));
	req._response_sent = 0;
	req.set_state(RESPONDING);
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
	std::string error_uri = find_error_page(code, req._location_name, req._server, http_config);
	if (error_uri.empty() || req._is_error_request)
		return build_html_body(code);
	req.set_target(error_uri);
	req._is_error_request = true;
	try { req._location_name = find_location_name_and_set_root(error_uri, req._server, req._location_root, req._autoindex); }
	catch (std::exception &e)
	{
		std::cerr << "Error: Finding location for error page: " << e.what() << std::endl;
		return build_html_body(code);
	}
	
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

std::string	find_error_page(const std::string& code, const std::string& location_name, /*const std::string& server_name,*/ ServerConfig& server, HTTPConfig& http)
{
	std::map<std::string, std::string>::iterator it;

	std::map<std::string, std::vector<ServerConfig> > server_list = http.get_server_list();
	//std::map<std::string, ServerConfig>::iterator it_serv = server_list.find(server_name);

	if (!location_name.empty())
	{
		//std::map<std::string, LocationConfig>::iterator it_loc = it_serv->second.get_location_list().find(location_name);
		std::map<std::string, LocationConfig>::iterator it_loc = server.get_location_list().find(location_name);
		if (it_loc != server.get_location_list().end())
		{
			std::map<std::string, std::string>& map = it_loc->second.get_map_location();
			it = map.find(code);
			if (it != map.end())
				return it->second;
		}
	}

	//if (it_serv == server_list.end())
	//{
	//	std::string server_default_name = server_name.substr(server_name.find(':') + 1);
	//	it_serv = server_list.find(server_default_name);
	//	if (it_serv != server_list.end())
	//	{
	//		std::map<std::string, std::string>& map = it_serv->second.get_map_server();
	//		it = map.find(code);
	//		if (it != map.end())
	//			return it->second;
	//	}
	//}
	//else
	//{
	//	std::map<std::string, std::string>& map = it_serv->second.get_map_server();
	//	std::map<std::string, std::string>::iterator it = map.find(code);
	//	if (it != map.end())
	//		return it->second;
	//}

	std::map<std::string, std::string>& map_server = server.get_map_server();
	it = map_server.find(code);
	if (it != map_server.end())
		return it->second;


	std::map<std::string, std::string> map = http.get_http_map();
	it = map.find(code);
	if (it != map.end())
		return it->second;
	return "";
}