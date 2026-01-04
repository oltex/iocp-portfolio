#pragma once
#include "../memory.h"
#include "../template.h"
#include "../thread-local/pool.h"
#include "../storage.h"
#include <optional>

namespace library::lockfree {
	template <typename type, bool multi_pop = true>
		requires library::memory_copy_safe<type> || (std::is_trivially_copy_constructible_v<type>&& std::is_trivially_destructible_v<type>)
	class queue {
	protected:
		struct node final {
			unsigned long long _next;
			type _value;
			inline explicit node(void) noexcept = delete;
			inline explicit node(node const&) noexcept = delete;
			inline explicit node(node&&) noexcept = delete;
			inline auto operator=(node const&) noexcept = delete;
			inline auto operator=(node&&) noexcept = delete;
			inline ~node(void) noexcept = delete;
		};
		using _pool = _thread_local::pool<node, 1024, false>;

		alignas(64) unsigned long long _head;
		alignas(64) unsigned long long _tail;
	public:
		class iterator final {
			node* _node;
		public:
			inline explicit iterator(void) noexcept = default;
			inline explicit iterator(node* node_) noexcept
				: _node(node_) {
			}
			inline explicit iterator(iterator const&) noexcept = delete;
			inline explicit iterator(iterator&&) noexcept = delete;
			inline auto operator=(iterator const&) noexcept -> iterator & = delete;
			inline auto operator=(iterator&&) noexcept -> iterator & = delete;
			inline ~iterator(void) noexcept = default;

			inline auto operator*(void) const noexcept -> type& {
				return _node->_value;
			}
			inline auto operator->(void) const noexcept -> type* const {
				return &_node->_value;
			}
			inline auto operator++(void) noexcept -> iterator& {
				_node = reinterpret_cast<node*>(0x00007FFFFFFFFFFEULL & _node->_next);
				return *this;
			}
			inline bool operator==(iterator const& rhs) const noexcept {
				return _node == rhs._node;
			}
		};

		inline explicit queue(void) noexcept {
			node* current = _pool::instance().allocate();
			current->_next = 0x0000800000000000ULL + reinterpret_cast<unsigned long long>(this);
			_head = _tail = reinterpret_cast<unsigned long long>(current) + 1;
		}
		inline explicit queue(queue const&) noexcept = delete;
		inline explicit queue(queue&&) noexcept = delete;
		inline auto operator=(queue const&) noexcept -> queue & = delete;
		inline auto operator=(queue&&) noexcept -> queue & = delete;
		inline ~queue(void) noexcept {
			auto head = reinterpret_cast<node*>(0x00007FFFFFFFFFFEULL & _head);
			for (;;) {
				node* current = library::exchange(head, reinterpret_cast<node*>(0x00007FFFFFFFFFFEULL & head->_next));
				_pool::instance().deallocate(current);
				if (reinterpret_cast<unsigned long long>(this) == reinterpret_cast<unsigned long long>(head))
					break;
				library::destruct<type>(head->_value);
			}
		};

		template<typename... argument>
		inline void emplace(argument&&... arg) noexcept {
			node* current = _pool::instance().allocate();
			library::construct<type>(current->_value, std::forward<argument>(arg)...);

			for (unsigned long long tail = _tail;;) {
				unsigned long long count = 0xFFFF800000000000ULL & tail;
				auto address = reinterpret_cast<node*>(0x00007FFFFFFFFFFEULL & tail);
				unsigned long long next = address->_next;

				unsigned long long next_count = count + 0x0000800000000000ULL;
				if (next_count == (0xFFFF800000000000ULL & next)) {
					if (1 == (0x1ULL & next)) {
						tail = _InterlockedCompareExchange(reinterpret_cast<unsigned long long volatile*>(&_tail), next, tail);
						continue;
					}
					else if (reinterpret_cast<unsigned long long>(this) == (0x00007FFFFFFFFFFFULL & next)) {
						unsigned long long next_tail = next_count + reinterpret_cast<unsigned long long>(current) + 1;
						current->_next = next_count + 0x0000800000000000ULL + reinterpret_cast<unsigned long long>(this);
						if (next == _InterlockedCompareExchange(reinterpret_cast<unsigned long long volatile*>(&address->_next), next_tail, next))
							break;
					}
				}
				tail = *reinterpret_cast<volatile unsigned long long*>(&_tail);

			}
		}
		inline auto pop(void) noexcept -> std::optional<type> requires (true == multi_pop) {
			for (unsigned long long head = _head, prev;;) {
				unsigned long long count = 0xFFFF800000000000ULL & head;
				auto address = reinterpret_cast<node*>(0x00007FFFFFFFFFFEULL & head);
				unsigned long long next = address->_next;

				if (count + 0x0000800000000000ULL == (0xFFFF800000000000ULL & next)) {
					if (reinterpret_cast<unsigned long long>(this) == (0x00007FFFFFFFFFFFULL & next))
						return std::nullopt;
					else if (1 == (0x1ULL & next)) {
						unsigned long long tail = _tail;
						if (tail == head)
							_InterlockedCompareExchange(reinterpret_cast<unsigned long long volatile*>(&_tail), next, tail);
							library::storage<type> result;
							result.relocate(reinterpret_cast<node*>(0x00007FFFFFFFFFFEULL & next)->_value);
							if (prev = _InterlockedCompareExchange(reinterpret_cast<unsigned long long volatile*>(&_head), next, head); prev == head) {
								_pool::instance().deallocate(address);
								return std::move(result.get());
							}
						head = prev;
						continue;
					}
				}
				head = *reinterpret_cast<volatile unsigned long long*>(&_head);
			}
		}

		inline auto pop(void) noexcept -> type requires (false == multi_pop) {
			unsigned long long head = _head;
			auto address = reinterpret_cast<node*>(0x00007FFFFFFFFFFEULL & head);
			unsigned long long next = address->_next;

			if (reinterpret_cast<unsigned long long>(this) == (0x00007FFFFFFFFFFFULL & next))
				__debugbreak();
			else if (1 != (0x1ULL & next))
				__debugbreak();

			unsigned long long tail = _tail;
			if (tail == head)
				_InterlockedCompareExchange(reinterpret_cast<unsigned long long volatile*>(&_tail), next, tail);

			library::storage<type> result;
			result.relocate(reinterpret_cast<node*>(0x00007FFFFFFFFFFEULL & next)->_value);
			_head = next;
			_pool::instance().deallocate(address);
			return std::move(result.get());
		}
		inline auto empty(void) const noexcept requires (false == multi_pop) {
			unsigned long long head = _head;
			auto address = reinterpret_cast<node*>(0x00007FFFFFFFFFFEULL & head);
			unsigned long long next = address->_next;

			if (reinterpret_cast<unsigned long long>(this) == (0x00007FFFFFFFFFFFULL & next))
				return true;
			return false;
		}
		inline auto begin(void) noexcept -> iterator requires(false == multi_pop) {
			return iterator(reinterpret_cast<node*>(0x00007FFFFFFFFFFEULL & reinterpret_cast<node*>(0x00007FFFFFFFFFFEULL & _head)->_next));
		}
		inline auto end(void) noexcept -> iterator requires(false == multi_pop) {
			return iterator(reinterpret_cast<node*>(this));
		}
		//inline void clear(void) noexcept {
		//	/*			auto head = reinterpret_cast<node*>(0x00007FFFFFFFFFFEULL & _head);
		//				for (;;) {
		//					node* current = library::exchange(head, reinterpret_cast<node*>(0x00007FFFFFFFFFFEULL & head->_next));
		//					_pool::instance().deallocate(current);
		//					if (reinterpret_cast<unsigned long long>(this) == reinterpret_cast<unsigned long long>(head))
		//						break;
		//					library::destruct<type>(head->_value);
		//	}*/
		//}
	};
}
//requires std::is_trivially_copy_constructible_v<type>&& std::is_trivially_destructible_v<type>
//type result = reinterpret_cast<node*>(0x00007FFFFFFFFFFEULL & next)->_value;
//_pool::instance().deallocate(address);
//return result;
