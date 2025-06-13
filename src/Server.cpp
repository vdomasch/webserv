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

void handle_signal(int signum) { if (signum) { g_running = false; } }

Server::Server()
{
	// Initialize the socket data
	FD_ZERO(&_socket_data.saved_readsockets);
	FD_ZERO(&_socket_data.ready_readsockets);
	FD_ZERO(&_socket_data.ready_readsockets);
	FD_ZERO(&_socket_data.ready_writesockets);
	FD_ZERO(&_socket_data.saved_readsockets);
	FD_ZERO(&_socket_data.saved_writesockets);
	//_socket_data.max_fd = 0;
	_port_to_socket_map.clear();
	_method_map["GET"] = &get_request;
	_method_map["POST"] = &post_request;
	_method_map["DELETE"] = &delete_request;
	_socket_data.serverFolder = "";
	_socket_data.requestedFilePath = "";
	_socket_data.max_fd = -1;
	_socket_data.response_len = 0;
	_socket_data.Content_Length = "default";
	_socket_data.Content_Type = "default";
	_socket_data.is_binaryContent = false;
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

bool	Server::is_conflicting_binding(const std::string& ip, std::string port, const std::set<std::string>& already_bound)
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
		std::cerr << "Error: Failed to create socket" << std::endl;
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
	if (listen(server_fd, 1024) < 0)
		return close_msg(server_fd, "Failed to listen on " + host_ip + ":" + server.get_port_number() , true, -1);
	return (server_fd);
}

void	Server::launch_server(HTTPConfig &http_config)
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

			//std::cout << "\033[3;32m++ Creating server socket on port " << port  << " ++\033[0m" << std::endl;
			int server_socket = initialize_server(server, servaddr);
			if (server_socket < 0)
			{
				std::cerr << "Error: Failed to initialize server" << std::endl;
				return;
			}
			_ip_port_bound.insert(key);
			_port_to_socket_map[port] = server_socket;
			_socket_to_port_map[server_socket] = port;
			FD_SET(server_socket, &_socket_data.saved_readsockets);
			if (server_socket > _socket_data.max_fd)
				_socket_data.max_fd = server_socket;
			std::cout << "\033[3;32m++ Server port " << port << " created on socket " << server_socket << " ++\033[0m\n" << std::endl;
		}
	}

	signal(SIGINT, handle_signal);
	signal(SIGTSTP, handle_signal);
	running_loop(http_config, servaddr);

	shutdown_all_sockets();
}

void Server::handle_new_connection(int fd, sockaddr_in &servaddr)
{
	socklen_t addr_len = sizeof(servaddr);
	int client_socket = accept(fd, reinterpret_cast<struct sockaddr *>(&servaddr), &addr_len);
	if (client_socket < 0)
	{
		std::cerr << "Error: Failed to accept new connection" << std::endl;
		return;
	}
	if (client_socket > FD_SETSIZE)
	{
		close_msg(client_socket, "Too many connections", 1, -1);
		return;
	}

	int flags = fcntl(client_socket, F_GETFL, 0);
	if (flags < 0 || fcntl(client_socket, F_SETFL, flags | O_NONBLOCK) < 0)
	{
		close_msg(client_socket, "Failed to set non-blocking mode", 1, -1);
		return;
	}

	FD_SET(client_socket, &_socket_data.saved_readsockets);
	if (client_socket > _socket_data.max_fd)
		_socket_data.max_fd = client_socket;
	std::cout << "\033[1;32mNew client connected on socket " << client_socket << " through server port " << _socket_to_port_map[fd] << "\033[0m" << std::endl;
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

bool	Server::reading_data(int fd)
{
	char buffer[BUFFER_SIZE];
	memset(buffer, 0, BUFFER_SIZE); // Clear the buffer
	ssize_t bytes_read;
	do {
		bytes_read = recv(fd, buffer, BUFFER_SIZE, 0);
		if (bytes_read < 0)
			;
		if (bytes_read == 0)
		{
			close_msg(fd, "Client Disconnected", 0, -1);
			return 1;
		}
		if (bytes_read > 0)
			_socket_states[fd].append_data(std::string(buffer, bytes_read));
	} while (bytes_read > 0);
	if (_socket_states[fd].has_error())
	{
		std::cerr << "Error in request: " << std::endl;
		send(fd, "HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\nConnexion: close\r\n\r\n", 39, 0);
		close_msg(fd, "Bad request", 1, -1);
		return 1;
	}
	if (!_socket_states[fd].is_ready())
		return 1;
	
	if (_socket_states[fd].get_method().empty())
	{
		std::cerr << "Error: No method found in request" << std::endl;
		send(fd, "HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\n\r\n", 39, 0);
		close_msg(fd, "Bad request", 1, -1);
		return 1;
	}
	return 0; // Request is ready
}

void	Server::handle_client_request(HTTPConfig &http_config, int fd)
{
	if (reading_data(fd))
		return;

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
			_method_map[method](http_config, _socket_states[fd], _socket_data, server_name);
		}
		else
		{
			// Méthode non autorisée
			std::cerr << "Method not allowed: " << method << std::endl;
			HttpResponse res;
			res.set_status("405", "Method Not Allowed");
			res.set_body("405 Method Not Allowed");
			res.add_header("Allow", "GET, POST, DELETE");
			res.add_header("Content-Type", "text/plain");
			res.add_header("Content-Length", convert<std::string>(res.get_body().size()));
			res.add_header("Connection", "close");
			_socket_states[fd].set_response(res.generate_response());
		}
		std::string response = _socket_states[fd].get_response();
		send(fd, response.c_str(), response.size(), 0);

		if (!_socket_states[fd].getKeepAlive()/* || _socket_states[fd].get_method() == "DELETE"*/) // To review ?? add POST in it ?
			close_msg(fd, "Connection closed (no keep-alive)", 0, 0);
		else 
			_socket_states[fd] = HttpRequest(); // Optionnel : reset request state for next request
	}
	if (_socket_states[fd].is_finished())
		_socket_states[fd] = HttpRequest();
}

void Server::running_loop(HTTPConfig &http_config, sockaddr_in &servaddr)
{
	while (g_running)
	{
		clean_sockets();

		// Timeout de 0.5 secondes pour éviter le blocage
		struct timeval timeout;
		timeout.tv_sec = 0;
		timeout.tv_usec = 500000;

		if (select(_socket_data.max_fd + 1, &_socket_data.ready_sockets, NULL, NULL, &timeout) < 0)
		{
			if (errno == EINTR) continue;
			std::cerr << "Select failed: " << strerror(errno) << std::endl;
			break;
		}

		// Parcours des sockets actifs
		time_t now = time(NULL);
		for (int i = 0; i <= _socket_data.max_fd; ++i) 
		{
			if (FD_ISSET(i, &_socket_data.ready_readsockets))
			{
				if (is_server_socket(i))
				{
					handle_new_connection(i, servaddr); // socket d'écoute
				}
				else
				{
					_socket_states[i].set_time(time(NULL));

					handle_client_request(http_config, i);
					if (_socket_states[i].is_ready())
						_socket_states[i] = HttpRequest();
				}
			}
			if (now - _socket_states[i].get_time() > 5)
			{
				close_msg(i, "Idle connection closed", 0, 0);
				send(i, "HTTP/1.1 408 Request Timeout\r\nContent-Length: 0\r\nConnection: close\r\n\r\n", 63, 0);
				_socket_states.erase(i); // Supprimer l'état de la socket
			}
		}
	}
}

void	Server::clean_sockets()
{
		// Rafraîchir la copie des sockets surveillés
		_socket_data.ready_readsockets = _socket_data.saved_readsockets;

		// Nettoyage préventif des sockets invalides
		for (int i = 0; i <= _socket_data.max_fd; ++i)
		{
			if (FD_ISSET(i, &_socket_data.saved_readsockets))
			{
				int error = 0;
				socklen_t len = sizeof(error);
				if (getsockopt(i, SOL_SOCKET, SO_ERROR, &error, &len) == -1) {
					std::cerr << "getsockopt failed for fd " << i << ": " << strerror(errno) << std::endl;
				}
			}
		}
}

void	Server::update_max_fd(int fd)
{
	if (fd == _socket_data.max_fd)
		while (_socket_data.max_fd > 0 && !FD_ISSET(_socket_data.max_fd, &_socket_data.saved_readsockets))
			_socket_data.max_fd--;
}

int Server::close_msg(int fd, const std::string &message, int err, int return_code)
{

	if (err)
		std::cerr << "\033[31m" << message << " (fd " << fd << ") - ERROR\033[0m" << std::endl;
	else
		std::cout << "\033[32m" << message << " (fd " << fd << ") - CLOSED\033[0m" << std::endl;

	close(fd);
	FD_CLR(fd, &_socket_data.saved_readsockets);
	update_max_fd(fd);
	return return_code;
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
	std::cout << "\n\033[3;31m++ Server shutting down ++\033[0m" << std::endl;
	for (int fd = 0; fd <= _socket_data.max_fd; ++fd)
	{
		if (FD_ISSET(fd, &_socket_data.saved_readsockets))
		{
			std::cout << "[Shutdown] Closing FD " << fd << std::endl;
			close(fd);
		}
	}
	FD_ZERO(&_socket_data.saved_readsockets);
	FD_ZERO(&_socket_data.ready_readsockets);
	_socket_data.max_fd = 0;

	for (std::map<int, int>::iterator it = _port_to_socket_map.begin(); it != _port_to_socket_map.end(); ++it)
	{
		int fd = it->second;
		if (FD_ISSET(fd, &_socket_data.saved_readsockets))
		{
			std::cout << "[Shutdown] Closing server socket on port " << it->first << std::endl;
			close(fd);
			FD_CLR(fd, &_socket_data.saved_readsockets);
		}
	}
	_port_to_socket_map.clear();
}