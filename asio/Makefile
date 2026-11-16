CXX=c++
CXXFLAGS=-Wall -Wextra -Wpedantic

all: $(patsubst %.cpp, %.x, $(wildcard *.cpp))

%.x: %.cpp
	$(CXX) -std=c++23 $(CXXFLAGS) -I$(HOME)/Repository/asio.git/include -o $@ $<

clean:
	$(RM) $(wildcard *.x)

.PHONY: all clean
