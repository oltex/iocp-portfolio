#pragma once
#include "library/iocp/actor/entity.h"
#include "network.h"

struct tick : public actor::job {
	tick(void) noexcept;
};

class dummy : public actor::entity {
public:
	enum state {
		disconnect, connect, interact, attack, timeout, error
		// disconnect : 끊김 -> 연결
		// connect : 연결 -> 끊김, send후 상호작용, send후 끊김, 잘못된send 후 공격, 아무것도 안하고 타임아웃 
		// interact : 상호작용 -> recv후 연결, recv후 끊김
		// attack : 공격 -> 끊김
		// timeout : 타임아웃 -> 끊김
		// error : 모종의 이유로 문제가생김
	};
	network& _network;
	state _state;
	tcp::handle _session_handle;

	std::stop_source _stop_source;
	std::atomic<int> _stop_count;

	int _test = 0;
public:
	dummy(network& network) noexcept;
	virtual ~dummy(void) noexcept;

	virtual auto callback(actor::job& job) noexcept -> iocp::coroutine<bool> override;
	auto wake_loop(void) noexcept -> iocp::coroutine<void>;

	auto state_disconnect(actor::job& job) noexcept -> iocp::coroutine<void>;
	auto state_connect(actor::job& job) noexcept -> iocp::coroutine<void>;
};