#include "echo_network.h"

auto echo_network::socket_accept(library::socket_address const& address) noexcept -> iocp::coroutine<bool> {
	co_return true;
};
void echo_network::session_create(handle handle) noexcept {

};
auto echo_network::session_receive(handle handle, iocp::message message) noexcept -> iocp::coroutine<bool> {
	if (8 != message.size())
		co_return false;
	unsigned long long value;
	message >> value;
	auto message_ = message_create(8);
	message_ << value;

	session_send(handle, message_);
	co_return true;
};
void echo_network::session_destroy(handle handle) noexcept {
};