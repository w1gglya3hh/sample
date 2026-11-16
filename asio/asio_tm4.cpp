#include <functional>
#include <print>
#include <system_error>
#include <thread>

#include <asio.hpp>

namespace
{
	class Printer final
	{
		asio::strand<asio::io_context::executor_type> _strand;
		asio::steady_timer _timer1;
		asio::steady_timer _timer2;
		int _count;
	public:
		Printer(asio::io_context& ioContext)
			: _strand{asio::make_strand(ioContext)}
			, _timer1{ioContext, asio::chrono::seconds{1}}
			, _timer2{ioContext, asio::chrono::seconds{1}}
			, _count{}
		{
			_timer1.async_wait(asio::bind_executor(_strand, [this](const std::error_code&) { Print1(); }));
			_timer2.async_wait(asio::bind_executor(_strand, [this](const std::error_code&) { Print2(); }));
		}

		~Printer()
		{
			std::println("Count: {}", _count);
		}

		void Print1()
		{
			if (_count < 10)
			{
				std::println("1: {}", _count++);
				_timer1.expires_at(_timer1.expiry() + asio::chrono::seconds{1});
				_timer1.async_wait(asio::bind_executor(_strand, [this](const std::error_code&) { Print1(); }));
			}
		}

		void Print2()
		{
			if (_count < 10)
			{
				std::println("2: {}", _count++);
				_timer2.expires_at(_timer2.expiry() + asio::chrono::seconds{2});
				_timer2.async_wait(asio::bind_executor(_strand, [this](const std::error_code&) { Print2(); }));
			}
		}
	};
}

int main()
{
	asio::io_context ioContext{};
	Printer printer{ioContext};
	std::jthread job{[&]() { ioContext.run(); }};
	ioContext.run();
}

