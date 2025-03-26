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

bool is_keyword(std::string key, std::string pattern)
{
	if (key == pattern || key == "{" || key.empty() || key == "}")
		return (true);
	return (false);
}

bool	HTTPConfig::parse_http()
{
	std::string line;
	std::ifstream infile(_filename.c_str());
	if (!infile.is_open())
	{
		std::cout << "Error, failed to open filename!" << std::endl;
		return (true);
	}
	while (std::getline(infile, line)) {
        std::istringstream iss(line);  
        std::string key;
		iss >> key;

		if (is_location(key))
		{
			std::cout << "location: " << key << std::endl;
			
		}
		else if (is_server(key))
		{
			std::cout << "server: " << key << std::endl;
			//_server_list.push_back(ServerConfig());
			//_server_list.back().server_config(iss);
		}
		else if (is_http(key))
		{
			std::cout << "http: " << key << std::endl;
			if (set_http_values(iss, key))
			{
				std::cerr << "Error: Invalid keyword" << std::endl;
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

bool	is_error_page_code(std::string code)
{
	int int_code = atoi(code.c_str());

	if (code.find(".html") != std::string::npos)
		return false;
	if (code.length() != 3)
		;
	else if (int_code >= 400 && int_code <= 599)
		return true;
	std::cerr << "Error: Invalid error_page_code: '" << code << "'!" << std::endl;
	return false;
}

bool	HTTPConfig::set_http_values(std::istringstream &iss, std::string key)
{
	std::string value;

	if (key == "client_max_body_size")
	{
		iss >> value;
		while (value.at(value.length() - 1) == ';')
			value.erase(value.length() - 1);
		_map_http[key] = value;
    }
	else if (key == "error_page")
	{
		std::vector<std::string> code_number;
		std::string error_code_path;
		
		iss >> error_code_path;
		while (is_error_page_code(error_code_path))
		{
			code_number.push_back(error_code_path);
			iss >> error_code_path;
		}
		while (!code_number.empty())
		{
			_map_http[code_number.back()] = error_code_path;
			code_number.pop_back();
		}
	}
	else if (is_keyword(key, "http"))
		;
	else
		return 1;
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