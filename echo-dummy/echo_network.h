#pragma once
#include "library/iocp/tcp/network.h"

class echo_network : public tcp::network {
	using base = tcp::network;
public:
	unsigned long _client_count = 0;
	bool _disconnect_test= false;
	bool _attack_test = false;
	unsigned long _over_send = 0;
	unsigned long _action_delay = 0;


	using base::base;

	virtual auto socket_accept(library::socket_address const& address) noexcept -> iocp::coroutine<bool> override;
	virtual void session_create(handle handle) noexcept override;
	virtual auto session_receive(handle handle, iocp::message message) noexcept -> iocp::coroutine<bool> override;
	virtual void session_destroy(handle handle) noexcept override;
};