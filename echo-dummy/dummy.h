#pragma once
#include "library/iocp/actor/entity.h"
#include "network.h"

struct tick : public actor::job {
	tick(void) noexcept;
};

class dummy : public actor::entity {
public:
	enum job {
		frame
	};
	enum state {
		disconnect, connect, interact, attack, timeout, error
		// disconnect : ²÷±è -> ¿¬°á
		// connect : ¿¬°á -> ²÷±è, sendÈÄ »óÈ£ÀÛ¿ë, sendÈÄ ²÷±è, Àß¸øµÈsend ÈÄ °ø°Ý, ¾Æ¹«°Íµµ ¾ÈÇÏ°í Å¸ÀÓ¾Æ¿ô 
		// interact : »óÈ£ÀÛ¿ë -> recvÈÄ ¿¬°á, recvÈÄ ²÷±è
		// attack : °ø°Ý -> ²÷±è
		// timeout : Å¸ÀÓ¾Æ¿ô -> ²÷±è
		// error : ¸ðÁ¾ÀÇ ÀÌÀ¯·Î ¹®Á¦°¡»ý±è
	};
	network& _network;
	state _state;
	tcp::handle _session_handle;

	unsigned long long _send_message;

	std::stop_source _stop_source;
	std::atomic<int> _stop_count;
public:
	dummy(network& network) noexcept;
	virtual ~dummy(void) noexcept;

	virtual auto actor_mailbox(actor::job& job) noexcept -> iocp::coroutine<bool> override;
	auto wake_loop(void) noexcept -> iocp::coroutine<void>;

	auto state_disconnect(actor::job& job) noexcept -> iocp::coroutine<void>;
	auto state_connect(actor::job& job) noexcept -> iocp::coroutine<void>;
};