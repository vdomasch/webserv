#include "./vinc_includes/Server.hpp"
#include "HTTPConfig.hpp"
#include <map>

Server::Server(): _server_fd(-1), _client_fd(-1) {}
Server::~Server() {}

int	Server::initialize_server()
{
	return (_server_fd);
}

void Server::handle_new_connection()
{
	// Accept new connection
	socklen_t addr_len = sizeof(_servaddr);
	int new_socket = accept(_server_fd, (struct sockaddr *)&_servaddr, &addr_len);
	// Check if accept failed
	if (new_socket < 0)
		return (perror("Failed to accept connection"), void());
	
	// Check if we have space for new socket
	if (new_socket >= FD_SETSIZE)
	{
		close(new_socket);
		fprintf(stderr, "Too many connections!\n");
		return ;
	}
	
	FD_SET(new_socket, &_socket_data.saved_sockets); // Add new socket to saved_sockets
	if (new_socket > _max_fd) // Update max_fd
		_max_fd = new_socket;
	printf("New client connected on socket %d\n", new_socket);
}

void	Server::handle_existing_client()
{
	std::cout << "\n\033[32m++ Handling existing client ++\033[0m\n" << std::endl;
	std::map<int , t_browser_request> request;
	// Handle existing client
	char buffer[BUFFER_SIZE] = {0};
	ssize_t bytes_read = recv(_client_fd, buffer, BUFFER_SIZE, 0);

	//std::cout << "Bytes read: " << bytes_read << std::endl;
	request[_client_fd].bytes_read = bytes_read;

	if (request[_client_fd].connection != "keep-alive" && bytes_read == 0)
	{
		// Client disconnected
		std::cout << "Client on socket " << _client_fd << " disconnected.\n" << std::endl;
		close(_client_fd);
		FD_CLR(_client_fd, &_socket_data.saved_sockets); // Remove socket from saved_sockets

		// Reduce max_socket safely
		if (_client_fd == _max_fd)
			while (_max_fd > 0 && !FD_ISSET(_max_fd, &_socket_data.saved_sockets))
				_max_fd--;
	}
	else
	{
		// Handle client request
		buffer[bytes_read] = '\0'; // Null-terminate string

		std::cout << "Received: " << buffer << std::endl;
		parse_request(request[_client_fd], buffer);

		// Check if the client requested to keep the connection alive

		// Prepare response
		std::string response = generate_http_response("200", "OK", request[_client_fd]);

		// Send the response
		send(_client_fd, response.c_str(), response.size(), 0);

		// Close the connection if not keep-alive
		std::cout << request[_client_fd].connection << std::endl;
		if (static_cast<std::string>(request[_client_fd].connection) /*static_cast<std::string>("keep-alive")*/ != "keep-alive")
		{
			std::cout << "CAN'T NOT STOP Closing connection on socket " << _client_fd << std::endl;
			close(_client_fd);
			FD_CLR(_client_fd, &_socket_data.saved_sockets);
			request.erase(_client_fd);
			if (_client_fd == _max_fd)
				while (_max_fd > 0 && !FD_ISSET(_max_fd, &_socket_data.saved_sockets))
					_max_fd--;
		}
	}
}

void	Server::run_server(HTTPConfig &http_config)
{
	if ((_server_fd = initialize_server()) < 0) // Initialize socket
		return (perror("Cannot bind to socket"), void());

	FD_ZERO(&_socket_data.saved_sockets); // Initialize the set
	FD_SET(_server_fd, &_socket_data.saved_sockets); // Add server_fd to the set
	_max_fd = _server_fd; // Start with server_fd as the highest FD

	// Main loop
	while (_running)
	{
		// Print message
		std::cout << "\n\033[31m++ Waiting for new connection ++\033[0m\n" << std::endl;
		_socket_data.ready_sockets = _socket_data.saved_sockets; // Copy saved_sockets to ready_sockets

		// Check if any socket is ready
		if (select(_max_fd + 1, &_socket_data.ready_sockets, NULL, NULL, NULL) < 0)
		{
			std::cerr << "Select failed!" << std::endl;
			return ;
		}

		// Iterate over all sockets
		for (_client_fd = 0; _client_fd <= _max_fd; _client_fd++)
		{
			// Check if socket is ready
			if (FD_ISSET(_client_fd, &_socket_data.ready_sockets))
			{
				// Check if new connection
				if (_client_fd == _server_fd) // New connection
					;
				else // Existing client
					;
			}
		}
	}
	return ;
}