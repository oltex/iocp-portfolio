#pragma once
#include "session.h"
#include "../worker.h"
#include "../promise.h"
#include "library/container/lockfree/free_list.h"
#include "module/grc/arena.h"
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
		using size_type = unsigned int;
		using handle = grc::arena<session, true>::handle;
		using node = grc::arena<session, true>::node;
		enum class key_type : unsigned char {
			accept = 0, session, destory
		};
		library::socket_extend _socket_extend;
		library::socket _listen_socket;
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
	public:
		network(void) noexcept;
		network(network const&) noexcept = delete;
		network(network&&) noexcept = delete;
		auto operator=(network const&) noexcept -> network & = delete;
		auto operator=(network&&) noexcept -> network & = delete;
		~network(void) noexcept;

		void listen_start(library::socket_address const& address, int backlog) noexcept;
		void listen_stop(void) noexcept;
		void socket_connect(library::socket_address const& address) noexcept;
		virtual auto socket_accept(library::socket_address const& address) noexcept -> iocp::coroutine<bool> {
			co_return true;
		};

		void session_send(handle handle, iocp::message message) noexcept;
		void session_cancel(handle handle) noexcept;
		void session_timeout(handle handle, unsigned long time, bool receive_or_send) noexcept;
		virtual void session_create(handle handle) noexcept {
			int a = 10;
		};
		virtual auto session_receive(handle handle, iocp::message message) noexcept -> iocp::coroutine<bool> {
			[](tcp::network& network, tcp::network::handle handle, iocp::message message) -> iocp::coroutine<void> {
				if (8 != message.size())
					__debugbreak();
				unsigned long long value;
				message >> value;
				auto message_ = network::message_create(8);
				message_ << value;

				network.session_send(handle, message_);
				co_return;
				}(*this, handle, message);
				co_return true;
		};
		virtual void session_destroy(handle handle) noexcept {};

		inline static auto message_create(size_type const size) noexcept -> iocp::message {
			auto message = iocp::message_pool::instance().allocate(sizeof(header) + size);
			header _header{ ._size = 8 };
			message.push(reinterpret_cast<unsigned char*>(&_header), sizeof(header));
			return message;
		}

		virtual void execute(bool result, unsigned long transferred, uintptr_t key, OVERLAPPED* overlapped) noexcept override;
		auto receive(node& node) noexcept -> iocp::coroutine<void>;
		auto timeout(void) noexcept -> iocp::coroutine<void>;
		auto monitor(void) noexcept -> iocp::coroutine<void>;
	};
}