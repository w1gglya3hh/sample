CXX=c++
CXXFLAGS=-Wall -Wextra -Wpedantic

all: $(patsubst %.cpp, %.x, $(wildcard *.cpp))

%.x: %.cpp
	$(CXX) -std=c++23 $(CXXFLAGS) $(shell pkg-config --cflags --libs sdl3) -Wl,-framework,OpenGL -o $@ $<

clean:
	$(RM) $(wildcard *.x)

.PHONY: all clean
