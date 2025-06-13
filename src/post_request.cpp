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
	if (content_type == "text/xml") return ".xml"; // For compatibility with old servers
	if (content_type == "application/javascript" || content_type == "application/x-javascript") return ".js";
	if (content_type == "application/xml") return ".xml";
	if (content_type == "application/x-www-form-urlencoded") return ".url"; 
	if (content_type == "image/gif") return ".gif";
	if (content_type == "image/png") return ".png";
	if (content_type == "image/webp") return ".webp";
	if (content_type == "image/jpeg" || content_type == "image/jpg") return ".jpg";
	if (content_type == "image/x-icon") return ".ico";
	if (content_type == "application/x-shockwave-flash") return ".swf";
	if (content_type == "application/x-tar") return ".tar";
	if (content_type == "application/x-7z-compressed") return ".7z";
	if (content_type == "application/x-rar-compressed") return ".rar";
	if (content_type == "application/pdf") return ".pdf";
	if ((content_type == "application/octet-stream" && content_type.find("zip") != std::string::npos) || content_type == "application/x-zip-compressed" || content_type == "application/zip") return ".zip";
	if (content_type == "application/octet-stream") return ".bin"; // Default for binary files
	return ".bin";
}

std::string	get_extension(const std::string& head)
{
	size_t pos = head.find("Content-Type: ");
	if (pos == std::string::npos)
		return ".bin";
	pos += 14; // Length of "Content-Type: "

	size_t end_pos = head.find("\r\n");
	if (end_pos == std::string::npos)
		return ".bin"; // Default extension if not found
	std::string content_type = head.substr(pos, end_pos - pos);
	return get_content_extension(content_type);
}

std::string	get_filename(const std::string& head)
{
	size_t pos = head.find("filename=\"");
	if (pos == std::string::npos)
		return "upload_"; // Default filename if not found
	pos += 10; // Length of "filename=\""

	size_t end_pos = head.find("\"", pos);
	if (end_pos == std::string::npos)
		return "upload_"; // Default filename if not found
	return head.substr(pos, end_pos - pos);
}

std::string create_filename(std::string& root, const std::string& head)
{
	int errcode = 0;
	std::string filename = get_filename(head);
	std::string extension = get_extension(head);

	if (filename.find(extension) == std::string::npos)
	{
		std::string file = root + filename;
		std::string file_ext = file + extension; 
		if (check_object_type(file, &errcode) == IS_EXISTINGFILE)
			filename += "_" + get_timestamp() + extension;
		else if (check_object_type(file_ext, &errcode) == IS_EXISTINGFILE)
			filename += "_" + get_timestamp();
		else
			filename += extension;
	}
	else
	{
		std::string file = root + filename;
		if (check_object_type(file, &errcode) == IS_EXISTINGFILE)
			filename = filename.erase(filename.find(extension)) + "_" + get_timestamp() + extension;
	}

	return filename;
}

void	parse_post_body(HttpRequest &req, std::string& head, std::string& body)
{
	body = req.get_body();
	if (body.empty())
	{
		std::cerr << "Error: POST body is empty." << std::endl;
		req.set_errorcode(400); // Bad Request
		return;
	}
	if (req.get_is_multipart())
	{
		std::string delimiter = req.get_boundary();
		if (delimiter.empty())
		{
			std::cerr << "Error: No boundary found in multipart POST request." << std::endl;
			req.set_errorcode(400); // Bad Request
			return;
		}
		if (body.find(delimiter) != std::string::npos)
			body = body.substr(body.find(delimiter) + delimiter.size(), body.rfind(delimiter + "--") - delimiter.size() - 2);

		size_t pos = body.find("\r\n\r\n");
		head = body.substr(0, pos);
		body = body.substr(pos + 4);
	}
}

bool	create_directories(std::string path, std::string root)    ////might be the issue
{
	std::istringstream iss(path);
	std::string token;
	std::string current = "/";

	while (getline(iss, token, '/'))
	{
		if (token.empty())
			continue; // Skip root directory or current directory

		if (!current.empty() && current.at(current.size() - 1) != '/')
			current += '/'; // Ensure we have a trailing slash
		current += token;

		std::cout << "Analysing create directories ... : (" << current.c_str() << ")\n";

		if (mkdir(current.c_str(), 0755) != 0)
		{
			if (errno == EEXIST)
				continue; // already exists, that's OK
			else
			{
				std::cerr << "Failed to create directory: " << current << std::endl;
				return false;
            }
		}
		std::ofstream authorized_delete_paths((root + "authorized_paths.txt").c_str(), std::ios::app);
		if (!authorized_delete_paths.is_open())
		{
			std::cerr << "Error: Could not open authorized delete paths file." << std::endl;
			return false;
		}

		std::cout << "Adding \"" << current << "\" to autorised delete path file !\n";

		authorized_delete_paths << current << std::endl; // Add the path to the authorized delete paths file
		authorized_delete_paths.close();
	}
	return true;
}

void	post_request(HTTPConfig &http_config, HttpRequest &req, t_fd_data &fd_data, std::string server_name)
{
	int errcode = 0;
	std::cout << "[post_request] Entering ....\n";
	std::string target = normalize_path(req.get_target());
	std::cout << "[post_request] path normed ....\n";

	// Trouver la configuration serveur
	ServerConfig &server = find_current_server(http_config, server_name);
	bool autoindex = server.get_autoindex();

	std::string location_name, root;
	try { location_name = find_location_name_and_set_root(target, server, root, autoindex); }
	catch (std::exception &e)
	{
		std::cerr << "Error finding matching location: " << e.what() << std::endl;
		return (build_response(req, "404", displayErrorPage("404", location_name, http_config, req, fd_data, server_name), false));
	}
	std::string error_code = validate_request_context(location_name, root, errcode, server, "POST");
	if (!error_code.empty())
	{
		std::cerr << "Error validating request context: " << error_code << std::endl;
		return (build_response(req, error_code, displayErrorPage(error_code, location_name, http_config, req, fd_data, server_name), false));
	}


	// std::cout << "[post_request]location name is \"" << location_name << "\" !\n";  /// fine for now as long as we have a cgi-bin location
	if (location_name == "/cgi-bin/")
	{
		// PRINT_DEBUG2
		fd_data.Content_Type = req.get_header("Content-Type"); // Assurez-vous que le Content-Type est présent
		fd_data.Content_Length = req.get_header("Content-Length"); // Assurez-vous que le Content-Length est présent
		// std::cout << "Content Type is : \n" << fd_data.Content_Type << "\n";
		// std::cout << "Content Length is : \n" << fd_data.Content_Length << "\n";

		std::string body;

		body = handleCGI(req, fd_data, &errcode);
		std::cout << "Fresh out of CGI : \n" << body << "\n";
		build_response(req, "200", body, req.getKeepAlive());
		return ;
	}

	std::string head;
	std::string body;
	parse_post_body(req, head, body);

	std::string file_name = remove_prefix(create_filename(root, head), location_name);
	std::cout << "file_name: " << file_name << std::endl;

	if (!location_name.find("upload"))
		root += "/uploads/";
	std::string file_path = root + file_name; // Chemin complet du fichier
	std::cout << "File path: " << file_path << std::endl;

	if (create_directories(file_path.substr(0, file_path.rfind('/')), server.get_root()) == false)
	{
		std::cerr << "Error: Failed to create directories for POST data." << std::endl;
		return (build_response(req, "500", "Failed to create directories for POST data", false));
	}
	std::ofstream out(file_path.c_str(), std::ios::binary);
	if (!out.is_open())
	{
		std::cerr << "Error: Failed to open file for writing: " << file_path << std::endl;
		return (build_response(req, "500", "Failed to store POST data", false));
	}
	out << body; // Stockage brut, sans analyse de type MIME
	out.close();

	std::ostringstream response_body;
	std::string filename = file_path.substr(file_path.rfind('/') + 1); // Extraire le nom de fichier
	response_body << req.get_target().substr(0, req.get_target().rfind('/') + 1) + "uploads/" + filename;

	build_response(req, "201", response_body.str(), req.getKeepAlive());
}