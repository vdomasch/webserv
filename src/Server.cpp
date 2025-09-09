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

int	stock_childpid(int pid, bool replace) //!
{
	static int	saved_pid = 0;

	if (replace == true)
		saved_pid = pid;
	return (saved_pid);
}

Server::Server()
{
	FD_ZERO(&_socket_data.saved_readsockets);
	FD_ZERO(&_socket_data.ready_readsockets);
	FD_ZERO(&_socket_data.ready_readsockets);
	FD_ZERO(&_socket_data.ready_writesockets);
	FD_ZERO(&_socket_data.saved_readsockets);
	FD_ZERO(&_socket_data.saved_writesockets);
	_port_to_socket_map.clear();
	_method_map["GET"] = &get_request;
	_method_map["POST"] = &post_request;
	_method_map["DELETE"] = &delete_request;
	_socket_data.requestedFilePath = "";
	_socket_data.max_fd = -1;
	_socket_data.Content_Length = "default";
	_socket_data.Content_Type = "default";
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
	return "0.0.0.0";
}

bool	Server::is_conflicting_binding(const std::string& ip, std::string port, const std::set<std::string>& already_bound)
{
	std::string this_key = ip + ":" + port;
	std::string any_key = "0.0.0.0:" + port;

	if (ip == "0.0.0.0")
	{
		for (std::set<std::string>::const_iterator it = already_bound.begin(); it != already_bound.end(); ++it)
			if (it->substr(it->find(":")) == ":" + port)
				return true; 
	}
	else	
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
	sockaddr_in servaddr = sockaddr_in();

	for (std::map<std::string, ServerConfig>::iterator it = servers_list.begin(); it != servers_list.end(); ++it)
	{
		ServerConfig server = it->second;
		int port = server.get_uint_port_number();

		if (_port_to_socket_map.find(port) == _port_to_socket_map.end())
		{
			std::string ip = server.get_host_ip();
			std::string str_port = server.get_port_number();
			std::string key = ip + ":" + str_port;

			if (is_conflicting_binding(ip, str_port, _ip_port_bound))
			{
				std::cerr << "Error: conflict: cannot bind to " << key << " because it overlaps with an existing binding." << std::endl;
				continue;
			}

			int server_socket = initialize_server(server, servaddr);
			if (server_socket < 0)
			{
				std::cerr << "Error: Failed to initialize server" << std::endl;
				return;
			}
			_ip_port_bound.insert(key);
			_port_to_socket_map[port] = server_socket;
			_socket_to_port_map[server_socket] = port;
			_socket_states[server_socket] = HttpRequest();
			_socket_states[server_socket].set_is_server_socket(true);
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

	_socket_states[client_socket] = HttpRequest();
	_socket_states[client_socket].set_time(time(NULL));
}

std::string Server::get_server_name(int fd)
{
	std::map<int, int>::iterator it = _socket_to_port_map.find(fd);
	if (it != _socket_to_port_map.end())
	{
		int port = it->second;
		std::string port_str;
		port_str = convert<std::string>(port);

		std::string host = _socket_states[fd].get_header("Host");
		if (host.empty())
			return port_str;

		std::string host_port = host.substr(host.find(":") + 1);
		if (host_port.empty())
			return host + port_str;
		else if (port_str.find_first_not_of("0123456789") != std::string::npos)
			throw std::runtime_error("Invalid port number in Host header: " + host_port);
		else
			return host;
	}
	return ""; 
}

int	Server::reading_data(int fd)
{
	char buffer[BUFFER_SIZE] = {0};
	ssize_t bytes_read;
	do {
		bytes_read = recv(fd, buffer, BUFFER_SIZE, 0);

		
		if (bytes_read < 0)
			continue ;
		if (bytes_read == 0)
		{
			close_msg(fd, "Client Disconnected", 0, 0); //!
			return 0;
		}
		if (bytes_read > 0)
			_socket_states[fd].append_data(std::string(buffer, bytes_read));
	} while (bytes_read > 0);
	if (_socket_states[fd].has_error())
	{
		// _socket_states[fd].print_state_status(); //!
		std::cerr << "Error: Error in request" << std::endl;
		return 2;
	}
	if (!_socket_states[fd].is_ready())
		return 0;
	
	if (_socket_states[fd].get_method().empty())
	{
		std::cerr << "Error: No method found in request" << std::endl;
		return 2;
	}
	return 0;
}

bool	Server::client_body_size_too_large(HttpRequest &request, HTTPConfig &http_config)
{
	size_t max_body_size = request._server.get_location_list()[request._location_name].get_client_max_body_size();
	if (max_body_size == 0)
	{
		max_body_size = request._server.get_client_max_body_size();
		if (max_body_size == 0)
			max_body_size = http_config.get_client_max_body_size();
		if (max_body_size == 0)
			return false;
	}

	size_t	content_length = request.get_content_length();
	if (content_length == 0)
	{
		content_length = request.get_body().size(); // If no Content-Length header, use body size
		if (content_length == 0)
			return false; // No body, so no size issue
	}
	if (content_length > max_body_size)
	{
		std::cerr << "Error: Client body size too large (" << content_length << " > " << max_body_size << ")" << std::endl;
		return true; // Body size exceeds the limit
	}
	return false; // Body size is within the limit
}

void	Server::handle_client_request(HTTPConfig &http_config, int fd)
{
	if (_socket_states[fd]._server_name.empty())
	{
		try { 
		 		_socket_states[fd]._server_name = get_server_name(fd);
				_socket_states[fd]._server = find_current_server(http_config, _socket_states[fd]._server_name);
				_socket_states[fd]._autoindex = _socket_states[fd]._server.get_autoindex();
				_socket_states[fd].set_rootpath(_socket_states[fd]._server.get_root());
		}
		catch (std::exception &e)
		{
			std::cerr << "Error: Getting server name: " << e.what() << std::endl;
			build_response(_socket_states[fd], "404", displayErrorPage("404", http_config, _socket_states[fd], _socket_data), false);
		}
	}
	if (reading_data(fd))
		return (build_response(_socket_states[fd], "400", displayErrorPage("400", http_config, _socket_states[fd], _socket_data), _socket_states[fd].getKeepAlive()));
	if (_socket_states[fd].get_state() == RECEIVING_BODY)
		return ;
	if (_socket_states[fd].get_state() == COMPLETE)
	{
		try { 
			_socket_states[fd]._location_name = find_location_name_and_set_root(_socket_states[fd].get_target(), _socket_states[fd]._server, _socket_states[fd]._location_root, _socket_states[fd]._autoindex);
			if (client_body_size_too_large(_socket_states[fd], http_config))
			{
				std::cerr << "Error: Client body size too large" << std::endl;
				build_response(_socket_states[fd], "413", displayErrorPage("413", http_config, _socket_states[fd], _socket_data), _socket_states[fd].getKeepAlive());
			}
		}
		catch (std::exception &e)
		{
			std::cerr << "Error: Location not found" << std::endl;
			build_response(_socket_states[fd], "404", displayErrorPage("404", http_config, _socket_states[fd], _socket_data), _socket_states[fd].getKeepAlive());
		}
	}



	std::map<std::string, ServerConfig> server_list = http_config.get_server_list();

	if (_socket_states[fd].is_ready())
	{
		std::string method = _socket_states[fd].get_method();
		if (_method_map.count(method))
		{
			_method_map[method](http_config, _socket_states[fd], _socket_data);
		}
		else
		{
			std::cerr << "Error: Method not allowed: " << method << std::endl;
			HttpResponse res;
			res.set_status("405", "Method Not Allowed");
			res.set_body("405 Method Not Allowed");
			res.add_header("Allow", "GET, POST, DELETE");
			res.add_header("Content-Type", "text/plain");
			res.add_header("Content-Length", convert<std::string>(res.get_body().size()));
			res.add_header("Connection", "close");
			_socket_states[fd].set_response(res.generate_response(_socket_states[fd]._is_php_cgi));
		}
	}
	_socket_states[fd]._response_sent = 0;
	_socket_states[fd].set_state(RESPONDING);
}

void Server::running_loop(HTTPConfig &http_config, sockaddr_in &servaddr)
{
	while (g_running)
	{
		clean_sockets();

		struct timeval timeout;
		timeout.tv_sec = SELECT_TIMEOUT_SEC;
		timeout.tv_usec = SELECT_TIMEOUT_USEC;

		if (select(_socket_data.max_fd + 1, &_socket_data.ready_readsockets, &_socket_data.ready_writesockets, NULL, &timeout) < 0)
		{
			if (errno == EINTR) continue;
			std::cerr << "Error: Select failed: " << strerror(errno) << std::endl;
			break;
		}
		unsigned long now = time(NULL);
		for (int i = 3; i <= _socket_data.max_fd; ++i) 
		{
			if (FD_ISSET(i, &_socket_data.ready_readsockets))
			{
				if (is_server_socket(i))
				{
					handle_new_connection(i, servaddr);
				}
				else
				{
					_socket_states[i].set_time(time(NULL));

					if (_socket_states[i].is_finished())
						_socket_states[i] = HttpRequest();
					handle_client_request(http_config, i);
				}
			}

			if (FD_ISSET(i, &_socket_data.ready_writesockets))
			{
				HttpRequest &req = _socket_states[i];
				const std::string &resp = req.get_response();
				ssize_t sent = send(i, resp.c_str() + req._response_sent, resp.size() - req._response_sent, 0);
				if (sent <= 0)
				{
					close_msg(i, "Error sending response", 1, -1);
					_socket_states.erase(i);
					continue ;
				}
				req._response_sent += sent;
				if (req._response_sent == resp.size())
				{
					req.set_state(RESPONDED);
					if (!req.getKeepAlive())
					{
						close_msg(i, "Connection closed (no keep-alive)", 0, 0);
						_socket_states.erase(i);
						continue;
					}
				}
			}

			if (!_socket_states[i].get_is_server_socket() && now - _socket_states[i].get_time() > TIMEOUT_SEC)
			{
				if (_socket_states[i].get_state() != RESPONDED)
				{
					if (send(i, "HTTP/1.1 408 Request Timeout\r\nContent-Length: 19\r\nConnection: close\r\n\r\n408 Request Timeout", 91, 0) < 0)
					{
						std::cerr << "Error: Timeout" << std::endl;
						close_msg(i, "Failed to send timeout response", 1, -1);
						_socket_states.erase(i);
						continue;
					}
					if (!_socket_states[i].getKeepAlive())
						close_msg(i, "Connection closed (no keep-alive)", 0, 0);
					if (is_error(_socket_states[i].get_status_code()))
						close_msg(i, "Error response sent", 1, _socket_states[i].get_status_code());
				}
				close_msg(i, "Idle connection closed", 0, 0);
				_socket_states.erase(i);
			}
		}
	}
}

void	Server::clean_sockets()
{
		// Update the ready readsockets set
		_socket_data.ready_readsockets = _socket_data.saved_readsockets;

		// Check for invalid sockets
		for (int i = 0; i <= _socket_data.max_fd; ++i)
		{
			if (FD_ISSET(i, &_socket_data.saved_readsockets))
			{
				int error = 0;
				socklen_t len = sizeof(error);
				if (getsockopt(i, SOL_SOCKET, SO_ERROR, &error, &len) == -1)
				{
					std::cerr << "Error: Socket option failed for fd " << i << ": " << strerror(errno) << std::endl;
					close_msg(i, "Cleaning invalid socket", 1, 0);
					_socket_states.erase(i);
				}
			}
		}
		FD_ZERO(&_socket_data.ready_writesockets);
		for (std::map<int, HttpRequest>::iterator it = _socket_states.begin(); it != _socket_states.end(); ++it)
		{
			int fd = it->first;
			HttpRequest &req = it->second;
			if (!req.get_response().empty() && req.get_state() == RESPONDING)
				FD_SET(fd, &_socket_data.ready_writesockets);
		}
}

void	Server::update_max_fd(int fd)
{
	// Update the maximum file descriptor if necessary
	if (fd == _socket_data.max_fd)
		while (_socket_data.max_fd > 0 && !FD_ISSET(_socket_data.max_fd, &_socket_data.saved_readsockets))
			_socket_data.max_fd--;
}

int Server::close_msg(int fd, const std::string &message, int err, int code)
{
	if (err)
		std::cerr << "\033[31m" << message << " (fd " << fd << ") - ERROR " << code << "\033[0m" << std::endl;
	else
		std::cout << "\033[32m" << message << " (fd " << fd << ") - CLOSED\033[0m" << std::endl;

	close(fd);
	FD_CLR(fd, &_socket_data.saved_readsockets);
	update_max_fd(fd);
	return code;
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