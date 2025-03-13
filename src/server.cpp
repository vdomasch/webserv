/* ************************************************************************** */
/*																			*/
/*														:::	  ::::::::   */
/*   server.cpp										 :+:	  :+:	:+:   */
/*													+:+ +:+		 +:+	 */
/*   By: vdomasch <vdomasch@student.42lyon.fr>	  +#+  +:+	   +#+		*/
/*												+#+#+#+#+#+   +#+		   */
/*   Created: 2025/02/25 09:57:04 by lchapard		  #+#	#+#			 */
/*   Updated: 2025/03/10 13:59:23 by vdomasch		 ###   ########.fr	   */
/*																			*/
/* ************************************************************************** */

#include "../includes/webserv.hpp"

int	initialize_socket(sockaddr_in *servaddr, t_fd_data *socket_data)
{
	int	server_fd;

	// Create socket
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
		return (perror("cannot create socket"), -1); 

	// Initialize server address
	bzero(servaddr, sizeof(*servaddr));
	bzero(socket_data, sizeof(*socket_data));

	servaddr->sin_family = AF_INET; // IPv4
	servaddr->sin_port = htons(SERV_PORT); // Port
	//servaddr->sin_addr.s_addr = htonl(INADDR_ANY); // bind to all ports
	//OR 
	servaddr->sin_addr.s_addr = inet_addr("127.0.0.1"); // if to localhost only

	if (bind(server_fd, (struct sockaddr *)servaddr, sizeof(*servaddr)) < 0) // Bind socket to address
		return (perror("cannot bind to socket"), -1); 

	if (listen(server_fd, 10) < 0) // Listen for incoming connections
		return (perror("Failed to listen ! "), -1);
	return (server_fd);
}

void handle_new_connection(int server_fd, sockaddr_in &servaddr, t_fd_data &s_data, int &max_socket)
{
	// Accept new connection
	socklen_t addr_len = sizeof(servaddr);
	int new_socket = accept(server_fd, (struct sockaddr *)&servaddr, &addr_len);
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
	
	FD_SET(new_socket, &s_data.saved_sockets); // Add new socket to saved_sockets
	if (new_socket > max_socket) // Update max_socket
		max_socket = new_socket;
	printf("New client connected on socket %d\n", new_socket);
}

int main(int argc, char **argv)
{
	int server_fd;	// to store server socket
	struct sockaddr_in servaddr; // to store server address
	t_fd_data s_data; // to keep track of all active sockets
	int max_socket; // to keep track of highest FD and not iterate on all 1024 FDs but only on all active ones

	if (argc != 2 || argv == NULL)
		return (std::cout << "Wrong number of arguments! " << std::endl, 0);

	server_fd = initialize_socket(&servaddr, &s_data); // Initialize socket
	if (server_fd < 0) // Check if socket failed
		return (perror("Cannot bind to socket"), 0);

	FD_ZERO(&s_data.saved_sockets); // Initialize the set
	FD_SET(server_fd, &s_data.saved_sockets); // Add server_fd to the set
	max_socket = server_fd; // Start with server_fd as the highest FD

	// Main loop
	while (true)
	{
		// Print message
		printf("\n\033[31m++ Waiting for new connection ++\033[0m\n\n");
		s_data.ready_sockets = s_data.saved_sockets; // Copy saved_sockets to ready_sockets

		// Check if any socket is ready
		if (select(max_socket + 1, &s_data.ready_sockets, NULL, NULL, NULL) < 0)
			return (perror("Select failed!"), 0);

		// Iterate over all sockets
		for (int i = 0; i <= max_socket; i++)
		{
			// Check if socket is ready
			if (FD_ISSET(i, &s_data.ready_sockets))
			{
				// Check if new connection

				if (i == server_fd) // New connection
				{
					handle_new_connection(server_fd, servaddr, s_data, max_socket);
				}
				else // Existing client
				{
					// Handle existing client
					char buffer[BUFFER_SIZE] = {0};
					ssize_t bytes_read = read(i, buffer, BUFFER_SIZE);

					if (bytes_read <= 0)
					{
						// Client disconnected
						printf("Client on socket %d disconnected.\n", i);
						//close(i);
						FD_CLR(i, &s_data.saved_sockets); // Remove socket from saved_sockets

						// Reduce max_socket safely
						if (i == max_socket)
							while (max_socket > 0 && !FD_ISSET(max_socket, &s_data.saved_sockets))
								max_socket--;
					}
					else
					{
						// Handle client request
						printf("Received: %s\n", buffer);
						write(i, "Hello from server", 18);
					}
				}
			}
		}
	}
	return (0);
}



//else handle client request
						/*if(strncmp(buffer, "index", 5) == 0)
						{
							std::string content;
							std::ifstream file("server_files/index.html");
							std::getline(file, content, '\0');
							std::string response =
								"HTTP/1.1 200 OK\r\n"
								"Date: Mon, 23 May 2005 22:38:34 GMT\r\n"
								"Content-Type: text/html; charset=UTF-8\r\n"
								"Content-Length: 564\r\n"
								"Last-Modified: Wed, 08 Jan 2003 23:11:55 GMT\r\n"
								"Server: Webserv/1.0\r\n"
								"Connection: close\r\n"
								"\r\n" + content;
							write(i, response.data(), response.size()); // Send response
						}
						else
						{
							std::string content;
							std::ifstream file("server_files/error_404.html");
							std::getline(file, content, '\0');
							std::string response =
								"HTTP/1.1 200 OK\r\n"
								"Date: Mon, 23 May 2005 22:38:34 GMT\r\n"
								"Content-Type: text/html; charset=UTF-8\r\n"
								"Content-Length: 564\r\n"
								"Last-Modified: Wed, 08 Jan 2003 23:11:55 GMT\r\n"
								"Server: Webserv/1.0\r\n"
								"Connection: close\r\n"
								"\r\n" + content;
							write(i, response.data(), response.size()); // Send response
						}*/