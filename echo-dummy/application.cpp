#include "application.h"
#include "library/iocp/monitor.h"
#include "library/iocp/timer.h"

application::application(void) noexcept
	: _echo_network(200, 0xaabbccddeeff) {
	_echo_network.listen_start(library::socket_address_ipv4("127.0.0.1", 6000), 65535);

	_stop_count.store(1);
	thread_monitor();

	_command.add("Test", [&](std::span<command::parameter> arg) {
		printf("HELLOOOOOOOOOOOOO");
		});
	for (;;) {
		std::string buffer;
		if (!std::getline(std::cin, buffer))
			break;
		parser parser;
		parser.execute(buffer);
		for (auto& iter : parser) 
			_command.execute(iter.first, iter.second);
	}
}
application::~application(void) noexcept {
	_stop_source.request_stop();
	for (int stop_count; 0 != (stop_count = _stop_count.load()); )
		_stop_count.wait(stop_count);
	_echo_network.listen_stop();
	_echo_network.session_clear();
}

auto application::thread_monitor(void) noexcept -> iocp::coroutine<void> {
	auto stop_token = _stop_source.get_token();
	while (false == stop_token.stop_requested()) {
		co_await iocp::sleep(1000);
	}
	_stop_count.fetch_sub(1);
	_stop_count.notify_all();
	co_return;
}
