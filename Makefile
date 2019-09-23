CXX = g++
CXXFLAGS = -std=c++17 -Wall
OPTIMIZATION = -O3
INCLUDE = -Iinclude
MODULES = $(shell find src -name *.cpp)
NAME = mind-engine
TEST_MODULES = test-framework.cpp
TEST_NAME = mind-engine-test

all: $(NAME)

$(NAME): $(MODULES)
	$(CXX) $(CXXFLAGS) $(OPTIMIZATION) $(CXXFLAGS_WARN_OFF) $(INCLUDE) $^ -o $@

run:
	@make all > /dev/null
	@./mind-engine
	@make clean > /dev/null

.PHONY: clean
clean: 
	rm -f $(NAME) *.o