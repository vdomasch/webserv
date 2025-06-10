#include "webserv.hpp"

void	delete_request(HTTPConfig &http_config, HttpRequest &req, t_fd_data &fd_data, std::string server_name)
{
	int errcode = 0;

	std::string target = normalize_path(req.get_target());

	std::map<std::string, ServerConfig> &server_list = http_config.get_server_list();
	std::map<std::string, ServerConfig>::iterator it_serv = server_list.find(server_name);
	if (it_serv == server_list.end())
	{
		server_name = server_name.substr(server_name.find(':') + 1, server_name.size());
		if ((it_serv = server_list.find(server_name)) == server_list.end())
		{
			std::cerr << "Server not found: " << server_name << std::endl;
			build_response(req, "404", displayErrorPage("404", "", http_config, req, fd_data, server_name), false);
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
		build_response(req, "404", displayErrorPage("404", location_name, http_config, req, fd_data, server_name), false);
		return;
	}
	if (root == "")
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

	if (check_allowed_methods(server, it_loc->second, "DELETE") == false)
	{
		std::cerr << "Error: Method DELETE not allowed for location: " << location_name << std::endl;
		build_response(req, "405", displayErrorPage("405", location_name, http_config, req, fd_data, server_name), false);
		return;
	}



	std::cout << req.get_target() << std::endl;

	std::string path = root.substr(0, root.size() - 1) + req.get_target().substr(req.get_target().find("uploads") + 7, req.get_target().size() - 1); // remove the root and the /uploads/ part from the target
	std::string authorized_paths("authorized_paths.txt");


	std::ifstream valid_paths((server.get_root() + authorized_paths).c_str(), std::ios::binary); // Open our list of paths
	if (!valid_paths.is_open())
	{
		std::cerr << "Error: Could not open authorized paths file: " << authorized_paths << std::endl;
		build_response(req, "500", displayErrorPage("500", location_name, http_config, req, fd_data, server_name), false);
	}


	std::cout << "Checking if path is authorized: " << path << std::endl;
	std::string line;
	while (std::getline(valid_paths, line)) // check if file requested in authorized paths
	{
		if (line.empty())
			continue;
		std::cout << "Checking line: " << line << std::endl;
		if (path.find(line) != std::string::npos)
			break;
	}
	if (valid_paths.eof())
	{
		std::cerr << "Forbidden Request" << std::endl;
		valid_paths.close();
		std::cerr << "Error: Path not authorized: " << path << std::endl;
		build_response(req, "403", displayErrorPage("403", location_name, http_config, req, fd_data, server_name), false);
		return ;
	}
	valid_paths.close();






	std::ifstream test(path.c_str()); // check if file exist
	if (!test.is_open())
    {
		std::cerr << "Error: File not found: " << path << std::endl;
		build_response(req, "404", displayErrorPage("404", location_name, http_config, req, fd_data, server_name), false);
		return;
    }
	test.close();




	std::remove(path.c_str());




	std::ifstream test2(path.c_str()); // check if file correctly deleted
	if (test2.is_open())
    {
		test.close();
		std::cerr << "Error: File not deleted: " << path << std::endl;
		build_response(req, "500", displayErrorPage("500", location_name, http_config, req, fd_data, server_name), false);
		return;
    }
	




	std::ostringstream response_body;
	response_body << "<html><body><h1>DELETE State ?</h1><p>File deleted is: " << req.get_target() << "</p></body></html>";

	build_response(req, "204", response_body.str(), req.getKeepAlive());

}
