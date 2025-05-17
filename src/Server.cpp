#include "Server.hpp"
#include "HTTPConfig.hpp"
#include <map>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <algorithm>
#include <cctype>

bool g_running = true;

void handle_signal(int signum) { if (signum == SIGINT) { std::cout << "\n\033[31m++ Server shutting down ++\033[0m\n" << std::endl, g_running = false; } }

Server::Server()
{
	// Initialize the socket data
	FD_ZERO(&_socket_data.saved_sockets);
	FD_ZERO(&_socket_data.ready_sockets);
	_socket_data.max_fd = 0;
	_port_to_socket_map.clear();
	_method_map["GET"] = &get_request;
	_method_map["POST"] = &post_request;
	_method_map["DELETE"] = &delete_request;
}

Server::~Server() {}

std::map<int, int> Server::get_port_to_socket_map() const { return _port_to_socket_map; }
std::map<int, int> Server::get_socket_to_port_map() const { return _socket_to_port_map; }

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

void	Server::run_server(HTTPConfig &http_config)
{
	std::map<std::string, ServerConfig> servers_list = http_config.get_server_list();
	sockaddr_in servaddr;

	for (std::map<std::string, ServerConfig>::iterator it = servers_list.begin(); it != servers_list.end(); ++it)
	{
		ServerConfig server = it->second;
		int port = server.get_port_number();

		// Only create a socket once per port
		if (_port_to_socket_map.find(port) == _port_to_socket_map.end())
		{
			std::cout << "Creating server socket on port " << port << std::endl;
			int server_socket = initialize_server(server, servaddr);
			if (server_socket < 0)
			{
				std::cerr << "Failed to initialize server" << std::endl;
				return;
			}
			_port_to_socket_map[port] = server_socket;
			_socket_to_port_map[server_socket] = port;
			FD_SET(server_socket, &_socket_data.saved_sockets);
			if (server_socket > _socket_data.max_fd)
				_socket_data.max_fd = server_socket;
			std::cout << "Server port " << port << " created on socket " << server_socket << std::endl;
		}
	}

	signal(SIGINT, handle_signal);
	
	running_loop(http_config, servaddr);

	shutdown_all_sockets();
}

void Server::handle_new_connection(int fd, sockaddr_in &servaddr)
{
	socklen_t addr_len = sizeof(servaddr);
	int client_socket = accept(fd, reinterpret_cast<struct sockaddr *>(&servaddr), &addr_len);
	if (client_socket < 0)
	{
		std::cerr << "Failed to accept new connection" << std::endl;
		return;
	}
	if (client_socket > FD_SETSIZE)
	{
		close_msg(client_socket, "Too many connections", 1);
		return;
	}
	FD_SET(client_socket, &_socket_data.saved_sockets);
	if (client_socket > _socket_data.max_fd)
		_socket_data.max_fd = client_socket;
	std::cout << "New client connected on socket " << client_socket << " through server port " << _socket_to_port_map[fd] << std::endl;
	_socket_to_port_map[client_socket] = _socket_to_port_map[fd];
}

void	Server::handle_client_request(HTTPConfig &http_config, int fd)
{
	char buffer[BUFFER_SIZE] = {0};
	ssize_t bytes_read = recv(fd, buffer, BUFFER_SIZE, 0);

	if (bytes_read <= 0)
	{
		close_msg(fd, "Disconnected or read error", 0);
		return;
	}

	buffer[bytes_read] = '\0';
	_socket_states[fd].append_data(buffer);

	if (_socket_states[fd].has_error())
	{
		send(fd, "HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\n\r\n", 39, 0);
		close_msg(fd, "Bad request", 1);
		return;
	}

	if (_socket_states[fd].is_ready())
	{
		std::string response = _socket_states[fd].get_response();
		send(fd, response.c_str(), response.size(), 0);

		if (!_socket_states[fd].getKeepAlive()) close_msg(fd, "Connection closed (no keep-alive)", 0);
		else _socket_states[fd] = HttpRequest(); // Optionnel : reset request state for next request
	}
}

void Server::running_loop(HTTPConfig &http_config, sockaddr_in &servaddr)
{
	while (g_running)
	{
		std::cout << "\n\033[31m++ Waiting for new connection ++\033[0m\n";

		// Rafraîchir la copie des sockets surveillés
		_socket_data.ready_sockets = _socket_data.saved_sockets;

		// Timeout de 5 secondes pour éviter le blocage
		struct timeval timeout;
		timeout.tv_sec = 5;
		timeout.tv_usec = 0;

		// Nettoyage préventif des sockets invalides
		for (int i = 0; i <= _socket_data.max_fd; ++i)
		{
			if (FD_ISSET(i, &_socket_data.saved_sockets))
			{
				int error = 0;
				socklen_t len = sizeof(error);
				if (getsockopt(i, SOL_SOCKET, SO_ERROR, &error, &len) == -1) {
					std::cerr << "[DEBUG] BAD FD: " << i << " (closed or invalid!)" << std::endl;
				}
			}
		}

		// Appel à select()
		if (select(_socket_data.max_fd + 1, &_socket_data.ready_sockets, NULL, NULL, &timeout) < 0)
		{
			if (errno == EINTR) continue;
			std::cerr << "Select failed: " << strerror(errno) << std::endl;
			break;
		}

		// Parcours des sockets actifs
		for (int i = 0; i <= _socket_data.max_fd; ++i) 
		{
			std::cout << "\n\033[32m========= i = " << i << " =========\033[0m\n" << std::endl;
			if (FD_ISSET(i, &_socket_data.ready_sockets))
			{
				if (is_server_socket(i))
				{
					handle_new_connection(i, servaddr); // socket d'écoute
				}
				else
				{
					// Initialisation si première interaction
					if (_socket_states.find(i) == _socket_states.end()) _socket_states[i] = HttpRequest();

					// Traitement de la requête
					handle_client_request(http_config, i);
				}
			}
		}
	}
}

void	Server::update_max_fd(int fd)
{
	if (fd == _socket_data.max_fd)
		while (_socket_data.max_fd > 0 && !FD_ISSET(_socket_data.max_fd, &_socket_data.saved_sockets))
			_socket_data.max_fd--;
}

void Server::close_msg(int fd, const std::string &message, int err)
{
	if (err)
		std::cerr << "\033[31mClose Message Error: " << "\033[0m" << std::endl;
	else if (!message.empty())
		;
	else
		std::cout << "\033[30m" << message << fd << " closed" << "\033[0m" << std::endl;
	close(fd);
	FD_CLR(fd, &_socket_data.saved_sockets);
	update_max_fd(fd);
}

bool	Server::is_server_socket(int fd)
{
	for (std::map<int, int>::iterator it = _port_to_socket_map.begin(); it != _port_to_socket_map.end(); ++it)
	{
		if (it->second == fd)
			return true;
	}
	return false;
}

void	Server::shutdown_all_sockets()
{
	for (int fd = 0; fd <= _socket_data.max_fd; ++fd)
	{
		if (FD_ISSET(fd, &_socket_data.saved_sockets))
		{
			std::cout << "[Shutdown] Closing FD " << fd << std::endl;
			close(fd);
		}
	}
	FD_ZERO(&_socket_data.saved_sockets);
	FD_ZERO(&_socket_data.ready_sockets);
	_socket_data.max_fd = 0;

	for (std::map<int, int>::iterator it = _port_to_socket_map.begin(); it != _port_to_socket_map.end(); ++it)
	{
		int fd = it->second;
		if (FD_ISSET(fd, &_socket_data.saved_sockets))
		{
			std::cout << "[Shutdown] Closing server socket on port " << it->first << std::endl;
			close(fd);
			FD_CLR(fd, &_socket_data.saved_sockets);
		}
	}
	_port_to_socket_map.clear();
}