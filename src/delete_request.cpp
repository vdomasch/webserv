#include "webserv.hpp"

void	delete_request(HTTPConfig &http_config, HttpRequest &req, t_fd_data &fd_data, std::string server_name)
{
	int errcode = 0;

	std::string target = normalize_path(req.get_target());

	ServerConfig &server = find_current_server(http_config, server_name);
	bool autoindex = server.get_autoindex();

	std::string location_name, root;
	try { location_name = find_location_name_and_set_root(target, server, root, autoindex); }
	catch (std::exception &e)
	{
		std::cerr << "Error finding matching location: " << e.what() << std::endl;
		return (build_response(req, "404", displayErrorPage("404", location_name, http_config, req, fd_data, server_name), false));
	}
	std::string error_code = validate_request_context(location_name, root, errcode, server, "DELETE");
	if (!error_code.empty())
		return (build_response(req, error_code, displayErrorPage(error_code, location_name, http_config, req, fd_data, server_name), false));



	std::cout << req.get_target() << std::endl;
	std::cout << target << std::endl;

	std::string path = root.substr(0, root.size() - 1) + req.get_target().substr(req.get_target().find("uploads") + 7, req.get_target().size() - 1); // remove the root and the /uploads/ part from the target


	std::string authorized_paths("authorized_paths.txt");


	std::ifstream valid_paths((server.get_root() + authorized_paths).c_str(), std::ios::binary); // Open our list of paths
	if (!valid_paths.is_open())
	{
		std::cerr << "Error: Could not open authorized paths file: " << authorized_paths << std::endl;
		return (build_response(req, "500", displayErrorPage("500", location_name, http_config, req, fd_data, server_name), false));
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
		valid_paths.close();
		std::cerr << "Error: Path not authorized: " << path << std::endl;
		return (build_response(req, "403", displayErrorPage("403", location_name, http_config, req, fd_data, server_name), false));
	}
	valid_paths.close();






	std::ifstream test(path.c_str()); // check if file exist
	if (!test.is_open())
    {
		std::cerr << "Error: File not found: " << path << std::endl;
		return (build_response(req, "404", displayErrorPage("404", location_name, http_config, req, fd_data, server_name), false));
    }
	test.close();




	std::remove(path.c_str());




	std::ifstream test2(path.c_str()); // check if file correctly deleted
	if (test2.is_open())
    {
		test.close();
		std::cerr << "Error: File not deleted: " << path << std::endl;
		return (build_response(req, "500", displayErrorPage("500", location_name, http_config, req, fd_data, server_name), false));
    }
	




	std::ostringstream response_body;
	response_body << "<html><body><h1>DELETE State ?</h1><p>File deleted is: " << req.get_target() << "</p></body></html>";

	build_response(req, "204", response_body.str(), req.getKeepAlive());

}
