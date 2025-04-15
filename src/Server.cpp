#include "Server.hpp"
#include "HTTPConfig.hpp"
#include <map>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <algorithm>
#include <cctype>

Server::Server() {
	static_cast<void>(_socket_data);
}

Server::~Server() {}

bool g_running = true;

void	handle_signal(int signum)
{
	if (signum == SIGINT)
	{
		std::cout << "\n\033[31m++ Server shutting down ++\033[0m\n" << std::endl;
		g_running = false;
	}
}

int	Server::initialize_server(ServerConfig &server, sockaddr_in &servaddr)
{
	int server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd < 0)
	{
		std::cerr << "Failed to create socket" << std::endl;
		return (-1);
	}
	int opt = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) == -1)
	{
		std::cerr << "Failed to set socket options" << std::endl;
		close(server_fd);
		FD_CLR(server_fd, &_socket_data.saved_sockets);
		return (-1);
	}
	
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(server.get_port_number());
	servaddr.sin_addr.s_addr = INADDR_ANY; //inet_addr(server.get_map_server()["host"].c_str());


	if (bind(server_fd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
	{
		std::cerr << "Failed to bind to port " << server.get_port_number() << std::endl;
		close(server_fd);
		FD_CLR(server_fd, &_socket_data.saved_sockets);
		return (-1);
	}
	if (listen(server_fd, 10) < 0)
	{
		std::cerr << "Failed to listen" << std::endl;
		close(server_fd);
		FD_CLR(server_fd, &_socket_data.saved_sockets);
		return (-1);
	}
	return (server_fd);
}

bool is_keep_alive(const std::string &request)
{
	std::string lower_request = request;
	std::transform(lower_request.begin(), lower_request.end(), lower_request.begin(), ::tolower);
	return lower_request.find("connection: keep-alive") != std::string::npos;
}


void	Server::run_server(HTTPConfig &http_config)
{
	std::map<int, std::vector<ServerConfig> > servers_by_port;
	std::map<std::string, ServerConfig> servers_list = http_config.get_server_list();
	sockaddr_in servaddr;

	for (std::map<std::string, ServerConfig>::iterator it = servers_list.begin(); it != servers_list.end(); it++)
	{
		ServerConfig server = it->second;
		int port = server.get_port_number();
		servers_by_port[port].push_back(server);

		if (_port_socket_map.find(server.get_port_number()) == _port_socket_map.end())
		{
			int server_socket = initialize_server(server, servaddr);
			if (server_socket < 0)
			{
				std::cerr << "Failed to initialize server" << std::endl;
				return ;
			}
			_port_socket_map[server.get_port_number()] = server_socket;
			FD_SET(server_socket, &_socket_data.saved_sockets);
			if (server_socket > _socket_data.max_fd)
				_socket_data.max_fd = server_socket;
		}
	}

	signal(SIGINT, handle_signal);
	
	while (g_running)
	{
		std::cout << "\n\033[31m++ Waiting for new connection ++\033[0m\n" << std::endl;
		_socket_data.ready_sockets = _socket_data.saved_sockets;

		struct timeval timeout;
		timeout.tv_sec = 5;
		timeout.tv_usec = 0;

		if (select(_socket_data.max_fd + 1, &_socket_data.ready_sockets, NULL, NULL, &timeout) < 0)
		{
			if (errno == EINTR)
				continue;
			std::cerr << "Select failed: " << strerror(errno) << std::endl;
			return;
		}

		for (int i = 0; i <= _socket_data.max_fd; i++)
		{
			if (FD_ISSET(i, &_socket_data.ready_sockets))
			{
				std::cout << "\n\033[32m========= i = " << i << " =========\033[0m\n" << std::endl;
				bool is_server_socket = false;
				for (std::map<int, int>::iterator it = _port_socket_map.begin(); it != _port_socket_map.end(); ++it)
				{
					if (it->second == i)
					{
						is_server_socket = true;
						break;
					}
				}
				if (is_server_socket) // New connection
				{
					socklen_t addr_len = sizeof(servaddr);
					int client_socket = accept(i, reinterpret_cast<struct sockaddr *>(&servaddr), &addr_len);
					if (client_socket < 0)
					{
						std::cerr << "Failed to accept new connection" << std::endl;
						continue;
					}
					if (client_socket > FD_SETSIZE)
					{
						std::cerr << "Too many connections" << std::endl;
						close(client_socket);
						FD_CLR(client_socket, &_socket_data.saved_sockets);
						continue;
					}
					FD_SET(client_socket, &_socket_data.saved_sockets);
					if (client_socket > _socket_data.max_fd)
						_socket_data.max_fd = client_socket;
					std::cout << "New client connected on socket " << client_socket << std::endl;
				}
				else
				{
					std::cout << "Handling existing client on socket " << i << std::endl;
					char buffer[BUFFER_SIZE] = {0};
					ssize_t bytes_read = recv(i, buffer, BUFFER_SIZE, 0);
					if (bytes_read < 0)
					{
						std::cout << "recv() failed" << std::endl;
						close(i);
						FD_CLR(i, &_socket_data.saved_sockets);
						update_max_fd(i);
						continue;
					}
					if (bytes_read == 0)
					{
						std::cout << "Client on socket " << i << " disconnected." << std::endl;
						close(i);
						FD_CLR(i, &_socket_data.saved_sockets);
						update_max_fd(i);
						continue;
					}

					std::cout << "Bytes read: " << bytes_read << std::endl;
					if (bytes_read < BUFFER_SIZE)
						buffer[bytes_read] = '\0';
					else
						buffer[BUFFER_SIZE - 1] = '\0';
					
					std::string request(buffer);
					bool keep_alive = is_keep_alive(request);

					std::cout << "Received: " << buffer << std::endl;
					const char* http_response;

					if (!keep_alive)
					{
						http_response = "HTTP/1.1 200 OK\r\n"
										"Content-Type: text/plain\r\n"
										"Content-Length: 16\r\n"
										"Connection: close\r\n"
										"\r\n"
										"Message received";
					}
					else
					{
						http_response =	"HTTP/1.1 200 OK\r\n"
													"Content-Type: text/plain\r\n"
													"Content-Length: 16\r\n"
													"Connection: keep-alive\r\n"
													"\r\n"
													"Message received";
					}

					send(i, http_response, strlen(http_response), 0); // Echo back to client
					if (!keep_alive)
					{
						close(i);
						FD_CLR(i, &_socket_data.saved_sockets);
						update_max_fd(i);
					}
				}
			}
		}
		static_cast<void>(http_config);
	}
	FD_ZERO(&_socket_data.saved_sockets);
	FD_ZERO(&_socket_data.ready_sockets);
	_socket_data.max_fd = 0;
}

void	Server::update_max_fd(int fd)
{
	if (fd == _socket_data.max_fd)
		while (_socket_data.max_fd > 0 && !FD_ISSET(_socket_data.max_fd, &_socket_data.saved_sockets))
			_socket_data.max_fd--;
}