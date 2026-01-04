#pragma once
#include "session.h"
#include "../worker.h"
#include "../promise.h"
#include "../../grc/arena.h"
#include <stop_token>
#include <atomic>

namespace tcp {
	class accept : public library::awaiter {
		std::coroutine_handle<void> _handle;
		library::socket_extend& _socket_extend;
		library::socket& _listen_socket;
		library::socket& _accept_socket;
		library::overlap _overlap;
		void* _output_buffer;
		bool _result;
	public:
		accept(library::socket_extend& socket_extend, library::socket& listen_socket, library::socket& accept_socket, void* output_buffer) noexcept;
		auto await_suspend(std::coroutine_handle<void> handle) noexcept -> bool;
		auto await_resume(void) const noexcept -> bool;
		void execute(bool result) noexcept;
		inline static auto recover(OVERLAPPED& overlapped) noexcept -> accept& {
			return *reinterpret_cast<accept*>(reinterpret_cast<unsigned char*>(&library::overlap::recover(overlapped)) - offsetof(accept, _overlap));
		}
	};
	class connect : public library::awaiter {
		std::coroutine_handle<void> _handle;
		library::socket_extend& _socket_extend;
		library::socket& _connect_socket;
		library::socket_address_storage const& _address;
		overlap _overlap;
		bool _result;
	public:
		connect(library::socket_extend& socket_extend, library::socket& connect_socket, library::socket_address_storage const& address) noexcept;
		auto await_suspend(std::coroutine_handle<void> handle) noexcept -> bool;
		auto await_resume(void) const noexcept -> bool;
		void execute(bool result) noexcept;
		inline static auto recover(overlap& overlapped) noexcept -> connect& {
			return *reinterpret_cast<connect*>(reinterpret_cast<unsigned char*>(&overlapped) - offsetof(connect, _overlap));
		}
	};
	class receive : public library::awaiter {
		friend class network;
		std::coroutine_handle<void> _handle;
		network& _network;
		grc::arena<session, true>::node& _node;
		iocp::message _message;
		overlap _overlap;
		unsigned long _transferred;
	public:
		receive(network& network, grc::arena<session, true>::node& node, iocp::message message) noexcept;
		auto await_suspend(std::coroutine_handle<void> handle) noexcept -> bool;
		auto await_resume(void) const noexcept -> unsigned long;
		void execute(unsigned long transferred) noexcept;
		inline static auto recover(overlap& overlapped) noexcept -> receive& {
			return *reinterpret_cast<receive*>(reinterpret_cast<unsigned char*>(&overlapped) - offsetof(receive, _overlap));
		}
	};

	class network : public iocp::worker {
	protected:
		using size_type = unsigned int;
		using handle = grc::arena<session, true>::handle;
		using node = grc::arena<session, true>::node;
		enum class key_type : unsigned char {
			accept = 0, session, destory
		};
		library::socket_extend _socket_extend;
		library::socket _listen_socket;
		std::stop_source _accept_source;
		std::atomic<int> _accept_count;

		grc::arena<session, true> _session_arena;
		std::stop_source _stop_source;
		std::atomic<int> _stop_count;

		//엑셉트 ip당 접속 제한을 설정해두기 umap으로 특정 ip는 허용케금 하기
		//엑셉트 특정 ip는 아예 차단하기 bloom filter를 써서 막기
		//엑셉트 ip 대역을 차단시키는 방법도 연구하기

		unsigned long _receive_timeout = 1000000;
		unsigned long _receive_bytelimit = 512;
		unsigned long _header_bytelimit = 128;

		unsigned long _send_timeout = 1000000;
		unsigned long _send_bytelimit = 2048;

		unsigned long _accept_bucket = 0;
		unsigned long _receive_bucket = 0;
		unsigned long _send_bucket = 0;
		struct metric {
			unsigned long long _accept_total = 0;
			unsigned long long _connect_total = 0;
			unsigned long _accept_sec = 0;
			unsigned long _receive_sec = 0;
			unsigned long _send_sec = 0;

			unsigned long _session_count = 0;

			unsigned long long _receive_timeout_total = 0;
			unsigned long long _header_bytelimit_total = 0;
			unsigned long long _send_timeout_total = 0;
			unsigned long long _send_bytelimit_total = 0;
		} _metric;
	public:
		network(unsigned long const session_capacity) noexcept;
		network(network const&) noexcept = delete;
		network(network&&) noexcept = delete;
		auto operator=(network const&) noexcept -> network & = delete;
		auto operator=(network&&) noexcept -> network & = delete;
		virtual ~network(void) noexcept;

		void listen_start(library::socket_address const& address, int backlog) noexcept;
		void listen_stop(void) noexcept;
		void socket_connect(library::socket_address const& address) noexcept;
		virtual auto socket_accept(library::socket_address const& address) noexcept -> iocp::coroutine<bool> = 0;

		void session_send(handle handle, iocp::message message) noexcept;
		void session_timeout(handle handle, unsigned long time, bool receive_or_send) noexcept;
		void session_cancel(handle handle) noexcept;
		void session_clear(void) noexcept;
		virtual void session_create(handle handle) noexcept = 0;
		virtual auto session_receive(handle handle, iocp::message message) noexcept -> iocp::coroutine<bool> = 0;
		virtual void session_destroy(handle handle) noexcept = 0;

		inline static auto message_create(size_type const size) noexcept -> iocp::message {
			auto message = iocp::message_pool::instance().allocate(sizeof(header) + size);
			header _header{ ._size = 8 };
			message.push(reinterpret_cast<unsigned char*>(&_header), sizeof(header));
			return message;
		}
		auto network_metric(void) noexcept -> metric;

		virtual void execute(bool result, unsigned long transferred, uintptr_t key, OVERLAPPED* overlapped) noexcept override;
		auto receive(node& node) noexcept -> iocp::coroutine<void>;
		auto timeout(void) noexcept -> iocp::coroutine<void>;
		auto monitor(void) noexcept -> iocp::coroutine<void>;
	};
}