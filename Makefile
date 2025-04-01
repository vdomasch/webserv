# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: bhumeau <bhumeau@student.42.fr>            +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2024/08/01 16:57:20 by lchapard          #+#    #+#              #
#    Updated: 2025/03/31 11:23:30 by bhumeau          ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME_S = server
NAME_C = client

CC = c++
CFLAGS = -Wall -Wextra -Werror -g3 -std=c++98 -MMD -MP 

SRCS_DIR = src
OBJS_DIR = build
INCLUDE_DIR = includes

SRV_FILES = server			\
			HTTPConfig		\
			ServerConfig	\
			LocationConfig	\
			parsing_utils

CLIENT_FILES = client

SRCS_C = $(addprefix $(SRCS_DIR)/,$(addsuffix .cpp,$(CLIENT_FILES)))
SRCS_S = $(addprefix $(SRCS_DIR)/,$(addsuffix .cpp,$(SRV_FILES)))
OBJS_C = $(patsubst $(SRCS_DIR)/%.cpp,$(OBJS_DIR)/%.o,$(SRCS_C))
OBJS_S = $(patsubst $(SRCS_DIR)/%.cpp,$(OBJS_DIR)/%.o,$(SRCS_S))

DEPS_C = $(OBJS_C:.o=.d)
DEPS_S = $(OBJS_S:.o=.d)

all : $(NAME_C) $(NAME_S)

$(NAME_C) : $(OBJS_C) 
	$(CC) $(CFLAGS) -I $(INCLUDE_DIR) -o $@ $^

$(NAME_S) : $(OBJS_S) 
	$(CC) $(CFLAGS) -I $(INCLUDE_DIR) -o $@ $^

$(OBJS_DIR)/%.o: $(SRCS_DIR)/%.cpp Makefile | $(OBJS_DIR)
	$(CC) $(CFLAGS) -I $(INCLUDE_DIR) -c $< -o $@

$(OBJS_DIR):
	mkdir -p $(OBJS_DIR)

clean :
	rm -f $(OBJS_S) $(OBJS_C) $(DEPS_C) $(DEPS_S)
	rm -rf $(OBJS_DIR)

fclean : clean
	rm -rf $(NAME_C) $(NAME_S)

re : fclean all

.PHONY : all clean fclean re

-include $(DEPS_C) $(DEPS_S)