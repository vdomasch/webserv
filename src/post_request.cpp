#include "webserv.hpp"
#include "HttpResponse.hpp"

std::string	get_timestamp()
{
	time_t now = time(0);
	struct tm tstruct;
	char buf[80];
	tstruct = *localtime(&now);
	strftime(buf, sizeof(buf), "%Y%m%d%H%M%S", &tstruct);
	return std::string(buf);
}

std::string	get_content_extension(const std::string& content_type)
{
	if (content_type == "text/html") return ".html";
	if (content_type == "text/plain") return ".txt";
	if (content_type == "text/css") return ".css";
	//if (content_type == "text/xml") return ".xml";
	//if (content_type == "application/javascript" || content_type == "application/x-javascript") return ".js";
	//if (content_type == "application/xml") return ".xml";
	//if (content_type == "application/x-www-form-urlencoded") return ".url"; 
	if (content_type == "image/gif") return ".gif";
	if (content_type == "image/png") return ".png";
	//if (content_type == "image/webp") return ".webp";
	if (content_type == "image/jpeg" || content_type == "image/jpg") return ".jpg";
	if (content_type == "image/x-icon") return ".ico";
	if (content_type == "video/mp4") return ".mp4";
	if (content_type == "audio/mpeg") return ".mp3";
	//if (content_type == "application/x-shockwave-flash") return ".swf";
	//if (content_type == "application/x-tar") return ".tar";
	//if (content_type == "application/x-7z-compressed") return ".7z";
	//if (content_type == "application/x-rar-compressed") return ".rar";
	//if (content_type == "application/pdf") return ".pdf";
	//if ((content_type == "application/octet-stream" && content_type.find("zip") != std::string::npos) || content_type == "application/x-zip-compressed" || content_type == "application/zip") return ".zip";
	//if (content_type == "application/octet-stream") return ".bin";
	return "";
}

std::string	get_extension(const std::string& head)
{
	size_t pos = head.find("Content-Type: ");
	if (pos == std::string::npos)
		return "";
	pos += 14;
	size_t end_pos = head.find("\r\n");
	if (end_pos == std::string::npos)
		return "";
	std::string content_type = head.substr(pos, end_pos - pos);
	return get_content_extension(content_type);
}

std::string	get_filename(const std::string& head)
{
	size_t pos = head.find("filename=\"");
	if (pos == std::string::npos)
		return "upload_"; // Default filename if not found
	pos += 10;

	size_t end_pos = head.find("\"", pos);
	if (end_pos == std::string::npos)
		return "upload_"; // Default filename if not found
	return head.substr(pos, end_pos - pos);
}

std::string create_filename(std::string& root, const std::string& head)
{
	int		errcode = 0;
	size_t	cpt = 0;
	std::string filename = get_filename(head);
	std::string extension = get_extension(head);

	if (filename.find(extension) == std::string::npos || extension.empty())
		return "";
	else
	{
		std::string file = root + filename;

		std::string filename_tmp = filename;
		while (check_object_type(root + filename_tmp, &errcode) == IS_EXISTINGFILE)
		{
			cpt += 1;
			std::string cpt_str;
			try { cpt_str = convert<std::string>(cpt); }
			catch (...) { return ("");};
			filename_tmp = filename.substr(0, filename.find(extension)) + "_" + cpt_str + extension;
		}
		filename = filename_tmp;
	}
	std::cout << "Upload filename: " << filename << std::endl;
	return filename;
}

void	parse_post_body(HttpRequest &req, std::string& head, std::string& body)
{
	body = req.get_body();
	if (body.empty())
	{
		std::cerr << "Error: POST body is empty." << std::endl;
		req.set_status_code(400);
		return;
	}
	if (req.get_is_multipart())
	{
		std::string delimiter = req.get_boundary();
		if (delimiter.empty())
		{
			std::cerr << "Error: No boundary found in multipart POST request." << std::endl;
			req.set_status_code(400);
			return;
		}
		if (body.find(delimiter) != std::string::npos)
			body = body.substr(body.find(delimiter) + delimiter.size(), body.rfind(delimiter + "--") - delimiter.size() - 2);

		size_t pos = body.find("\r\n\r\n");
		head = body.substr(0, pos);
		body = body.substr(pos + 4);
	}
}

bool	create_directories(ServerConfig& server, std::string path) 
{
	std::istringstream iss(path);
	std::string token;
	std::string current = "/";

	while (getline(iss, token, '/'))
	{
		if (token.empty())
			continue;

		if (!current.empty() && current.at(current.size() - 1) != '/')
			current += '/';
		current += token;

		if (mkdir(current.c_str(), 0755) != 0)
		{
			if (errno == EEXIST)
				continue;
			else
			{
				std::cerr << "Error: Failed to create directory: " << current << std::endl;
				return false;
            }
		}
		server.add_authorized_paths(current);
	}
	return true;
}

bool	extension_IsAllowed(ServerConfig &server, std::string target, int *errcode)
{
	*errcode = 0;
	std::map<std::string, LocationConfig>::iterator it_loc = server.get_location_list().find("/cgi-bin/");
	if (it_loc != server.get_location_list().end())
	{
		std::map<std::string, std::string> &map = it_loc->second.get_map_location();
		if (map.find("cgi_ext") != map.end())
		{
			std::string allowed_extensions = map.find("cgi_ext")->second;
			// std::cout << "Allowed extensions : " << allowed_extensions << "\n";

			std::size_t	pos = target.rfind(".");
			if (pos==std::string::npos)
			{
				*errcode = 403;
				return (false);
			}
			std::string	target_ext = target.substr(pos);
			if (allowed_extensions.find(target_ext) != std::string::npos)
				return (true);
			else
			{
				*errcode = 403;
				return (false);
			}
		}
		else // Index "cgi_ext" was NOT found because it wasn't created, which means the "cgi_ext" line is not specified in config file
		{
			std::cerr << "Error: \"cgi_ext\" line is missing from cgi_bin location, all extensions are forbidden by default.\n";
			*errcode = 403;
			return (false);
		}
	}

	if ((target.find(".py") != std::string::npos || target.find(".php") != std::string::npos))
		return (true);
	else
		return (false);
}

void	post_request(HTTPConfig &http_config, HttpRequest &req, t_fd_data &fd_data)
{
	int errcode = 0;
	std::string	target = req.get_target();
	std::string	root = req._location_root;

	//ServerConfig &server = find_current_server(http_config, req._server_name);

	std::string context_status_errcode = validate_request_context(req._location_name, root, errcode, req._server, "POST");
	if (!context_status_errcode.empty())
	{
		std::cerr << "Error validating request context: " << context_status_errcode << std::endl;
		return (build_response(req, context_status_errcode, displayErrorPage(context_status_errcode, http_config, req, fd_data), req.getKeepAlive()));
	}
	if (req._location_name == "/cgi-bin/" && extension_IsAllowed(req._server, target, &errcode)) //!
	{
		std::string body;

		fd_data.Content_Type = req.get_header("Content-Type");
		fd_data.Content_Length = req.get_header("Content-Length");

		body = handleCGI(req, fd_data, &errcode);
		if (body.empty())
		{
			if (errcode == 500)
			{
				std::cerr << "Error: System call failed" << target << std::endl;
				return (build_response(req, "500", displayErrorPage("500", http_config, req, fd_data), req.getKeepAlive()));
			}
		}
		return (build_response(req, "200", body, req.getKeepAlive()));
	}
	else if (errcode == 403)
	{
		std::cerr << "Error: CGI Extension used is forbidden." << std::endl;
		return (build_response(req, "403", displayErrorPage("403", http_config, req, fd_data), req.getKeepAlive()));
	}
	else if (req._location_name == "/cgi-bin/")
		req._autoindex = false;
	else if (target.find("cgi-bin/") != std::string::npos)
	{
		std::cerr << "Error: No CGI location" << std::endl;
		return (build_response(req, "403", displayErrorPage("403", http_config, req, fd_data), req.getKeepAlive()));
	}

	std::string head, body;
	parse_post_body(req, head, body);
	
	std::string filename = create_filename(root, head);
	if (filename.empty())
	{
		PRINT_DEBUG2
		return (build_response(req, "400", displayErrorPage("400", http_config, req, fd_data), req.getKeepAlive()));
	}

	std::string file_path = root + filename; // Full path of file

	if (create_directories(req._server, file_path.substr(0, file_path.rfind('/'))) == false)
	{
		std::cerr << "Error: Failed to create directories for POST data." << std::endl;
		return (build_response(req, "500", "Failed to create directories for POST data", req.getKeepAlive()));
	}
	std::ofstream out(file_path.c_str(), std::ios::binary);
	if (!out.is_open())
	{
		std::cerr << "Error: Failed to open file for writing: " << file_path << std::endl;
		return (build_response(req, "500", "Failed to store POST data", req.getKeepAlive()));
	}
	out << body;
	out.close();

	std::ostringstream response_body;
	response_body << req.get_target().substr(0, req.get_target().rfind('/', req.get_target().size() - 2) + 1) + filename;

	build_response(req, "201", response_body.str(), req.getKeepAlive());
}