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

void	post_request(HTTPConfig &http_config, HttpRequest &req, std::map<std::string, ServerConfig> &server_list, t_fd_data &fd_data, std::string server_name)
{
	static_cast<void>(http_config);
	static_cast<void>(req);
	static_cast<void>(server_name);
	static_cast<void>(server_list);
	static_cast<void>(fd_data);

	PRINT_DEBUG
	std::cout << "Handling POST request for target: " << req.get_target() << std::endl;

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
			root = it_loc->second.get_root();
		else
			throw std::runtime_error("Location not found");
	} catch (std::exception &e) {
		build_response(req, 404, "Not Found", "text/html", displayErrorPage("404", "Page Not Found", find_error_page("404", NULL, server, http_config), http_config, req, server_list, fd_data, server_name, true), true);
		return;
	}

	// Vérifier que le dossier existe
	if (check_object_type(root, &errcode) != IS_DIRECTORY)
	{
		build_response(req, 500, "Internal Server Error", "text/html", displayErrorPage("500", "Root Invalid", find_error_page("500", NULL, server, http_config), http_config, req, server_list, fd_data, server_name, true), true);
		return;
	}

	// Générer un nom de fichier unique (timestamp ou compteur)
	std::string file_path = root + "/upload_" + get_timestamp() + ".dat";

	// Écrire le corps de la requête dans un fichier
	std::ofstream out(file_path.c_str(), std::ios::binary);
	if (!out.is_open())
	{
		build_response(req, 500, "Internal Server Error", "text/html", "Failed to store POST data", false);
		return;
	}
	out << req.get_body(); // Stockage brut, sans analyse de type MIME
	out.close();

	std::ostringstream response_body;
	response_body << "<html><body><h1>POST Success</h1><p>File saved as: " << file_path << "</p></body></html>";

	build_response(req, 201, "Created", "text/html", response_body.str(), false);
}