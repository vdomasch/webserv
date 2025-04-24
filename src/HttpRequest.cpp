#include "HttpRequest.hpp"
#include "webserv.hpp"

HttpRequest::HttpRequest() {}
HttpRequest::~HttpRequest() {}

std::string HttpRequest::getMethod() const		{ return _method; }
std::string HttpRequest::getHost() const		{ return _host; }
std::string HttpRequest::getConnection() const	{ return _connection; }
std::string HttpRequest::getPath() const		{ return _path; }
std::string	HttpRequest::getPort() const		{ return _port; }
std::string HttpRequest::getCookie() const		{ return _cookie; }
bool		HttpRequest::getKeepAlive() const	{ return _keep_alive; }
int			HttpRequest::getDone() const		{ return _done; }
std::string HttpRequest::getResponse() const	{ return _response; }


void HttpRequest::setMethod(const std::string& method)			{ _method = method; }
void HttpRequest::setHost(const std::string& host)				{ _host = host; }
void HttpRequest::setConnection(const std::string& connection)	{ _connection = connection; }
void HttpRequest::setPath(const std::string& path)				{ _path = path; }
void HttpRequest::setPort(const std::string& port)				{ _port = port; }
void HttpRequest::setCookie(const std::string& cookie)			{ _cookie = cookie; }
void HttpRequest::setKeepAlive(bool keep_alive)					{ _keep_alive = keep_alive; }
void HttpRequest::setDone(int done)								{ _done = done; }
void HttpRequest::setResponse(const std::string& response)		{ _response = response; }

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

void	HttpRequest::parseRequest(const std::string &request, int port)
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
		//std::cout << "DEBUG: PARSING LINE" << std::endl;
		std::getline(iss, line);
		for (int i = 0; i < 2; i++)
		{
			//std::cout << "DEBUG: COMPARING LINE" << std::endl;
			if (!found_flags[i])
			{
				//std::cout << compare[i] << std::endl;
				std::size_t found = line.find(compare[i]);
				if (found != std::string::npos)
				{
					std::string value = line.substr(found + compare[i].length() + 2);
					switch (i)
					{
						case 0:
							_host = value;
							break;
						case 1:
							_connection = value;
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
	//_path = _path.substr(1);
	_port = tostr(port);
}


