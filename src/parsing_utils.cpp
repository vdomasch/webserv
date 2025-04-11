# include "parsing_utils.hpp"
# include "ServerConfig.hpp"

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
		std::cerr << "Error: Server name already exists for this port" << std::endl;
		return true;
	}
	else if (server_list[server_temp.get_string_port_number()].get_server_name() == server_temp.get_server_name())
	{
		std::cerr << "Error: Server name already exists for this port" << std::endl;
		return true;
	}
	return false;
}
