#include "Server.hpp"
#include "HTTPConfig.hpp"
#include <map>

Server::Server(): _server_fd(-1), _client_fd(-1) {}
Server::~Server() {}

bool g_running = true;

void	handle_signal(int signum)
{
	if (signum == SIGINT)
	{
		std::cout << "\n\033[31m++ Server shutting down ++\033[0m\n" << std::endl;
		g_running = false;
	}
}

int	Server::initialize_server(HTTPConfig &http_config)
{
	std::vector<ServerConfig> server_list = http_config.get_server_list();
	
	for (std::vector<ServerConfig>::iterator it = server_list.begin(); it != server_list.end(); ++it)
	{
		int port = it;
		
	}
	bzero(&_servaddr, sizeof(_servaddr));
	bzero(&_socket_data, sizeof(_socket_data));

	_servaddr.sin_family = AF_INET;
	
}

void	Server::run_server(HTTPConfig &http_config)
{
	_server_fd = initialize_server(http_config);
	if (_server_fd < 0)
		return (perror("Cannot bind to socket"), void());

	FD_ZERO(&_socket_data.saved_sockets);
	FD_SET(_server_fd, &_socket_data.saved_sockets);
	_socket_data.max_fd = _server_fd;

	signal(SIGINT, handle_signal);
	
	while (g_running)
	{
		static_cast<void>(http_config);
	}
}