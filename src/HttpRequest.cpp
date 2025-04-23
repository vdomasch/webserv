#include "HttpRequest.hpp"

HttpRequest::HttpRequest() {}
HttpRequest::~HttpRequest() {}

std::string HttpRequest::getMethod() const		{ return _method; }
std::string HttpRequest::getHost() const		{ return _host; }
std::string HttpRequest::getConnection() const	{ return _connection; }
std::string HttpRequest::getCookie() const		{ return _cookie; }
std::string HttpRequest::getPath() const		{ return _path; }
bool		HttpRequest::getKeepAlive() const	{ return _keep_alive; }
int		HttpRequest::getDone() const		{ return _done; }

void HttpRequest::setMethod(const std::string& method)			{ _method = method; }
void HttpRequest::setHost(const std::string& host)				{ _host = host; }
void HttpRequest::setConnection(const std::string& connection)	{ _connection = connection; }
void HttpRequest::setCookie(const std::string& cookie)			{ _cookie = cookie; }
void HttpRequest::setPath(const std::string& path)				{ _path = path; }
void HttpRequest::setKeepAlive(bool keep_alive)					{ _keep_alive = keep_alive; }
void HttpRequest::setDone(int done)								{ _done = done; }

std::ostream& operator<<(std::ostream &os, const HttpRequest &req)
{
	os << "-----------------------" << std::endl;
	os << "Method: " << req.getMethod() << std::endl;
	os << "Host: " << req.getHost() << std::endl;
	os << "Connection: " << req.getConnection() << std::endl;
	os << "Path: " << req.getPath() << std::endl;
	os << "Cookie: " << req.getCookie() << std::endl;
	os << "Keep-Alive: " << (req.getKeepAlive() ? "true" : "false") << std::endl;
	os << "-----------------------" << std::endl;
	return os;
}

void	HttpRequest::parseRequest(HttpRequest &req, const std::string &request)
{
	if (request.empty())
		return ;

	req._done = 0;
	std::istringstream iss(request);
	std::string line;
	std::getline(iss, req._method, ' ');
	std::getline(iss, req._path, ' ');
	std::string compare[2] = {"Host", "Connection"};

	bool found_flags[2] = {false, false};

	while (req._done < 2)
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
							req._host = value;
							break;
						case 1:
							req._connection = value;
							break;
					}
					found_flags[i] = true;
					req._done++;
				}
			}
		}
		if (iss.eof())
			break;
	}
	if (req._connection == "keep-alive")
		req._keep_alive = true;
	else
		req._keep_alive = false;
}


