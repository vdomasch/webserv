#include "HttpRequest.hpp"
#include "webserv.hpp"
#include <iostream>
#include <string>

HttpRequest::HttpRequest(): _is_error_request(false), _state(RECEIVING_HEADER), _content_length(0), _header_parsed(false), _keep_alive(true), _errcode(0), _content_type("text/html") {}
HttpRequest::~HttpRequest() {}

void HttpRequest::append_data(const std::string &data)
{
	std::cout << data << std::endl;
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
				_state = COMPLETE;
			else
				_state = RECEIVING_BODY;
		}
		was_in_header = true;
	}
	if (_state == RECEIVING_BODY)
	{
		if (!was_in_header)
			_body += data;
		if (_is_multipart) 
		{
			// multipart → fin du body par le boundary final
			if (_body.find(_boundary + "--\r\n") != std::string::npos || _body.find(_boundary + "--") != std::string::npos)
				_state = COMPLETE;
		}
		else
		{
			if (_body.length() >= _content_length)
				_state = COMPLETE;
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

void	HttpRequest::parse_headers()
{
	std::istringstream stream(_header);
	std::string line;

	// Première ligne = ligne de requête
	if (!std::getline(stream, line))
	{
		std::cerr << "Error reading request line" << std::endl;
		_state = ERROR;
		_errcode = 400;
		return;
	}

	std::istringstream req_line(line);
	if (!(req_line >> _method >> _target >> _http_version))
	{
		std::cerr << "Invalid request line: " << line << std::endl;
		_state = ERROR;
		_errcode = 400;
		return;
	}

	size_t query_pos = _target.find('?');
	if (query_pos != std::string::npos)
	{
		_query_string = _target.substr(query_pos + 1);
		_target = _target.substr(0, query_pos);
	}

	if (_http_version != "HTTP/1.1")
	{
		std::cerr << "Unsupported HTTP version: " << _http_version << std::endl;
		_state = ERROR;
		_errcode = 505; // HTTP Version Not Supported
		return;
	}

	// En-têtes HTTP (key: value)
	while (std::getline(stream, line))
	{
		if (line == "\r" || line.empty()) continue;
		size_t colon = line.find(':');
		if (colon == std::string::npos) continue;

		std::string key = line.substr(0, colon);
		std::string value = line.substr(colon + 1);
		// Nettoyage des espaces
		key.erase(key.find_last_not_of(" \t\r") + 1);
		value.erase(0, value.find_first_not_of(" \t"));
		value.erase(value.find_last_not_of(" \t\r") + 1);
		_headers_map[key] = value;
	}

	if (_headers_map.count("Content-Length"))
	{
		try { _content_length = convert<size_t>(_headers_map["Content-Length"]); }
		catch (const std::exception &e)	{
			std::cerr << "Error converting Content-Length: " << e.what() << std::endl;
			_state = ERROR;
			_errcode = 400; // Bad Request
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
				_boundary = "--" + content_type.substr(pos + 9); // "boundary=..." → on ajoute les "--"
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

bool	HttpRequest::is_ready() const		{ return _state == COMPLETE; }
bool	HttpRequest::is_finished() const	{ return _state == RESPONDED; }
bool	HttpRequest::has_error() const		{ return (_state == ERROR || _errcode != 0); } 

/////////// GETTERS ///////////

std::string HttpRequest::get_response() const		{ return _response; }
bool		HttpRequest::getKeepAlive() const		{ return _keep_alive; }
bool 		HttpRequest::get_is_multipart() const	{ return _is_multipart; }
std::string HttpRequest::get_method() const			{ return _method; }
std::string HttpRequest::get_target() const			{ return _target; }
std::string HttpRequest::get_boundary() const		{ return _boundary; }
std::string HttpRequest::get_content_type() const	{ return _content_type; }
std::string HttpRequest::get_body() const
{
	if (_state == RECEIVING_BODY || _state == COMPLETE)
		return _body;
	return "";
}
std::string HttpRequest::get_header(const std::string& key) const
{
	std::map<std::string, std::string>::const_iterator it = _headers_map.find(key);
	if (it != _headers_map.end())
		return it->second;
	return "";
}

////////// SETTERS ///////////

void	HttpRequest::set_response(const std::string& response)	{ _response = response; }
void	HttpRequest::set_errorcode(int code)					{ _state = ERROR, _errcode = code; }
void	HttpRequest::set_target(const std::string& target)		{ _target = target; }
void	HttpRequest::set_state(enum RequestState value)			{ _state = value; }
void	HttpRequest::set_content_type(const std::string& type)	{ _content_type = type; }

//////// OPERATOR OVERLOAD ///////////

std::ostream& operator<<(std::ostream &os, const HttpRequest &req)
{
	os << "-----------------------" << std::endl;
	os << "Method: " << req.get_method() << std::endl;
	os << "Keep-Alive: " << (req.getKeepAlive() ? "true" : "false") << std::endl;
	os << "-----------------------" << std::endl;
	return os;
}
