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

	if ((my_socket = accept(server_fd, (struct sockaddr *)servaddr, (socklen_t*)&addr_len))<0)
	{
		perror("In accept");
		return(0); //exit ?
	}
	return (my_socket);
}

int main(int argc, char **argv)
{

	(void)argv;
	int my_socket;
	int	server_fd;
	struct sockaddr_in servaddr;
	t_fd_data s_data; // to set select

	if (argc != 2)
	{
		std::cout << "Wrong number of arguments ! " << std::endl;
		return (0);
	}	

	server_fd = initialize_socket(&servaddr, &s_data);
	if (server_fd < 0)
	{
		perror("cannot bind to socket");
		return (0);
	}
	FD_ZERO(&s_data.saved_sockets);
	FD_SET(server_fd, &s_data.saved_sockets);
	while(42)
	{	
		printf("\n\033[31m++ Waiting for new connection ++\033[0m\n\n");
		s_data.ready_sockets = s_data.saved_sockets;
		if (select(FD_SETSIZE, &s_data.ready_sockets, NULL, NULL, NULL) < 0)
		{
			perror("Select failed ! ");
			return (0);
		}

		for (int i = 0; i < FD_SETSIZE; i++)
		{
			if (FD_ISSET(i, &s_data.ready_sockets))
			{
				if (i == server_fd)
				{
					my_socket = accept_connexion(server_fd, &servaddr);
					FD_SET(my_socket, &s_data.saved_sockets); //add new connection to current set
				}
				else
					printf("Nuhhh uhhh\n");
			}
		}

				
		char buffer[BUFFER_SIZE] = {0};
		const char *mess = "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 12\n\nHello world!";
		if (read(my_socket , buffer, BUFFER_SIZE) < 0)
		{
			perror("Failed to read ! ");
			return (0);
		}
		printf("------------------------\n");
		printf("%s\n",buffer );
		printf("------------------------\n");
		write(my_socket , mess , strlen(mess));
		std::cout << "message sent from server !\n" << std::endl;
		close(my_socket);
	}
	return (0);
}
