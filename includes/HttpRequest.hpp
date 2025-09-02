#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include "struct.hpp"
#include "ServerConfig.hpp"

# include <iostream> 

enum RequestState
{
	RECEIVING_HEADER,
	RECEIVING_BODY,
	COMPLETE,
	ERROR,
	RESPONDING,
	RESPONDED
};

class	HTTPConfig;

class	HttpRequest
{
	public:
		HttpRequest();
		~HttpRequest();

		bool			_is_error_request;
		bool			_autoindex;
		bool			_is_php_cgi;
		size_t			_response_sent;
		std::string		_server_name;
		std::string		_location_name;
		std::string		_location_root;
		ServerConfig	_server;

		

		void		append_data(const std::string &data);
		bool		is_ready() const;
		bool		has_error() const;
		bool		is_finished() const;



		bool			getKeepAlive() const;
		bool			get_is_multipart() const;
		bool			get_is_server_socket() const;
		bool			get_is_redirection() const;
		int				get_status_code() const;
		ssize_t			get_content_length() const;
		std::string		get_response() const;
		std::string		get_method() const;
		std::string		get_target() const;
		std::string		get_header(const std::string& key) const;
		std::string		get_body() const;
		std::string		get_boundary() const;
		std::string		get_content_type() const;
		std::string		get_rootpath() const;
		std::string		get_query_string() const;
		std::string		get_redirection() const;
		unsigned long	get_time() const;
		RequestState	get_state() const;

		void	set_is_server_socket(bool is_server_socket);
		void	set_response(const std::string& response);
		void	set_status_code(int code);
		void	set_target(const std::string& target);
		void	set_state(enum RequestState);
		void	set_content_type(const std::string& type);
		void	set_time(unsigned long t);
		void	set_rootpath(const std::string& rootpath);
		void	set_is_redirection(bool value);
		void	set_redirection(const std::string& uri);

	private:
		bool		_is_server_socket;
		std::string	_raw_data;
		std::string	_body;
		std::string	_header;
		std::string	_method;
		std::string	_target;
		std::string	_query_string;
		std::string	_http_version;
		std::string	_rootpath;
		std::string _redirection;
		std::map<std::string, std::string>	_headers_map;

		RequestState	_state;
		size_t			_content_length;
		bool			_header_parsed;
		bool			_keep_alive;
		bool			_is_multipart;
		bool			_is_redirection;
		int				_status_code;
		unsigned long	_last_time;
		std::string		_response;
		std::string		_boundary;
		std::string		_content_type;

		void	parse_headers();
		bool	check_keep_alive() const;
};

std::ostream&	operator<<(std::ostream &os, const HttpRequest &req);

#endif 