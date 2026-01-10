#pragma once
#include "../message.h"
#include "../promise.h"
#include "../scheduler.h"
#include "../../queue.h"
#include "../../grc/arena.h"

namespace actor {
	class entity;
	using handle = grc::arena<entity, false>::handle;

	struct mail {
		unsigned short _type;
		mail(unsigned short type) noexcept;
	};

	class entity : public iocp::task, public grc::arena<entity, false>::self {
		using self = grc::arena<entity, false>::self;
		friend class system;
		friend class wait;
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
	private:
		auto flag_ready(void) noexcept -> bool;
		void flag_finish(void) noexcept;
		virtual void task_execute(void) noexcept override;
		using self::arena_handle;
	public:
		auto entity_handle(void) noexcept -> handle;
		virtual auto entity_mailbox(mail& mail) noexcept -> iocp::coroutine<bool> = 0;
	};
}