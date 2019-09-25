CXX = g++
CXXFLAGS = -std=c++17 -Wall
OPTIMIZATION = -O3
INCLUDE = -Iinclude
MODULES = $(shell find src -name *.cpp)
NAME = wtdal-engine

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