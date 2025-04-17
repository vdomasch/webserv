# include "parsing_utils.hpp"
# include "ServerConfig.hpp"

bool	is_valid_to_clean_semicolon(std::string key)
{
	if (key.find_first_not_of(";") == std::string::npos)
	{
		if (key.empty())
			std::cerr << "Error: Empty value!" << std::endl;
		else
			std::cerr << "Error: Invalid value '" << key << "' !" << std::endl;
		return false;
	}
	return true;
}

std::string	clean_semicolon(std::string text)
{
	while (text.at(text.length() - 1) == ';')
		text.erase(text.length() - 1);
	return text;
}

bool is_keyword(std::string key, std::string pattern)
{
	if (key == pattern || key.empty() || key == "}")
		return (true);
	return (false);
}

bool	is_error_page_code(std::string code)
{
	int int_code = atoi(code.c_str());

	if (code.length() != 3)
		;
	else if (int_code >= 400 && int_code <= 599)
		return true;
	return false;
}

bool	is_server_name_already_used(std::map<std::string, ServerConfig> &server_list, ServerConfig &server_temp)
{
	if (server_list.find(server_temp.get_string_port_number() + static_cast<std::string>(":") + server_temp.get_server_name()) != server_list.end())
	{
		std::cerr << "Error: Server name already exists for this port!" << std::endl;
		return true;
	}
	else if (server_list[server_temp.get_string_port_number()].get_server_name() == server_temp.get_server_name())
	{
		std::cerr << "Error: Server name already exists for this port!" << std::endl;
		return true;
	}
	return false;
}

bool	handle_error_page(std::istringstream &iss, std::map<std::string, std::string> &_current_map)
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
			std::cerr << "Error: Keyword error_page needs a valid error number before path!" << std::endl;
			return 1;
		}
		iss >> error_code;
	}
	while (!code_numbers.empty())
	{
		if (!_current_map[code_numbers.back()].empty())
		{
			std::cerr << "Error: Keyword error_page already set for code " << code_numbers.back() << "!" << std::endl;
			return 1;
		}
		error_code = clean_semicolon(error_code);
		_current_map[code_numbers.back()] = error_code;
		code_numbers.pop_back();
	}
	if (!iss.eof())
	{
		std::cerr << "Error: There are values after ';' for keyword error_page!" << std::endl;
		return 1;
	}
	return 0;
}

bool	handle_autoindex(std::istringstream &iss, std::map<std::string, std::string> &_map_server)
{
	if (!_map_server["autoindex"].empty())
	{
		std::cerr << "Error: Keyword autoindex already set!" << std::endl;
		return 1;
	}
	std::string value;
	iss >> value;
	if (!is_valid_to_clean_semicolon(value))
		return 1;
	value = clean_semicolon(value);
	if (value == "on" || value == "off")
		_map_server["autoindex"] = value;
	else
	{
		std::cerr << "Error: Invalid autoindex value '" << value << "'!" << std::endl;
		return 1;
	}
	if (!iss.eof())
	{
		std::cerr << "Error: There are values after ';' for keyword autoindex!" << std::endl;
		return 1;
	}
	return 0;
}

bool	handle_allow_methods(std::istringstream &iss, std::map<std::string, std::string> &_current_map)
{
	std::string key;
	while (iss >> key)
	{
		if (!iss.eof() && key.find(";") != std::string::npos)
		{
			std::cerr << "Error: There are values after ';' for keyword allow_methods!" << std::endl;
			return 1;
		}
		if (!is_valid_to_clean_semicolon(key))
			return 1;
		key = clean_semicolon(key);
		if (key == "POST" || key == "GET" || key == "DELETE")
		{
			if (!_current_map[key].empty())
			{
				std::cerr << "Error: Keyword allow_methods already set for method " << key << "!" << std::endl;
				return 1;
			}
			_current_map[key] = "true";
		}
		else
		{
			std::cerr << "Error: Invalid allow_methods value '" << key << "'!" << std::endl;
			return 1;
		}
	}
	return 0;
}