#include "network.h"

network::network(application& application, unsigned long const session_capacity, 
	unsigned long receive_timeout, unsigned long receive_bytelimit,
	unsigned long long header_fixed, unsigned long header_bytelimit,
	unsigned long send_timeout, unsigned long send_bytelimit) noexcept
	: base(session_capacity, receive_timeout, receive_bytelimit, header_fixed, header_bytelimit, send_timeout, send_bytelimit), _application(application) {
}

auto network::socket_accept(library::socket_address const& address) noexcept -> iocp::coroutine<bool> {
	co_return true;
};
void network::session_create(handle handle) noexcept {

};
auto network::session_receive(handle handle, iocp::message message) noexcept -> iocp::coroutine<bool> {
	if (8 != message.size())
		co_return false;
	unsigned long long value;
	message >> value;
	auto message_ = message_create(8);
	message_ << value;

	session_send(handle, message_);
	co_return true;
};
void network::session_destroy(handle handle) noexcept {
};