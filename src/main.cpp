#include "webserv.hpp"
#include "Server.hpp"

void	create_authorized_delete_paths_files(std::map<std::string, ServerConfig> server_list)
{
	for (std::map<std::string, ServerConfig>::iterator it = server_list.begin(); it != server_list.end(); ++it)
	{
		ServerConfig &server = it->second;
		std::string root = server.get_root();
		if (root.empty())
			continue; // Skip if root is not set

		std::string authorized_paths = root + "authorized_paths.txt";
		std::ofstream authorized_delete_paths(authorized_paths.c_str(), std::ios::trunc);
		if (!authorized_delete_paths.is_open())
		{
			std::cerr << "Error: Could not open authorized delete paths file: " << authorized_paths << std::endl;
			continue;
		}
		std::map<std::string, LocationConfig>::iterator it_loc = server.get_location_list().find("/uploads/");
		if (it_loc != server.get_location_list().end())
		{
			LocationConfig upload_location = it_loc->second;
			std::string upload_root = upload_location.get_root();
			authorized_delete_paths << upload_root << std::endl;
		}
		authorized_delete_paths.close();
	}
}

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

	create_authorized_delete_paths_files(http_config.get_server_list());

	Server server;
	
	server.launch_server(http_config); // Start the server

	return (0);
}
