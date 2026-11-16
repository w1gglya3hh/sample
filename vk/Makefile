CXX=c++
CXXFLAGS=-Wall -Wextra -Wpedantic

all: $(patsubst %.cpp, %.x, $(wildcard *.cpp))

%.x: %.cpp
	$(CXX) -std=c++23 $(CXXFLAGS) $(shell pkg-config --cflags sdl3 vulkan) $(shell pkg-config --libs sdl3) -o $@ $<

clean:
	$(RM) $(wildcard *.x)

.PHONY: all clean
