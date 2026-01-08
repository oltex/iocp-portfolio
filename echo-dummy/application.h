#pragma once
#include "command.h"
#include "network.h"
#include "library/thread.h"

class application {
	network _network;
	std::stop_source _stop_source;
	std::atomic<int> _stop_count;

	command _command;
	unsigned long _client_count = 0;
	bool _disconnect_test = false;
	bool _attack_test = false;
	unsigned long _over_send = 0;
	unsigned long _action_delay = 0;

public:
	application(void) noexcept;
	~application(void) noexcept;

	auto thread_monitor(void) noexcept -> iocp::coroutine<void>;
};