#include "application.h"
#include "library/iocp/monitor.h"
#include "library/iocp/timer.h"

application::application(void) noexcept
	: _network(*this, 200, 40000, 4000, 0xaabbccddeeff, 128, 40000, 4000) {
	_stop_count.store(1);
	thread_monitor();

	_command.add("Client_Count", [&](std::span<command::parameter> arg) {
		_client_count = arg[0].get<int>();
		});
	_command.add("Disconnect_Test", [&](std::span<command::parameter> arg) {
		_disconnect_test = arg[0].get<bool>();
		});
	_command.add("Over_Send", [&](std::span<command::parameter> arg) {
		_over_send = arg[0].get<int>();
		});
	_command.add("Action_Delay", [&](std::span<command::parameter> arg) {
		_action_delay = arg[0].get<int>();
		});
	_command.add("Start", [&](std::span<command::parameter> arg) {
		for(auto index =0; index < _client_count; ++index)
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
	_network.session_clear();
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
