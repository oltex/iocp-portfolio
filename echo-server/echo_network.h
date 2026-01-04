#pragma once
#include "library/iocp/tcp/network.h"

class echo_network : public tcp::network {
	using base = tcp::network;
public:
	using base::base;

	virtual auto socket_accept(library::socket_address const& address) noexcept -> iocp::coroutine<bool> override;
	virtual void session_create(handle handle) noexcept override;
	virtual auto session_receive(handle handle, iocp::message message) noexcept -> iocp::coroutine<bool> override;
	virtual void session_destroy(handle handle) noexcept override;
};