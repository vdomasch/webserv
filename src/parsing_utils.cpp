/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parsing_utils.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bhumeau <bhumeau@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/25 09:57:04 by lchapard          #+#    #+#             */
/*   Updated: 2025/04/04 11:10:47 by bhumeau          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/parsing_utils.hpp"

std::string	clean_semicolon(std::string text)
{
	while (text.at(text.length() - 1) == ';')
		text.erase(text.length() - 1);
	return text;
}

bool is_keyword(std::string key, std::string pattern)
{
	if (key == pattern/* || key == "{"*/ || key.empty() || key == "}")
		return (true);
	return (false);
}

bool	is_error_page_code(std::string code)
{
	int int_code = atoi(code.c_str());

	if (code.length() != 3)
		;
	else if (int_code >= 400 && int_code <= 599)
		return true;
	return false;
}