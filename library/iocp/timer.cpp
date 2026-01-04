#include "timer.h"
#include "scheduler.h"
#include "library/container/queue.h"
#include <queue>

namespace iocp {
	sleep::sleep(size_type const time) noexcept
		: _time(library::time_get_time() + time) {
	}
	void sleep::await_suspend(std::coroutine_handle<void> handle) noexcept {
		_handle = handle;
		timer::instance().insert(this);
	}
	auto sleep::time(void) const noexcept -> unsigned long {
		return _time;
	}
	void sleep::execute(void) noexcept {
		_handle.resume();
	}

	timer::timer(void) noexcept
		: _thread(&timer::execute, 0, this), _insert_size(0) {
		library::time_begin_period(1);
	}
	timer::~timer(void) noexcept {
		library::interlock_or(_insert_size, 0x80000000);
		library::wake_by_address_all(_insert_size);
		_thread.wait_for_single(INFINITE);
		library::time_end_period(1);
	}
	void timer::insert(sleep* sleep) noexcept {
		library::interlock_increment(_insert_size);
		_insert_queue.emplace(sleep);
		library::wake_by_address_single(_insert_size);
	}
	void timer::execute(void) noexcept {
		library::priority_queue < sleep*, decltype([](sleep* left, sleep* right) {
			return left->time() < right->time();
			}) > ready_queue;
		auto wait = INFINITE;
		for (;;) {
			unsigned long compare = 0;
			library::wait_on_address(_insert_size, compare, wait);
			if (_insert_size & 0x80000000)
				break;
			else {
				while (!_insert_queue.empty()) {
					auto result = _insert_queue.pop();
					library::interlock_decrement(_insert_size);
					ready_queue.emplace(result);
				}
				wait = INFINITE;
				auto time = library::time_get_time();

				while (!ready_queue.empty()) {
					if (auto top = ready_queue.top(); time < top->time()) {
						wait = top->time() - time;
						break;
					}
					else {
						ready_queue.pop();
						scheduler::instance().post(*top);
					}
				}
			}
		}
	}
}