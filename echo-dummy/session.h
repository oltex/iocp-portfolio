#pragma once
#include "library/iocp/tcp/session.h"

class session : public tcp::session {
	using base = tcp::session;
public:
	session(library::socket& socket, unsigned long receive_timeout, unsigned long receive_bytelimit, unsigned long send_timeout, unsigned long send_bytelimit) noexcept;
	virtual ~session(void) noexcept override = default;

	virtual auto session_receive(iocp::message message) noexcept -> iocp::coroutine<bool> override;
	inline static void session_destruct(tcp::session* pointer) noexcept {
		delete pointer;
	}
};