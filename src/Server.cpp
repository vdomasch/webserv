#include "Server.hpp"
#include "HTTPConfig.hpp"
#include "HttpResponse.hpp"
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

std::string ServerConfig::get_host_ip() const
{
	std::map<std::string, std::string>::const_iterator it = _map_server.find("host");
	if (it != _map_server.end() && !it->second.empty()) {
		return it->second;
	}
	return "0.0.0.0"; // Default if not provided
}

bool is_conflicting_binding(const std::string& ip, std::string port, const std::set<std::string>& already_bound)
{
	std::string this_key = ip + ":" + port;
	std::string any_key = "0.0.0.0:" + port;

	// If we're binding to 0.0.0.0, check if any specific IP is already bound on that port
	if (ip == "0.0.0.0")
	{
		for (std::set<std::string>::const_iterator it = already_bound.begin(); it != already_bound.end(); ++it)
			if (it->substr(it->find(":")) == ":" + port)
				return true; // same port is already bound to a specific IP
	}
	else // If we're binding to a specific IP, check if INADDR_ANY is already bound		
		if (already_bound.find(any_key) != already_bound.end())
			return true;
	return already_bound.find(this_key) != already_bound.end();
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
		return close_msg(server_fd, "Failed to set socket options", true, -1);
	
	memset(&servaddr, 0, sizeof(servaddr));

	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(server.get_uint_port_number());

	std::string host_ip = server.get_host_ip();

	if (host_ip == "0.0.0.0")
	    servaddr.sin_addr.s_addr = INADDR_ANY;
	else
	    if (inet_pton(AF_INET, host_ip.c_str(), &servaddr.sin_addr) != 1)
			return close_msg(server_fd, "Invalid IP address in config: " + host_ip, true, -1);

	if (bind(server_fd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
		return close_msg(server_fd, "Failed to bind port" + server.get_port_number(), true, -1);
	if (listen(server_fd, 10) < 0)
		return close_msg(server_fd, "Failed to listen on " + host_ip + ":" + server.get_port_number() , true, -1);
	return (server_fd);
}

void	Server::run_server(HTTPConfig &http_config)
{
	std::map<std::string, ServerConfig> servers_list = http_config.get_server_list();
	sockaddr_in servaddr;

	for (std::map<std::string, ServerConfig>::iterator it = servers_list.begin(); it != servers_list.end(); ++it)
	{
		ServerConfig server = it->second;
		int port = server.get_uint_port_number();

		// Only create a socket once per port
		if (_port_to_socket_map.find(port) == _port_to_socket_map.end())
		{
			std::string ip = server.get_host_ip();
			std::string str_port = server.get_port_number();
			std::string key = ip + ":" + str_port;

			if (is_conflicting_binding(ip, str_port, _ip_port_bound))
			{
				std::cerr << "Conflict: Cannot bind to " << key << " because it overlaps with an existing binding." << std::endl;
				continue; // or return, depending on how strict the error handling should be
			}

			std::cout << "Creating server socket on port " << port << std::endl;
			int server_socket = initialize_server(server, servaddr);
			if (server_socket < 0)
			{
				std::cerr << "Failed to initialize server" << std::endl;
				return;
			}
			_ip_port_bound.insert(key);
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
		close_msg(client_socket, "Too many connections", 1, -1);
		return;
	}
	FD_SET(client_socket, &_socket_data.saved_sockets);
	if (client_socket > _socket_data.max_fd)
		_socket_data.max_fd = client_socket;
	std::cout << "New client connected on socket " << client_socket << " through server port " << _socket_to_port_map[fd] << std::endl;
	_socket_to_port_map[client_socket] = _socket_to_port_map[fd];
}

std::string Server::get_server_name(int fd)
{
	std::map<int, int>::iterator it = _socket_to_port_map.find(fd);
	if (it != _socket_to_port_map.end())
	{
		int port = it->second;
		std::string port_str;
		port_str = convert<std::string>(port); // get port to str

		std::string host = _socket_states[fd].get_header("Host"); // get host from header
		if (host.empty())
			return port_str; // if host is empty, return port

		std::string host_port = host.substr(host.find(":") + 1); // if host has port
		if (host_port.empty())
		{
			return host + port_str; // if host has no port, return host + port
		}
		else if (port_str.find_first_not_of("0123456789") != std::string::npos) // if port is not a number
		{
			throw std::runtime_error("Invalid port number in Host header: " + host_port); // throw exception
		}
		else
			return host; // retrun host
	}
	return ""; 
}

void	Server::handle_client_request(HTTPConfig &http_config, int fd)
{
	static_cast<void>(http_config);
	char buffer[BUFFER_SIZE] = {0};
	ssize_t bytes_read = recv(fd, buffer, BUFFER_SIZE, 0);

	std::cout << "Bytes read: " << bytes_read << std::endl << std::endl << std::endl;

	if (bytes_read <= 0)
	{
		close_msg(fd, "Disconnected or read error", 0, -1);
		return;
	}

	buffer[bytes_read] = '\0';
	std::cout << "Received data: " << buffer << std::endl;
	_socket_states[fd].append_data(buffer);
	if (_socket_states[fd].has_error())
	{
		send(fd, "HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\nConnexion: close\r\n\r\n", 39, 0);
		close_msg(fd, "Bad request", 1, -1);
		return;
	}

	std::string server_name;

	try { server_name = get_server_name(fd); }
	catch (std::exception &e)
	{
		std::cerr << "Error getting server name: " << e.what() << std::endl;
		send(fd, "HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\n\r\n", 39, 0);
		close_msg(fd, "Bad request", 1, -1);
		return;
	}

	std::map<std::string, ServerConfig> server_list = http_config.get_server_list();

	if (_socket_states[fd].is_ready())
	{
		std::string method = _socket_states[fd].get_method();
		if (_method_map.count(method))
		{
			_method_map[method](_socket_states[fd], server_list, _socket_data, server_name);
		}
		else
		{
			// Méthode non autorisée
			HttpResponse res;
			res.set_status(405, "Method Not Allowed");
			res.set_body("405 Method Not Allowed");
			res.add_header("Content-Type", "text/plain");
			res.add_header("Content-Length", convert<std::string>(res.get_body().size()));
			res.add_header("Connection", "close");
			_socket_states[fd].set_response(res.generate_response());
		}

		std::string response = _socket_states[fd].get_response();
		std::cout << "RESPONSE:\n-------------------------------------------\n" << response << "\n---------------------------------" << std::endl;
		send(fd, response.c_str(), response.size(), 0);

		if (!_socket_states[fd].getKeepAlive())
			close_msg(fd, "Connection closed (no keep-alive)", 0, 0);
		else 
			_socket_states[fd] = HttpRequest(); // Optionnel : reset request state for next request
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
					std::cout << "Handling request on socket " << i << std::endl;
					// Initialisation si première interaction
					if (_socket_states.find(i) == _socket_states.end()) _socket_states[i] = HttpRequest();

					// Traitement de la requête
					handle_client_request(http_config, i);
				}
			}
		}
	}
	std::cout << "\n\033[31m++ Server shutting down ++\033[0m\n" << std::endl;
	shutdown_all_sockets();
}

void	Server::update_max_fd(int fd)
{
	if (fd == _socket_data.max_fd)
		while (_socket_data.max_fd > 0 && !FD_ISSET(_socket_data.max_fd, &_socket_data.saved_sockets))
			_socket_data.max_fd--;
}

int Server::close_msg(int fd, const std::string &message, int err, int return_code)
{

	if (err)
		std::cerr << "\033[31m" << message << " (fd " << fd << ") - ERROR\033[0m" << std::endl;
	else
		std::cout << "\033[32m" << message << " (fd " << fd << ") - CLOSED\033[0m" << std::endl;

	close(fd);
	FD_CLR(fd, &_socket_data.saved_sockets);
	update_max_fd(fd);
	return return_code;

	/*if (err)
		std::cerr << "\033[31mClose Message Error: " << "\033[0m" << std::endl;
	else if (!message.empty())
		;
	else
		std::cout << "\033[30m" << message << fd << " closed" << "\033[0m" << std::endl;
	close(fd);
	FD_CLR(fd, &_socket_data.saved_sockets);
	update_max_fd(fd);*/
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