#include "HttpResponse.hpp"

HttpResponse::HttpResponse(): _status_code("200"), _status_message("OK") {}

void	HttpResponse::set_status(const std::string& code, const std::string& message)
{
    _status_code = code;
    _status_message = message;
}

void	HttpResponse::set_body(const std::string& body)	{ _body = body; }
void	HttpResponse::set_keep_alive(bool keep_alive)	{ _keep_alive = keep_alive; }

bool	HttpResponse::get_keep_alive() const			{ return _keep_alive; }
std::string HttpResponse::get_body() const				{ return _body; }

void HttpResponse::add_header(const std::string& key, const std::string& value)
{
    _headers[key] = value;
}

std::string HttpResponse::generate_response() const
{
    std::ostringstream response;
    response << "HTTP/1.1 " << _status_code << " " << _status_message << "\r\n";
    for (std::map<std::string, std::string>::const_iterator it = _headers.begin(); it != _headers.end(); ++it)
        response << it->first << ": " << it->second << "\r\n";
    response << "\r\n";
    response << _body;
    return response.str();
}