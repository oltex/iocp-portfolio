#pragma once
#include "scheduler.h"
#include "../time.h"
#include "../thread.h"
#include "../synchronize.h"
#include "../coroutine.h"
#include "../lockfree/queue.h"

namespace iocp {
	class sleep : public library::awaiter, public task {
		using size_type = unsigned int;
		std::coroutine_handle<void> _handle;
		unsigned long _time;
	public:
		sleep(size_type const time) noexcept;
		void await_suspend(std::coroutine_handle<void> handle) noexcept;
		auto time(void) const noexcept -> unsigned long;
		virtual void task_execute(void) noexcept override;
	};

	class timer : public library::singleton<timer, true> {
		friend class library::singleton<timer, true>;
		library::thread _thread;
		library::lockfree::queue<sleep*, false> _insert_queue;
		unsigned long _insert_size;

		timer(void) noexcept;
		inline timer(timer const&) noexcept = delete;
		inline timer(timer&&) noexcept = delete;
		inline auto operator=(timer const&) noexcept -> timer & = delete;
		inline auto operator=(timer&&) noexcept -> timer & = delete;
		~timer(void) noexcept;
	public:
		void insert(sleep* sleep) noexcept;
		void execute(void) noexcept;
	};
}