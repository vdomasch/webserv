#include "Server.hpp"

Server::Server(): _server_fd(-1), _max_fd(-1), _client_fd(-1), _running(true) {}
Server::~Server() {}

int	Server::initialize_server()
{
	// Initialize server address
	bzero(&_servaddr, sizeof(_servaddr));
	bzero(&_socket_data, sizeof(_socket_data));

	_servaddr.sin_family = AF_INET; // IPv4
	_servaddr.sin_port = htons(SERV_PORT); // Port
	//servaddr->sin_addr.s_addr = htonl(INADDR_ANY); // bind to all ports
	//OR 
	_servaddr.sin_addr.s_addr = inet_addr("127.0.0.1"); // if to localhost only

	// Create socket
	if ((_server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		std::cerr << "Failed to create socket" << std::endl;
		return (-1);
	}

	if (bind(_server_fd, (struct sockaddr *)&_servaddr, sizeof(_servaddr)) < 0) // Bind socket to address
	{
		close(_server_fd);
		std::cerr << "Failed to bind to socket" << std::endl;
		return (-1);
	}

	if (listen(_server_fd, 10) < 0) // Listen for incoming connections
	{
		close(_server_fd);
		std::cerr << "Failed to listen" << std::endl;
		return (-1);
	}
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


std::string	write_response(std::string status_code, std::string status_message, std::string body_size, std::string connection, std::string body, t_browser_request &request)
{
	std::string response;
	response += "HTTP/1.1 " + status_code + " " + status_message + "\r\n";
	response += "Content-Type: text/html\r\n";
	response += "Content-Length: " + body_size + "\r\n";
	response += "Connection: " + connection + "\r\n";
	if (request.connection == "keep-alive")
		response += "Keep-Alive: timeout=10, max=1000\r\n";
	response += "\r\n";
	response += body;

	return (response);
}

void	Server::handle_existing_client()
{
	t_browser_request request;
	// Handle existing client
	char buffer[BUFFER_SIZE] = {0};
	ssize_t bytes_read = recv(_client_fd, buffer, BUFFER_SIZE, 0);

	//std::cout << "Bytes read: " << bytes_read << std::endl;
	request.bytes_read = bytes_read;

	if (bytes_read == 0)
	{
		// Client disconnected
		std::cout << "Client on socket" << _client_fd << "disconnected.\n" << std::endl;
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
		parse_request(request, buffer);

		// Check if the client requested to keep the connection alive

		// Prepare response
		std::string body = "<DOCTYPE html>\r\n<html>\r\n\t<head>\r\n\t\t<title>Test</title>\r\n\t</head>\r\n\t<body>\r\n\t\t<h1>Hello World!</h1>\r\n\t</body>\r\n</html>";
		std::string response = write_response("200", "OK", "117", request.connection, body, request);

		// Send the response
		send(_client_fd, response.c_str(), response.size(), 0);

		// Close the connection if not keep-alive
		if (request.connection != "keep-alive")
		{
			close(_client_fd);
			FD_CLR(_client_fd, &_socket_data.saved_sockets);
			if (_client_fd == _max_fd)
				while (_max_fd > 0 && !FD_ISSET(_max_fd, &_socket_data.saved_sockets))
					_max_fd--;
		}
	}
}

void	Server::run_server()
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
			return (perror("Select failed!"), void());

		// Iterate over all sockets
		for (_client_fd = 0; _client_fd <= _max_fd; _client_fd++)
		{
			// Check if socket is ready
			if (FD_ISSET(_client_fd, &_socket_data.ready_sockets))
			{
				// Check if new connection
				if (_client_fd == _server_fd) // New connection
					handle_new_connection();
				else // Existing client
					handle_existing_client();
			}
		}
	}
	return ;
}