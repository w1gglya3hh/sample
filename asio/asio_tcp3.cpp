#include <cstddef>

#include <chrono>
#include <exception>
#include <format>
#include <iostream>
#include <memory>
#include <print>
#include <string>
#include <system_error>

#include <asio.hpp>

namespace
{
	class TcpConnection final : public std::enable_shared_from_this<TcpConnection>
	{
		asio::ip::tcp::socket _socket;
		std::string _message;
	public:
		TcpConnection(asio::io_context& ioContext) : _socket{ioContext}, _message{} {}

		void Start()
		{
			_message = std::format("{}\n", std::chrono::system_clock::now());
			asio::async_write(_socket, asio::buffer(_message), [](const std::error_code&, std::size_t) {});
		}

		[[nodiscard]] asio::ip::tcp::socket& Socket() noexcept
		{
			return _socket;
		}
	};

	class TcpServer final
	{
		asio::io_context& _ioContext;
		asio::ip::tcp::acceptor _acceptor;

		void StartAccept()
		{
			auto connection{std::make_shared<TcpConnection>(_ioContext)};
			_acceptor.async_accept(connection->Socket(), [this, connection](const std::error_code& errorCode)
			{
				HandleAccept(connection, errorCode);
			});
		}

		void HandleAccept(std::shared_ptr<TcpConnection> connection, const std::error_code& errorCode)
		{
			if (!errorCode)
				connection->Start();
			StartAccept();
		}
	public:
		TcpServer(asio::io_context& ioContext)
			: _ioContext{ioContext}, _acceptor{ioContext, asio::ip::tcp::endpoint{asio::ip::tcp::v4(), 13}}
		{
			StartAccept();
		}
	};
}

int main()
{
	try
	{
		asio::io_context ioContext{};
		TcpServer tcpServer{ioContext};
		ioContext.run();
	}
	catch (const std::exception& exception)
	{
		std::println(std::cerr, "exception caught: {}", exception.what());
		return 1;
	}
}
