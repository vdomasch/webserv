#include "webserv.hpp"
#include <sstream>

void	parse_request(t_browser_request &request, char *buffer)
{
	if (buffer == NULL || request.bytes_read == 0)
		return ;

	request.done = 0;

	std::string str(buffer);	
	std::istringstream iss(str);
	std::getline(iss, request.method, ' ');
	std::getline(iss, request.location, ' ');

	std::string compare[] = {"Host", "Accept", "Connection"};
	bool found_flags[3] = {false, false, false};

	while (request.done < 3)
	{
		std::string line;
		std::getline(iss, line);

		for (int i = 0; i < 3; i++)
		{
			if (!found_flags[i])
			{
				std::size_t found = line.find(compare[i]);
				if (found != std::string::npos)
				{
					std::string value = line.substr(found + compare[i].length() + 2);
					switch (i)
					{
						case 0:
							request.host = value;
							break;
						case 1:
							request.accept = value;
							break;
						case 2:
							request.connection = value;
							break;
					}
					found_flags[i] = true;
					request.done++;
				}
			}
		}
	}
}