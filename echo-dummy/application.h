#pragma once
#include "command.h"
#include "network.h"
#include "library/thread.h"
#include "library/hash_table.h"
#include "library/iocp/actor/system.h"
#include "library/iocp/timer.h"
#include "library/lock.h"

class application : public library::singleton<application, true> {
	friend class library::singleton<application, true>;
public:
	network _network;
	actor::system _system;
	struct {
		unsigned long _client_count = 50;
		bool _disconnect_test = false;
		bool _attack_test = false;
		unsigned long _over_send = 100;
		unsigned long _action_delay = 0;
	} _option;
	struct {
		unsigned long _connect_fail = 0;
		unsigned long _echo_not_receive = 0;
	} _metric;
private:
	command _command;
	std::stop_source _stop_source;
	std::atomic<int> _stop_count;
	library::unorder_set<actor::handle> _dummy_handle;
	library::spin_lock _dummy_lock;

	application(void) noexcept;
	~application(void) noexcept;
public:
	auto thread_monitor(void) noexcept -> iocp::coroutine<void>;
	auto wake_loop(void) noexcept -> iocp::coroutine<void>;
	void execute(void) noexcept;
 };