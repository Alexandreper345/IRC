NAME		= ircserv

CXX			= c++
CXXFLAGS	= -Wall -Wextra -Werror -std=c++98
INCLUDES	= -I./include

SRC_DIR		= src
OBJ_DIR		= obj

SRCS		= main.cpp \
			  $(SRC_DIR)/server/Server.cpp \
			  $(SRC_DIR)/server/ServerCommands.cpp \
			  $(SRC_DIR)/server/serverExceptions.cpp \
			  $(SRC_DIR)/client/Client.cpp

OBJS		= $(SRCS:%.cpp=$(OBJ_DIR)/%.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)

$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
