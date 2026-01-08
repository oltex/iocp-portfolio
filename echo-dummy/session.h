#pragma once
#include "library/iocp/tcp/session.h"

class session : public tcp::session {
	using base = tcp::session;
public:
	session(void) noexcept;
	virtual ~session(void) noexcept override = default;

	virtual auto session_receive(iocp::message message) noexcept -> iocp::coroutine<bool> override;
};