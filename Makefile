CXX := g++
CXXFLAGS := -Wall -Wextra -std=c++23 -g
LDFLAGS :=

SRC := repl.cc buffer.cc
OBJ := $(SRC:.cc=.o)
TARGET := ace

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(OBJ) -o $(@) $(LDFLAGS)

%.o: %.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)