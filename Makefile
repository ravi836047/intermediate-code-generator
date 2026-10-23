CXX := g++
CXXFLAGS := -std=c++17 -D_POSIX_C_SOURCE=200809L -Wall -Wextra -Wpedantic
BUILD := build
TARGET := syntax_analyzer

.PHONY: all clean test

all: $(TARGET)

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/parser.tab.cpp $(BUILD)/parser.tab.hpp: src/parser.y | $(BUILD)
	bison -d -o $(BUILD)/parser.tab.cpp src/parser.y

$(BUILD)/lexer.yy.cpp: src/lexer.l $(BUILD)/parser.tab.hpp | $(BUILD)
	flex -o $@ src/lexer.l

$(TARGET): src/main.cpp src/tokens.cpp src/tokens.h src/ast.cpp src/ast.hpp src/semantic.cpp src/semantic.hpp src/ir.cpp src/ir.hpp $(BUILD)/parser.tab.cpp $(BUILD)/lexer.yy.cpp
	$(CXX) $(CXXFLAGS) -I$(BUILD) -Isrc -o $@ src/main.cpp src/tokens.cpp src/ast.cpp src/semantic.cpp src/ir.cpp $(BUILD)/parser.tab.cpp $(BUILD)/lexer.yy.cpp

test: $(TARGET)
	chmod +x test/run_all_tests.sh
	./test/run_all_tests.sh ./$(TARGET)

clean:
	rm -rf $(BUILD) $(TARGET)
