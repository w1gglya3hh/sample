#include <functional>
#include <print>
#include <system_error>

#include <asio.hpp>

namespace
{
	void print(const std::error_code&, asio::steady_timer& timer, int& count)
	{
		if (count < 5)
		{
			std::println("{}", count++);
			timer.expires_at(timer.expiry() + asio::chrono::seconds{1});
			timer.async_wait([&](const std::error_code& errorCode) {
				print(errorCode, timer, count);
			});
		}
	}
}

int main()
{
	asio::io_context ioContext{};
	asio::steady_timer timer{ioContext, asio::chrono::seconds{1}};
	int count{};
	timer.async_wait([&](const std::error_code& errorCode) {
		print(errorCode, timer, count);
	});
	ioContext.run();
	std::println("Count: {}", count);
}

