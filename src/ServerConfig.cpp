#include "ServerConfig.hpp"

ServerConfig::ServerConfig() {
	_server_directives[0] = "listen";
	_server_directives[1] = "host"; // INVALID?
	_server_directives[2] = "server_name";
	_server_directives[3] = "error_page";
	_server_directives[4] = "client_max_body_size";
	_server_directives[5] = "root";
	_server_directives[6] = "index";
	_server_directives[6] = "listen";
	_server_directives[7] = "host";
	_server_directives[8] = "server_name";
	_server_directives[9] = "error_page";
	_server_directives[10] = "client_max_body_size";
	_server_directives[11] = "root";
	_server_directives[12] = "index";
	_server_directives[13] = "alias";
	_server_directives[14] = "autoindex";
	_server_directives[15] = "allow_methods";
	_server_directives[16] = "cgi_path";
	_server_directives[17] = "cgi_ext";
	_server_directives[18] = "return";
	_server_directives[19] = "rewrite";
	_server_directives[20] = "access_log";
	_server_directives[21] = "error_log";
	_server_directives[22] = "keepalive_timeout";
}

ServerConfig::~ServerConfig() {}

bool	ServerConfig::parse_server(std::istringstream &iss, std::string key)
{
	for (int i = 0; i < 23; i++)
	{
		if (key == _server_directives[i])
			return 0;
	}
	std::cerr << "Error: Invalid keyword: " << key << std::endl;
	iss >> key;
	return 1;

}

bool	ServerConfig::is_server_variable(std::string key)
{
	for (int i = 0; i < 23; i++)
		if (key == _server_directives[i])
			return true;
	return false;
}

bool	ServerConfig::set_server_values(std::istringstream &iss, std::string key)
{
	if (key == "error_page")
	{
		std::vector<std::string> code_numbers;

		iss >> key;
		while (!key.empty())
		{
			if (is_error_page_code(key))
				code_numbers.push_back(key);
			else if (key.find(".html") != std::string::npos && !code_numbers.empty())
				break ;
			else
			{
				std::cerr << "Error: error_page need a valid error number before path!" << std::endl;
				return 1;
			}
			iss >> key;
		}
		while (!code_numbers.empty())
		{
			key = clean_semicolon(key);
			_map_server[code_numbers.back()] = key;
			code_numbers.pop_back();
		}
	}
	else if (key == "listen")
	{
		iss >> key;
		while (!key.empty())
		{
			if (key.find(";") != std::string::npos)
			{
				key = clean_semicolon(key);
				_listen_ports.push_back(key);
				_map_server["listen"] = key;
				break ;
			}
			else
			{
				_listen_ports.push_back(key);
				_map_server["listen"] = key;
			}
			iss >> key;
		}
	}
	else if (is_server_variable(key))
	{
		std::string value;
		iss >> value;
		value = clean_semicolon(value);
		_map_server[key] = value;
    }
	else if (is_keyword(key, "location"))
		;
	else
	{
		std::cerr << "Error: Invalid keyword: " << key << std::endl; 
		return 1;
	}
	return 0;
}

void	ServerConfig::add_location(std::string key, int location_number)
{
	_location_list.push_back(LocationConfig());
	_location_list[location_number].set_path(key);
}

bool	ServerConfig::select_current_location(std::istringstream &iss, std::string key, int location_number)
{
	if (_location_list[location_number].parse_location(iss, key))
		return 1;
	return 0;
}

unsigned int	ServerConfig::get_port_number()
{
	return (atol(_listen_ports[0].c_str()));
}

std::string	ServerConfig::get_string_port_number()
{
	return (_listen_ports[0]);
}

std::string ServerConfig::DEBUG_test()
{
	std::string str = "DEBUG: ";
	for (std::map<std::string, std::string>::iterator it = _map_server.begin(); it != _map_server.end(); ++it)
	{
		str += it->first + ": " + it->second + "\n";
	}
	return str;
}

void	ServerConfig::show()
{
	std::cout << _server_directives[0] << std::endl;
}

std::string ServerConfig::get_server_name()
{
	return _map_server["server_name"];
}

std::map<std::string, std::string> ServerConfig::get_map_server()
{
	return _map_server;
}