#pragma once
#include "network.h"
#include "command.h"

class application : public library::singleton<application, true> {
	friend class library::singleton<application, true>;
public:
	network _network;
private:
	command _command;
	std::stop_source _monitor_source;
	std::atomic<int> _monitor_count;

	application(void) noexcept;
	~application(void) noexcept;
public:
	void execute(void) noexcept;
	auto monitor(void) noexcept -> iocp::coroutine<void>;
};