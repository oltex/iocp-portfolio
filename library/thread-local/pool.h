#pragma once
#include "../memory.h"
#include "../function.h"
#include "../lockfree/pool.h"
#include "../singleton.h"
#include "singleton.h"
#include "../interlock.h"

namespace library::_thread_local {
	template<typename type, size_t bucket_size = 1024, bool placement = true, bool compress = true>
	class pool : public library::singleton<pool<type, bucket_size, placement, compress>> {
	protected:
		friend class library::singleton<pool<type, bucket_size, placement, compress>>;
		friend class context;
		using size_type = unsigned int;
		union union_node final {
			union_node* _next;
			type _value;
			inline explicit union_node(void) noexcept = delete;
			inline explicit union_node(union_node const&) noexcept = delete;
			inline explicit union_node(union_node&&) noexcept = delete;
			inline auto operator=(union_node const&) noexcept = delete;
			inline auto operator=(union_node&&) noexcept = delete;
			inline ~union_node(void) noexcept = delete;
		};
		struct strcut_node final {
			strcut_node* _next;
			type _value;
			inline explicit strcut_node(void) noexcept = delete;
			inline explicit strcut_node(strcut_node const&) noexcept = delete;
			inline explicit strcut_node(strcut_node&&) noexcept = delete;
			inline auto operator=(strcut_node const&) noexcept = delete;
			inline auto operator=(strcut_node&&) noexcept = delete;
			inline ~strcut_node(void) noexcept = delete;
		};
		using node = typename std::conditional<compress, union union_node, struct strcut_node>::type;
		struct bucket final {
			bucket* _next;
			node* _value;
			size_type _size;
			inline explicit bucket(void) noexcept = delete;
			inline explicit bucket(bucket const&) noexcept = delete;
			inline explicit bucket(bucket&&) noexcept = delete;
			inline auto operator=(bucket const&) noexcept = delete;
			inline auto operator=(bucket&&) noexcept = delete;
			inline ~bucket(void) noexcept = delete;
		};
		struct context : public singleton<context> {
			friend class singleton<context>;
			size_type _size;
			node* _head;
			node* _break;

			inline explicit context(void) noexcept
				: _size(0), _head(nullptr), _break(nullptr) {
			};
			inline explicit context(context const&) noexcept = delete;
			inline explicit context(context&&) noexcept = delete;
			inline auto operator=(context const&) noexcept -> context & = delete;
			inline auto operator=(context&&) noexcept -> context & = delete;
			inline ~context(void) noexcept {
				if (bucket_size < _size) {
					pool::instance().deallocate(_head, _size - bucket_size);
					_head = _break;
					_size = bucket_size;
				}
				if (0 < _size)
					pool::instance().deallocate(_head, _size);
			};
		};

		alignas(64) unsigned long long _head = 0;
		lockfree::pool<bucket, false> _pool;
		bucket* _stack = nullptr;

		inline explicit pool(void) noexcept = default;
		inline explicit pool(pool const&) noexcept = delete;
		inline explicit pool(pool&&) noexcept = delete;
		inline auto operator=(pool const&) noexcept -> pool & = delete;
		inline auto operator=(pool&&) noexcept -> pool & = delete;
		inline ~pool(void) noexcept {
			auto stack = _stack;
			while (nullptr != stack) {
				library::deallocate<node>(stack->_value);
				_pool.deallocate(library::exchange(stack, stack->_next));
			}

			auto head = reinterpret_cast<bucket*>(0x00007FFFFFFFFFFFULL & _head);
			while (nullptr != head)
				_pool.deallocate(library::exchange(head, head->_next));
		};

		inline void deallocate(node* value, size_type const size) noexcept {
			bucket* current = _pool.allocate();
			current->_value = value;
			current->_size = size;

			for (unsigned long long head = _head, prev;; head = prev) {
				current->_next = reinterpret_cast<bucket*>(0x00007FFFFFFFFFFFULL & head);
				unsigned long long next = reinterpret_cast<unsigned long long>(current) + (0xFFFF800000000000ULL & head);
				if (prev = _InterlockedCompareExchange(reinterpret_cast<unsigned long long volatile*>(&_head), next, head); prev == head)
					break;
			}
		}
	public:
		template<typename... argument>
		inline auto allocate(argument&&... arg) noexcept -> type* {
			auto& context = context::instance();

			if (0 == context._size) {
				for (unsigned long long head = _head, prev;; head = prev) {
					bucket* address = reinterpret_cast<bucket*>(0x00007FFFFFFFFFFFULL & head);
					if (nullptr == address) {
						context._head = library::allocate<node>(bucket_size);
						context._size = static_cast<size_type>(bucket_size);
						{
							auto current = context._head;
							auto next = current + 1;
							for (size_type index = 0; index < bucket_size - 1; ++index, current = next++)
#pragma warning(suppress: 6011)
								current->_next = next;
							current->_next = nullptr;
						}
						{
							auto current = _pool.allocate();
							current->_value = context._head;
							for (bucket* head = _stack, *prev;; head = prev) {
								current->_next = head;
								if (prev = library::interlock_compare_exchange(_stack, current, head); prev == head)
									break;
							}
						}
						break;
					}
					unsigned long long next = reinterpret_cast<unsigned long long>(address->_next) + (0xFFFF800000000000ULL & head) + 0x0000800000000000ULL;
					if (prev = library::interlock_compare_exchange(_head, next, head); prev == head) {
						context._head = address->_value;
						context._size = address->_size;
						_pool.deallocate(address);
						break;
					}
				}
			}
			auto current = library::exchange(context._head, context._head->_next);
			if constexpr (true == placement)
				library::construct<type, argument...>(current->_value, std::forward<argument>(arg)...);
			--context._size;
			return &current->_value;
		}
		inline void deallocate(type* value) noexcept {
			auto& context = context::instance();
			if constexpr (true == placement)
				library::destruct<type>(*value);
			auto current = reinterpret_cast<node*>(reinterpret_cast<unsigned char*>(value) - offsetof(node, _value));
			current->_next = library::exchange(context._head, current);
			++context._size;

			if (bucket_size == context._size)
				context._break = context._head;
			else if (bucket_size * 2 == context._size) {
				deallocate(context._head, bucket_size);
				context._head = context._break;
				context._size -= bucket_size;
			}
		}
	};
}