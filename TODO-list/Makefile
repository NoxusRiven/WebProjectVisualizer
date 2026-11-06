#CFLAGS = -O3 -std=c++23 -I"/mnt/d/C++ libs/asio-1.30.2/include"
CFLAGS = -std=c++23 -I"/mnt/d/C++ libs/asio-1.30.2/include"

SRC_DIR = src
BIN_DIR = bin

SERVER_ROOT = $(SRC_DIR)/server_tcp/server
DOM_GEN_ROOT = $(SRC_DIR)/DOM_Generator/DOM_Generator
JSON_ROOT = $(SRC_DIR)/JSON/JSON_Parser

TODO_DIR = $(SRC_DIR)/TODO

all: $(BIN_DIR)/main

run: $(BIN_DIR)/main
	./$(BIN_DIR)/main

$(BIN_DIR)/server.o: $(SERVER_ROOT).cpp $(SERVER_ROOT).hpp
	g++ $(CFLAGS) -c $< -o $@

$(BIN_DIR)/DOM_Generator.o: $(DOM_GEN_ROOT).cpp $(DOM_GEN_ROOT).hpp
	g++ $(CFLAGS) -c $< -o $@	

$(BIN_DIR)/json.o: $(JSON_ROOT).cpp $(JSON_ROOT).hpp
	g++ $(CFLAGS) -c $< -o $@

$(BIN_DIR)/TODO_web.o: $(TODO_DIR)/web/TODO.cpp $(TODO_DIR)/lib/TODO.hpp $(TODO_DIR)/web/TODO.hpp $(JSON_ROOT).hpp $(DOM_GEN_ROOT).hpp
	g++ $(CFLAGS) -c $< -o $@

$(BIN_DIR)/TODO_lib.o: $(TODO_DIR)/lib/TODO.cpp $(TODO_DIR)/lib/TODO.hpp
	g++ $(CFLAGS) -c $< -o $@

$(BIN_DIR)/main.o: $(SRC_DIR)/main.cpp $(TODO_DIR)/web/TODO.hpp
	g++ $(CFLAGS) -c $< -o $@


$(BIN_DIR)/main: $(BIN_DIR)/main.o $(BIN_DIR)/server.o $(BIN_DIR)/DOM_Generator.o $(BIN_DIR)/json.o  $(BIN_DIR)/TODO_lib.o $(BIN_DIR)/TODO_web.o
	g++ $^ -o $@

clean:
	rm -f $(BIN_DIR)/*.o $(BIN_DIR)/main

.PHONY: all run