#include "network.h"

namespace rudp {
	receive::receive(library::socket& socket, iocp::message message) noexcept
		: _socket(socket), _message(message) {
	}
	auto receive::await_suspend(std::coroutine_handle<void> handle) noexcept -> bool {
		_handle = handle;
		WSABUF wsa_buffer{ .len = _message.remain(), .buf = reinterpret_cast<char*>(_message.data()) + _message.rear() };
		unsigned long flag = 0;
		switch (_socket.receive_from(&wsa_buffer, 1, &_transferred, &flag, _address, _overlap)) {
			using enum library::socket::result;
		case pending:
			return true;
		case complet:
		case close:
			return false;
		default:
			__debugbreak();
		}
	}
	auto receive::await_resume(void) const noexcept -> library::pair<unsigned long, library::socket_address_storage> {
		return { _transferred, _address };
	}
	void receive::execute(unsigned long transferred) noexcept {
		_transferred = transferred;
		_handle.resume();
	}

	network::network(library::socket_address const& address, int backlog) noexcept
		: _session_arena(200) {
		library::wsa_start_up();
		_socket.create(AF_INET, SOCK_DGRAM, IPPROTO_UDP, WSA_FLAG_OVERLAPPED);
		_socket.set_option_dont_fragment(true);
		_socket.io_control_udp_connect_reset(false);
		_socket.bind(address);
		_scheduler.connect(*this, _socket, static_cast<uintptr_t>(key_type::dispatch));
		_socket.set_file_complet_notify_mode(FILE_SKIP_COMPLETION_PORT_ON_SUCCESS | FILE_SKIP_SET_EVENT_ON_HANDLE);
		for (auto index = 0; index < 8; ++index)
			dispatch();
	}
	network::~network(void) noexcept {
		_socket.close();
		//¸ðµç dispatch°¡ ´ÝÇû´ÂÁö È®ÀÎ
		library::wsa_clean_up();
	}

	void network::listen_start(void) noexcept {
		_listen = true;
	}
	void network::listen_stop(void) noexcept {
		_listen = false;
	}
	void network::socket_connect(library::socket_address const& address) noexcept {
		auto node = _session_arena.allocate();
		if (nullptr == node)
			return;
		node->allocate(address);

		//_socket.send_to();
		//[](network& network, library::socket_address_storage const address) noexcept -> iocp::coroutine<void> {
		//	library::socket connect_socket;
		//	connect_socket.create(AF_INET, SOCK_STREAM, IPPROTO_TCP, WSA_FLAG_OVERLAPPED);
		//	connect_socket.set_option_linger(1, 0);
		//	connect_socket.set_option_send_buffer(0);
		//	network._scheduler.connect(network, connect_socket, static_cast<uintptr_t>(key_type::session));
		//	connect_socket.set_file_complet_notify_mode(FILE_SKIP_COMPLETION_PORT_ON_SUCCESS | FILE_SKIP_SET_EVENT_ON_HANDLE);

		//	library::socket_address_ipv4 local_address;
		//	local_address.ip("0.0.0.0");
		//	local_address.port(0);
		//	connect_socket.bind(local_address);

		//	if (false == co_await iocp::connect(network._socket_extend, connect_socket, address))
		//		co_return;
		//	connect_socket.set_option_update_connect_context();
		//	auto node = network._session_arena.allocate();
		//	if (nullptr == node)
		//		co_return;
		//	node->allocate(connect_socket, network._receive_timeout, network._send_timeout, network._send_bytelimit);
		//	network.session_create(node->_key);
		//	network.receive(*node);

		//	co_return;
		//	}(*this, address);
	}

	void network::execute(bool result, unsigned long transferred, uintptr_t key, OVERLAPPED* overlapped) noexcept {
		switch (auto key_type = static_cast<network::key_type>(key)) {
		case key_type::dispatch: {
			auto& receive = receive::recover(*overlapped);
			receive.execute(transferred);
		} break;
		default:
			__debugbreak();
		}
	}
	auto network::dispatch(void) noexcept -> iocp::coroutine<void> {
		for (;;) {
			auto message = iocp::message_pool::instance().allocate(1472);
			auto [transferred, address] = co_await rudp::receive(_socket, message);
			if (0 == transferred)
				break;
			message.move_rear(transferred);

			header _header;
			message.peek(reinterpret_cast<unsigned char*>(&_header), sizeof(header));

			if (header::key_type::synchronize & _header._key) {
				auto node = _session_arena.allocate();


			}
		}
		co_return;
	}
}