#include "lock.h"
#include "scheduler.h"
#include "library/algorithm/sort.h"

namespace iocp {
	auto lock::await_suspend(std::coroutine_handle<void> handle) noexcept -> bool {
		_handle = handle;

		auto success = false;
		library::select_sort(_proxy.begin(), _proxy.end(), [](proxy& first, proxy& second) {
			return first._mutex < second._mutex;
			});
		for (auto& iter : _proxy) {
			iter._mutex->_spin.lock();

			if (!iter._mutex->_waiter.empty()) {
				iter._mutex->_waiter.emplace_back(this, iter._mode);
				++_dependency;
				success = true;
			}
			else if (mutex::mode::exclude == iter._mode) {
				if (0 != iter._mutex->_shared || true == iter._mutex->_exclusive) {
					iter._mutex->_waiter.emplace_back(this, iter._mode);
					++_dependency;
					success = true;
				}
				else
					iter._mutex->_exclusive = true;
			}
			else {
				if (true == iter._mutex->_exclusive) {
					iter._mutex->_waiter.emplace_back(this, iter._mode);
					++_dependency;
					success = true;
				}
				else {
					++iter._mutex->_shared;
				}
			}
		}
		for (auto& iter : _proxy)
			iter._mutex->_spin.unlock();
		return success;
	}
	void lock::execute(void) noexcept {
		_handle.resume();
	}
	void unlock(library::vector<proxy>& arg) noexcept {
		library::list<lock*> successor;

		for (auto& iter : arg) {
			successor.clear();
			iter._mutex->_spin.lock();
			bool success = false;
			if (mutex::mode::exclude == iter._mode) {
				iter._mutex->_exclusive = false;
				success = true;
			}
			else if (0 == --iter._mutex->_shared) {
				success = true;
			}

			if (true == success) {
				while (!iter._mutex->_waiter.empty()) {
					auto& wait = iter._mutex->_waiter.front();
					if (mutex::mode::exclude == wait._mode) {
						if (0 != iter._mutex->_shared)
							break;
						iter._mutex->_exclusive = true;
						successor.emplace_back(wait._lock);
						iter._mutex->_waiter.pop_front();
						break;
					}
					else {
						++iter._mutex->_shared;
						successor.emplace_back(wait._lock);
						iter._mutex->_waiter.pop_front();
					}
				}
			}
			iter._mutex->_spin.unlock();
			for (auto& iter : successor) {
				if (0 == library::interlock_decrement(iter->_dependency))
					scheduler::instance().post(*iter);
			}
		}
	}

	fork::fork(size_type count) noexcept
		: _count(count + 1) {
	}
	auto fork::await_suspend(std::coroutine_handle<void> handle) noexcept -> bool {
		_handle = handle;
		return 0 != library::interlock_decrement(_count);
	}
	void fork::join(void) noexcept {
		if (0 == library::interlock_decrement(_count))
			scheduler::instance().post(*this);
	}
	void fork::execute(void) noexcept {
		_handle.resume();
	}
}