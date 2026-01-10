#include "session.h"
#include "application.h"

auto session::session_receive(iocp::message message) noexcept -> iocp::coroutine<bool> {
	auto& application = application::instance();
	auto& network = application::instance()._network;

	if (8 != message.size())
		co_return false;
	unsigned long long value;
	message >> value;
	auto message_ = network.message_create(8);
	message_ << value;

	network.session_send(session_handle(), message_);
	co_return true;
}
