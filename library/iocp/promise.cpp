#include "promise.h"

namespace iocp {
	finalize::finalize(std::coroutine_handle<void> parent) noexcept
		: _parent(parent) {
	}
	auto finalize::await_ready(void) noexcept -> bool {
		return std::noop_coroutine() == _parent;
	}
	auto finalize::await_suspend(std::coroutine_handle<void> handle) const noexcept -> std::coroutine_handle<void> {
		return _parent;
	}
	auto promise<void>::final_suspend(void) noexcept -> finalize {
		return finalize(_parent);
	}
	void promise<void>::execute(void) noexcept {
		handle().resume();
	}
}