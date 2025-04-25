#include "webserv.hpp"
#include <sys/types.h>
#include <sys/socket.h>

void	get_request(HttpRequest &req, std::map<std::string, ServerConfig> &server_list)
{
	static_cast<void>(server_list);

	std::cout << "GET request received" << std::endl;
	//std::cout << req << std::endl;

	if (req.getPath().find("favicon.ico") != std::string::npos)
	{
		std::cout << "Favicon request received" << std::endl;
		return;
	}
	else if (!req.getPath().empty())
	{
		std::cout << "Path Request Recieved" << std::endl;
		//std::cout << "Path: " << req.getPath() << std::endl;
		std::cout << "Host: " << req.getHost() << std::endl;
		std::cout << "Host: " << req.getHost().substr(0, req.getHost().find(':')) << std::endl;
		std::cout << "Port: " << req.getPort() << std::endl;

		std::string server_name(req.getHost().substr(0, req.getHost().find(':')));

		std::string key(req.getPort() + ":" + server_name);

		if (req.getHost().find(':') != std::string::npos)		// DEMANDER BEN SI 8080:server_name -> peut changer en server_name:8080
		{
			if (server_list.find(key) != server_list.end())
			{
				std::cout << "Server found 1" << std::endl;
				ServerConfig server = server_list[key];

				if (req.getPath().rfind('/') != 0)
				{
					std::vector<LocationConfig> locations = server.get_location_list();
					for (std::vector<LocationConfig>::iterator it = locations.begin(); it != locations.end(); ++it)
					{
						if (it.base()->get_map_location().find("path") != it.base()->get_map_location().end())
						{
							std::string location_path = it.base()->get_map_location()["path"];

							std::cout << "Location path: " << location_path << std::endl;
							std::cout << "Request path: " << req.getPath() << std::endl;

							std::string path = req.getPath().substr(0, req.getPath().rfind('/'));
							std::cout << "Path: " << path << std::endl;

							if (path == location_path)
							{
								std::cout << "Location found" << std::endl;
								if (server.get_map_server()["root"].at(server.get_map_server()["root"].size() - 1) == '/' && req.getPath().at(0) == '/')
									req.setPath(req.getPath().substr(1));
								std::string complete_path = server.get_map_server()["root"] + req.getPath();
								std::cout << "Complete path: " << complete_path << std::endl;

								std::ifstream file;
								file.open(complete_path.c_str(), std::ios::in);
								if (file)
								{
									std::cout << "File found" << std::endl;
									std::ostringstream ss;
									ss << file.rdbuf();

									if (req.getKeepAlive())
										req.setResponse(create_header("200 OK", "text/html", tostr(file.width()), "keep-alive") + ss.str());
									else
										req.setResponse(create_header("200 OK", "text/html", tostr(file.width()), "close") + ss.str());
									
									//req.setResponse(ss.str() + req.getResponse());
									file.close();
									return ;
								}
								else
								{
									std::cout << "File not found" << std::endl;
									return;
								}
								return;
							}
							else
							{
								std::cout << "Location not found" << std::endl;
								return;
							}
						}
					}
				}
				else
				{
					std::cout << "Path is root" << std::endl;
					return;
				}
			}
			else if (server_list.find(req.getPort()) != server_list.end())
			{
				std::cout << "Server found 2" << std::endl;
				return;
			}
			else
			{
				std::cout << "Server not found" << std::endl;
				return;
			}
		}

	}
	else
	{
		std::cout << "Request error" << std::endl;
		return;
	}

}

void	post_request(HttpRequest &req, std::map<std::string, ServerConfig> &server_list)
{
	static_cast<void>(server_list);

	std::cout << "POST request received" << std::endl;
	std::cout << req << std::endl;

	std::string response = create_header("200 OK", "text/plain", "16", "keep-alive");

}

void	delete_request(HttpRequest &req, std::map<std::string, ServerConfig> &server_list)
{
	static_cast<void>(server_list);
	
	std::cout << "DELETE request received" << std::endl;
	std::cout << req << std::endl;


}

std::string create_header(const std::string &status, const std::string &content_type, const std::string &content_length, const std::string &connection)
{
	std::string header = "HTTP/1.1 " + status + "\r\n";
	header += "Content-Type: " + content_type + "\r\n";
	header += "Content-Length: " + content_length + "\r\n";
	header += "Connection: " + connection + "\r\n\r\n";
	return header;
}