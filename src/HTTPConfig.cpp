#include "HTTPConfig.hpp"

HTTPConfig::HTTPConfig(): _is_http(false), _is_server(false), _is_location(false)
{
	_map_http["client_max_body_size"] = "UNSET";
}

HTTPConfig::~HTTPConfig() {}

bool HTTPConfig::is_http(std::string key)
{
	if (key == "}" && _is_http)
	{
		_is_http = false;
		return true;
	}
	if (_is_server || _is_location)
		return false;
	if (key == "http" && _is_http)
		return false;
	else if (key == "http" || _is_http)
	{
		_is_http = true;
		return true;
	}
	return false;
}

bool HTTPConfig::is_server(std::string key)
{
	if (key == "}" && _is_server)
	{
		_is_server = false;
		return true;
	}
	if (_is_location)
		return false;
	if (key == "server" && _is_server)
		return false;
	if (key == "server" || _is_server)
	{
		_is_server = true;
		return true;
	}
	return false;
}

bool HTTPConfig::is_location(std::string key)
{
	if (key == "}" && _is_location)
	{
		_is_location = false;
		return true;
	}
	if (key == "location" && _is_location)
		return false;
	if (key == "location" || _is_location)
	{
		_is_location = true;
		return true;
	}
	return false;
}

bool	HTTPConfig::parse_http()
{
	ServerConfig server_temp;
	int	location_number = -1;

	std::string line;
	std::ifstream infile(_filename.c_str());
	if (!infile.is_open())
	{
		std::cerr << "Error: Failed to open filename '" << _filename.c_str() << "'!" << std::endl;
		return (true);
	}
	while (std::getline(infile, line)) {
        std::istringstream iss(line);  
        std::string key;
		iss >> key;

		if (is_location(key))
		{
			if (key == "location")
			{
				location_number++;
				if (!is_location_valid(iss))
					return 1;
				iss >> key;
				server_temp.add_location(key, location_number);
			}
			else if (!key.empty() && key != "}")
			{
				if (server_temp.select_current_location(iss, key, location_number))
					return 1;
			}
		}
		else if (is_server(key))
		{
			if (key == "server")
			{
				server_temp = ServerConfig();
				location_number = -1;
				if (!(iss >> key))
				{
					std::cerr << "Error: Keyword server need an openning bracket '{'!" << std::endl;
					return 1;
				}
				if (key != "{")
				{
					std::cerr << "Error: Keyword server must be followed by '{'!" << std::endl;
					return 1;
				}
				if (!iss.eof())
				{
					std::cerr << "Error: Keyword server has too many values!" << std::endl;
					return 1;
				}
			}
			else if (!key.empty() && key != "}")
			{
				if (server_temp.parse_server(iss, key))
					return 1;
				if (server_temp.set_server_values(iss, key))
					return 1;
			}
			else if (key == "}")
			{
				if (are_mandatory_directives_missing(server_temp))
					return 1;
				if (server_temp.duplicate_server(_server_list))
					return 1;
				if (_server_list.find(server_temp.get_string_port_number()) == _server_list.end())
					_server_list[server_temp.get_string_port_number()] = server_temp;
				else if (is_server_name_already_used(_server_list, server_temp))
					return 1;
				else
					_server_list[server_temp.get_string_port_number() + static_cast<std::string>(":") + server_temp.get_server_name()] = server_temp;
			}
		}
		else if (is_http(key))
		{
			if (set_http_values(iss, key))
				return 1;
		}
		else
		{
			std::cerr << "Error: Element declared incorrectly!" << std::endl;
			return 1;
		}
	}
	if (_is_location)
	{
		std::cerr << "Error: Closing bracket for location is missing!" << std::endl;
		return 1;
	}
	if (_is_server)
	{
		std::cerr << "Error: Closing bracket for server is missing!" << std::endl;
		return 1;
	}
	if (_is_http)
	{
		std::cerr << "Error: Closing bracket for http is missing!" << std::endl;
		return 1;
	}
	if (_server_list.empty())
	{
		std::cerr << "Error: File '" << _filename.c_str() << "' is empty!" << std::endl;
		return 1;
	}
	return 0;
}

bool	HTTPConfig::set_http_values(std::istringstream &iss, std::string key)
{
	std::string value;

	if (key == "client_max_body_size")
	{
		if (_map_http["client_max_body_size"] != "UNSET")
		{
			std::cerr << "Error: Keyword client_max_body_size already set!" << std::endl;
			return 1;
		}
		iss >> value;
		if (!is_valid_to_clean_semicolon(value))
					return 1;
		value = clean_semicolon(value);
		_map_http[key] = value;
		if (iss >> value)
		{
			std::cerr << "Error: Keyword client_max_body_size has too many values!" << std::endl;
			return 1;
		}
    }
	else if (key == "error_page")
	{
		if (handle_error_page(iss, _map_http))
			return 1;
	}
	else if (is_keyword(key, "http"))
	{
		if (iss >> key && key != "{")
			return 1;
		if (!iss.eof())
		{
			std::cerr << "Error: Keyword http has too many values!" << std::endl;
			return 1;
		}
	}
	else
	{
		std::cerr << "Error: Invalid keyword: " << key << "!" << std::endl; 
		return 1;
	}
	return 0;
}

bool	HTTPConfig::is_http_variable(std::string key)
{
	if (key == "error_page")
		return true;
	return false;
}

void	HTTPConfig::set_filename(std::string filename)
{
	_filename = filename;
}

bool	HTTPConfig::is_location_valid(std::istringstream &iss)
{
	std::string str = iss.str();
    std::istringstream iss_copy(str);
	std::string key;

	unsigned int count = 0;
	while (iss_copy >> key)
	{
		if (count == 1 && key.at(0) != '/')
		{
			std::cerr << "Error: Keyword location path must start with '/'!" << std::endl;
			return false;
		}
		count++;
	}
	if (count < 2)
	{
		std::cerr << "Error: Keyword location has no path!" << std::endl;
		return false;
	}
	else if (count < 3)
	{
		std::cerr << "Error: Keyword location needs an opening bracket!" << std::endl;
		return false;
	}
	else if (count > 3)
	{
		std::cerr << "Error: Keyword location has too many values!" << std::endl;
		return false;
	}
	return true;
}

void	HTTPConfig::DEBUG_HTTP_show()
{
	std::cout << "DEBUG: show everything contained in servers" << std::endl;
	std::cout << std::endl;
	for (std::map<std::string, ServerConfig>::iterator it = _server_list.begin(); it != _server_list.end(); ++it)
	{
		std::cout << "Server port: " << it->first << std::endl;
		std::cout << it->second.DEBUG_test() << std::endl;
	}
}

bool	HTTPConfig::are_mandatory_directives_missing(ServerConfig &server_temp)
{
	std::map<std::string, std::string> server_map = server_temp.get_map_server();
	if (server_map.find("listen") == server_map.end())
	{
		std::cerr << "Error: Mandatory keyword 'listen' missing!" << std::endl;
		return true;
	}
	if (server_map.find("index") == server_map.end())
	{
		std::cerr << "Error: Mandatory keyword 'index' missing!" << std::endl;
		return true;
	}
	if (server_map.find("root") == server_map.end())
	{
		std::cerr << "Error: Mandatory keyword 'root' missing!" << std::endl;
		return true;
	}
	return false;
}
