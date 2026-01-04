#pragma once
#include "echo_network.h"

class application {
	echo_network _echo_network;
	std::stop_source _monitor_source;
	std::atomic<int> _monitor_count;
public:
	application(void) noexcept;
	~application(void) noexcept;

	auto monitor(void) noexcept -> iocp::coroutine<void>;
};