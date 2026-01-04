#pragma once
#include "library/system/socket.h"

namespace rudp {
	struct header {
		enum key_type : unsigned long long {
			synchronize = 0xf000000000000000ull,
			mask = 0x0fffffffffffffffull,
		};
		unsigned long long _key;
		unsigned long _sequence;
		unsigned long _acknowledge;
	};

	struct packet {

	};

	class session {
		unsigned long _sequence;
		library::socket_address_storage _address;
	public:
		session(library::socket_address_storage address) noexcept;
	};
}