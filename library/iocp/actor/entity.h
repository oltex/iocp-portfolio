#pragma once
#include "../message.h"
#include "../task.h"
#include "../promise.h"
#include "../../lockfree/queue.h"
#include "../../grc/arena.h"

namespace actor {
	struct job {
		unsigned short _type;
		job(unsigned short type) noexcept;
	};

	class entity : public iocp::task, public grc::arena<entity, false>::self {
		friend class system;
		std::coroutine_handle<void> _handle;
		unsigned short _destroy_flag;
		unsigned short _queue_flag;
		library::lockfree::queue<iocp::message, false> _queue;
	public:
		entity(void) noexcept;
		entity(entity const&) noexcept = delete;
		entity(entity&&) noexcept = delete;
		auto operator=(entity const&) noexcept -> entity & = delete;
		auto operator=(entity&&) noexcept -> entity & = delete;
		virtual ~entity(void) noexcept = default;

		auto flag_ready(void) noexcept -> bool;
		void flag_finish(void) noexcept;
		virtual void execute(void) noexcept override;

		virtual auto callback(job& job) noexcept -> iocp::coroutine<bool> = 0;
	};
}