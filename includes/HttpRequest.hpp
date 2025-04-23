#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

# include <iostream>
# include <sstream>

class HttpRequest
{
	public:
		HttpRequest();
		~HttpRequest();

		std::string getMethod() const;
		std::string getHost() const;
		std::string getConnection() const;
		std::string getCookie() const;
		std::string getBody() const;
		std::string getPath() const;
		bool		getKeepAlive() const;
		int			getDone() const;

		void setMethod(const std::string& method);
		void setHost(const std::string& host);
		void setConnection(const std::string& connection);
		void setCookie(const std::string& cookie);
		void setPath(const std::string& path);
		void setKeepAlive(bool keep_alive);
		void setDone(int done);

		void parseRequest(HttpRequest &rea, const std::string& request);

	private:
		std::string _method;
		std::string _host;
		std::string _connection;
		std::string _path;
		std::string _cookie;
		bool 		_keep_alive;
		int 		_done;

};

std::ostream& operator<<(std::ostream &os, const HttpRequest &req);

#endif 