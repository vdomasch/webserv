#include "webserv.hpp"
#include "Server.hpp"

bool ends_with(const std::string& str, const std::string& suffix)
{
	return str.size() >= suffix.size() &&
	       str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

//static void	authorized_delete_paths(HTTPConfig& http_config)
//{
//	std::map<std::string, ServerConfig>& server_list = http_config.get_server_list();
//	if (!server_list.empty())
//	{
//		for (std::map<std::string, ServerConfig>::iterator it = server_list.begin(); it != server_list.end(); ++it)
//		{
//			ServerConfig &server = it->second;

//			std::map<std::string, LocationConfig> location_list = server.get_location_list();
//			for (std::map<std::string, LocationConfig>::iterator it_loc = location_list.begin(); it_loc != location_list.end(); ++it_loc)
//			{ 
//				LocationConfig &location = it_loc->second;
//				std::map<std::string, std::string> location_map = location.get_map_location();
//				if ((location_map.count("allow_methods") && location_map.count("DELETE")) || server.get_map_server().count("DELETE"))
//				{
//					std::string location_root = location.get_root();
//					if (!location_root.empty())
//					{
//						location_root += "uploads/";
//						server.add_authorized_paths(location_root);
//					}
//				}
//			}
//		}
//	}
//}

int main(int argc, char **argv)
{
	HTTPConfig http_config;
	if (argc == 1)
		http_config.set_filename("configs/server.conf");
	else if (argc != 2)
	{
		std::cerr << "Error: Please execute as ./webserv \"config_file_name\"!" << std::endl;
		return (1);
	}
	else
		http_config.set_filename(argv[1]);
	if (http_config.parse_http())
		return (1);

	//authorized_delete_paths(http_config);

	Server server;
	
	server.launch_server(http_config);
	if (server._socket_data.cg.cgi_forkfd == 0)
		return (42);
	return (0);
}
