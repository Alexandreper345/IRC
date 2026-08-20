NAME        = ircserv

CXX         = c++
CXXFLAGS    = -Wall -Wextra -Werror -std=c++98

INCLUDES    = -I./include

SRC_DIR     = src
OBJ_DIR     = obj

SRCS        = main.cpp \
              $(SRC_DIR)/server/Server.cpp \
              $(SRC_DIR)/client/Client.cpp \
              $(SRC_DIR)/server/serverExceptions.cpp

OBJS        = $(OBJ_DIR)/main.o \
              $(OBJ_DIR)/server/Server.o \
              $(OBJ_DIR)/client/Client.o \
              $(OBJ_DIR)/server/serverExceptions.o

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)

$(OBJ_DIR)/main.o: main.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_DIR)/server/%.o: $(SRC_DIR)/server/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_DIR)/client/%.o: $(SRC_DIR)/client/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re