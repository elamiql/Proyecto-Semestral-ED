CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2

SRC_DIR  = src
INC_DIR  = include
OBJ_DIR  = build

SRCS = $(SRC_DIR)/Graph.cpp $(SRC_DIR)/GraphLoader.cpp $(SRC_DIR)/Metrics.cpp $(SRC_DIR)/main.cpp
OBJS = $(OBJ_DIR)/Graph.o   $(OBJ_DIR)/GraphLoader.o   $(OBJ_DIR)/Metrics.o   $(OBJ_DIR)/main.o

TARGET = proyecto_ed

all: $(OBJ_DIR) $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(OBJ_DIR)/Graph.o: $(SRC_DIR)/Graph.cpp $(INC_DIR)/Graph.hpp
	$(CXX) $(CXXFLAGS) -I$(INC_DIR) -c $< -o $@

$(OBJ_DIR)/GraphLoader.o: $(SRC_DIR)/GraphLoader.cpp $(INC_DIR)/GraphLoader.hpp $(INC_DIR)/Graph.hpp
	$(CXX) $(CXXFLAGS) -I$(INC_DIR) -c $< -o $@

$(OBJ_DIR)/Metrics.o: $(SRC_DIR)/Metrics.cpp $(INC_DIR)/Metrics.hpp $(INC_DIR)/Graph.hpp
	$(CXX) $(CXXFLAGS) -I$(INC_DIR) -c $< -o $@

$(OBJ_DIR)/main.o: $(SRC_DIR)/main.cpp $(INC_DIR)/Graph.hpp $(INC_DIR)/GraphLoader.hpp
	$(CXX) $(CXXFLAGS) -I$(INC_DIR) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

clean:
	if exist $(OBJ_DIR) rmdir /s /q $(OBJ_DIR)
	if exist $(TARGET).exe del /q $(TARGET).exe
	if exist -p rmdir -p

.PHONY: all clean