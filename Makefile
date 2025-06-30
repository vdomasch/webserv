NAME = webserv

CC = c++
CFLAGS = -Wall -Wextra -Werror -g3 -std=c++98 -MMD -MP

SRCS_DIR = src
OBJS_DIR = build
INCLUDE_DIR = includes

SRV_FILES = main			\
			HTTPConfig		\
			ServerConfig	\
			LocationConfig	\
			parsing_utils	\
			Server			\
			HttpRequest		\
			utils			\
			get_request		\
			post_request	\
			HttpResponse	\
			build_autoindex_page \
			methods_utils	\
			delete_request	\
			CGIContent

SRCS = $(addprefix $(SRCS_DIR)/,$(addsuffix .cpp,$(SRV_FILES)))
OBJS = $(patsubst $(SRCS_DIR)/%.cpp,$(OBJS_DIR)/%.o,$(SRCS))

DEPS_S = $(OBJS:.o=.d)

all : $(NAME)

$(NAME) : $(OBJS) 
	$(CC) $(CFLAGS) -I $(INCLUDE_DIR) -o $@ $^

$(OBJS_DIR)/%.o: $(SRCS_DIR)/%.cpp Makefile | $(OBJS_DIR)
	$(CC) $(CFLAGS) -I $(INCLUDE_DIR) -c $< -o $@

$(OBJS_DIR):
	mkdir -p $(OBJS_DIR)

clean :
	rm -f $(OBJS) $(DEPS)
	rm -rf $(OBJS_DIR)

fclean : clean
	rm -rf $(NAME)

re : fclean all

.PHONY : all clean fclean re

-include $(DEPS_S)