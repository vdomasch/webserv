#include "HttpRequest.hpp"
#include "HTTPConfig.hpp"
#include "webserv.hpp"
#include <iostream>
#include <string>

HttpRequest::HttpRequest():	_is_error_request(false), _autoindex(true), _is_php_cgi(false), _is_server_socket(false),
							_state(RECEIVING_HEADER), _content_length(0), _header_parsed(false), _keep_alive(true),
							_is_multipart(false),_is_redirection(false), _status_code(0), _content_type("text/html") //!
{
	_last_time = time(NULL);
}

HttpRequest::~HttpRequest() {}

void HttpRequest::append_data(const std::string &data)
{
	
	_raw_data += data;
	bool was_in_header = false;
	if (_state == RECEIVING_HEADER)
	{
		size_t pos = _raw_data.find("\r\n\r\n");
		if (pos != std::string::npos)
		{
			_header = _raw_data.substr(0, pos);
			_body = _raw_data.substr(pos + 4);
			
			parse_headers();
			_header_parsed = true;
			if (!_is_multipart && _content_length == 0)
			{
				_state = COMPLETE;
				_last_time = time(NULL);
			}
			else
			{
				_state = RECEIVING_BODY;
				_last_time = time(NULL);
			}
		}
		was_in_header = true;
	}
	if (_state == RECEIVING_BODY)
	{
		if (!was_in_header)
			_body += data;
		if (_is_multipart)
		{
			if (_body.find(_boundary + "--\r\n") != std::string::npos || _body.find(_boundary + "--") != std::string::npos)
			{
				_state = COMPLETE;
				_last_time = time(NULL);
			}
		}
		else
		{
			if (_body.length() >= _content_length)
			{
				_state = COMPLETE;
				_last_time = time(NULL);
			}
		}
	}
}

bool HttpRequest::check_keep_alive() const
{
	std::map<std::string, std::string>::const_iterator it = _headers_map.find("Connection");
	if (it != _headers_map.end())
	{
		std::string connection = it->second;
		std::transform(connection.begin(), connection.end(), connection.begin(), ::tolower);
		return (connection != "close");
	}
	return true;
}


std::string	normalize_path(const std::string &path)
{
    if (path.empty())
        return "/";

    std::stringstream ss(path);
    std::string segment;
    std::vector<std::string> segments;
    
    while (std::getline(ss, segment, '/'))
	{
        if (segment.empty() || segment == ".")
            continue; 
		else if (segment == "..")
		{
            if (!segments.empty() && segments.back() != "..")
                segments.pop_back();
			else
                continue;
        }
		else
            segments.push_back(segment);
    }

    std::string normalized_path;
    for (size_t i = 0; i < segments.size(); ++i)
	{
        normalized_path += segments[i];
        if (i < segments.size() - 1)
            normalized_path += "/";
    }

    if (normalized_path.empty())
        return "/";

    return normalized_path;
}

void	HttpRequest::parse_headers()
{
	std::istringstream stream(_header);
	std::string line;

	if (!std::getline(stream, line))
	{
		std::cerr << "Error reading request line" << std::endl;
		_state = ERROR;
		_status_code = 400;
		return;
	}

	std::istringstream req_line(line);
	if (!(req_line >> _method >> _target >> _http_version))
	{
		std::cerr << "Error: Invalid request line: " << line << std::endl;
		_state = ERROR;
		_status_code = 400;
		return;
	}

	size_t query_pos = _target.find('?');
	if (query_pos != std::string::npos)
	{
		_query_string = _target.substr(query_pos + 1);
		_target = normalize_path(_target.substr(0, query_pos));
	}
	else 
	{
		_query_string.clear();
		_target = normalize_path(_target);
	}

	if (_http_version != "HTTP/1.1")
	{
		std::cerr << "Error: Unsupported HTTP version: " << _http_version << std::endl;
		_state = ERROR;
		_status_code = 505;
		return;
	}

	while (std::getline(stream, line))
	{
		if (line == "\r" || line.empty()) continue;
		size_t colon = line.find(':');
		if (colon == std::string::npos) continue;

		std::string key = line.substr(0, colon);
		std::string value = line.substr(colon + 1);

		key.erase(key.find_last_not_of(" \t\r") + 1);
		value.erase(0, value.find_first_not_of(" \t"));
		value.erase(value.find_last_not_of(" \t\r") + 1);
		_headers_map[key] = value;
	}

	if (_headers_map.count("Content-Length"))
	{
		try { _content_length = convert<size_t>(_headers_map["Content-Length"]); }
		catch (const std::exception &e)	{
			std::cerr << "Error: Converting Content-Length: " << e.what() << std::endl;
			_state = ERROR;
			_status_code = 400;
			return;
		}
	}

	if (_headers_map.count("Content-Type"))
	{
		std::string content_type = _headers_map["Content-Type"];
		if (content_type.find("multipart/form-data") != std::string::npos)
		{
			size_t pos = content_type.find("boundary=");
			if (pos != std::string::npos)
			{
				_boundary = "--" + content_type.substr(pos + 9);
				_is_multipart = true;
			}
		}
	}

	if (check_keep_alive())
		_headers_map["Connection"] = "keep-alive";
	else
	{
		_headers_map["Connection"] = "close";
		_keep_alive = false;
	}
}

bool	HttpRequest::is_ready() const			{ return _state == COMPLETE && !is_error(_status_code); }
bool	HttpRequest::is_finished() const		{ return _state == RESPONDED; }
bool	HttpRequest::has_error() const			{ return (_state == ERROR || is_error(_status_code) != 0); }
void	HttpRequest::print_state_status() const { std::cerr << "State : " <<_state << " | Status : " << _status_code << "\n\n";}  //!


/////////// GETTERS ///////////

bool			HttpRequest::get_is_server_socket() const	{ return _is_server_socket; }
bool			HttpRequest::getKeepAlive() const			{ return _keep_alive; }
bool			HttpRequest::get_is_multipart() const		{ return _is_multipart; }
bool			HttpRequest::get_is_redirection() const		{ return _is_redirection; }
int				HttpRequest::get_status_code() const		{ return _status_code; }
ssize_t			HttpRequest::get_content_length() const		{ return _content_length; }
std::string		HttpRequest::get_response() const			{ return _response; }
std::string		HttpRequest::get_method() const				{ return _method; }
std::string		HttpRequest::get_target() const				{ return _target; }
std::string		HttpRequest::get_rootpath() const			{ return _rootpath; }
std::string		HttpRequest::get_boundary() const			{ return _boundary; }
std::string		HttpRequest::get_content_type() const		{ return _content_type; }
std::string 	HttpRequest::get_body() const				{ return _body;}
std::string		HttpRequest::get_query_string() const		{ return _query_string; }
std::string		HttpRequest::get_redirection() const		{ return _redirection; }
unsigned long	HttpRequest::get_time() const				{ return _last_time; };
RequestState	HttpRequest::get_state() const				{ return _state; }
std::string HttpRequest::get_header(const std::string& key) const
{
	std::map<std::string, std::string>::const_iterator it = _headers_map.find(key);
	if (it != _headers_map.end())
		return it->second;
	return "";
}
	

////////// SETTERS ///////////

void	HttpRequest::set_is_server_socket(bool is_server_sock)	{ _is_server_socket = is_server_sock; }
void	HttpRequest::set_response(const std::string& response)	{ _response = response; }
void	HttpRequest::set_target(const std::string& target)		{ _target = target; }
void	HttpRequest::set_rootpath(const std::string& rootpath)	{ _rootpath = rootpath; }
void	HttpRequest::set_state(enum RequestState value)			{ _state = value; }
void	HttpRequest::set_content_type(const std::string& type)	{ _content_type = type; }
void	HttpRequest::set_time(unsigned long t) 					{ _last_time = t; };
void	HttpRequest::set_is_redirection(bool value)				{ _is_redirection = value; }
void	HttpRequest::set_redirection(const std::string& uri)		{ _redirection = uri; }

void	HttpRequest::set_status_code(int code)					
{
	if (code >= 400)
	{
		_state = ERROR;
	}
	_status_code = code;
}

std::ostream& operator<<(std::ostream &os, const HttpRequest &req)
{
	os << "-----------------------" << std::endl;
	os << "Method: " << req.get_method() << std::endl;
	os << "Keep-Alive: " << (req.getKeepAlive() ? "true" : "false") << std::endl;
	os << "-----------------------" << std::endl;
	return os;
}
