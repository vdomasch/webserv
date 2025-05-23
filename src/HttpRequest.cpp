#include "HttpRequest.hpp"
#include "webserv.hpp"
#include <iostream>
#include <string>

HttpRequest::HttpRequest(): _is_error_request(false), _state(RECEIVING_HEADER), _content_length(0), _header_parsed(false), _keep_alive(true), _errcode(0) {}
HttpRequest::~HttpRequest() {}

void HttpRequest::append_data(const std::string &data)
{
	_raw_data += data;

	if (_state == RECEIVING_HEADER)
	{
		size_t pos = _raw_data.find("\r\n\r\n");
		if (pos != std::string::npos)
		{
			_header = _raw_data.substr(0, pos);
			_body = _raw_data.substr(pos + 4);
			parse_headers();
			_header_parsed = true;
			// Exemple simplifié : extraire Content-Length
			size_t cl = _header.find("Content-Length:");
			if (cl != std::string::npos)
			{
				_content_length = atoi(_header.substr(cl + 15).c_str());
				_state = RECEIVING_BODY;
			}
			else _state = COMPLETE; // Pas de body
		}
	}

  	if (_state == RECEIVING_BODY && _body.length() >= _content_length)
	{
		_state = COMPLETE;
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
		_state = ERROR;
		_errcode = 400;
		return;
	}

	std::istringstream req_line(line);
	if (!(req_line >> _method >> _target >> _http_version))
	{
		_state = ERROR;
		_errcode = 400;
		return;
	}

	if (_http_version != "HTTP/1.1")
	{
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
		_content_length = atoi(_headers_map["Content-Length"].c_str());
	
	if (check_keep_alive())
		_headers_map["Connection"] = "keep-alive";
	else
	{
		_headers_map["Connection"] = "close";
		_keep_alive = false;
	}
}

bool	HttpRequest::is_ready() const { return _state == COMPLETE; }
bool	HttpRequest::has_error() const { return (_state == ERROR || _errcode != 0); } 

/////////// GETTERS ///////////

std::string HttpRequest::get_response() const	{ return _response; }
bool		HttpRequest::getKeepAlive() const	{ return _keep_alive; }
std::string HttpRequest::get_method() const		{ return _method; }
std::string HttpRequest::get_target() const		{ return _target; }
std::string HttpRequest::get_header(const std::string& key) const
{
	std::map<std::string, std::string>::const_iterator it = _headers_map.find(key);
	if (it != _headers_map.end())
		return it->second;
	return "";
}

////////// SETTERS ///////////

void HttpRequest::set_response(const std::string& response)	{ _response = response; }
void HttpRequest::set_errorcode(int code)					{ _state = ERROR, _errcode = code; }
void HttpRequest::set_target(const std::string& target)		{ _target = target; }

//////// OPERATOR OVERLOAD ///////////

std::ostream& operator<<(std::ostream &os, const HttpRequest &req)
{
	os << "-----------------------" << std::endl;
	os << "Method: " << req.get_method() << std::endl;
	os << "Keep-Alive: " << (req.getKeepAlive() ? "true" : "false") << std::endl;
	os << "-----------------------" << std::endl;
	return os;
}
