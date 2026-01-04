#include "session.h"
#include "network.h"

namespace tcp {
	session::session(library::socket& socket, unsigned long receive_timeout, unsigned long send_timeout, unsigned long send_bytelimit) noexcept
		:_socket(std::move(socket)),
		_cancel_flag(false),
		_send_overlap{ ._mode = overlap::mode::send },
		_receive_time(library::get_tick_count64()),
		_receive_timeout(receive_timeout),
		_send_time(0xffffffff00000000ull),
		_send_timeout(send_timeout),
		_send_byte(0),
		_send_bytelimit(send_bytelimit) {
	}
	session::~session(void) noexcept {
	}

	void session::receive_time(unsigned long long time) noexcept {
		_receive_time = time;
	}
	auto session::receive_post(WSABUF& wsa_buffer, unsigned long& transferred, library::overlap& overlap) -> library::socket::result {
		unsigned long flag = 0;
		auto result = _socket.receive(&wsa_buffer, 1, &transferred, &flag, overlap);
		return result;
	}
	auto session::send_enqueue(iocp::message message) noexcept -> bool {
		if (_send_bytelimit < library::interlock_exchange_add(_send_byte, message.size()))
			return false;
		_send_queue.emplace(message);
		return true;
	}
	auto session::send_ready(void) noexcept -> bool {
		while (false == _cancel_flag && !_send_queue.empty() &&
			0xffffffff00000000ull == library::interlock_compare_exhange(_send_time, library::get_tick_count64(), 0xffffffff00000000ull)) {
			if (_send_queue.empty())
				library::interlock_exchange(_send_time, 0xffffffff00000000ull);
			else 
				return true;
		}
		return false;
	}
	auto session::send_post(unsigned long& transferred) noexcept -> library::socket::result {
		WSABUF wsa_buffer[512];
		_send_size = 0;
		for (auto iter = _send_queue.begin(), end = _send_queue.end(); iter != end && _send_size < 512; ++iter, ++_send_size) {
			wsa_buffer[_send_size].buf = reinterpret_cast<char*>(iter->data() + iter->front());
			wsa_buffer[_send_size].len = iter->size();
		}
		unsigned long flag = 0;
		auto result = _socket.send(wsa_buffer, _send_size, &transferred, flag, _send_overlap._overlap);
		return result;
	}
	void session::send_finish(unsigned long transferred) noexcept {
		library::interlock_exchange_add(_send_byte, -library::cast<long>(transferred));
		for (auto index = 0u; index < _send_size; ++index)
			_send_queue.pop();
		library::interlock_exchange(_send_time, 0xffffffff00000000ull);
	}
	void session::cancel(void) noexcept {
		_cancel_flag = true;
		_socket.cancel_io_ex();
	}

}
