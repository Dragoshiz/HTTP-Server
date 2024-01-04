# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: dodo <dodo@student.42.fr>                  +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2023/05/16 17:24:49 by vfuhlenb          #+#    #+#              #
#    Updated: 2023/10/23 17:41:09 by dodo             ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME = webserv
TESTSERV = testserv

CC = c++

CFLAGS = -Wall -Werror -Wextra -std=c++98 -g #-fsanitize=address

SRCS =	src/main.cpp \
		src/configParser.cpp \
		src/Client.class.cpp \
		src/getMethod.cpp \
		src/postMethod.cpp \
		src/DeleteMethod.cpp\
		src/CGI.cpp \
		src/Errors.cpp \

OBJS = ${SRCS:.cpp=.o}

RM = rm -rf

MKDIR = mkdir -p

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

all: $(NAME)

testing: $(TESTSERV)

$(TESTSERV): $(OBJS)
	$(CC) $(CFLAGS) -D Debug=1 -o $(NAME) $(OBJS)
	@echo "Testserv created: \033[32;1m./$(TESTSERV)\033[0m"

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) -o $(NAME) $(OBJS)
	@echo "executable created: \033[32;1m./$(NAME)\033[0m"

clean:
	$(RM) $(OBJS)
	@echo "\033[34mclean success\033[0m"

fclean:
	$(RM) $(OBJS) $(NAME) $(TESTSERV) siegeResult
	@echo "\033[34mfclean success\033[0m"

re: fclean all

run:all
	./webserv

test:testing
	./webserv ./data/conf/test.conf

siege:
	./tests/siege10.sh

siegebig:
	./tests/siege10000.sh

valgrind:
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --track-fds=yes  ./webserv

.PHONY: all clean fclean re build start stop
