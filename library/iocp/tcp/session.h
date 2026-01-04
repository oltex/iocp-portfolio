#pragma once
#include "../message.h"
#include "../../grc/handle.h"
#include "../../socket.h"
#include "../../time.h"
#include "../../lockfree/queue.h"

namespace tcp {
	struct header final {
		unsigned short _size;
	};
	struct overlap {
		enum class mode { connect, receive, send };
		mode _mode;
		library::overlap _overlap;
		inline static auto recover(OVERLAPPED& overlapped) noexcept -> overlap& {
			return *reinterpret_cast<overlap*>(reinterpret_cast<unsigned char*>(&library::overlap::recover(overlapped)) - offsetof(overlap, _overlap));
		}
	};

	class network;
	class session {
	public:
		library::socket _socket;
		bool _cancel_flag;

		unsigned long long _receive_time;
		unsigned long _receive_timeout;
		//unsigned long _receive_byte;
		//unsigned long _receive_bytelimit;

		unsigned long long _send_time;
		unsigned long _send_timeout;
		unsigned long _send_byte = 0;
		unsigned long _send_bytelimit = 5000;
		unsigned long _send_size;
		library::lockfree::queue<iocp::message, false> _send_queue;
		overlap _send_overlap;
	public:
		session(library::socket& socket, unsigned long receive_timeout, unsigned long send_timeout, unsigned long send_bytelimit) noexcept;
		~session(void) noexcept;

		void receive_time(unsigned long long time) noexcept;
		auto receive_post(WSABUF& wsa_buffer, unsigned long& transferred, library::overlap& overlap) -> library::socket::result;

		auto send_enqueue(iocp::message message) noexcept -> bool;
		auto send_ready(void) noexcept -> bool;
		auto send_post(unsigned long& transferred) noexcept -> library::socket::result;
		void send_finish(unsigned long transferred) noexcept;
		void cancel(void) noexcept;
		inline static auto recover(overlap& overlapped) noexcept -> session& {
			return *reinterpret_cast<session*>(reinterpret_cast<unsigned char*>(&overlapped) - offsetof(session, _send_overlap));
		}
	};
}