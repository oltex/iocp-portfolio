#include "dummy.h"
#include "session.h"
#include "library/iocp/timer.h"
#include "library/iocp/actor/system.h"

tick::tick(void) noexcept
	: actor::job(0) {
}

dummy::dummy(network& network) noexcept
	: _network(network), _state(state::disconnect) {
	_stop_count.store(1);
	wake_loop();
}
dummy::~dummy(void) noexcept {
	_stop_source.request_stop();
	for (int stop_count; 0 != (stop_count = _stop_count.load()); )
		_stop_count.wait(stop_count);
}
auto dummy::callback(actor::job& job) noexcept -> iocp::coroutine<bool> {
	if (1 != library::interlock_increment(_test))
		int a = 10;
	switch (_state) {
	case disconnect:
		co_await state_disconnect(job);
		break;
	case connect:
		co_await state_disconnect(job);
		break;
	}
	if (0 != library::interlock_decrement(_test))
		int a = 10;
	co_return true;
}
auto dummy::wake_loop(void) noexcept -> iocp::coroutine<void> {
	co_await iocp::sleep(1000);
	auto& instance = actor::system::instance();
	auto stop_token = _stop_source.get_token();
	while (false == stop_token.stop_requested()) {
		instance.entity_enqueue(arena_handle(), tick());
		co_await iocp::sleep(1000);
	}
	_stop_count.fetch_sub(1);
	_stop_count.notify_all();
	co_return;
}
auto dummy::state_disconnect(actor::job& job) noexcept -> iocp::coroutine<void> {
	switch (job._type) {
	case 0: {
		auto function = _network.socket_connect(library::socket_address_ipv4("127.0.0.1", 6000));
		if (auto result = co_await function; 0 != result) {
			library::pair<tcp::session*, void(*)(tcp::session*)> pair{
				new session(*reinterpret_cast<library::socket*>(result), 40000, 512, 40000, 3000),
				session::session_destruct
			};
			_session_handle = reinterpret_cast<tcp::handle>(co_await function(reinterpret_cast<void*>(&pair)));
			co_await function;
		}
		else 
			_state = error;
		break;
	}
	default:
		__debugbreak();
	}
	co_return;
}
auto dummy::state_connect(actor::job& job) noexcept -> iocp::coroutine<void> {
	co_return;
}
