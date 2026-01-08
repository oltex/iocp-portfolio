#pragma once
#include "library/iocp/tcp/network.h"

class network : public tcp::network {
	using base = tcp::network;
public:
	network(unsigned long const session_capacity,
		unsigned long long header_fixed, unsigned long header_bytelimit) noexcept;
	virtual ~network(void) noexcept = default;

	auto socket_accept(const library::socket_address& address) noexcept -> iocp::coroutine<void*> override;
};