#include "HttpRequest.hpp"
#include "webserv.hpp"

HttpRequest::HttpRequest():_method(""), _host(""), _connection(""), _content_length(""), _content_type(""), _path(""), _port(""), _cookie(""), _keep_alive(false), _done(0), _response("") {}
HttpRequest::~HttpRequest() {}

std::string HttpRequest::getMethod() const			{ return _method; }
std::string HttpRequest::getHost() const			{ return _host; }
std::string HttpRequest::getConnection() const		{ return _connection; }
std::string HttpRequest::getContentLength() const	{ return _content_length; }
std::string HttpRequest::getContentType() const		{ return _content_type; }
std::string HttpRequest::getPath() const			{ return _path; }
std::string	HttpRequest::getPort() const			{ return _port; }
std::string HttpRequest::getCookie() const			{ return _cookie; }
bool		HttpRequest::getKeepAlive() const		{ return _keep_alive; }
int			HttpRequest::getDone() const			{ return _done; }
std::string HttpRequest::getResponse() const		{ return _response; }


void HttpRequest::setMethod(const std::string& method)					{ _method = method; }
void HttpRequest::setHost(const std::string& host)						{ _host = host; }
void HttpRequest::setConnection(const std::string& connection)			{ _connection = connection; }
void HttpRequest::setContentLength(const std::string& content_length)	{ _content_length = content_length; }
void HttpRequest::setContentType(const std::string& content_type)		{ _content_type = content_type; }
void HttpRequest::setPath(const std::string& path)						{ _path = path; }
void HttpRequest::setPort(const std::string& port)						{ _port = port; }
void HttpRequest::setCookie(const std::string& cookie)					{ _cookie = cookie; }
void HttpRequest::setKeepAlive(bool keep_alive)							{ _keep_alive = keep_alive; }
void HttpRequest::setDone(int done)										{ _done = done; }
void HttpRequest::setResponse(const std::string& response)				{ _response = response; }


std::ostream& operator<<(std::ostream &os, const HttpRequest &req)
{
	os << "-----------------------" << std::endl;
	os << "Method: " << req.getMethod() << std::endl;
	os << "Host: " << req.getHost() << std::endl;
	os << "Connection: " << req.getConnection() << std::endl;
	os << "Path: " << req.getPath() << std::endl;
	os << "Port: " << req.getPort() << std::endl;
	os << "Cookie: " << req.getCookie() << std::endl;
	os << "Keep-Alive: " << (req.getKeepAlive() ? "true" : "false") << std::endl;
	os << "-----------------------" << std::endl;
	return os;
}

std::string trim(const std::string& str)
{
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos)
        return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, last - first + 1);
}

/*void	HttpRequest::parseRequest(const std::string &request, int port)
{
	if (request.empty())
		return ;

	_done = 0;
	std::istringstream iss(request);
	std::string line;
	std::getline(iss, _method, ' ');
	std::getline(iss, _path, ' ');
	std::string compare[2] = {"Host", "Connection"};

	bool found_flags[2] = {false, false};

	while (_done < 2)
	{
		std::getline(iss, line);
		for (int i = 0; i < 2; i++)
		{
			if (!found_flags[i])
			{
				std::size_t found = line.find(compare[i]);
				if (found != std::string::npos)
				{
					std::string value = line.substr(found + compare[i].size() + 2);
					switch (i)
					{
						case 0:
							_host = trim(value);
							break;
						case 1:
							_connection = trim(value);
							break;
					}
					found_flags[i] = true;
					_done++;
				}
			}
		}
		if (iss.eof())
			break;
	}
	if (_connection == "keep-alive")
		_keep_alive = true;
	else
		_keep_alive = false;
	//_path = _path.substr(1); // Remove leading slash TO KEEP OR NOT ?
	_port = tostr(port);
}*/

int	HttpRequest::analyseHeader(t_requeste_state &state, int port)
{
	if (state.request.empty())
		return ;

	_done = 0;
	std::istringstream iss(state.request);
	std::string line;
	std::getline(iss, _method, ' ');
	std::getline(iss, _path, ' ');
	std::string compare[4] = {"Host", "Connection", "Content-length", "Content-type"};

	bool found_flags[4] = {false, false, false, false};

	while (_done < 4)
	{
		std::getline(iss, line);
		for (int i = 0; i < 4; i++)
		{
			if (!found_flags[i])
			{
				std::size_t found = line.find(compare[i]);
				if (found != std::string::npos)
				{
					std::string value = line.substr(found + compare[i].size() + 2);
					switch (i)
					{
						case 0:
							_host = trim(value);
							break;
						case 1:
							_connection = trim(value);
							break;
						case 2:
							_content_length = trim(value);
							break;
						case 3:
							_content_type = trim(value);
							break;
					}
					found_flags[i] = true;
					_done++;
				}
			}
		}
		if (iss.eof())
			break;
	}
	_keep_alive = (_connection == "keep-alive");
	//_path = _path.substr(1); // Remove leading slash TO KEEP OR NOT ?
	_port = tostr(port);
	state.bytesRead = state.request.size() - (state.request.find("\r\n\r\n") + 4);
	if (strto_safe(_content_length, state.content_length) == false)
	{
		std::cerr << "Content length is not a number" << std::endl;
		return ;
	}
	if (state.content_length < 0)
	{
		std::cerr << "Content length is negative" << std::endl;
		return ;
	}
}

void	HttpRequest::constructBody(t_requeste_state &state, int port)
{
	static_cast<void>(port);
	if (state.request.empty())
		return ;

	std::string body = state.request.substr(state.request.find("\r\n\r\n") + 4);
	if (body.empty())
		return ;
	if (body.size() > state.content_length)
		std::cerr << "Body size is greater than content length" << std::endl;
	else if (body.size() == state.content_length)
	{
		state.ready_to_process = true;
		state.body = body;
	}
}