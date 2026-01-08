#include "network.h"

network::network(unsigned long const session_capacity,
	unsigned long long header_fixed, unsigned long header_bytelimit) noexcept
	: base(session_capacity, header_fixed, header_bytelimit) {
}
auto network::socket_accept(const library::socket_address& address) noexcept ->iocp::coroutine<void*> {
	co_return reinterpret_cast<void*>(0);
}