#include "LocationConfig.hpp"

LocationConfig::LocationConfig()
{
	_location_directives[0] = "error_page";
	_location_directives[1] = "client_max_body_size";
	_location_directives[2] = "root";
	_location_directives[3] = "index";
	_location_directives[4] = "rewrite";
	_location_directives[5] = "autoindex";
	_location_directives[6] = "allow_methods";
	_location_directives[7] = "return";
	_location_directives[8] = "cgi_path";
	_location_directives[9] = "cgi_ext";
}

LocationConfig::LocationConfig(std::string path)
{
	_location_directives[0] = "error_page";
	_location_directives[1] = "client_max_body_size";
	_location_directives[2] = "root";
	_location_directives[3] = "index";
	_location_directives[4] = "rewrite";
	_location_directives[5] = "autoindex";
	_location_directives[6] = "allow_methods";
	_location_directives[7] = "cgi_path";
	_location_directives[8] = "cgi_ext";
	_location_directives[9] = "return";

	_map_location["path"] = path;
}

//LocationConfig::LocationConfig(const LocationConfig& param) {}

LocationConfig::~LocationConfig() {}


std::string	LocationConfig::get_path()
{
	std::map<std::string, std::string>::iterator it = _map_location.find("path");
	return (it != _map_location.end()) ? it->second : "";
}

std::string LocationConfig::get_root()
{
	std::map<std::string, std::string>::iterator it = _map_location.find("root");
	return (it != _map_location.end()) ? it->second : ""; 
}

std::string LocationConfig::get_index()
{ 
	std::map<std::string, std::string>::iterator it = _map_location.find("index");
	return (it != _map_location.end()) ? it->second : "";
}

std::map<std::string, std::string>& LocationConfig::get_map_location()	{ return _map_location; }

bool	LocationConfig::get_autoindex()
{
	std::map<std::string, std::string>::iterator it = _map_location.find("autoindex");
	if (it != _map_location.end())
	{
		if (it->second == "on")
			return true;
		else if (it->second == "off")
			return false;
	}
	return false;
}

int	LocationConfig::get_client_max_body_size()
{
	int client_max_body_size = 0;
	std::map<std::string, std::string>::iterator it = _map_location.find("client_max_body_size");
	if (it != _map_location.end())
	{
			try { client_max_body_size = convert<int>(it->second); }
			catch (std::exception &e) { return (std::cerr << "Error: client_max_body_size is not a number!" << std::endl, 0); }
			if (client_max_body_size < 0)
				return (std::cerr << "Error: client_max_body_size is negative!" << std::endl, 0);
	}
	return client_max_body_size;
}


bool	LocationConfig::parse_location(std::istringstream &iss, std::string key)
{
	if (!is_location_variable(key))
	{
		std::cerr << "Error: Invalid keyword '" << key << "'!" << std::endl;
		return 1;
	}
	else if (key == "error_page")
	{
		if (handle_error_page(iss, _map_location))
			return 1;
	}
	else if (key == "allow_methods")
	{
		if (handle_allow_methods(iss, _map_location))
			return 1;
	}
	else if (key == "autoindex")
	{
		if (handle_autoindex(iss, _map_location))
			return 1;
	}
	else if (key == "index")
	{
		if (handle_index(iss, _map_location))
			return 1;
	}
	else if (key == "root")
	{
		if (handle_root(iss, _map_location))
			return 1;
	}
	else if (key == "cgi_path")
	{
		if (handle_cgi_path(iss, _map_location))
			return 1;
	}
	else if (key == "cgi_ext")
	{
		if (handle_cgi_ext(iss, _map_location))
			return 1;
	}
	else
	{
		if (_map_location.count(key))
		{
			std::cerr << "Error: Keyword " << key << " already set!" << std::endl;
			return 1;
		}
		std::string value;
		iss >> value;
		if (!is_valid_to_clean_semicolon(value))
			return 1;
		value = clean_semicolon(value);
		_map_location[key] = value;
		if (!iss.eof())
		{
			std::cerr << "Error: There are values after ';' for keyword: " << key << "!" << std::endl;
			return 1;
		}
	}
	return 0;
}

bool	LocationConfig::is_location_variable(std::string key)
{
	for (int i = 0; i < 10; i++)
		if (key == _location_directives[i])
			return true;
	return false;
}

std::string	LocationConfig::DEBUG_test()
{
	std::string str;
	for (std::map<std::string, std::string>::iterator it = _map_location.begin(); it != _map_location.end(); ++it)
	{
		str += it->first + ": " + it->second + "\n";
	}
	str += "\n";
	return str;
}

bool	LocationConfig::check_cgi()
{
	if (_map_location.count("cgi_path") != _map_location.count("cgi_ext"))
	{
		if (_map_location.count("cgi_path") == 0)
			std::cerr << "Error: Missing cgi_path!" << std::endl;
		else
			std::cerr << "Error: Missing cgi_ext!" << std::endl;
		return 1;
	}
	return 0;
}

bool	LocationConfig::handle_cgi_path(std::istringstream &iss, std::map<std::string, std::string> &_current_map)
{
	std::string value;
	if (!(iss >> value))
	{
		std::cerr << "Error: Keyword cgi_path has no value!" << std::endl;
		return 1;
	}	
	if (_current_map.count("cgi_path"))
	{
		std::cerr << "Error: Keyword cgi_path already set!" << std::endl;
		return 1;
	}
	if (!is_valid_to_clean_semicolon(value))
		return 1;
	if (value.find(";") == std::string::npos)
	{
		std::cerr << "Error: Semicolon is missing for keyword: cgi_path!" << std::endl;
		return 1;
	}
	value = clean_semicolon(value);
	if (value.find_first_of("/") == 0 || value.find_last_of("/") == value.length() - 1)
	{
		std::cerr << "Error: Invalid cgi_path value '" << value << "', must not start or end with '/'!" << std::endl;
		return 1;
	}
	_current_map["cgi_path"] = value;
	if (iss >> value)
	{
		std::cerr << "Error: Keyword cgi_path has too many values!" << std::endl;
		return 1;
	}
	return 0;
}

bool	LocationConfig::handle_cgi_ext(std::istringstream &iss, std::map<std::string, std::string> &_current_map)
{
	std::string value;
	std::string tmp;
	if (!(iss >> tmp))
	{
		std::cerr << "Error: Keyword cgi_ext has no value!" << std::endl;
		return 1;
	}
	if (_current_map.count("cgi_ext"))
	{
		std::cerr << "Error: Keyword cgi_ext already set!" << std::endl;
		return 1;
	}
	value = tmp;
	while (iss >> tmp)
	{
		if (tmp.at(0) != '.')
		{
			std::cerr << "Error: Invalid cgi_ext value '" << tmp << "'!" << std::endl;
			return 1;
		}
		value += " " + tmp;
	}
	if (value.find(";") == std::string::npos)
	{
		std::cerr << "Error: Semicolon is missing for keyword: cgi_ext!" << std::endl;
		return 1;
	}
	if (value.find_first_of(";") != value.length() - 1)
	{
		std::cerr << "Error: Invalid cgi_ext value, semicolon must be only ';' at the end!" << std::endl;
		return 1;
	}
	if (!is_valid_to_clean_semicolon(value))
		return 1;
	value = clean_semicolon(value);
	_current_map["cgi_ext"] = value;
	return 0;
}
