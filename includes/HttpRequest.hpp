#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

# include <iostream> 

class HTTPConfig;

typedef struct s_requeste_state	t_requeste_state;

class HttpRequest
{
	public:
		HttpRequest();
		~HttpRequest();

		std::string	getMethod() const;
		std::string	getHost() const;
		std::string	getConnection() const;
		std::string	getContentLength() const;
		std::string	getContentType() const;
		std::string	getCookie() const;
		std::string	getPath() const;
		std::string	getPort() const;
		bool		getKeepAlive() const;
		int			getDone() const;
		std::string	getResponse() const;

		void	setMethod(const std::string& method);
		void	setHost(const std::string& host);
		void	setConnection(const std::string& connection);
		void	setContentLength(const std::string& content_length);
		void	setContentType(const std::string& content_type);
		void	setCookie(const std::string& cookie);
		void	setPath(const std::string& path);
		void	setPort(const std::string& port);
		void	setKeepAlive(bool keep_alive);
		void	setDone(int done);
		void	setResponse(const std::string& response);

		void	parseRequest(const std::string& request, int port);

		int		analyseHeader(t_requeste_state &state, int port);

		void	constructBody(t_requeste_state &state, int port);

		bool	check_if_body_size_greater_than_limit(t_requeste_state &state, int port, HTTPConfig &http_config);

	private:
		std::string	_method;
		std::string	_host;
		std::string	_connection;
		std::string	_content_length;
		std::string	_content_type;
		std::string	_path;
		std::string	_port;
		std::string	_cookie;
		bool 		_keep_alive;
		int 		_done;

		std::string	_response;
};

std::ostream& operator<<(std::ostream &os, const HttpRequest &req);

#endif 