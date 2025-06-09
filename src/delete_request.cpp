#include "webserv.hpp"

void	delete_request(HTTPConfig &http_config, HttpRequest &req, std::map<std::string, ServerConfig> &server_list, t_fd_data &fd_data, std::string server_name)
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

	std::cout << "WHAT we have:" << req.get_target() + req.get_body() << std::endl; 

	std::ostringstream response_body;
	response_body << "<html><body><h1>DELETE State ?</h1><p>File deleted is: " << req.get_target() << "</p></body></html>";

	build_response(req, 201, "Created", "text/html", response_body.str(), req.getKeepAlive());
}
