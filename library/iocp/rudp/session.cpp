#include "session.h"
#include "library/algorithm/random.h"

namespace rudp {
	session::session(library::socket_address_storage address) noexcept
		: _sequence(library::random()) {
	}
}