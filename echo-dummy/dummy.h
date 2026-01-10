#pragma once
#include "library/iocp/actor/entity.h"
#include "network.h"
#include <deque>

class application;
class dummy : public actor::entity {
public:
	enum state {
		disconnect, connect, interact, attack, timeout, error
		// disconnect : 끊김 -> 연결
		// connect : 연결 -> 끊김, send후 상호작용, 잘못된send 후 공격, 아무것도 안하고 타임아웃 
		// interact : 상호작용 -> recv후 연결, 
		// attack : 공격 -> 끊김
		// timeout : 타임아웃 -> 끊김
		// error : 모종의 이유로 문제가생김
	};
	state _state;
	tcp::handle _session_handle;
	std::deque<unsigned long long> _send_message;

	std::stop_source _stop_source;
	std::atomic<int> _stop_count;
public:
	dummy(void) noexcept;
	virtual ~dummy(void) noexcept;

	virtual auto entity_mailbox(actor::mail& mail) noexcept -> iocp::coroutine<bool> override;
	auto state_disconnect(actor::mail& mail) noexcept -> iocp::coroutine<void>;
	auto state_connect(actor::mail& mail) noexcept -> iocp::coroutine<void>;
	auto state_interact(actor::mail& mail) noexcept -> iocp::coroutine<void>;
};