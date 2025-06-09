#include "webserv.hpp"
#include "Server.hpp"

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		std::cerr << "Please execute as ./Webserv \"config_file_name\"!" << std::endl;
		return (1);
	}
	std::string config_variables[7] = {"listen", "host", "server_name", "error_page", "client_max_body_size", "root", "index"};
	HTTPConfig http_config;
	http_config.set_filename(argv[1]);
	if (http_config.parse_http())
		return (1);
	http_config.DEBUG_HTTP_show();

	std::cout << "root of '/': " << http_config.get_server_list()["9090"].get_location_list()["/"].get_root() << std::endl;

	Server server;
	
	server.launch_server(http_config); // Start the server

	return (0);
}
