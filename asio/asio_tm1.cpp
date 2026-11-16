#include <print>
#include <system_error>

#include <asio.hpp>

namespace
{
	void print(const std::error_code&)
	{
		std::println("Hello World");
	}
}

int main()
{
	asio::io_context ioContext{};
	asio::steady_timer timer{ioContext, asio::chrono::seconds{5}};
	timer.async_wait(&print);
	ioContext.run();
}

