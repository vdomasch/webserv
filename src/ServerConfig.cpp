#include "ServerConfig.hpp"

ServerConfig::ServerConfig() {
	_server_directives[0] = "listen";
	_server_directives[1] = "host";
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
		//std::cout << "DEBUG: key = " << key << " i = " << _server_directives[i] << std::endl;
		if (key == _server_directives[i])
		{
			return 0;
		}
	}
	std::cerr << "Error: Invalid keyword:" << key << std::endl;
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

void	ServerConfig::show()
{
	std::cout << _server_directives[0] << std::endl;
}

bool	ServerConfig::set_server_values(std::istringstream &iss, std::string key)
{
	std::string value;

	if (key == "error_page")
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
			_map_server[code_numbers.back()] = error_code;
			code_numbers.pop_back();
		}
	}
	else if (key == "listen")
	{
		std::cout << "DEBUG: do server " << key << std::endl; 
	}
	else if (is_server_variable(key))
	{
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
