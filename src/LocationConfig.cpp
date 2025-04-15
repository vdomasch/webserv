#include "LocationConfig.hpp"

LocationConfig::LocationConfig()
{
	_location_directives[0] = "error_page";
	_location_directives[1] = "client_max_body_size";
	_location_directives[2] = "root";
	_location_directives[3] = "index";
	_location_directives[4] = "alias";
	_location_directives[5] = "autoindex";
	_location_directives[6] = "allow_methods";
	_location_directives[7] = "cgi_path";
	_location_directives[8] = "cgi_ext";
	_location_directives[9] = "return";
	_location_directives[10] = "rewrite";
}

//LocationConfig::LocationConfig(const LocationConfig& param) {}

LocationConfig::~LocationConfig() {}

void	LocationConfig::set_path(std::string key)
{
	_map_location["path"] = key;
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
				std::cerr << "Error: Keyword error_page needs a valid error number before path!" << std::endl;
				return 1;
			}
			iss >> key;
		}
		while (!code_numbers.empty())
		{
			key = clean_semicolon(key);
			_map_location[code_numbers.back()] = key;
			code_numbers.pop_back();
		}
	}
	else if (key == "allow_methods")
	{
		while (iss >> key)
		{
			if (!is_valid_to_clean_semicolon(key))
				return 1;
			key = clean_semicolon(key);
			if (key == "POST" || key == "GET" || key == "DELETE")
				_map_location[key] = "true";
			else
			{
				std::cerr << "Error: Invalid allow_methods value '" << key << "'!" << std::endl;
				return 1;
			}
		}
	}
	else if (key == "autoindex")
	{
		std::string value;
		iss >> value;
		if (!is_valid_to_clean_semicolon(value))
			return 1;
		value = clean_semicolon(value);
		if (value == "on" || value == "off")
			_map_location[key] = value;
		else
		{
			std::cerr << "Error: Invalid autoindex value '" << value << "'!" << std::endl;
			return 1;
		}
	}
	else
	{
		std::string value;
		iss >> value;
		if (!is_valid_to_clean_semicolon(value))
			return 1;
		value = clean_semicolon(value);
		_map_location[key] = value;
	}
	return 0;
}

bool	LocationConfig::is_location_variable(std::string key)
{
	for (int i = 0; i < 11; i++)
		if (key == _location_directives[i])
			return true;
	return false;
}
