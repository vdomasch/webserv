#include "webserv.hpp"


bool	is_authorized_path(const std::string &path, const std::set<std::string> &authorized_paths)
{
	int status = 0;
	status = check_object_type(path, &status);
	if (status == FILE_NOT_FOUND)
	{
		std::cerr << "Error: Path not found: " << path << std::endl;
		return false;
	}
	if (status == IS_DIRECTORY)
	{
		std::cerr << "Error: Path is a directory, not a file: " << path << std::endl;
		return false;
	}
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
		return (build_response(req, error_code, displayErrorPage(error_code, http_config, req, fd_data), req.getKeepAlive()));
	std::string path = root + target;

	int status = check_object_type(path, &errcode);
	if (status == FILE_NOT_FOUND)
	{
		std::cerr << "Error: File not found: " << path << std::endl;
		return (build_response(req, "404", displayErrorPage("404", http_config, req, fd_data), req.getKeepAlive()));
	}
	else if (status == IS_DIRECTORY)
	{
		std::cerr << "Error: Forbidden request: " << path << std::endl;
		return (build_response(req, "403", displayErrorPage("403", http_config, req, fd_data), req.getKeepAlive()));
	}

	if (!is_authorized_path(path, server.get_authorized_paths()))
	{
		if (file_exists(path))
		{
			std::cerr << "Error: Unauthorized DELETE request for path: " << path << std::endl;
			return (build_response(req, "403", displayErrorPage("403", http_config, req, fd_data), req.getKeepAlive()));
		}
		else
		{
			std::cerr << "Error: File not found" << std::endl;
			return (build_response(req, "404", displayErrorPage("404", http_config, req, fd_data), req.getKeepAlive()));
		}
	}

	if (std::remove(path.c_str()) != 0)
	{	
    	if (errno == ENOENT)
		{
    	    std::cerr << "Error: File in unauthorized location not found" << path << std::endl;
			return build_response(req, "404", displayErrorPage("4", http_config, req, fd_data), req.getKeepAlive());
    	}
		else
		{
    	    std::cerr << "Error deleting file: " << path << std::endl;
    		return build_response(req, "500", displayErrorPage("500", http_config, req, fd_data), req.getKeepAlive());
		}	
	}

	std::ostringstream response_body;
	response_body << "<html><body><h1>DELETE State ?</h1><p>File deleted is: " << req.get_target() << "</p></body></html>";

	build_response(req, "204", response_body.str(), req.getKeepAlive());
}
