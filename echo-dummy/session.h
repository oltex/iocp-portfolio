#pragma once
#include "library/iocp/tcp/session.h"
#include "library/iocp/actor/system.h"

class session : public tcp::session {
	using base = tcp::session;
	actor::handle _dummy_handle;
public:
	session(library::socket& socket, unsigned long receive_timeout, unsigned long receive_bytelimit, unsigned long send_timeout, unsigned long send_bytelimit,
		actor::handle dummy_handle) noexcept;
	virtual ~session(void) noexcept override;

	virtual auto session_receive(iocp::message message) noexcept -> iocp::coroutine<bool> override;
};