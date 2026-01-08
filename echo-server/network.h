#pragma once
#include "library/iocp/tcp/network.h"

class network : public tcp::network {
	using base = tcp::network;
public:
	using base::base;
	virtual ~network(void) noexcept = default;

	auto socket_accept(library::socket_address const& address) noexcept ->iocp::coroutine<void*> override;
};