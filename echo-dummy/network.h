#pragma once
#include "library/iocp/tcp/network.h"

class application;
class network : public tcp::network {
	using base = tcp::network;
	application& _application;
public:
	network(application& application, unsigned long const session_capacity,
		unsigned long receive_timeout, unsigned long receive_bytelimit,
		unsigned long long header_fixed, unsigned long header_bytelimit,
		unsigned long send_timeout, unsigned long send_bytelimit) noexcept;

	virtual auto socket_accept(library::socket_address const& address) noexcept -> iocp::coroutine<bool> override;
	virtual void session_create(handle handle) noexcept override;
	virtual auto session_receive(handle handle, iocp::message message) noexcept -> iocp::coroutine<bool> override;
	virtual void session_destroy(handle handle) noexcept override;
};