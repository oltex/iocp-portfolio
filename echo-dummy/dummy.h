#pragma once
#include "library/iocp/actor/entity.h"
#include "network.h"

class dummy : public actor::entity {
	network& _network;
	std::stop_source _stop_source;
	std::atomic<int> _stop_count;
public:
	dummy(network& network) noexcept;
	virtual ~dummy(void) noexcept ;

	virtual auto callback(actor::job& job) noexcept -> iocp::coroutine<bool> override;
	auto wake_loop(void) noexcept -> iocp::coroutine<void>;
};