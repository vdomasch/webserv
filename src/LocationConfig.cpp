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
	_location_directives[7] = "cgi_ext";
	_location_directives[8] = "return";
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
	_location_directives[7] = "cgi_ext";
	_location_directives[8] = "return";

	_map_location["path"] = path;
}

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

bool	LocationConfig::get_autoindex(bool autoindex)
{
	std::map<std::string, std::string>::iterator it = _map_location.find("autoindex");
	if (it != _map_location.end())
	{
		if (it->second == "on")
			return true;
		else if (it->second == "off")
			return false;
	}
	return autoindex;
}

size_t	LocationConfig::get_client_max_body_size()
{
	ssize_t client_max_body_size = 0;
	std::map<std::string, std::string>::iterator it = _map_location.find("client_max_body_size");
	if (it != _map_location.end())
	{
			try { client_max_body_size = convert<ssize_t>(it->second); }
			catch (std::exception &e) { return (std::cerr << "Error: Client_max_body_size is not a number!" << std::endl, 0); }
			if (client_max_body_size < 0)
				return (std::cerr << "Error: Client_max_body_size is negative!" << std::endl, 0);
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
	else if (key == "return")
	{
		if (handle_return(iss, _map_location))
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

static bool	is_cgi_extension_valid(std::string str)
{
	if (str.find(".php") != std::string::npos)
	{
		if (str == ".php")
			return true;
		for (size_t i = 4; i < str.size(); i++)
		{
			if (str.at(i) != ';')
				return false;
		}
		return true;
	}
	else if (str.find(".py") != std::string::npos)
	{
		if (str == ".py")
			return true;
		for (size_t i = 4; i < str.size(); i++)
		{
			if (str.at(i) != ';')
				return false;
		}
		return true;
	}
	return false;
}

static bool	is_cgi_extension_double_declared(std::string ext, std::string stream)
{
	if (ext.find(".py;") != std::string::npos)
	{
		if (stream.find(".py") != std::string::npos)
			return true;
	}
	else if (ext.find(".php;") != std::string::npos)
	{
		if (stream.find(".php") != std::string::npos)
			return true;
	}
	else
	{
		if (stream.find(ext) != std::string::npos)
			return true;
	}
	return false;
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
	if (!is_cgi_extension_valid(tmp))
	{
		std::cerr << "Error: Invalid extension '" << tmp << "' for cgi_ext!" << std::endl;
		return 1;
	}
	while (iss >> tmp)
	{
		if (!is_cgi_extension_valid(tmp))
		{
			std::cerr << "Error: Invalid extension '" << tmp << "' for cgi_ext!" << std::endl;
			return 1;
		}
		else if (is_cgi_extension_double_declared(tmp, value))
		{
			std::cerr << "Error: Double declaration of value '" << tmp << "' for cgi_ext!" << std::endl;
			return 1;
		}
		value += " " + tmp;
	}
	if (value.find(";") == std::string::npos)
	{
		std::cerr << "Error: Semicolon is missing for keyword: cgi_ext!" << std::endl;
		return 1;
	}
	if (iss.eof() && value.find(";") == std::string::npos)
	{
		std::cerr << "Error: Semicolon is missing for keyword: allow_methods!" << std::endl;
		return 1;
	}
	if (!is_valid_to_clean_semicolon(value))
		return 1;
	value = clean_semicolon(value);
	_current_map["cgi_ext"] = value;
	return 0;
}
