#include "session.h"
#include "application.h"
#include "mail.h"

session::session(library::socket& socket, unsigned long receive_timeout, unsigned long receive_bytelimit, unsigned long send_timeout, unsigned long send_bytelimit, actor::handle dummy_handle) noexcept
	: base(socket, receive_timeout, receive_bytelimit, send_timeout, send_bytelimit), _dummy_handle(dummy_handle) {
}
session::~session(void) noexcept {
	auto& application = application::instance();
	auto& system = application._system;
	system.entity_enqueue(_dummy_handle, actor::mail(mail_type::session_disconnect));
};

auto session::session_receive(iocp::message message) noexcept -> iocp::coroutine<bool> {
	if (8 != message.size())
		__debugbreak();
	unsigned long long value;
	message >> value;
	auto& application = application::instance();
	auto& system = application._system;
	system.entity_enqueue(_dummy_handle, receive_mail(value));
	co_return true;
}
