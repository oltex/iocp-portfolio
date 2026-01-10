#pragma once
#include "entity.h"
#include "../promise.h"
#include "../scheduler.h"
#include "../message.h"
#include "../../grc/arena.h"

namespace actor {
	class wait : public library::awaiter {
		using size_type = unsigned int;
		friend class system;
		system& _system;
		grc::arena<entity, false>::node& _node;
	public:
		wait(system& system, grc::arena<entity, false>::node& node) noexcept;
		auto await_suspend(std::coroutine_handle<void> handle) noexcept -> bool;
	};

	class system  {
	public:
		friend class wait;
		using node = grc::arena<entity, false>::node;
	private:
		grc::arena<entity, false> _entity_arena;
	public:
		system(unsigned long const entity_capacity) noexcept;
		system(system const&) noexcept = delete;
		system(system&&) noexcept = delete;
		auto operator=(system const&) noexcept -> system & = delete;
		auto operator=(system&&) noexcept -> system & = delete;
		~system(void) noexcept = default;

		template<typename derive>
		auto entity_attach(derive* pointer, void(*deleter)(entity*)) noexcept -> handle {
			auto node = _entity_arena.allocate();
			if (nullptr == node)
				__debugbreak();
			node->attach(pointer, deleter);

			auto coroutine = entity_loop(*node);
			coroutine.auto_start(false);
			node->_value->_handle = coroutine.data();

			return node->_key;
		}
		template<typename mail>
		void entity_enqueue(handle handle, mail const& value) noexcept {
			auto& node = _entity_arena.get(handle);
			if (node.acquire(handle)) {
				auto& pool = iocp::message_pool::instance();
				auto message = pool.allocate(sizeof(mail));
				message.push(reinterpret_cast<unsigned char const*>(&value), sizeof(mail));
				node._value->_queue.emplace(message);

				if (node._value->flag_ready()) {
					iocp::scheduler::instance().post(*node._value);
					return;
				}
			}
			if (node.release())
				_entity_arena.deallocate(&node);
		}
		void entity_destroy(handle handle) noexcept;
		auto entity_loop(node& node) noexcept -> iocp::coroutine<void>;
	};
}