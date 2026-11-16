CXX=c++
CXXFLAGS=-Wall -Wextra -Wpedantic

all: vk1

.cpp:
	$(CXX) -std=c++23 $(CXXFLAGS) $(shell pkg-config --cflags sdl3 vulkan) $(shell pkg-config --libs sdl3) -o $@ $<

clean:
	$(RM) vk1

.PHONY: all clean
