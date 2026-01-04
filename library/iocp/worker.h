#pragma once
#include "scheduler.h"

namespace iocp {
	class worker {
	protected:
		scheduler& _scheduler;
	public:
		worker(void) noexcept;
		inline worker(worker const&) noexcept = delete;
		inline worker(worker&&) noexcept = delete;
		inline auto operator=(worker const&) noexcept -> worker & = delete;
		inline auto operator=(worker&&) noexcept -> worker & = delete;
		inline ~worker(void) noexcept = default;

		virtual void execute(bool result, unsigned long transferred, uintptr_t key, OVERLAPPED* overlapped) noexcept = 0;
	};
}