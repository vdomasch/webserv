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
	//std::cout << "Content-Type found: " << content_type << std::endl;
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

	return root + filename;
}

void	parse_post_body(HttpRequest &req, std::string& head, std::string& body)
{
	// Ici, on suppose que req.get_body() contient les données POST à traiter
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
			body = body.substr(body.find(delimiter) + delimiter.size(), body.rfind(delimiter) - delimiter.size());

		size_t pos = body.find("\r\n\r\n");
		head = body.substr(0, pos);
		body = body.substr(pos + 4);
	}
}

bool	create_directories(std::string path)
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
		std::cout << "Directory: '" << current << "' created." << std::endl;
	}
	return true;
}

void	post_request(HTTPConfig &http_config, HttpRequest &req, std::map<std::string, ServerConfig> &server_list, t_fd_data &fd_data, std::string server_name)
{
	//std::cout << "Handling POST request for target: " << req.get_target() << std::endl;
	int errcode = 0;
	std::string target = normalize_path(req.get_target());

	// Trouver la configuration serveur
	std::map<std::string, ServerConfig>::iterator it_serv = server_list.find(server_name);
	if (it_serv == server_list.end())
	{
		server_name = server_name.substr(server_name.find(':') + 1);
		it_serv = server_list.find(server_name);
		if (it_serv == server_list.end())
		{
			req.set_response("HTTP/1.1 404 Not Found\r\n\r\n");
			return;
		}
	}

	ServerConfig &server = it_serv->second;
	std::string location_name, root;
	std::map<std::string, LocationConfig>::iterator it_loc;
	bool autoindex = server.get_autoindex();

	try {
		location_name = server.get_matching_location(target, autoindex);
		std::map<std::string, LocationConfig> &location_list = server.get_location_list();
		it_loc = location_list.find(location_name);
		if (it_loc != location_list.end())
			root = it_loc->second.get_root() + "uploads/";
		else
			throw std::runtime_error("Location not found");
	} catch (std::exception &e) {
		build_response(req, 404, "Not Found", "text/html", displayErrorPage("404", "Page Not Found", find_error_page("404", NULL, server, http_config), http_config, req, server_list, fd_data, server_name, true), false);
		return;
	}

	// Vérifier que le dossier existe
	if (check_object_type(root, &errcode) != IS_DIRECTORY)
	{
		build_response(req, 500, "Internal Server Error", "text/html", displayErrorPage("500", "Root Invalid", find_error_page("500", NULL, server, http_config), http_config, req, server_list, fd_data, server_name, true), false);
		return;
	}

	if (check_allowed_methods(server, it_loc->second, "POST") == false)
	{
		build_response(req, 405, "Method Not Allowed", "text/html", displayErrorPage("405", "Method Not Allowed", find_error_page("405", NULL, server, http_config), http_config, req, server_list, fd_data, server_name, true), false);
		return;
	}

	if (location_name == "/cgi-bin/")
	{



	}

	std::string head;
	std::string body;
	parse_post_body(req, head, body);

	std::string file_path = create_filename(root, head);	

	if (create_directories(file_path.substr(0, file_path.rfind('/'))) == false)
	{
		build_response(req, 500, "Internal Server Error", "text/html", "Failed to create directories for POST data", false);
		return;
	}
	std::ofstream out(file_path.c_str(), std::ios::binary);
	if (!out.is_open())
	{
		build_response(req, 500, "Internal Server Error", "text/html", "Failed to store POST data", false);
		return;
	}
	out << body; // Stockage brut, sans analyse de type MIME
	out.close();

	std::ostringstream response_body;
	std::string filename = file_path.substr(file_path.rfind('/') + 1); // Extraire le nom de fichier
	response_body << "<html><body><h1>POST Success</h1><p>File saved as: " << req.get_target().substr(0, req.get_target().rfind('/') + 1) + "uploads/" + filename << "</p></body></html>";

	build_response(req, 201, "Created", "text/html", response_body.str(), req.getKeepAlive());
}