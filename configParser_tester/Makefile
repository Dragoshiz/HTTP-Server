# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: vfuhlenb <vfuhlenb@students.42wolfsburg    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2023/05/16 17:24:49 by vfuhlenb          #+#    #+#              #
#    Updated: 2023/09/16 19:58:06 by vfuhlenb         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME = webserv

CC = c++

CFLAGS = -Wall -Werror -Wextra -std=c++98 -g

SRCS =	main.cpp \
		../src/configParser.cpp

OBJS = ${SRCS:.cpp=.o}

RM = rm -rf

MKDIR = mkdir -p

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) -o $(NAME) $(OBJS)
	@echo "executable created: \033[32;1m./$(NAME)\033[0m"

clean:
	$(RM) $(OBJS)
	@echo "\033[34mclean success\033[0m"

fclean:
	$(RM) $(OBJS) $(NAME)
	@find log/ -type f ! -name .gitkeep -exec $(RM) {} \;
	@echo "\033[34mfclean success\033[0m"

re: fclean all

.PHONY: all clean fclean re build
