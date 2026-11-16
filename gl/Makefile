CXX=c++
CXXFLAGS=-Wall -Wextra -Wpedantic

all: gl1 gl2 gl3 gl4 gl5 gl6

.cpp:
	$(CXX) -std=c++23 $(CXXFLAGS) $(shell pkg-config --cflags --libs sdl3) -Wl,-framework,OpenGL -o $@ $<

clean:
	$(RM) gl1 gl2 gl3 gl4 gl5 gl6

.PHONY: all clean
