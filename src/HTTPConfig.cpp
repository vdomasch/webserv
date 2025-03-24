#include "HTTPConfig.hpp"

HTTPConfig::HTTPConfig(): _error_page("UNSET"), _client_max_body_size("UNSET") {}

HTTPConfig::~HTTPConfig() {}

bool	HTTPConfig::parse_http()
{
	std::string line;
	std::ifstream infile(_filename.c_str());
	if (!infile.is_open())
	{
		std::cout << "Error, failed to open filename!" << std::endl;
		return ("NULL");
	}
	while (std::getline(infile, line)) {	
        std::istringstream iss(line);  
        std::string key;
		iss >> key;
		
		if (key == "http")
		{
			set_http_values(iss);
		}
		else if (key == "server")
		{
			_server_list.push_back(ServerConfig());
			//_server_list.back().server_config(iss);
		}

		//std::cout << std::endl;
	}
	return (false);
}

void	HTTPConfig::set_http_values(std::istringstream &iss)
{
	std::string value;
    while (iss >> value) {
        std::cout << "HTTP Directive: " << value << std::endl;
    }

}

void	HTTPConfig::set_filename(std::string filename)
{
	_filename = filename;
}