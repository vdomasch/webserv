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

void	delete_request(HTTPConfig &http_config, HttpRequest &req, t_fd_data &fd_data)
{
	int errcode = 0;

	std::string target = req.get_target();

	ServerConfig &server = find_current_server(http_config, req._server_name);

	std::string root = req._location_root;
	std::string error_code = validate_request_context(req._location_name, root, errcode, server, "DELETE");
	if (!error_code.empty())
		return (build_response(req, error_code, displayErrorPage(error_code, http_config, req, fd_data), false));

	std::string filename = remove_prefix(target, req._location_name);
	std::string path = root + filename;

	//std::string path = root.substr(0, root.size() - 1) + req.get_target()/*.substr(req.get_target().find("uploads") + 7, req.get_target().size() - 1)*/; // remove the root and the /uploads/ part from the target

	if (!is_authorized_path(path, server.get_authorized_paths()))
	{
		std::cerr << "Error: Path not authorized: " << path << std::endl;
		return (build_response(req, "403", displayErrorPage("403", http_config, req, fd_data), false));
	}

	if (std::remove(path.c_str()) != 0)
	{	
    	if (errno == ENOENT)
		{
    	    std::cerr << "Error: File not found: " << path << std::endl;
			return build_response(req, "404", displayErrorPage("404", http_config, req, fd_data), false);
    	}
		else
		{
    	    std::cerr << "Error deleting file: " << path << std::endl;
    		return build_response(req, "500", displayErrorPage("500", http_config, req, fd_data), false);
		}	
	}

	std::ostringstream response_body;
	response_body << "<html><body><h1>DELETE State ?</h1><p>File deleted is: " << req.get_target() << "</p></body></html>";

	build_response(req, "204", response_body.str(), req.getKeepAlive());
}
