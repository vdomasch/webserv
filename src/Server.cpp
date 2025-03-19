#include "Server.hpp"

Server::Server() {}
Server::~Server() {}

int	Server::initialize_server()
{
	// Create socket
	if ((_server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
		return (perror("cannot create socket"), -1); 

	// Initialize server address
	bzero(&_servaddr, sizeof(_servaddr));
	bzero(&_socket_data, sizeof(_socket_data));

	_servaddr.sin_family = AF_INET; // IPv4
	_servaddr.sin_port = htons(SERV_PORT); // Port
	//servaddr->sin_addr.s_addr = htonl(INADDR_ANY); // bind to all ports
	//OR 
	_servaddr.sin_addr.s_addr = inet_addr("127.0.0.1"); // if to localhost only

	if (bind(_server_fd, (struct sockaddr *)&_servaddr, sizeof(_servaddr)) < 0) // Bind socket to address
		return (perror("cannot bind to socket"), -1); 

	if (listen(_server_fd, 10) < 0) // Listen for incoming connections
		return (perror("Failed to listen ! "), -1);
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
	t_browser_request request;
	// Handle existing client
	char buffer[BUFFER_SIZE] = {0};
	ssize_t bytes_read = recv(_client_fd, buffer, BUFFER_SIZE, 0);

	//std::cout << "Bytes read: " << bytes_read << std::endl;
	request.bytes_read = bytes_read;

	if (bytes_read == 0)
	{
		// Client disconnected
		printf("Client on socket %d disconnected.\n", _client_fd);
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

		printf("Received: %s\n", buffer);
		parse_request(request, buffer, _client_fd);

		// Check if the client requested to keep the connection alive
		std::string response = "HTTP/1.1 200 OK\r\nContent-Length: 17\r\nContent-Type: text/plain\r\n";
		if (strstr(buffer, "Connection: keep-alive"))
			response += "Connection: keep-alive\r\n";
		else
			response += "Connection: close\r\n";
		response += "\r\nHello from server";

		// Send the response
		write(_client_fd, response.c_str(), response.size());

		// Close the connection if not keep-alive
		if (request.connection != "keep-alive")
		{
			close(_client_fd);
			FD_CLR(_client_fd, &_socket_data.saved_sockets);
			if (_client_fd == _max_fd)
				while (_max_fd > 0 && !FD_ISSET(_max_fd, &_socket_data.saved_sockets))
					_max_fd--;
		}
		send(_client_fd, "Hello from server", 17, 0);
	}
}

void	Server::run_server()
{
	_server_fd = initialize_server(); // Initialize socket
	if (_server_fd < 0) // Check if socket failed
		return (perror("Cannot bind to socket"), void());

	FD_ZERO(&_socket_data.saved_sockets); // Initialize the set
	FD_SET(_server_fd, &_socket_data.saved_sockets); // Add server_fd to the set
	_max_fd = _server_fd; // Start with server_fd as the highest FD

	// Main loop
	while (true)
	{
		// Print message
		printf("\n\033[31m++ Waiting for new connection ++\033[0m\n\n");
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