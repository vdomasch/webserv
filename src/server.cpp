#include "../includes/webserv.hpp"

int	initialize_socket(sockaddr_in *servaddr, t_fd_data *socket_data)
{
	int	server_fd;
	int intopt = 1;

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

	setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT | SO_REUSEADDR, &intopt, sizeof(intopt));
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

std::string	openAndReadFile(std::string file, int *errcode)
{
	char	buffer[BUFFER_SIZE];
	int		bytes_read;
	int		fd;

	fd = open(file.c_str(), O_RDONLY);	
	if (fd < 0)
	{
		*errcode = -1;
		return ("void");
	}
	bytes_read = read(fd, buffer, BUFFER_SIZE);
	if (bytes_read < 0)
	{
		*errcode = -1;
		close(fd);
		return ("void");
	}
	// printf("\033[36m->Bytes read : (%d)\n\033[0m\n", bytes_read);
	*errcode = 0;
	if (file == "/home/lchapard/Documents/Webserv/server_files/favicon.ico")
		*errcode = 2;
	close(fd);
	std::string response(buffer);
	memset(buffer, '\0', sizeof(buffer)); // useless ? -> it's not ???
	return (response);
}

int	checkObjectType(std::string filename, t_fd_data *d, int *errcode)
{
	struct stat fileinfo;  
	std::string current_pwd(getcwd(NULL, 0));
	std::string pathToCheck;
	std::string	fileContent;
	

	if (filename == "/") // then redirect to index  --> to check ??
	{
		d->requestedFilePath = "";
		return (IS_INDEXDIR);
	}
	pathToCheck = current_pwd + d->serverFolder + filename; // we need to check if len > 0 before ? 
	// printf("\033[35m(%s)\n\033[0m\n", pathToCheck.c_str());

    if (stat (pathToCheck.c_str(), &fileinfo) == 0) // then file exists --> to secure better, check requestedFilePath too
		printf("\033[34mFound it !\n\033[0m\n");
	else
		printf("\033[34mGone :(\n\033[0m\n");
	d->requestedFilePath = pathToCheck;
	return (IS_EXISTINGFILE);
}

std::string	analyse_request(char buffer[BUFFER_SIZE], bool full, t_fd_data *d, int *errcode)
{
	std::string request(buffer);
	std::string first_line;
	std::string requested_file;
	std::string response;
	char		objType;
	char		method_name;
	size_t		filename_start;
	size_t		filename_end;

	// method_name = find_method_name(request);
	first_line = request.substr(0, request.find('\n')); // doesn´t work if curl
	filename_start = first_line.find_first_of(' ');
	filename_end = first_line.find_last_of(' ');
	requested_file = first_line.substr(filename_start + 1, filename_end - filename_start - 1);
	
	if (full)
	{
		printf("\033[34m------------------------\n");
		printf("%s\n", buffer);
		printf("------------------------\033[0m\n");
	}
	else
	{
		printf("\033[34m------------------------\n");
		printf("(%s)\n",requested_file.c_str() );
		printf("------------------------\033[0m\n");
		
	}

	objType = checkObjectType(requested_file, d, errcode); // is it a file ? or a folder ?

	if (objType == IS_INDEXDIR)
		response = openAndReadFile(d->requestedFilePath, errcode);
	else
		response = openAndReadFile(d->requestedFilePath, errcode);	
	return (response);
}


std::ifstream::pos_type filesize(const char *filename)
{
    std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
    return in.tellg(); 
}

std::string	defineRequestHeaderResponseCode(int errcode, std::string requestBody, t_fd_data *d)
{
	std::string	responseCode;
	std::ifstream::pos_type dataFile;
	std::ostringstream oss;


	//--------------------------------------------------------//
    std::cout << "body size is " << requestBody.length() << "\n"; //temporary until ico zorks in binary
	if (requestBody.length() == 0)
	{
		std::cout << "I'm out ! 1.3 sec\n" << std::endl;
		return (""); 
	}
	//--------------------------------------------------------//

	dataFile = filesize(d->requestedFilePath.c_str());
	oss << dataFile;

	switch (errcode)
	{
	case 0:
		responseCode = "HTTP/1.1 200 OK\nContent-Type: text/html\r\nContent-Lenght: ";
		responseCode.append(oss.str());
		responseCode.append("\r\n\r\n\n");
		break;
	
	case 2:
		responseCode = "HTTP/1.1 200 OK\nContent-Type: image/x-icon\r\nContent-Lenght: ";
		responseCode.append(oss.str());
		responseCode.append("\r\n\r\n\n");
		break;

	default:
		responseCode = "HTTP/1.1 200 OK\nContent-Type: text/html\r\nContent-Lenght: ";
		responseCode.append(oss.str());
		responseCode.append("\r\n\r\n\n");
		break;
	}
	responseCode = responseCode + requestBody;
	return (responseCode);
}

int	handle_client_request(int socket, t_fd_data *d)
{
	char buffer[BUFFER_SIZE] = {0}; /// in this case, header size is included in buffer_size = bad ?????
	std::string	requestBody;
	std::string	finalMessage;
	int			errcode;
	ssize_t 	bytesRead;
	
	//Receive the new message : 
	bytesRead = read(socket , buffer, BUFFER_SIZE);
	if (bytesRead < 0)
	{
		perror("Failed to read ! ");
		return (-1);
	}
	requestBody = analyse_request(buffer, false, d, &errcode); // decide how to interpret the request
	memset(buffer, '\0', sizeof(buffer));

	//Sending a response :
	finalMessage = defineRequestHeaderResponseCode(errcode, requestBody, d);

	printf("\033[35m------------------------\n");
	printf("(%s)\n",finalMessage.c_str() );
	printf(" \nERROR CODE(%d)\n",errcode);
	printf("------------------------\033[0m\n");

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// std::ifstream file("/home/lchapard/Documents/Webserv/server_files/favicon.ico", std::ios::binary);
	// if (!file.is_open()) 
	// 	std::cerr << "Could not open .ico file" << std::endl;
	// else 
	// {
	// 	file.seekg(0, std::ios::end);
	// 	size_t file_size = file.tellg();
	// 	file.seekg(0, std::ios::beg);
		
	// 	std::vector<char> buffer2(file_size);
	// 	file.read(&buffer2[0], file_size);
	// 	file.close();
		
	// 	std::ostringstream response;
	// 	response << "HTTP/1.1 200 OK\r\n"
	// 	<< "Content-Type: image/x-icon\r\n"
	// 	<< "Content-Length: " << file_size << "\r\n"
	// 	<< "\r\n";
		
	// 	send(socket, response.str().c_str(), response.str().length(), 0); // check if all the bytes were sent ?
	// 	send(socket, &buffer2[0], buffer2.size(), 0);
	// }
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	
	if (!finalMessage.empty())
	{
		//write(socket , finalMessage.c_str() , strlen(finalMessage.c_str()));
		if (send(socket , finalMessage.c_str() , finalMessage.length(), 0) == -1)
			printf ("\033[34mI COULN'T SEND IT ALL MB MB\n\n\033[0m");
		std::cout << "message sent from server !\n" << std::endl;
	}
	
	

	//clean the buffer  ?

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

	s_data.serverFolder = "/server_files"; //after parsing
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
					handle_client_request(i, &s_data);
					FD_CLR(i, &s_data.saved_sockets);
				}
			}
		}
		
		my_socket = -1;
	}
	return (argc * 0);
}
