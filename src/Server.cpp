#include "Server.hpp"
#include "HTTPConfig.hpp"
#include <map>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstring>
#include <iostream>

Server::Server(): _server_fd(-1), _client_fd(-1) {
	static_cast<void>(_server_fd);
	static_cast<void>(_client_fd);
	static_cast<void>(_socket_data);
}

Server::~Server() {
	// Close all sockets
	for (std::map<int, int>::iterator it = _port_socket_map.begin(); it != _port_socket_map.end(); ++it)
	{
		close(it->second);
	}
	_port_socket_map.clear();
	std::cout << "Server closed." << std::endl;
}

bool g_running = true;

void	handle_signal(int signum)
{
	if (signum == SIGINT)
	{
		std::cout << "\n\033[31m++ Server shutting down ++\033[0m\n" << std::endl;
		g_running = false;
	}
}

int	Server::initialize_server(ServerConfig &server)
{
	int server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd < 0)
	{
		std::cerr << "Failed to create socket" << std::endl;
		return (-1);
	}
	int opt = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
	{
		std::cerr << "Failed to set socket options" << std::endl;
		close(server_fd);
		return (-1);
	}
	
	sockaddr_in servaddr;
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(server.get_port_number());
	servaddr.sin_addr.s_addr = INADDR_ANY; //inet_addr(server.get_map_server()["host"].c_str());


	if (bind(server_fd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
	{
		std::cerr << "Failed to bind to port" << server.get_port_number() << std::endl;
		close(server_fd);
		return (-1);
	}
	if (listen(server_fd, 10) < 0)
	{
		std::cerr << "Failed to listen" << std::endl;
		close(server_fd);
		return (-1);
	}
	return (server_fd);
}

void	Server::run_server(HTTPConfig &http_config)
{
	std::map<int, std::vector<ServerConfig> > servers_by_port;
	std::map<std::string, ServerConfig> servers_list = http_config.get_server_list();
	
	for (std::map<std::string, ServerConfig>::iterator it = servers_list.begin(); it != servers_list.end(); it++)
	{
		ServerConfig server = it->second;
		int port = server.get_port_number();
		servers_by_port[port].push_back(server);

		if (_port_socket_map.find(server.get_port_number()) == _port_socket_map.end())
		{
			int server_socket = initialize_server(server);
			if (server_socket < 0)
			{
				std::cerr << "Failed to initialize server" << std::endl;
				return ;
			}
			_port_socket_map[server.get_port_number()] = server_socket;
		}
	}

	signal(SIGINT, handle_signal);
	
	while (g_running)
	{
		static_cast<void>(http_config);
	}
}