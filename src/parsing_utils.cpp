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

bool	is_server_name_already_used(std::map<std::string, ServerConfig> &server_list, ServerConfig &server_temp)
{
	if (server_list.find(server_temp.get_server_name() + static_cast<std::string>(":") + server_temp.get_port_number()) != server_list.end())
	{
		std::cerr << "Error: Server name already exists for this port!" << std::endl;
		return true;
	}
	else if (server_list[server_temp.get_port_number()].get_server_name() == server_temp.get_server_name())
	{
		std::cerr << "Error: Server name already exists for this port!" << std::endl;
		return true;
	}
	return false;
}

bool	is_error_page_code(std::string code)
{
	int int_code = atoi(code.c_str());

	if (code.length() == 3 && int_code >= 400 && int_code <= 599)
		return true;
	return false;
}

bool	is_page_extension(std::string name)
{
	size_t code_size = name.length();
	if (name.find(".html") == code_size - 5
		|| name.find(".htm") == code_size - 4
		|| name.find(".txt") == code_size - 4
		|| name.find(".php") == code_size - 4)
		return true;
	return false;
}

bool	is_error_page_extension(std::string error_code)
{
	if (error_code.empty())
	{
		std::cerr << "Error: Keyword error_page has no page path!" << std::endl;
		return 1;
	}
	if (error_code.find(";") == std::string::npos)
	{
		std::cerr << "Error: Semicolon is missing for keyword: error_page!" << std::endl;
		return false;
	}
	while (error_code.at(error_code.length() - 1) == ';')
		error_code.erase(error_code.length() - 1);
	if (is_page_extension(error_code))
		;
	else
	{
		std::cerr << "Error: Invalid error_page path '" << error_code << "'!" << std::endl;
		return false;
	}
	return true;
}

bool	handle_error_page(std::istringstream &iss, std::map<std::string, std::string> &_current_map)
{
	std::vector<std::string> code_numbers;
	std::string error_code;

	iss >> error_code;
	if (error_code.empty())
	{
		std::cerr << "Error: Keyword error_page has no value!" << std::endl;
		return 1;
	}
	while (!error_code.empty())
	{
		if (error_code.at(0) == '/')
			break ;
		if (is_error_page_code(error_code))
			code_numbers.push_back(error_code);
		else
		{
			std::cerr << "Error: Invalid error_page code '" << error_code << "'!" << std::endl;
			return 1;
		}
		error_code.clear();
		iss >> error_code;
	}
	if (code_numbers.empty())
	{
		std::cerr << "Error: Keyword error_page has no page code!" << std::endl;
		return 1;if (error_code.find(";") == std::string::npos)
		{
			std::cerr << "Error: Semicolon is missing for keyword: error_page!" << std::endl;
			return false;
		}
	}
	if (!is_error_page_extension(error_code))
		return 1;
	if ((iss >> error_code))
	{
		std::cerr << "Error: There are values after ';' for keyword error_page!" << std::endl;
		return 1;
	}
	while (!code_numbers.empty())
	{
		if (_current_map.count(code_numbers.back()))
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

bool	handle_autoindex(std::istringstream &iss, std::map<std::string, std::string> &_current_map)
{
	if (_current_map.count("autoindex"))
	{
		std::cerr << "Error: Keyword autoindex already set!" << std::endl;
		return 1;
	}
	std::string value;
	iss >> value;
	if (!is_valid_to_clean_semicolon(value))
		return 1;
	if (value.find(";") == std::string::npos)
	{
		std::cerr << "Error: Semicolon is missing for keyword: autoindex!" << std::endl;
		return 1;
	}
	value = clean_semicolon(value);
	if (value == "on" || value == "off")
		_current_map["autoindex"] = value;
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
		if (iss.eof() && key.find(";") == std::string::npos)
		{
			std::cerr << "Error: Semicolon is missing for keyword: allow_methods!" << std::endl;
			return 1;
		}
		if (!is_valid_to_clean_semicolon(key))
			return 1;
		key = clean_semicolon(key);
		if (key == "POST" || key == "GET" || key == "DELETE")
		{
			if (_current_map.count(key))
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

bool	handle_index(std::istringstream &iss, std::map<std::string, std::string> &_current_map)
{
	std::string value;
	if (!(iss >> value))
	{
		std::cerr << "Error: Keyword index has no value!" << std::endl;
		return 1;
	}	
	if (_current_map.count("index"))
	{
		std::cerr << "Error: Keyword index already set!" << std::endl;
		return 1;
	}
	if (!is_valid_to_clean_semicolon(value))
		return 1;
	if (value.find(";") == std::string::npos)
	{
		std::cerr << "Error: Semicolon is missing for keyword: index!" << std::endl;
		return 1;
	}
	value = clean_semicolon(value);
	if (value.find("/") != std::string::npos)
	{
		if (value.at(0) == '/' || value.at(value.length() - 1) == '/')
		{
			std::cerr << "Error: Invalid index value '" << value << "'!" << std::endl;
			return 1;
		}
	}
	if (!is_page_extension(value))
	{
		std::cerr << "Error: Invalid index value '" << value << "'!" << std::endl;
		return 1;
	}
	_current_map["index"] = value;
	if (iss >> value)
	{
		std::cerr << "Error: Keyword index has too many values!" << std::endl;
		return 1;
	}
	return 0;
}

bool	handle_root(std::istringstream &iss, std::map<std::string, std::string> &_current_map)
{
	std::string value;
	if (!(iss >> value))
	{
		std::cerr << "Error: Keyword root has no value!" << std::endl;
		return 1;
	}
	if (_current_map.count("root"))
	{
		std::cerr << "Error: Keyword root already set!" << std::endl;
		return 1;
	}
	if (!is_valid_to_clean_semicolon(value))
		return 1;
	if (value.find(";") == std::string::npos)
	{
		std::cerr << "Error: Semicolon is missing for keyword: root!" << std::endl;
		return 1;
	}
	value = clean_semicolon(value);
	if (value.find_last_of("/") != value.length() - 1)
	{
		std::cerr << "Error: Invalid root value '" << value << "', must ends with '/'!" << std::endl;
		return 1;
	}
	_current_map["root"] = value;
	return 0;
}
