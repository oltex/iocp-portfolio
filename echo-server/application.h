#pragma once
#include "network.h"

class application {
	network _network;
	std::stop_source _monitor_source;
	std::atomic<int> _monitor_count;
public:
	application(void) noexcept;
	~application(void) noexcept;

	auto monitor(void) noexcept -> iocp::coroutine<void>;
};