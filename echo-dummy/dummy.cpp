#include "dummy.h"
#include "library/iocp/timer.h"
#include "library/iocp/actor/system.h"

dummy::dummy(network& network) noexcept
	: _network(network) {
	_stop_count.store(1);
	wake_loop();
}
dummy::~dummy(void) noexcept {
	_stop_source.request_stop();
	for (int stop_count; 0 != (stop_count = _stop_count.load()); )
		_stop_count.wait(stop_count);
	printf("Finish\n");
}

auto dummy::callback(actor::job& job) noexcept -> iocp::coroutine<bool> {
	printf("HELLO!\n");
	co_return true;
}
auto dummy::wake_loop(void) noexcept -> iocp::coroutine<void> {
	co_await iocp::sleep(1000);
	auto& instance = actor::system::instance();
	auto stop_token = _stop_source.get_token();
	while (false == stop_token.stop_requested()) {

		actor::job job(10);
		instance.entity_enqueue(arena_handle(), job);
		co_await iocp::sleep(200);
	}
	printf("SSSS\n");
	_stop_count.fetch_sub(1);
	_stop_count.notify_all();
	co_return;
}
