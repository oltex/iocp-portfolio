#pragma once
#include "../message.h"
#include "session.h"
#include "library/system/socket.h"
#include "module/iocp/promise.h"
#include "module/iocp/worker.h"
#include "module/grc/arena.h"

namespace rudp {
	class receive : public library::awaiter {
		std::coroutine_handle<void> _handle;
		library::socket& _socket;
		library::socket_address_storage _address;
		library::overlap _overlap;
		iocp::message _message;
		unsigned long _transferred;
	public:
		receive(library::socket& socket, iocp::message message) noexcept;
		auto await_suspend(std::coroutine_handle<void> handle) noexcept -> bool;
		auto await_resume(void) const noexcept -> library::pair<unsigned long, library::socket_address_storage>;
		void execute(unsigned long transferred) noexcept;
		inline static auto recover(OVERLAPPED& overlapped) noexcept -> receive& {
			return *reinterpret_cast<receive*>(reinterpret_cast<unsigned char*>(&library::overlap::recover(overlapped)) - offsetof(receive, _overlap));
		}
	};

	class network : public iocp::worker {
		enum class key_type : unsigned char {
			dispatch = 0, connect, session, destory
		};
		library::socket _socket;
		bool _listen = false;
		grc::arena<session, true> _session_arena;

	public:
		network(library::socket_address const& address, int backlog) noexcept;
		~network(void) noexcept;

		void listen_start(void) noexcept;
		void listen_stop(void) noexcept;
		void socket_connect(library::socket_address const& address) noexcept;

		virtual void execute(bool result, unsigned long transferred, uintptr_t key, OVERLAPPED* overlapped) noexcept;
		auto dispatch(void) noexcept -> iocp::coroutine<void>;
	};
}