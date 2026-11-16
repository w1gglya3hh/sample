CXX=c++
CXXFLAGS=-Wall -Wextra -Wpedantic

all: asio_tcp1 asio_tcp2 asio_tcp3 asio_tm1 asio_tm2 asio_tm3 asio_tm4

.cpp:
	$(CXX) -std=c++23 $(CXXFLAGS) -I$(HOME)/Repository/asio.git/include -o $@ $<

clean:
	$(RM) asio_tcp1 asio_tcp2 asio_tcp3 asio_tm1 asio_tm2 asio_tm3 asio_tm4

.PHONY: all clean
