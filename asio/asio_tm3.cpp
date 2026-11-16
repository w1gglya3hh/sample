#include <functional>
#include <print>
#include <system_error>

#include <asio.hpp>

namespace
{
	class Printer final
	{
		asio::steady_timer _timer;
		int _count;
	public:
		Printer(asio::io_context& ioContext) : _timer{ioContext, asio::chrono::seconds{1}}, _count{}
		{
			_timer.async_wait([this](const std::error_code&) { Print(); });
		}

		~Printer()
		{
			std::println("Count: {}", _count);
		}

		void Print()
		{
			if (_count < 5)
			{
				std::println("{}", _count++);
				_timer.expires_at(_timer.expiry() + asio::chrono::seconds{1});
				_timer.async_wait([this](const std::error_code&) { Print(); });
			}
		}
	};
}

int main()
{
	asio::io_context ioContext{};
	Printer printer{ioContext};
	ioContext.run();
}

