#include <chrono>
#include <exception>
#include <format>
#include <iostream>
#include <string>
#include <system_error>

#include <asio.hpp>

int main()
{
	try
	{
		asio::io_context ioContext{};
		asio::ip::tcp::acceptor tcpAcceptor{ioContext, asio::ip::tcp::endpoint{asio::ip::tcp::v4(), 13}};

		for (;;)
		{
			asio::ip::tcp::socket tcpSocket{ioContext};
			std::error_code errorCode{};
			const std::string message{std::format("{}\n", std::chrono::system_clock::now())};

			tcpAcceptor.accept(tcpSocket);
			asio::write(tcpSocket, asio::buffer(message), errorCode);
			(void)errorCode;
		}
	}
	catch (const std::exception& exception)
	{
		std::println(std::cerr, "exception caught: {}", exception.what());
		return 1;
	}
}
