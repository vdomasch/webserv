#include "ServerConfig.hpp"

ServerConfig::ServerConfig(): _listen("UNSET"), _host("UNSET"), _server_name("UNSET"), _error_page("UNSET"), _client_max_body_size("UNSET"), _root("UNSET"), _index("UNSET"), _content("") {
	_congif_values[0] = &_listen;
	_congif_values[1] = &_host;
	_congif_values[2] = &_server_name;
	_congif_values[3] = &_error_page;
	_congif_values[4] = &_client_max_body_size;
	_congif_values[5] = &_root;
	_congif_values[6] = &_index;
	_config_variables[0] = "listen";
	_config_variables[1] = "host";
	_config_variables[2] = "server_name";
	_config_variables[3] = "error_page";
	_config_variables[4] = "client_max_body_size";
	_config_variables[5] = "root";
	_config_variables[6] = "index";
}
ServerConfig::~ServerConfig() {}

//ServerConfig::ServerConfig(const ServerConfig& param)
//{
//}

std::string ServerConfig::find_in_config_file(std::string variable_name)
{
	std::size_t i = 0;
	i = _content.find(variable_name);
	if (i != _content.rfind(variable_name))
		return ("-1");
	if (i == std::string::npos)
		return ("unset");
	i += variable_name.length();
	std::string res;
	while (_content[i] && _content[i] != ';')
	{
		res.push_back(_content.at(i));
		i++;
	}
	return (res);
}

bool	ServerConfig::copy_variable_values()
{
	for (int i = 0; i < 7; i++)
	{
		*_congif_values[i] = find_in_config_file(_config_variables[i]);
		std::cout << _config_variables[i] << " = " << _congif_values[i] << std::endl;
		if (*_congif_values[i] == "unset")
		{
			std::cout << _config_variables[i] << " not set in config file!" << std::endl;
			return (true);
		}
		if (*_congif_values[i] == "-1")
		{
			std::cout << _config_variables[i] << " set more than once in config file!" << std::endl;
			return (true);
		}
	}
	return false;
}

std::string	ServerConfig::copy_content(std::string filename)
{
	//std::string content;
	std::string line;
	std::ifstream infile(filename.c_str());
	if (!infile.is_open())
	{
		std::cerr << "Error, failed to open filename!" << std::endl;
		return ("NULL");
	}
	while(infile)
	{
		std::getline(infile, line);
		if(!infile)
			break;
		_content += line + '\n';
		line.clear();
	}
	if (_content.empty())
	{
		std::cerr << "Config file is empty!" << std::endl;
		return ("NULL");
	}
	std::cout << _content << std::endl;
	return (_content);
}

bool	ServerConfig::server_config(std::string filename)
{
	std::string content = copy_content(filename);
	if (content == "NULL")
		return true;
	if (!copy_variable_values())
		return true;
	return false;
}

void	ServerConfig::parse_server()
{
	_listen += "55";
}

void	ServerConfig::show()
{
	std::cout << _listen << std::endl;
}
