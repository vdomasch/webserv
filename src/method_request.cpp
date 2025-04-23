#include "webserv.hpp"

void	get_request(HttpRequest &req, std::map<std::string, ServerConfig> &server_list)
{
	static_cast<void>(server_list);

	std::cout << "GET request received" << std::endl;
	std::cout << req << std::endl;
}

void	post_request(HttpRequest &req, std::map<std::string, ServerConfig> &server_list)
{
	static_cast<void>(server_list);

	std::cout << "POST request received" << std::endl;
	std::cout << req << std::endl;

	std::string response = create_header("200 OK", "text/plain", "16", "keep-alive");

}

void	delete_request(HttpRequest &req, std::map<std::string, ServerConfig> &server_list)
{
	static_cast<void>(server_list);
	
	std::cout << "DELETE request received" << std::endl;
	std::cout << req << std::endl;


}

std::string create_header(const std::string &status, const std::string &content_type, const std::string &content_length, const std::string &connection)
{
	std::string header = "HTTP/1.1 " + status + "\r\n";
	header += "Content-Type: " + content_type + "\r\n";
	header += "Content-Length: " + content_length + "\r\n";
	header += "Connection: " + connection + "\r\n\r\n";

	std::cout << "Header created:\n" << header << std::endl;
	return header;
}