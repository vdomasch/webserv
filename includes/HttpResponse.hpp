#ifndef HTTPRESPONSE_HPP
# define HTTPRESPONSE_HPP


# include <string>
# include <map>
# include <sstream>

class HttpResponse
{
private:
	bool			_keep_alive;
    std::string		_status_code;
    std::string		_status_message;
    std::string		_body;
	
    std::map<std::string, std::string>	_headers;

public:
    HttpResponse();
	~HttpResponse() {}

	std::string get_body() const;
	bool get_keep_alive() const;


	void set_keep_alive(bool keep_alive);
    void set_status(const std::string& code, const std::string& message);
    void set_body(const std::string& body);
    void add_header(const std::string& key, const std::string& value);

    std::string generate_response() const;
};

#endif
