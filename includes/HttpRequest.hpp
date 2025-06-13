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
		bool		is_server_socket() const;
		std::string	get_response() const;
		std::string get_method() const;
		std::string get_target() const;
		std::string get_header(const std::string& key) const;
		std::string get_body() const;
		std::string get_boundary() const;
		std::string get_content_type() const;
		time_t		get_time() const;
		std::string get_rootpath() const; //for CGI full path, is to be modified
		RequestState get_state() const;



		void	set_is_server_socket(bool is_server_socket);
		void	set_response(const std::string& response);
		void	set_errorcode(int code);
		void	set_target(const std::string& target);
		void	set_state(enum RequestState);
		void	set_content_type(const std::string& type);
		void	set_time(unsigned long t);
		void	set_rootpath(const std::string& rootpath); //for CGI full path, is to be modified

	private:
		bool		_is_server_socket;
		std::string	_raw_data;
		std::string	_body;
		std::string	_header;
		std::string	_method;
		std::string	_target;
		std::string	_query_string;
		std::string _http_version;
		std::string _rootpath;   //for CGI full path, is to be modified
		std::map<std::string, std::string> _headers_map;

		RequestState	_state;
		size_t			_content_length;
		bool			_header_parsed;
		bool			_keep_alive;
		bool			_is_multipart;
		int				_errcode;
		time_t			_last_time;
		std::string		_response;
		std::string		_boundary;
		std::string		_content_type;
		std::string		_location_name;
		std::string		_server_name;

		void	parse_headers();
		bool 	check_keep_alive() const;
};

std::ostream& operator<<(std::ostream &os, const HttpRequest &req);

#endif 