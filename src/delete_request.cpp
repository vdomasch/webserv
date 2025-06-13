#include "webserv.hpp"


bool	is_authorized_path(const std::string &path, const std::set<std::string> &authorized_paths)
{
	for (std::set<std::string>::const_iterator it = authorized_paths.begin(); it != authorized_paths.end(); ++it)
	{
		if (path.find(*it) != std::string::npos)
			return true;
	}
	return false;
}

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


	std::string path = root.substr(0, root.size() - 1) + req.get_target()/*.substr(req.get_target().find("uploads") + 7, req.get_target().size() - 1)*/; // remove the root and the /uploads/ part from the target

	if (!is_authorized_path(path, server.get_authorized_paths()))
	{
		std::cerr << "Error: Path not authorized: " << path << std::endl;
		return (build_response(req, "403", displayErrorPage("403", location_name, http_config, req, fd_data, server_name), false));
	}

	if (std::remove(path.c_str()) != 0)
	{	
    	if (errno == ENOENT)
		{
    	    std::cerr << "Error: File not found: " << path << std::endl;
			return build_response(req, "404", displayErrorPage("404", location_name, http_config, req, fd_data, server_name), false);
    	}
		else
		{
    	    std::cerr << "Error deleting file: " << path << std::endl;
    		return build_response(req, "500", displayErrorPage("500", location_name, http_config, req, fd_data, server_name), false);
		}	
	}

	std::ostringstream response_body;
	response_body << "<html><body><h1>DELETE State ?</h1><p>File deleted is: " << req.get_target() << "</p></body></html>";

	build_response(req, "204", response_body.str(), req.getKeepAlive());
}
