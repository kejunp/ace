CXX := g++
CXXFLAGS := -std=c++20 -Wall -Wextra -Werror -pedantic -g -Icommand -Ieditor -Iui
LDFLAGS := 
TARGET := ace

SRC := \
    repl.cc \
    command/command.cc \
    editor/buffer.cc \
    ui/render.cc \
    ui/term.cc

OBJ := $(SRC:.cc=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(OBJ) -o $@ $(LDFLAGS)

%.o: %.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)
