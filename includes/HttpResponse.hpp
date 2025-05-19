#ifndef HTTPRESPONSE_HPP
# define HTTPRESPONSE_HPP


# include <string>
# include <map>
# include <sstream>

class HttpResponse
{
private:
    int _status_code;
    std::string _status_message;
    std::map<std::string, std::string> _headers;
    std::string _body;

public:
    HttpResponse();
	~HttpResponse() {}

	std::string get_body() const;

    void set_status(int code, const std::string& message);
    void set_body(const std::string& body);
    void add_header(const std::string& key, const std::string& value);

    std::string generate_response() const;
};

#endif
