#include "ServerConfig.hpp"

ServerConfig::ServerConfig() {
	_server_directives[0] = "listen";
	_server_directives[1] = "server_name";
	_server_directives[2] = "root";
	_server_directives[3] = "index";
	_server_directives[4] = "client_max_body_size";
	_server_directives[5] = "allow_methods";
	_server_directives[6] = "error_page";
	_server_directives[7] = "keepalive_timeout";
	_server_directives[8] = "autoindex";
	_server_directives[9] = "error_log";
	_server_directives[10] = "return";
	_server_directives[11] = "rewrite";
	_server_directives[12] = "access_log";
}

ServerConfig::ServerConfig(const ServerConfig& param, std::string port)
{
	_map_server = param._map_server;
	_location_list = param._location_list;
	_map_server["listen"] = port;
}

ServerConfig::~ServerConfig() {}

size_t	ServerConfig::get_client_max_body_size()
{
	int client_max_body_size = 0;
	std::map<std::string, std::string>::iterator it = _map_server.find("client_max_body_size");
	if (it != _map_server.end())
	{
		try { client_max_body_size = convert<int>(it->second); }
		catch (std::exception &e) { return (std::cerr << "Error: client_max_body_size is not a number!" << std::endl, 0); }
		if (client_max_body_size < 0)
			return (std::cerr << "Error: client_max_body_size is negative!" << std::endl, 0);
	}
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
	//else if (key == "host")
	//{
	//	if (handle_host(iss, _map_server))
	//		return 1;
	//}
	else if (key == "allow_methods")
	{
		if (handle_allow_methods(iss, _map_server))
			return 1;
	}
	else if (key == "index")
	{
		if (handle_index(iss, _map_server))
			return 1;
	}
	else if (key == "root")
	{
		if (handle_root(iss, _map_server))
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

bool	ServerConfig::add_location(std::string key)
{
	if (_location_list.count((key)))
	{
		std::cerr << "Error: Location '" << key << "' already set!" << std::endl;
		return false;
	}
	_location_list[key] = LocationConfig();
	return true;
}

bool	ServerConfig::select_current_location_to_parse(std::istringstream &iss, std::string key, std::string current_location_path)
{
	if (_location_list[current_location_path].parse_location(iss, key))
		return 1;
	return 0;
}

unsigned int	ServerConfig::get_uint_port_number()
{
	return (atol(_map_server["listen"].c_str()));
}

std::string	ServerConfig::get_port_number()
{
	return (_map_server["listen"]);
}

bool ServerConfig::get_autoindex()
{
	std::map<std::string, std::string>::iterator it = _map_server.find("autoindex");
	if (it != _map_server.end())
	{
		if (it->second == "on")
			return true;
		else if (it->second == "off")
			return false;
	}
	return false;
}

std::string ServerConfig::DEBUG_test()
{
	std::string str("DEBUG function: \n");
	for (std::map<std::string, std::string>::iterator it = _map_server.begin(); it != _map_server.end(); ++it)
	{
		str += it->first + ": " + it->second + "\n";
	}
	str += "\n";
	for (std::map<std::string, LocationConfig>::iterator it = _location_list.begin(); it != _location_list.end(); ++it)
	{
		str += "Location: " + it->first + "\n";
		str += it->second.DEBUG_test();
	}
	return str;
}

std::string ServerConfig::get_server_name()
{
	return _map_server["server_name"];
}

std::map<std::string, std::string>& ServerConfig::get_map_server()
{
	return _map_server;
}

std::map<std::string, LocationConfig>& ServerConfig::get_location_list()
{
	return _location_list;
}

std::string ServerConfig::get_root()
{
	std::map<std::string, std::string>::iterator it = _map_server.find("root");
	return (it != _map_server.end()) ? it->second : ""; 
}

bool	ServerConfig::duplicate_server(std::map<std::string, ServerConfig> &server_list)
{
	for (std::vector<std::string>::iterator it = ++_listen_ports.begin(); it != _listen_ports.end(); it++)
	{
		ServerConfig server_temp = ServerConfig(*this, *it);
		server_temp._map_server["host"] = _ip_and_ports_association[*it];
		if (server_list.find(server_temp.get_port_number()) == server_list.end())
			server_list[server_temp.get_port_number()] = server_temp;
		else if (is_server_name_already_used(server_list, server_temp))
			return 1;
		else
			server_list[server_temp.get_server_name() + static_cast<std::string>(":") + server_temp.get_port_number()] = server_temp;
	}
	this->_map_server["host"] = _ip_and_ports_association[this->get_port_number()];
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
		std::cerr << "Error: Keyword listen formats are PORT or IP:PORT!" << std::endl;
		return 1;
	}
	if (_map_server.count("listen"))
	{
		std::cerr << "Error: Keyword listen already set!" << std::endl;
		return 1;
	}
	while (!value.empty())
	{
		if (value.find_first_not_of("0123456789;:.") != std::string::npos)
		{
			std::cerr << "Error: listen value '" << value << "' is invalid!" << std::endl;
			return 1;
		}
		else if (value.find_first_of(":") != std::string::npos)
		{
			if (handle_listen_ip_port(value))
				return 1;
		}
		else
		{
			if (handle_listen_port(value))
				return 1;
		}
		if (value.find_first_of(";") != std::string::npos || !(iss >> value))
			break;
	}
	if (!(iss.eof()))
	{
		std::cerr << "Error: There are values after ';' for keyword listen" << std::endl;
		return 1;
	}
	if (value.find_first_of(";") == std::string::npos)
	{
		std::cerr << "Error: Semicolon is missing for keyword: listen!" << std::endl;
		return 1;
	}
	if (_listen_ports.back().find_last_of(";") != std::string::npos)
	{
		if (!is_valid_to_clean_semicolon(_listen_ports.back()))
			return 1;
		_listen_ports.back() = clean_semicolon(_listen_ports.back());
	}
	return 0;
}

bool	ServerConfig::handle_listen_ip_port(std::string &value)
{
	std::string host_value;
	std::string port_value;
	if (value.length() < 12)
	{
		std::cerr << "Error: value '" << value << "' is invalid for keyword listen, format is 'x.x.x.x:xxxx'!" << std::endl;
		return 1;
	}
	if (value.find_first_of(":") != value.find_last_of(":"))
	{
		std::cerr << "Error: Invalid listen value '" << value << "'!" << std::endl;
		return 1;
	}
	host_value.assign(value, 0, value.find(":"));
	value.erase(0, value.find(":") + 1);
	port_value.assign(value, 0, value.find(";"));
	if (handle_listen_port(port_value))
		return 1;
	if (handle_host(host_value))
		return 1;
	_ip_and_ports_association[port_value] = host_value;
	return 0;
}

bool	ServerConfig::handle_listen_port(std::string &value)
{
	if (value.length() > 6 || atol(value.c_str()) > 65535 || atol(value.c_str()) < 1024)
	{
		std::cerr << "Error: value '" << value << "' is invalid for keyword listen!" << std::endl;
		return 1;
	}
	if (_ip_and_ports_association.count(value))
	{
		std::cerr << "Error: Port '" << value << "' already set!" << std::endl;
		return 1;
	}
	std::string value_tmp;
	value_tmp.assign(value, 0, value.find(";"));
	_listen_ports.push_back(value_tmp);
	_map_server["listen"] = _listen_ports.begin()->c_str();
	_ip_and_ports_association[value_tmp] = "0.0.0.0";
	return 0;
}

bool	ServerConfig::handle_host(std::string value)
{
	if (value == "0.0.0.0" || value == "127.0.0.1")
		return 0;
	if (value.at(0) != '1' || value.at(1) != '2' || value.at(2) != '7' || value.at(3) != '.')
	{
		std::cerr << "Error: Invalid ip value '" << value << "!" << std::endl;
		return 1;
	}
	std::string value_tmp = value;
	std::string ip_1;
	std::string ip_2;
	std::string ip_3;
	value_tmp.erase(0, 4);
	if (value_tmp.find(".") == std::string::npos)
	{
		std::cerr << "Error: value_tmp '" << value << "' is invalid for keyword listen, format is 'x.x.x.x'!" << std::endl;
		return 1;
	}
	ip_1.assign(value_tmp, 0, value_tmp.find("."));
	value_tmp.erase(0, value_tmp.find(".") + 1);
	if (value_tmp.find(".") == std::string::npos)
	{
		std::cerr << "Error: value_tmp '" << value << "' is invalid for keyword listen, format is 'x.x.x.x'!" << std::endl;
		return 1;
	}
	ip_2.assign(value_tmp, 0, value_tmp.find("."));
	value_tmp.erase(0, value_tmp.find(".") + 1);
	if (value_tmp.find(".") != std::string::npos)
	{
		std::cerr << "Error: value_tmp '" << value << "' is invalid for keyword listen, format is 'x.x.x.x'!" << std::endl;
		return 1;
	}
	ip_3.assign(value_tmp);
	if (ip_1.length() > 3 || ip_2.length() > 3 || ip_3.length() > 3 || ip_1.length() == 0 || ip_2.length() == 0 || ip_3.length() == 0)
	{
		std::cerr << "Error: Invalid ip value '" << value << "', range is from 0 to 255!" << std::endl;
		return 1;
	}
	if (atol(ip_1.c_str()) > 255 || atol(ip_2.c_str()) > 255 || atol(ip_3.c_str()) > 255)
	{
		std::cerr << "Error: Invalid ip value '" << value << "', range is from 0 to 255!" << std::endl;
		return 1;
	}
	if (ip_1 == "0" && ip_2 == "0" && ip_3 == "0")
	{
		std::cerr << "Error: Invalid ip value, 127.0.0.0 isn't allowed!" << std::endl;
		return 1;
	}
	if (ip_1 == "255" && ip_2 == "255" && ip_3 == "255")
	{
		std::cerr << "Error: Invalid ip value, 127.255.255.255 isn't allowed!" << std::endl;
		return 1;
	}
	return 0;
}

std::string ServerConfig::get_matching_location(const std::string& target, bool &autoindex)  ///issue WAS here, fixed
{
	std::string best_match;
	size_t max_len = 0;
	//
	// int	nb = 0; //debug, nb time in loop

	std::string target_copy = target;
	if (target.at(target.size() - 1) != '/')
		target_copy += '/';
	for (std::map<std::string, LocationConfig>::iterator it = _location_list.begin(); it != _location_list.end(); ++it)
	{
		// printf("(%d)\n\n", ++nb);
		const std::string& loc_path = it->first;
		// printf("-> (%s)\n\n", loc_path.c_str());
		// printf("<-(%s)\n\n", target.c_str());
		if (target.compare(0, loc_path.size(), loc_path) == 0 && loc_path.size() > max_len)
		{
			
			best_match = loc_path;
			max_len = loc_path.size();
			autoindex = it->second.get_autoindex(autoindex);
		}
	}

	if (!best_match.empty())
		return best_match;

	// fallback: location /
	std::map<std::string, LocationConfig>::iterator it = _location_list.find("/");
	if (it != _location_list.end())
	{
		autoindex = it->second.get_autoindex(autoindex);
		std::cout << "Falling back to default location '/'" << std::endl;
		return "/";
	}
	
	throw std::runtime_error("No suitable location found for target: " + target);
}

bool	ServerConfig::select_current_location_to_check_gci(std::string current_location_path)
{
	if (_location_list[current_location_path].check_cgi())
		return 1;
	return 0;
}

bool	ServerConfig::is_allow_methods_declared(std::map<std::string, std::string> &_current_map)
{
	if (_current_map.count("GET") || _current_map.count("POST") || _current_map.count("DELETE"))
		return true;
	return false;
}

void	ServerConfig::set_get(std::map<std::string, std::string> &_current_map)
{
	//_current_map["allow_methods"] = "true";
	_current_map["GET"] = "true";
}