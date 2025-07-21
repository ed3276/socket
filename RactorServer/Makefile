CXX := g++
CXXFLAGS := -g -std=c++14 -Wall -Wextra -pthread
LDFLAGS := 

SRC_DIR := ./
BUILD_DIR := build

SERVER_INC := 
SERVER_SRC := server.cpp InetAddress.cpp Socket.cpp Epoll.cpp Channel.cpp
SERVER_OBJ := $(addprefix $(BUILD_DIR)/, $(SERVER_SRC:.cpp=.o))

CLIENT_INC := 
CLIENT_SRC := client.cpp
CLIENT_OBJ := $(addprefix $(BUILD_DIR)/, $(CLIENT_SRC:.cpp=.o))

$(shell mkdir -p $(BUILD_DIR))

CLIENT_TARGET := $(BUILD_DIR)/client
SERVER_TARGET := $(BUILD_DIR)/server

all: $(CLIENT_TARGET) $(SERVER_TARGET)

$(CLIENT_TARGET): $(CLIENT_OBJ)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

$(SERVER_TARGET): $(SERVER_OBJ)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.o: %.cpp
		$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY: clean all
clean:
	rm -rf $(BUILD_DIR)

