#pragma once
#include "command.h"
#include "echo_network.h"
#include "library/thread.h"

class application {
	echo_network _echo_network;
	std::stop_source _stop_source;
	std::atomic<int> _stop_count;

	command _command;
public:
	application(void) noexcept;
	~application(void) noexcept;

	auto thread_monitor(void) noexcept -> iocp::coroutine<void>;
};