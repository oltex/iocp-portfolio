#include "network.h"
#include "session.h"

auto network::socket_accept(library::socket_address const& address) noexcept ->iocp::coroutine<void*> {
	auto& socket = *reinterpret_cast<library::socket*>(co_yield reinterpret_cast<void*>(1));

	library::pair<tcp::session*, void(*)(tcp::session*)> pair{
		new session(socket, 40000, 512, 40000, 3000),
		[](tcp::session* pointer) noexcept { delete pointer; }
	};
	auto key = reinterpret_cast<tcp::handle>(co_yield reinterpret_cast<void*>(&pair));
	co_return reinterpret_cast<void*>(0);
}