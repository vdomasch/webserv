#include "webserv.hpp"

std::string	handleCGI(HttpRequest& req, t_fd_data &d, int *errcode)
{
	std::string	CGIBody;
	std::string	method;
	int			CGI_body_size;

	method = req.get_method();

	d.cg = CGIContent();	
	if (method == "POST")
		d.cg.setEnvCGI((req.get_rootpath() +  req.get_target()), d.Content_Type, d.Content_Length, method, req._is_php_cgi);
	else if (method == "GET")
		d.cg.setEnvCGI((req.get_rootpath() +  req.get_target()), d.QueryString, "none", method, req._is_php_cgi);
	d.cg.executeCGI();
	d.cg.sendCGIBody(req.get_body());
	CGIBody = d.cg.grabCGIBody(CGI_body_size);
	if (d.cg.get_exitcode())
	{
		*errcode = 500;
		return ("");
	}
	

	int status = 0;
	waitpid(d.cg.cgi_forkfd, &status, 0);
	int exit_code = WEXITSTATUS(status);
	if (exit_code != 0)
	{
		std::cerr << "Error: Ptit flop: child exited with code " << exit_code << std::endl;
		*errcode = 400;
		return ("");
	}
	d.response_len = CGI_body_size;
	*errcode = 0;
	return CGIBody;
}

int	check_object_type(const std::string& path, int *errcode)
{
	struct stat fileinfo;  

    if (stat (path.c_str(), &fileinfo) != 0)
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

std::string remove_prefix(std::string target, std::string prefix)
{
	if (prefix.empty())
		return target;

	if (!prefix.empty() && prefix.at(prefix.size() - 1) == '/')
		prefix.erase(prefix.size() - 1, 1);

	if (target.find(prefix) == 0 && (target.size() == prefix.size() || target.at(prefix.size()) == '/'))
		target.erase(0, prefix.size());

	if (target.empty())
		return "";
		
	if (target.at(0) == '/')
		target.erase(0, 1);
	return target;
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

	int status = check_object_type(root, &errcode);
	if (method != "DELETE" && status != IS_DIRECTORY)
	{
		std::cerr << "Error: Root directory does not exist or is not a directory: " << root << std::endl;
		return "500";
	}
	if (method == "DELETE")
	{
		if (status == IS_DIRECTORY)
			return "";
		else if (status == MISSINGFILE)
		{
			std::cerr << "Error: File not found for deletion: " << root << std::endl;
			return "404";
		}
		else if (status == FILE_NOT_FOUND)
		{
			std::cerr << "Error: File not found or is not a regular file: " << root << std::endl;
			return "404";
		}
		else if (status != IS_EXISTINGFILE)
		{
			std::cerr << "Error: Invalid object type for DELETE method: " << status << std::endl;
			return "500";
		}
	}
	if (check_allowed_methods(server, server.get_location_list().find(location_name)->second, method) == false)
	{
		std::cerr << "Error: Method " << method << " not allowed for location: " << location_name << std::endl;
		return "405";
	}
	return "";
}

