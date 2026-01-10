#include "application.h"
#include "dummy.h"
#include "mail.h"
#include "library/iocp/monitor.h"
#include "library/iocp/timer.h"

application::application(void) noexcept
	: _network(200, 0xaabbccddeeff, 128), _system(50) {
	_stop_count.store(2);
	thread_monitor();
	wake_loop();


}
application::~application(void) noexcept {
	_stop_source.request_stop();
	for (int stop_count; 0 != (stop_count = _stop_count.load()); )
		_stop_count.wait(stop_count);
	_network.session_clear();
}

void application::execute(void) noexcept {
	_command.add("Client_Count", [&](std::span<command::parameter> arg) {
		_option._client_count = arg[0].get<int>();
		});
	_command.add("Disconnect_Test", [&](std::span<command::parameter> arg) {
		_option._disconnect_test = arg[0].get<bool>();
		});
	_command.add("Over_Send", [&](std::span<command::parameter> arg) {
		_option._over_send = arg[0].get<int>();
		});
	_command.add("Action_Delay", [&](std::span<command::parameter> arg) {
		_option._action_delay = arg[0].get<int>();
		});
	_command.add("Start", [&](std::span<command::parameter> arg) {
		for (auto index = 0u; index < _option._client_count; ++index) {
			auto pointer = new dummy;
			auto handle = _system.entity_attach(pointer, [](actor::entity* pointer) {
				delete pointer;
				});
			_dummy_lock.lock();
			_dummy_handle.emplace(handle);
			_dummy_lock.unlock();
		}
		});
	_command.add("Stop", [&](std::span<command::parameter> arg) {
		_dummy_lock.lock();
		for (auto& iter : _dummy_handle)
			_system.entity_destroy(iter);
		_dummy_handle.clear();
		_dummy_lock.unlock();
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

auto application::thread_monitor(void) noexcept -> iocp::coroutine<void> {
	auto stop_token = _stop_source.get_token();
	while (false == stop_token.stop_requested()) {
		//printf("Thread Monitor\n");
		co_await iocp::sleep(1000);
	}
	_stop_count.fetch_sub(1);
	_stop_count.notify_all();
	co_return;
}

auto application::wake_loop(void) noexcept -> iocp::coroutine<void> {
	auto stop_token = _stop_source.get_token();
	while (false == stop_token.stop_requested()) {
		_dummy_lock.lock();
		for (auto& iter : _dummy_handle) 
			_system.entity_enqueue(iter, actor::mail(mail_type::dummy_tick));
		_dummy_lock.unlock();

		co_await iocp::sleep(500);
	}
	_stop_count.fetch_sub(1);
	_stop_count.notify_all();
	co_return;
}
