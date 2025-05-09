#include "ServerConfig.hpp"

ServerConfig::ServerConfig() {
	_server_directives[0] = "listen";
	_server_directives[1] = "host";
	_server_directives[2] = "server_name";
	_server_directives[3] = "error_page";
	_server_directives[4] = "client_max_body_size";
	_server_directives[5] = "root";
	_server_directives[6] = "index";
	_server_directives[7] = "alias";
	_server_directives[8] = "autoindex";
	_server_directives[9] = "allow_methods";
	_server_directives[10] = "cgi_path";
	_server_directives[11] = "cgi_ext";
	_server_directives[12] = "return";
	_server_directives[13] = "rewrite";
	_server_directives[14] = "access_log";
	_server_directives[15] = "error_log";
	_server_directives[16] = "keepalive_timeout";
}

ServerConfig::ServerConfig(const ServerConfig& param, std::string port)
{
	_map_server = param._map_server;
	_location_list = param._location_list;
	_map_server["listen"] = port;
}

ServerConfig::~ServerConfig() {}

int	ServerConfig::get_client_max_body_size()
{
	int client_max_body_size = 0;
	std::map<std::string, std::string>::iterator it = _map_server.find("client_max_body_size");
	if (it != _map_server.end())
	{
		if (!convert(it->second, client_max_body_size))
			std::cerr << "Error: client_max_body_size is not a number!" << std::endl;
		else if (client_max_body_size < 0)
			std::cerr << "Error: client_max_body_size is negative!" << std::endl;
	}
	std::cout << "client_max_body_size: " << client_max_body_size << std::endl;
	return client_max_body_size;
}

bool	ServerConfig::parse_server(std::istringstream &iss, std::string key)
{
	for (int i = 0; i < 17; i++)
	{
		if (key == _server_directives[i])
			return 0;
	}
	std::cerr << "Error: Invalid keyword '" << key << "'!" << std::endl;
	iss >> key;
	return 1;
}

bool	ServerConfig::is_server_variable(std::string key)
{
	for (int i = 0; i < 17; i++)
		if (key == _server_directives[i])
			return true;
	return false;
}

bool	ServerConfig::set_server_values(std::istringstream &iss, std::string key)
{
	if (key == "error_page")
	{
		if (handle_error_page(iss, _map_server))
			return 1;
	}
	else if (key == "listen")
	{
		if (handle_listen(iss, _map_server))
			return 1;
	}
	else if (key == "autoindex")
	{
		if (handle_autoindex(iss, _map_server))
			return 1;
	}
	else if (key == "host")
	{
		if (handle_host(iss, _map_server))
			return 1;
	}
	else if (key == "allow_methods")
	{
		if (handle_allow_methods(iss, _map_server))
			return 1;
	}
	else if (is_server_variable(key))
	{
		if (!_map_server[key].empty())
		{
			std::cerr << "Error: Keyword " << key << " already set!" << std::endl;
			return 1;
		}
		std::string value;
		iss >> value;
		if (!is_valid_to_clean_semicolon(value))
			return 1;
		if (value.at(value.length() - 1) == ';')
		{
			value = clean_semicolon(value);
			_map_server[key] = value;
		}
		else
		{
			std::cerr << "Error: Semicolon is missing for keyword: " << key << std::endl;
			return 1;
		}
		if (!iss.eof())
		{
			std::cerr << "Error: Too many values for keyword: " << key << "!" << std::endl;
			return 1;
		}
    }
	else if (!is_keyword(key, "location"))
		;
	else
	{
		std::cerr << "Error: Invalid keyword: " << key << "!" << std::endl; 
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
	return (atol(_map_server["listen"].c_str()));
}

std::string	ServerConfig::get_string_port_number()
{
	return (_map_server["listen"]);
}

std::string ServerConfig::DEBUG_test()
{
	std::string str("DEBUG function: \n");
	for (std::map<std::string, std::string>::iterator it = _map_server.begin(); it != _map_server.end(); ++it)
	{
		str += it->first + ": " + it->second + "\n";
	}
	return str;
}

std::string ServerConfig::get_server_name()
{
	return _map_server["server_name"];
}

std::map<std::string, std::string> ServerConfig::get_map_server()
{
	return _map_server;
}

std::vector<LocationConfig> ServerConfig::get_location_list()
{
	return _location_list;
}

bool	ServerConfig::duplicate_server(std::map<std::string, ServerConfig> &server_list)
{
	for (std::vector<std::string>::iterator it = ++_listen_ports.begin(); it != _listen_ports.end(); it++)
	{
		ServerConfig server_temp = ServerConfig(*this, *it);
		if (server_list.find(server_temp.get_string_port_number()) == server_list.end())
			server_list[server_temp.get_string_port_number()] = server_temp;
		else if (is_server_name_already_used(server_list, server_temp))
			return 1;
		else
			server_list[server_temp.get_string_port_number() + static_cast<std::string>(":") + server_temp.get_server_name()] = server_temp;
	}
	_listen_ports.clear();
	return 0;
}

bool	ServerConfig::handle_listen(std::istringstream &iss, std::map<std::string, std::string> &_map_server)
{
	std::string value;
	if (!iss.eof())
		iss >> value;
	else
	{
		std::cerr << "Error: Keyword listen needs a port number!" << std::endl;
		return 1;
	}
	while (!value.empty())
	{
		if (value.find_first_not_of("0123456789;") != std::string::npos)
		{
			std::cerr << "Error: value '" << value << "' is invalid for keyword listen!" << std::endl;
			return 1;
		}
		if (value.length() > 6 || atol(value.c_str()) > 65535 || atol(value.c_str()) < 1024)
		{
			std::cerr << "Error: value '" << value << "' is invalid for keyword listen!" << std::endl;
			return 1;
		}
		if (value.find(";") != std::string::npos)
		{
			if (!is_valid_to_clean_semicolon(value))
				return 1;
			value = clean_semicolon(value);
			_listen_ports.push_back(value);
			_map_server["listen"] = _listen_ports.begin()->c_str();
			break ;
		}
		else
		{
			_listen_ports.push_back(value);
			_map_server["listen"] = _listen_ports.begin()->c_str();
		}
		value.empty();
		if (!iss.eof())
			iss >> value;
		else
		{
			std::cerr << "Error: Semicolon is missing for keyword: listen!" << std::endl;
			return 1;
		}
	}
	if (!iss.eof())
	{
		std::cerr << "Error: There are values after ';' for keyword listen!" << std::endl;
		return 1;
	}
	return 0;
}

bool	ServerConfig::handle_host(std::istringstream &iss, std::map<std::string, std::string> &_map_server)
{
	std::string value;
	iss >> value;
	std::string ip_1;
	std::string ip_2;
	std::string ip_3;

	if (!iss.eof())
	{
		std::cerr << "Error: Keyword host has too many values!" << std::endl;
		return 1;
	}
	if (!_map_server["host"].empty())
	{
		std::cerr << "Error: Keyword host already set!" << std::endl;
		return 1;
	}
	if (value.empty())
	{
		std::cerr << "Error: Keyword host needs a value!" << std::endl;
		return 1;
	}
	if (value.at(value.length() - 1) != ';')
	{
		std::cerr << "Error: Semicolon is missing for keyword: host!" << std::endl;
		return 1;
	}
	if (!is_valid_to_clean_semicolon(value))
		return 1;
	value = clean_semicolon(value);
	if (value.find("127.") != 0)
	{
		std::cerr << "Error: Invalid host value '" << value << "', must start by '127.'!" << std::endl;
		return 1;
	}
	value.erase(0, 4);
	if (value.find(".") == std::string::npos)
	{
		std::cerr << "Error '.' is missing!" << std::endl;
		return 1;
	}
	ip_1.assign(value, 0, value.find("."));
	value.erase(0, value.find(".") + 1);
	if (value.find(".") == std::string::npos)
	{
		std::cerr << "Error '.' is missing!" << std::endl;
		return 1;
	}
	ip_2.assign(value, 0, value.find("."));
	value.erase(0, value.find(".") + 1);
	ip_3.assign(value);
	if (ip_1.find_first_not_of("0123456789") != std::string::npos || ip_2.find_first_not_of("0123456789") != std::string::npos || ip_3.find_first_not_of("0123456789") != std::string::npos)
	{
		std::cerr << "Error: Invalid host value, only numbers are allowed!" << std::endl;
		return 1;
	}
	if (ip_1.length() > 3 || ip_2.length() > 3 || ip_3.length() > 3 || ip_1.length() == 0 || ip_2.length() == 0 || ip_3.length() == 0)
	{
		std::cerr << "Error: Invalid host value, range is from 0 to 255!" << std::endl;
		return 1;
	}
	if (atol(ip_1.c_str()) > 255 || atol(ip_2.c_str()) > 255 || atol(ip_3.c_str()) > 255)
	{
		std::cerr << "Error: Invalid host value, range is from 0 to 255!" << std::endl;
		return 1;
	}
	if (ip_1 == "0" && ip_2 == "0" && ip_3 == "0")
	{
		std::cerr << "Error: Invalid host value, 127.0.0.0 isn't allowed!" << std::endl;
		return 1;
	}
	if (ip_1 == "255" && ip_2 == "255" && ip_3 == "255")
	{
		std::cerr << "Error: Invalid host value, 127.255.255.255 isn't allowed!" << std::endl;
		return 1;
	}
	_map_server["host"] = "127." + ip_1 + "." + ip_2 + "." + ip_3;
	return 0;
}
