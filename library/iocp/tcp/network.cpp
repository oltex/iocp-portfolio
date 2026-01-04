#include "network.h"
#include "../timer.h"
#include "../monitor.h"
#include "module/_pdh/query.h"

namespace tcp {
	accept::accept(library::socket_extend& socket_extend, library::socket& listen_socket, library::socket& accept_socket, void* output_buffer) noexcept
		: _socket_extend(socket_extend), _listen_socket(listen_socket), _accept_socket(accept_socket), _output_buffer(output_buffer), _result(false) {
	}
	auto accept::await_suspend(std::coroutine_handle<void> handle) noexcept -> bool {
		_handle = handle;
		switch (_socket_extend.accept(_listen_socket, _accept_socket, _output_buffer, sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16, _overlap)) {
			using enum library::socket::result;
		case pending:
		case complet:
			return true;
		case close:
			return false;
		default:
			__debugbreak();
		}
	}
	auto accept::await_resume(void) const noexcept -> bool {
		return _result;
	}
	void accept::execute(bool result) noexcept {
		_result = result;
		_handle.resume();
	}

	connect::connect(library::socket_extend& socket_extend, library::socket& connect_socket, library::socket_address_storage const& address) noexcept
		: _socket_extend(socket_extend), _connect_socket(connect_socket), _address(address), _overlap{ ._mode = overlap::mode::connect } {
	}
	auto connect::await_suspend(std::coroutine_handle<void> handle) noexcept -> bool {
		_handle = handle;
		switch (_socket_extend.connect(_connect_socket, _address, _overlap._overlap)) {
			using enum library::socket::result;
		case pending:
			return true;
		case complet:
			_result = true;
		case close:
			_result = false;
			return false;
		default:
			__debugbreak();
		}
	}
	auto connect::await_resume(void) const noexcept -> bool {
		return _result;
	}
	void connect::execute(bool result) noexcept {
		_result = result;
		_handle.resume();
	}

	receive::receive(network& network, grc::arena<session, true>::node& node, iocp::message message) noexcept
		: _network(network), _node(node), _message(message), _overlap{ ._mode = overlap::mode::receive }, _transferred(0) {
	}
	auto receive::await_suspend(std::coroutine_handle<void> handle) noexcept -> bool {
		_handle = handle;
		if (true == _node._value._cancel_flag)
			return false;
		WSABUF wsa_buffer{ .len = _message.remain(), .buf = reinterpret_cast<char*>(_message.data()) + _message.rear() };
		auto& network = _network;
		auto& node = _node;
		auto key = node._key;
		switch (node._value.receive_post(wsa_buffer, _transferred, _overlap._overlap)) {
			using enum library::socket::result;
		case pending:
			if (true == node._value._cancel_flag)
				network.session_cancel(key);
			return true;
		case complet:
		case close:
			return false;
		default:
			__debugbreak();
		}
	}
	auto receive::await_resume(void) const noexcept -> unsigned long {
		return _transferred;
	}
	void receive::execute(unsigned long transferred) noexcept {
		_transferred = transferred;
		_handle.resume();
	}

	network::network(void) noexcept 
		: _session_arena(200) {
		library::wsa_start_up();
		library::socket socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		_socket_extend.wsa_io_control_acccept_ex(socket);
		_socket_extend.wsa_io_control_acccept_ex(socket);
		_socket_extend.wsa_io_control_connect_ex(socket);
		_socket_extend.wsa_io_control_get_accept_ex_sockaddr(socket);

		_stop_count.store(2);
		timeout();
		monitor();
	}
	network::~network(void) noexcept {
		// 세션을 다 정리하기(종료요청)
		//for (auto& iter : _session_arena) {
		//	_session_arena.acquire(iter);
		//	if (iter._value.acquire())
		//		iter._value.cancel();
		//	if (iter._value.release()) {
		//		on_destroy_session(iter._value._key);
		//		_session_array.release(iter._value);
		//	}
		//}
		// 세션이 다 종료됐는지 확인하기
		//_session_array.wait();

		_stop_source.request_stop();
		for (int stop_count; 0 != (stop_count = _stop_count.load()); )
			_stop_count.wait(stop_count);

		library::wsa_clean_up();
	}
	void network::listen_start(library::socket_address const& address, int backlog) noexcept {
		_listen_socket.create(AF_INET, SOCK_STREAM, IPPROTO_TCP, WSA_FLAG_OVERLAPPED);
		_listen_socket.set_option_linger(1, 0);
		_listen_socket.set_option_send_buffer(0);
		_listen_socket.bind(address);

		_listen_socket.listen(backlog);
		_scheduler.connect(*this, _listen_socket, static_cast<uintptr_t>(key_type::accept));
		_listen_socket.set_file_complet_notify_mode(FILE_SKIP_COMPLETION_PORT_ON_SUCCESS | FILE_SKIP_SET_EVENT_ON_HANDLE);

		for (auto index = 0; index < 1; ++index) {
			[](network& network) noexcept -> iocp::coroutine<void> {
				for (;;) {
					library::socket accept_socket;
					library::array<char, 64> buffer;
					accept_socket.create(AF_INET, SOCK_STREAM, IPPROTO_TCP, WSA_FLAG_OVERLAPPED);
					if (false == co_await tcp::accept(network._socket_extend, network._listen_socket, accept_socket, buffer.data()))
						break;
					accept_socket.set_option_update_accept_context(network._listen_socket);
					auto address = network._socket_extend.get_accept_ex_socket_address(buffer.data())._second;
					if (false == co_await network.socket_accept(address))
						break;
					auto node = network._session_arena.allocate();
					if (nullptr == node)
						continue;
					node->allocate(accept_socket, network._receive_timeout, network._send_timeout, network._send_bytelimit);
					network._scheduler.connect(network, node->_value._socket, static_cast<uintptr_t>(key_type::session));
					node->_value._socket.set_file_complet_notify_mode(FILE_SKIP_COMPLETION_PORT_ON_SUCCESS | FILE_SKIP_SET_EVENT_ON_HANDLE);
					network.session_create(node->_key);
					network.receive(*node);
				}
				co_return;
				}(*this);
		}
	}
	void network::listen_stop(void) noexcept {
		_listen_socket.close();
		//accept가 다 취소되길 기다려야함
	}
	void network::socket_connect(library::socket_address const& address) noexcept {
		[](network& network, library::socket_address_storage const address) noexcept -> iocp::coroutine<void> {
			library::socket connect_socket;
			connect_socket.create(AF_INET, SOCK_STREAM, IPPROTO_TCP, WSA_FLAG_OVERLAPPED);
			connect_socket.set_option_linger(1, 0);
			connect_socket.set_option_send_buffer(0);
			network._scheduler.connect(network, connect_socket, static_cast<uintptr_t>(key_type::session));
			connect_socket.set_file_complet_notify_mode(FILE_SKIP_COMPLETION_PORT_ON_SUCCESS | FILE_SKIP_SET_EVENT_ON_HANDLE);

			library::socket_address_ipv4 local_address;
			local_address.ip("0.0.0.0");
			local_address.port(0);
			connect_socket.bind(local_address);

			if (false == co_await tcp::connect(network._socket_extend, connect_socket, address))
				co_return;
			connect_socket.set_option_update_connect_context();
			auto node = network._session_arena.allocate();
			if (nullptr == node)
				co_return;
			node->allocate(connect_socket, network._receive_timeout, network._send_timeout, network._send_bytelimit);
			network.session_create(node->_key);
			network.receive(*node);

			co_return;
			}(*this, address);
	}
	void network::session_send(handle handle, iocp::message message) noexcept {
		auto& node = _session_arena.get(handle);
		auto& session = node._value;
		if (node.acquire(handle)) {
			if (false == session.send_enqueue(message))
				session.cancel();
			else {
				for (;;) {
					if (false == session.send_ready())
						break;
					unsigned long transferred = 0;
					switch (session.send_post(transferred)) {
						using enum library::socket::result;
					case pending:
						if (true == session._cancel_flag)
							session_cancel(handle);
						return;
					case complet:
					case close:
						break;
					default:
						__debugbreak();
					}
					if (0 == transferred)
						break;
					session.send_finish(transferred);
				}
			}
		}
		if (node.release())
			_scheduler.post(*this, 0, static_cast<uintptr_t>(key_type::destory), reinterpret_cast<OVERLAPPED*>(&node));
	}
	void network::session_cancel(handle handle) noexcept {
		auto& node = _session_arena.get(handle);
		if (node.acquire(handle))
			node._value.cancel();
		if (node.release())
			_scheduler.post(*this, 0, static_cast<uintptr_t>(key_type::destory), reinterpret_cast<OVERLAPPED*>(&node));
	}
	void network::session_timeout(handle handle, unsigned long time, bool receive_or_send) noexcept {
		auto& node = _session_arena.get(handle);
		if (node.acquire(handle))
			node._value.cancel();
		if (node.release())
			_scheduler.post(*this, 0, static_cast<uintptr_t>(key_type::destory), reinterpret_cast<OVERLAPPED*>(&node));
	}
	void network::execute(bool result, unsigned long transferred, uintptr_t key, OVERLAPPED* overlapped) noexcept {
		switch (auto key_type = static_cast<network::key_type>(key)) {
		case key_type::accept: {
			auto& accept = accept::recover(*overlapped);
			accept.execute(result);
		} break;
		case key_type::session: {
			auto& overlap = overlap::recover(*overlapped);
			if (overlap::mode::connect == overlap._mode) {
				auto& connect = connect::recover(overlap);
				connect.execute(result);
			}
			else if (overlap::mode::receive == overlap._mode) {
				auto& receive = receive::recover(overlap);
				receive.execute(transferred);
			}
			else {
				auto& session = session::recover(overlap);
				auto& node = node::recover(session);
				auto key = node._key;
				while (0 != transferred) {
					session.send_finish(transferred);
					if (false == session.send_ready())
						break;
					transferred = 0;
					switch (session.send_post(transferred)) {
						using enum library::socket::result;
					case pending:
						if (true == session._cancel_flag)
							session_cancel(key);
						return;
					case complet:
					case close:
						break;
					default:
						__debugbreak();
					}
				}
				if (node.release()) {
					session_destroy(node._key);
					_session_arena.deallocate(&node);
				}
			}
		} break;
		case key_type::destory: {
			__debugbreak();
		} break;
		default:
			__debugbreak();
		}
	}
	auto network::receive(node& node) noexcept -> iocp::coroutine<void> {
		session& session = node._value;
		iocp::message receive_message;
		for (;;) {
			{
				auto message = iocp::message_pool::instance().allocate(_receive_bytelimit);
				if (!receive_message.empty())
					message.push(receive_message.data() + receive_message.front(), receive_message.size());
				receive_message = std::move(message);
			}
			auto transferred = co_await tcp::receive(*this, node, receive_message);
			if (0 == transferred)
				break;
			receive_message.move_rear(transferred);

			for (;;) {
				if (sizeof(header) > receive_message.size())
					break;
				header _header;
				receive_message.peek(reinterpret_cast<unsigned char*>(&_header), sizeof(header));
				if (sizeof(header) + _header._size > _header_bytelimit)
					goto receive_exit;
				else if (sizeof(header) + _header._size > receive_message.size())
					break;
				receive_message.pop(sizeof(header));
				iocp::message message(receive_message, receive_message.data() + receive_message.front(), 0, _header._size, _header._size);
				receive_message.pop(_header._size);

				session.receive_time(0xffffffff00000000ull);
				if (false == co_await session_receive(node._key, message))
					goto receive_exit;
				session.receive_time(library::get_tick_count64());
			}
		}
	receive_exit:
		if (node.release()) {
			session_destroy(node._key);
			_session_arena.deallocate(&node);
		}
		co_return;
	};
	auto network::timeout(void) noexcept -> iocp::coroutine<void> {
		auto stop_token = _stop_source.get_token();
		while (false == stop_token.stop_requested()) {
			auto crrent_time = library::get_tick_count64();
			for (auto iter = _session_arena.begin(), end = _session_arena.end(); iter != end; ++iter) {
				if (iter->acquire()) {
					if (iter->_value._receive_time + iter->_value._receive_timeout < crrent_time ||
						iter->_value._send_time + iter->_value._send_timeout < crrent_time) {
						iter->_value.cancel();
					}
				}
				if (iter->release()) {
					session_destroy(iter->_key);
					_session_arena.deallocate(&*iter);
				}
			}
			co_await iocp::sleep(1000);
		}
		_stop_count.fetch_sub(1);
		_stop_count.notify_all();
		co_return;
	}
	auto network::monitor(void) noexcept -> iocp::coroutine<void> {
		auto& monitor = iocp::monitor::instance();
		auto stop_token = _stop_source.get_token();
		while (false == stop_token.stop_requested()) {
			auto processor = monitor.get_processor();
			auto process = monitor.get_process();
			auto system = monitor.get_system();
			auto memory = monitor.get_memory();
			auto tcpv4 = monitor.get_tcpv4();
			auto ipv4 = monitor.get_ipv4();
			auto nic = monitor.get_nic();

			printf("----------------------------------------------------------------------------\n"\
				"[Processor Monitor]\n"\
				" CPU Layer User Time     : %.2f%%\n"\
				"           Privilege Time: %.2f%%\n"\
				"           Interrupt Time: %.2f%%\n"\
				"           DPC Time      : %.2f%%\n"\
				" System Layer Processor Queue Length: %ld\n"\
				"              Context Switch: %.2f/sec\n"\
				"              System Call: %.2f/sec\n"\
				"              Exception Dispatch: %.2f/sec\n"\
				" Physical Layer Available: %.2f GB\n"\
				" Virtual Layer Commit    : %.2f MB\n"\
				"               Non-Paged : %.2f MB\n"\
				" I/O Layer Page: %.2f/sec\n"
				" TCP Layer Connect Establish : %ld\n"\
				"           Segment Receive   : %.2f/sec\n"\
				"                   Sent      : %.2f\n"\
				"                   Retransmit: %.2f\n"\
				" IP Layer Datagram Receive: %.2f/sec (Discard: %lld)\n"\
				"                   Sent   : %.2f (Discard: %lld)\n"\
				"          Fragment Receive : %.2f/sec\n"\
				"                   Datagram: %.2f\n"\
				" NIC Layer Packet Receive: %.2f/sec (Discard: %lld Error: %lld)\n"\
				"                  Sent   : %.2f (Discard: %lld Error: %lld)\n"\
				"           Byte Receive: %.2f/sec\n"\
				"                Sent   : %.2f\n"\
				"           Output Queue Length: %ld\n"\
				"[Process Moniter]\n"\
				" Thread Count: %ld\n"\
				" Handle Count: %ld\n"\
				" CPU Layer User Time     : %.2f%%\n"\
				"            Privilege Time: %.2f%%\n"\
				" Physical Layer Working Set: %.2f MB \n"\
				" Virtual Layer Private   : %.2f MB\n"\
				"               Non-Paged : %.2f MB\n"\
				" I/O Layer Page Fault: %.2f/sec\n",
				processor._user_time,
				processor._privilege_time,
				processor._interrupt_time,
				processor._dpc_time,

				system._processor_queue_len,
				system._context_switch_sec,
				system._system_call_sec,
				system._except_dispatch_sec,

				memory._physics._avail_byte / (double)0x40000000ull,
				memory._virtual._commit_byte / (double)0x100000ull,
				memory._virtual._pool_nonpage_byte / (double)0x100000ull,
				memory._io._page_sec,

				tcpv4._connect._establish,
				tcpv4._segment._receive_sec,
				tcpv4._segment._sent_sec,
				tcpv4._segment._retransmit_sec,

				ipv4._datagram._receive_sec, ipv4._datagram._receive_discard,
				ipv4._datagram._sent_sec, ipv4._datagram._outbound_discard,
				ipv4._fragment._receive_sec,
				ipv4._fragment._datagram_sec,

				nic._packet._receive_sec, nic._packet._receive_discard, nic._packet._receive_error,
				nic._packet._sent_sec, nic._packet._outbound_discard, nic._packet._outbound_error,
				nic._byte._receive_sec,
				nic._byte._sent_sec,
				nic.nic_output_queue_len,

				process._thread_count,
				process._handle_count,
				process._user_time,
				process._privilege_time,
				process._physics._working_set_private / (double)0x100000ull,
				process._virtual._private_byte / (double)0x100000ull,
				process._virtual._pool_nonpage_byte / (double)0x100000ull,
				process._page_fault_sec
			);

			co_await iocp::sleep(1000);
		}
		_stop_count.fetch_sub(1);
		_stop_count.notify_all();
		co_return;
	}
}