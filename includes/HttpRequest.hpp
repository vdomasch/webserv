#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include "struct.hpp"

# include <iostream> 

enum RequestState
{
    RECEIVING_HEADER,
    RECEIVING_BODY,
    COMPLETE,
    ERROR
};

class HTTPConfig;

class HttpRequest
{
	public:
		HttpRequest();
		~HttpRequest();

		t_request_state	_state;

		std::string	get_method() const;
		std::string	get_response() const;

		void		append_data(const std::string &data);
		bool		is_ready() const;
		bool		has_error() const;


		bool		getKeepAlive() const;
		std::string	get_response() const;
		std::string get_method() const;
		std::string get_target() const;
		std::string get_header(const std::string& key) const;

		void	set_response(const std::string& response);
		void	set_errorcode(int code);

	private:
		std::string	_raw_data;
		std::string	_body;
		std::string	_header;
		std::string	_method;
		std::string	_target;
		std::string _http_version;
		std::map<std::string, std::string> _headers_map;

		RequestState	_state;
		size_t			_content_length;
		bool			_header_parsed;
		bool			_keep_alive;
		int				_errcode;
		std::string		_response;

		void		parse_headers();
};

std::ostream& operator<<(std::ostream &os, const HttpRequest &req);

#endif 