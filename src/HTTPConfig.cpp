#include "HTTPConfig.hpp"

HTTPConfig::HTTPConfig(): _is_http(false), _is_server(false), _is_location(false)
{
	_map_http["error_page"] = "UNSET";
	_map_http["client_max_body_size"] = "UNSET";
}

HTTPConfig::~HTTPConfig() {}

bool HTTPConfig::is_http(std::string key)
{
	if (key == "}" && _is_http)
	{
		_is_http = false;
		//std::cout << "close http" << std::endl;
		return true;
	}
	if (_is_server || _is_location)
		return false;
	if (key == "http" || _is_http)
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
		//std::cout << "close server" << std::endl;
		return true;
	}
	if (_is_location)
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
		//std::cout << "close location" << std::endl;
		return true;
	}
	if (key == "location" || _is_location)
	{
		_is_location = true;
		return true;
	}
	return false;
}

bool	HTTPConfig::parse_http()
{
	int	server_number = 0;
	int	location_number = -1;

	std::string line;
	std::ifstream infile(_filename.c_str());
	if (!infile.is_open())
	{
		std::cerr << "Error, failed to open filename!" << std::endl;
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
				{
					std::cerr << "Error: location has no path!" << std::endl;
					return 1;
				}
				iss >> key;
				_server_list[server_number].add_location(key, location_number);
			}
			else if (!key.empty() && key != "}")
			{
				if (_server_list[server_number].select_current_location(iss, key, location_number))
					return 1;
			}
		}
		else if (is_server(key))
		{
			if (key == "server")
			{
				_server_list.push_back(ServerConfig());
			}
			else if (!key.empty() && key != "}")
			{
				if (_server_list[server_number].parse_server(iss, key))
					return 1;
				if (_server_list[server_number].set_server_values(iss, key))
					return 1;
			}
		}
		else if (is_http(key))
		{
			//std::cout << "http: " << key << std::endl;
			if (set_http_values(iss, key))
			{
				//std::cerr << "Error: Invalid keyword" << std::endl;
				return 1;
			}
		}
		else
		{
			std::cerr << "Error: Element declared incorrectly!" << std::endl;
			return 1;
		}
	}
	return 0;
}

bool	HTTPConfig::set_http_values(std::istringstream &iss, std::string key)
{
	std::string value;

	if (key == "client_max_body_size")
	{
		iss >> value;
		value = clean_semicolon(value);
		_map_http[key] = value;
    }
	else if (key == "error_page")
	{
		std::vector<std::string> code_numbers;
		std::string error_code;
		
		iss >> error_code;
		while (!error_code.empty())
		{
			if (is_error_page_code(error_code))
				code_numbers.push_back(error_code);
			else if (error_code.find(".html") != std::string::npos && !code_numbers.empty())
				break ;
			else
			{
				std::cerr << "Error: error_page need a valid error number before path!" << std::endl;
				return 1;
			}
			iss >> error_code;
		}
		while (!code_numbers.empty())
		{
			error_code = clean_semicolon(error_code);
			_map_http[code_numbers.back()] = error_code;
			code_numbers.pop_back();
		}
	}
	else if (is_keyword(key, "http"))
		;
	else
	{
		std::cerr << "Error: Invalid keyword: " << key << std::endl; 
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

	unsigned int count = 1;
	while (iss_copy >> key)
		count++;
	if (count < 3)
		return false;
	return true;
}
