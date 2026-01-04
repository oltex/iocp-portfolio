#pragma once
#include "socket.h"
#include "handle.h"
#include "tuple.h"
#pragma comment(lib,"ws2_32.lib")
#include <WinSock2.h>
#include <cassert>

namespace library {
	class inputoutput_complet_port final : public handle {
	public:
		inline inputoutput_complet_port(void) noexcept = default;
		inline inputoutput_complet_port(unsigned long const concurrent_thread) noexcept
			: handle(::CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, concurrent_thread)) {
		};
		inline inputoutput_complet_port(inputoutput_complet_port const&) noexcept = delete;
		inline inputoutput_complet_port(inputoutput_complet_port&& rhs) noexcept
			: handle(std::move(rhs)) {
		};
		inline auto operator=(inputoutput_complet_port const&) noexcept -> inputoutput_complet_port & = delete;
		inline auto operator=(inputoutput_complet_port&& rhs) noexcept -> inputoutput_complet_port& {
			handle::operator=(std::move(rhs));
			return *this;
		}
		inline virtual ~inputoutput_complet_port(void) noexcept override = default;

		inline void create(unsigned long const concurrent_thread) noexcept {
			_handle = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, concurrent_thread);
		}
		inline void connect(handle& handle, ULONG_PTR const key) noexcept {
			::CreateIoCompletionPort(handle.data(), _handle, key, 0);
		}
		inline void connect(socket& socket, ULONG_PTR const key) noexcept {
			auto result = ::CreateIoCompletionPort(reinterpret_cast<HANDLE>(socket.data()), _handle, key, 0);
			assert(NULL != result);
		}
		inline auto get_queue_state(unsigned long const milli_second) noexcept -> library::tuple<bool, DWORD, ULONG_PTR, OVERLAPPED*> {
			library::tuple<bool, DWORD, ULONG_PTR, OVERLAPPED*> result;
			result.get<0>() = ::GetQueuedCompletionStatus(_handle, &result.get<1>(), &result.get<2>(), &result.get<3>(), milli_second);
			return result;
		}
		//inline auto get_queue_state_ex(void) noexcept {
		//	GetQueuedCompletionStatusEx()
		//}
		inline bool post_queue_state(unsigned long transferred, uintptr_t key, OVERLAPPED* overlapped) noexcept {
			return ::PostQueuedCompletionStatus(_handle, transferred, key, overlapped);
		}
	};

	class register_inputouput final {
	private:
		_RIO_EXTENSION_FUNCTION_TABLE _table;
	public:
		inline register_inputouput(library::socket& socket) noexcept {
			GUID guid = WSAID_MULTIPLE_RIO;
			unsigned long byte_return;
			socket.io_control(SIO_GET_MULTIPLE_EXTENSION_FUNCTION_POINTER, &guid, sizeof(GUID), &_table, sizeof(_RIO_EXTENSION_FUNCTION_TABLE), byte_return);
		}
		inline register_inputouput(register_inputouput const&) noexcept = default;
		inline register_inputouput(register_inputouput&&) noexcept = default;
		inline auto operator=(register_inputouput const&) noexcept -> register_inputouput & = default;
		inline auto operator=(register_inputouput&& rhs) noexcept -> register_inputouput & = default;
		inline ~register_inputouput(void) noexcept = default;

		inline auto regist_buffer(char* buffer, unsigned long length) const noexcept -> RIO_BUFFERID {
			return _table.RIORegisterBuffer(buffer, length);
		}
		inline void deregister_buffer(RIO_BUFFERID id) const noexcept {
			_table.RIODeregisterBuffer(id);
		}

		inline auto create_complet_queue(unsigned long size, RIO_NOTIFICATION_COMPLETION* notify = nullptr) const noexcept -> RIO_CQ {
			return _table.RIOCreateCompletionQueue(size, notify);
		}
		inline void close_complet_queue(RIO_CQ cq) const noexcept {
			_table.RIOCloseCompletionQueue(cq);
		}
		inline auto dequeue_complet(RIO_CQ cq, RIORESULT* result_array, unsigned long array_length) const noexcept -> unsigned long {
			return _table.RIODequeueCompletion(cq, result_array, array_length);
		}

		inline auto create_request_queue(library::socket& socket, unsigned long max_receive, unsigned long max_send, RIO_CQ receive_cq, RIO_CQ send_cq, void* context) const noexcept -> RIO_RQ {
			return _table.RIOCreateRequestQueue(socket.data(), max_receive, 1, max_send, 1, receive_cq, send_cq, context);
		}
		inline void resize_request_queue(RIO_RQ rq, unsigned long max_receive, unsigned long max_send) const noexcept {
			_table.RIOResizeRequestQueue(rq, max_receive, max_send);
		}
		inline bool send_request(RIO_RQ rq, RIO_BUF* buffer, unsigned long count, void* context = nullptr) const noexcept {
			return _table.RIOSend(rq, buffer, count, 0, context);
		}
		inline bool receive_request(RIO_RQ rq, RIO_BUF* buffer, unsigned long count, void* context = nullptr) const noexcept {
			return _table.RIOReceive(rq, buffer, count, 0, context);
		}

		inline void notify(RIO_CQ cq) const noexcept {
			_table.RIONotify(cq);
		}
	};
}