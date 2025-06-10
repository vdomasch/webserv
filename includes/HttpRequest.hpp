#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include "struct.hpp"

# include <iostream> 

enum RequestState
{
    RECEIVING_HEADER,
    RECEIVING_BODY,
    COMPLETE,
    ERROR,
	RESPONDED
};

class HTTPConfig;

class HttpRequest
{
	public:
		HttpRequest();
		~HttpRequest();

		bool		_is_error_request;
		

		void		append_data(const std::string &data);
		bool		is_ready() const;
		bool		has_error() const;
		bool		is_finished() const;



		bool		getKeepAlive() const;
		bool 		get_is_multipart() const;
		std::string	get_response() const;
		std::string get_method() const;
		std::string get_target() const;
		std::string get_header(const std::string& key) const;
		std::string get_body() const;
		std::string get_boundary() const;


		void	set_response(const std::string& response);
		void	set_errorcode(int code);
		void	set_target(const std::string& target);
		void	set_state(enum RequestState);

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
		bool			_is_multipart;
		int				_errcode;
		std::string		_response;
		std::string		_boundary;

		void	parse_headers();
		bool 	check_keep_alive() const;
};

std::ostream& operator<<(std::ostream &os, const HttpRequest &req);

#endif 