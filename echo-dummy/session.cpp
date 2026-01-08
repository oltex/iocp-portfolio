#include "session.h"

session::session(library::socket& socket, unsigned long receive_timeout, unsigned long receive_bytelimit, unsigned long send_timeout, unsigned long send_bytelimit) noexcept
	: base(socket, receive_timeout, receive_bytelimit, send_timeout, send_bytelimit) {
}

auto session::session_receive(iocp::message message) noexcept -> iocp::coroutine<bool> {

	co_return true;
}
