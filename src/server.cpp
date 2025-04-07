/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lchapard <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/25 09:57:04 by lchapard          #+#    #+#             */
/*   Updated: 2025/02/25 09:57:05 by lchapard         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/webserv.hpp"

int	initialize_socket(sockaddr_in *servaddr, t_fd_data *socket_data)
{
	int	server_fd;


	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
	{
		perror("cannot create socket"); 
		return (-1); 
	}
	
	bzero(servaddr, sizeof(*servaddr));
	bzero(socket_data, sizeof(*socket_data));

	servaddr->sin_family = AF_INET;
	servaddr->sin_port = htons(SERV_PORT);
	//servaddr->sin_addr.s_addr = htonl(INADDR_ANY); // bind to all ports
	//OR 
	servaddr->sin_addr.s_addr = inet_addr("127.0.0.1"); // if to localhost only

	if (bind(server_fd, (struct sockaddr *)servaddr, sizeof(*servaddr)) < 0)
	{
		perror("cannot bind to socket"); 
		return (-1); 
	}
	if (listen(server_fd, 10) < 0)
	{
		perror("Failed to listen ! ");
		return (-1);
	}
	return (server_fd);
}



int accept_connexion(int server_fd, sockaddr_in *servaddr)
{
	int	my_socket;
	int	addr_len = sizeof(servaddr);

	//here servaddr is the connecting ip
	if ((my_socket = accept(server_fd, (struct sockaddr *)servaddr, (socklen_t*)&addr_len))<0)
	{
		perror("In accept");
		return(0); //exit ?
	}
	return (my_socket);
}

char	find_method_name(std::string request_string)
{
	std::string method;
	int			i;

	method = request_string.substr(0, request_string.find(' '));

	std::string	(method_dictionary[4]) = {"GET", "POST", "DELETE", "PUT"};

	i = -1;
	while(++i < 4)
	{
		if (method == method_dictionary[i])
			break ;
	}

	
	switch (i)
	{
	case 0:
		return ('G');
	case 1:
		return ('P');
	case 2:
		return ('P');
	case 3:
		return ('U');
	default:
		return ('G');
	}

}

char	analyse_request(char buffer[BUFFER_SIZE], bool full)
{
	std::string request(buffer);
	std::string first_line;
	std::string requested_file;
	char		method_name;
	size_t		filename_start;
	size_t		filename_end;

	method_name = find_method_name(request);

	if (full)
	{
		printf("\033[34m------------------------\n");
		printf("%s\n", buffer);
		printf("------------------------\033[0m\n");
		return ('n');
	}
	else
	{

		first_line = request.substr(0, request.find('\n')); // doesn´t work if curl
		filename_start = first_line.find_first_of(' ');
		filename_end = first_line.find_last_of(' ');
		requested_file = first_line.substr(filename_start, filename_end - filename_start);
		printf("\033[34m------------------------\n");
		printf("%s\n",requested_file.c_str() );
		printf("------------------------\033[0m\n");
		return ('n');
	}
}

int	handle_client_request(int socket)
{
	char buffer[BUFFER_SIZE] = {0};
	
	//Receive the new message : 
	if (read(socket , buffer, BUFFER_SIZE) < 0)
	{
		perror("Failed to read ! ");
		return (-1);
	}
	analyse_request(buffer, true); // decide hoz to interpret the request
	
	//Sending a response : 

	//const char *mess = "HTTP/1.1 200 OK\nContent-Type: text/plain Content-Length: 12\n\n <html>\n<body>\n Hey ! <form method=\"GET\"> <input type=text name=\"birthyear\"> <input type=submit name=press value=\"OK\"> </form></body>\n</html>\n";
	const char *mess = "HTTP/1.1 200 OK\nContent-Type: text/html\r\n\r\n\n<html>\n<body>\n Hey ! <form method=\"POST\"> <input type=text name=\"birthyear\"> <input type=submit name=press value=\"OK\"> </form></body>\n</html>\n";
	write(socket , mess , strlen(mess));
	std::cout << "message sent from server !\n" << std::endl;
	close(socket);
	return (0);
}

int main(int argc, char **argv)
{

	(void)argv;
	int my_socket;
	int	server_fd;
	struct sockaddr_in servaddr;
	t_fd_data s_data; // to set select	

	server_fd = initialize_socket(&servaddr, &s_data);
	if (server_fd < 0)
	{
		perror("cannot bind to socket");
		return (0);
	}
	FD_ZERO(&s_data.saved_sockets);
	FD_SET(server_fd, &s_data.saved_sockets);
	s_data.max_sckt_fd = server_fd;
	while(42)
	{
		printf("\n\033[31m++ Waiting for new connection ++\033[0m\n\n");
		s_data.ready_sockets = s_data.saved_sockets;
		if (select(s_data.max_sckt_fd + 1, &s_data.ready_sockets, NULL, NULL, NULL) < 0)
		{
			perror("Select failed ! ");
			return (0);
		}

		for (int i = 0; i <= s_data.max_sckt_fd ; i++)
		{
			if (FD_ISSET(i, &s_data.ready_sockets))
			{
				printf("\n\033[32m========= i = %d =========\033[0m\n\n", i);
				if (i == server_fd) // there is a new connection available on the server socket
				{
					my_socket = accept_connexion(server_fd, &servaddr); // accept the new connection
					FD_SET(my_socket, &s_data.saved_sockets); //add new connection to current set
					printf( "i is %d, server_fd is %d, my_socket is %d\n", i, server_fd, my_socket);
					printf( "request from server_fd : %d\n", my_socket);
					if (my_socket > s_data.max_sckt_fd) // to set the new max
						s_data.max_sckt_fd = my_socket;
				}
				else
				{
					printf( "request from client %d : \n", i);
					handle_client_request(i);
					FD_CLR(i, &s_data.saved_sockets);
				}
			}
		}
		
		my_socket = -1;
	}
	return (argc * 0);
}
