#include "webserv.hpp"

std::string	handleCGI(HttpRequest& req, t_fd_data &d, int *errcode)
{
	std::string	CGIBody;
	std::string	method;
	int			CGI_body_size;

	printf("\033[35mHandeling CGI ....\n\n\033[0m");

	method = req.get_method();
	// reset CGI class
	d.cg = CGIContent();	
	if (method == "POST")
		d.cg.setEnvCGI((req.get_rootpath() +  req.get_target()), d.Content_Type, d.Content_Length, method);
	else if (method == "GET")
		d.cg.setEnvCGI((req.get_rootpath() +  req.get_target()), d.QueryString, "none", method);
	d.cg.executeCGI();
	d.cg.sendCGIBody(req.get_body());
	CGIBody = d.cg.grabCGIBody(CGI_body_size); // errcode si fail read ?
	//PRINT_DEBUG2
	//std::cout << "There: " << CGIBody << std::endl;

	//test, avoid zombie i guess ?
	int status = 0;
	waitpid(d.cg.cgi_forkfd, &status, 0);
	int exit_code = WEXITSTATUS(status);
	if (exit_code != 0)
	{
		printf("Ptit flop: child exited with code %d\n", exit_code);
		printf("Raw status: 0x%04x\n", status);
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

std::string remove_prefix(std::string target, std::string prefix)
{
	if (prefix.empty())
		return target;

	if (!prefix.empty() && prefix.at(prefix.size() - 1) == '/')
		prefix.erase(prefix.size() - 1, 1);

	if (target.find(prefix) == 0 && (target.size() == prefix.size() || target.at(prefix.size()) == '/'))
		target.erase(0, prefix.size());

	if (target.at(0) == '/')
		target.erase(0, 1);

	return target;
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

	if (check_object_type(root, &errcode) != IS_DIRECTORY)
	{
		std::cerr << "Error: Root directory does not exist or is not a directory: " << root << std::endl;
		return "500";
	}

	if (check_allowed_methods(server, server.get_location_list().find(location_name)->second, method) == false)
	{
		std::cerr << "Error: Method " << method << " not allowed for location: " << location_name << std::endl;
		return "405";
	}
	return "";
}

