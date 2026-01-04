#pragma once
#include "../interlock.h"
#include "../memory.h"
#include "../function.h"

namespace library::lockfree {
	template<typename type, bool placement = true, bool compress = true>
	class free_list {
	protected:
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
		unsigned long long _head;
		size_type _capacity;
		node* _array;
	public:
		class iterator final {
			node* _node;
		public:
			inline iterator(node* const node = nullptr) noexcept
				: _node(node) {
			}
			inline iterator(iterator const&) noexcept = default;
			inline iterator(iterator&&) noexcept = default;
			inline auto operator=(iterator const&) noexcept -> iterator & = default;
			inline auto operator=(iterator&&) noexcept -> iterator & = default;
			inline ~iterator() noexcept = default;

			inline auto operator*(void) const noexcept -> type& {
				return _node->_value;
			}
			inline auto operator->(void) const noexcept -> type* {
				return &_node->_value;
			}
			inline auto operator++(void) noexcept -> iterator& {
				++_node;
				return *this;
			}
			inline auto operator++(int) noexcept -> iterator {
				iterator iter(*this);
				++_node;
				return iter;
			}
			inline auto operator--(void) noexcept -> iterator& {
				--_node;
				return *this;
			}
			inline auto operator--(int) noexcept -> iterator {
				iterator iter(*this);
				--_node;
				return iter;
			}
			inline bool operator==(iterator const& rhs) const noexcept {
				return _node == rhs._node;
			}
		};

		inline free_list(size_type const capacity)noexcept
			: _capacity(capacity), _array(library::allocate<node>(capacity)) {
			_head = reinterpret_cast<unsigned long long>(_array);

			auto begin = _array;
			for (auto index = 0u; index < capacity - 1; ++index) {
				auto current = begin++;
				current->_next = begin;
			}
#pragma warning(suppress: 6011)
			begin->_next = nullptr;
		}
		inline free_list(free_list const&) noexcept = delete;
		inline free_list(free_list&&) noexcept = delete;
		inline auto operator=(free_list const&) noexcept -> free_list & = delete;
		inline auto operator=(free_list&&) noexcept -> free_list & = delete;
		inline ~free_list(void) noexcept {
			library::deallocate<node>(_array);
		};

		template<typename... argument>
		inline auto allocate(argument&&... arg) noexcept -> type* {
			for (unsigned long long head = _head, prev;; head = prev) {
				node* current = reinterpret_cast<node*>(0x00007FFFFFFFFFFFULL & head);
				if (nullptr == current)
					return nullptr;
				auto next = reinterpret_cast<unsigned long long>(current->_next) + (0xFFFF800000000000ULL & head) + 0x0000800000000000ULL;
				if (prev = library::interlock_compare_exhange(_head, next, head); prev == head) {
					if constexpr (true == placement)
						library::construct<type, argument...>(current->_value, std::forward<argument>(arg)...);
					return &current->_value;
				}
			}
		}
		inline void deallocate(type* value) noexcept {
			if constexpr (true == placement)
				library::destruct<type>(*value);
			auto current = reinterpret_cast<node*>(reinterpret_cast<unsigned char*>(value) - offsetof(node, _value));
			for (unsigned long long head = _head, prev;; head = prev) {
				current->_next = reinterpret_cast<node*>(head & 0x00007FFFFFFFFFFFULL);
				unsigned long long next = reinterpret_cast<unsigned long long>(current) + (head & 0xFFFF800000000000ULL);
				if (prev = library::interlock_compare_exhange(_head, next, head); prev == head)
					break;
			}
		}
		inline auto begin(void) const noexcept -> iterator {
			return iterator(_array);
		}
		inline auto end(void) const noexcept -> iterator {
			return iterator(_array + _capacity);
		}
		inline auto operator[](size_type const index) noexcept -> type& {
			return _array[index]._value;
		}
		inline auto capacity(void) const noexcept -> size_type {
			return _capacity;
		}
		inline auto data(void) noexcept {
			return _array;
		}
	};
}

//template<typename... argument>
//inline void reserve(size_type const capacity, argument&&... arg) noexcept {
//	_array = library::allocate<node>(capacity);
//	_capacity = capacity;
//
//	auto begin = _array;
//	for (size_type index = 0; index < capacity - 1; ++index) {
//		auto current = begin++;
//		current->_next = begin;
//		library::construct(current->_value, std::forward<argument>(arg)...);
//	}
//#pragma warning(suppress: 6011)
//	begin->_next = nullptr;
//	library::construct(begin->_value, std::forward<argument>(arg)...);
//
//	_head = reinterpret_cast<unsigned long long>(_array);
//}
//inline void clear(void) noexcept {
//	node* head = reinterpret_cast<node*>(0x00007FFFFFFFFFFFULL & _head);
//	while (nullptr != head)
//		library::destruct<type>(library::exchange(head, head->_next)->_value);
//	library::deallocate<node>(_array);
//}