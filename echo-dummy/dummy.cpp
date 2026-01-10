#include "dummy.h"
#include "session.h"
#include "application.h"
#include "library/iocp/actor/system.h"
#include "mail.h"

dummy::dummy(void) noexcept
	: _state(state::disconnect) {
}
dummy::~dummy(void) noexcept {
}
auto dummy::entity_mailbox(actor::mail& mail) noexcept -> iocp::coroutine<bool> {
	switch (_state) {
	case disconnect:
		co_await state_disconnect(mail);
		break;
	case connect:
		co_await state_connect(mail);
		break;
	case interact:
		co_await state_interact(mail);
	}
	co_return true;
}
auto dummy::state_disconnect(actor::mail& mail) noexcept -> iocp::coroutine<void> {
	switch (mail._type) {
	case mail_type::dummy_tick: {
		auto& application = application::instance();
		auto& network = application::instance()._network;
		auto function = network.socket_connect(library::socket_address_ipv4("127.0.0.1", 6000));
		if (auto result = co_await function; 0 != result) {
			library::pair<tcp::session*, void(*)(tcp::session*)> pair{
				new session(*reinterpret_cast<library::socket*>(result), 400000, 512, 400000, 3000, entity_handle()),
				[](tcp::session* pointer) noexcept { delete pointer; }
			};
			_session_handle = reinterpret_cast<tcp::handle>(co_await function(reinterpret_cast<void*>(&pair)));
			_state = state::connect;
			co_await function;
		}
		else {
			library::interlock_increment(application._metric._connect_fail);
			_state = state::error;
		}
		break;
	}
	default:
		__debugbreak();
	}
	co_return;
}
auto dummy::state_connect(actor::mail& mail) noexcept -> iocp::coroutine<void> {
	switch (mail._type) {
	case mail_type::dummy_tick: {
		auto& application = application::instance();
		auto& network = application::instance()._network;
		_send_message.resize(application._option._over_send);
		for (auto& iter : _send_message) {
			iter = rand();
			auto message = network.message_create(8);
			message << iter;
			network.session_send(_session_handle, message);
		}
		_state = state::interact;
	} break;
	case mail_type::session_disconnect:
		__debugbreak();
	default:
		__debugbreak();
	}
	co_return;
}
auto dummy::state_interact(actor::mail& mail) noexcept -> iocp::coroutine<void> {
	switch (mail._type) {
	case mail_type::dummy_tick:
		break;
	case mail_type::session_receive: {
		receive_mail& recv_mail = reinterpret_cast<receive_mail&>(mail);
		if (recv_mail._value == _send_message.front()) {
			_send_message.pop_front();
			if (_send_message.empty())
				_state = state::connect;
		}
		else {
			_state = state::error;
		}
	} break;
	case mail_type::session_disconnect:
		__debugbreak();
	default:
		__debugbreak();
	}
	co_return;
}
