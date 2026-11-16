#include <cstddef>

#include <array>
#include <chrono>
#include <exception>
#include <iostream>
#include <print>
#include <system_error>

#include <asio.hpp>

int main(int argc, char** argv)
{
	if (argc != 2)
	{
		std::println(std::cerr, "usage: {} [host]", argv[0]);
		return 1;
	}

	try
	{
		asio::io_context ioContext{};
		asio::ip::tcp::resolver tcpResolver{ioContext};
		const auto endPoints{tcpResolver.resolve(argv[1], "daytime")};

		asio::ip::tcp::socket tcpSocket{ioContext};
		asio::connect(tcpSocket, endPoints);

		for (;;)
		{
			std::array<char, 128> buffer{};
			std::error_code errorCode{};

			const std::size_t length{tcpSocket.read_some(asio::buffer(buffer), errorCode)};
			if (length)
				std::cout.write(buffer.data(), length);
			if (errorCode == asio::error::eof)
				break;
			else if (errorCode)
				throw std::system_error{errorCode};
		}
	}
	catch (const std::exception& exception)
	{
		std::println(std::cerr, "exception caught: {}", exception.what());
		return 1;
	}
}
