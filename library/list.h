#pragma once
#include "memory.h"
#include "function.h"
#include "pool.h"
#include <utility>
#include <stdlib.h>
#include <malloc.h>
#include <cassert>

namespace detail {
	template<typename trait, typename hash, typename predicate, bool duplicate>
	class hash_table;
}
namespace library {
	template<typename type, typename allocator = pool<type>, bool placement = true>
	class list final {
		template<typename trait, typename hash, typename predicate, bool duplicate>
		friend class detail::hash_table;
		using size_type = unsigned int;
		struct node final {
			node* _prev, * _next;
			type _value;
			inline explicit node(void) noexcept = delete;
			inline explicit node(node const&) noexcept = delete;
			inline explicit node(node&&) noexcept = delete;
			inline auto operator=(node const&) noexcept -> node & = delete;
			inline auto operator=(node&&) noexcept -> node & = delete;
			inline ~node(void) noexcept = delete;
		};
		size_type _size;
		node* _head;
		allocator::template rebind<node, false> _allocator;
	public:
		class iterator final {
		public:
			node* _node;
			inline explicit iterator(node* const node = nullptr) noexcept
				: _node(node) {
			}
			inline iterator(iterator const&) noexcept = default;
			inline explicit iterator(iterator&&) noexcept = default;
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
				_node = _node->_next;
				return *this;
			}
			inline auto operator++(int) noexcept -> iterator {
				iterator iter(*this);
				_node = _node->_next;
				return iter;
			}
			inline auto operator--(void) noexcept -> iterator& {
				_node = _node->_prev;
				return *this;
			}
			inline auto operator--(int) noexcept -> iterator {
				iterator iter(*this);
				_node = _node->_prev;
				return iter;
			}
			inline bool operator==(iterator const& rhs) const noexcept {
				return _node == rhs._node;
			}
		};

		inline explicit list(void) noexcept
			: _size(0), _head(reinterpret_cast<node*>(library::allocate(sizeof(node*) * 2))) {
#pragma warning(suppress: 6011)
			_head->_next = _head->_prev = _head;
		}
		inline explicit list(std::initializer_list<type> init_list) noexcept
			: list() {
			for (auto& iter : init_list)
				emplace_back(iter);
		}
		inline explicit list(iterator begin, iterator end) noexcept
			: list() {
			for (auto iter = begin; iter != end; ++iter)
				emplace_back(*iter);
		}
		inline list(list const& rhs) noexcept
			: list(rhs.begin(), rhs.end()) {
		}
		inline explicit list(list&& rhs) noexcept
			: list() {
			swap(rhs);
		}
		inline auto operator=(list const& rhs) noexcept -> list& {
			assert(this != &rhs && "self-assignment");
			list(rhs).swap(*this);
			return *this;
		};
		inline auto operator=(list&& rhs) noexcept -> list& {
			assert(this != &rhs && "self-assignment");
			list(std::move(rhs)).swap(*this);
			return *this;
		};
		inline ~list(void) noexcept {
			clear();
			library::deallocate(reinterpret_cast<void*>(_head));
		}

		template<typename... argument>
		inline auto emplace(iterator iter, argument&&... arg) noexcept -> iterator {
			auto current = allocate(std::forward<argument>(arg)...);
			link(iter._node, current);
			return iterator(current);
		}
		template<typename... argument>
		inline auto emplace_front(argument&&... arg) noexcept -> type& {
			return *emplace(begin(), std::forward<argument>(arg)...);
		}
		template<typename... argument>
		inline auto emplace_back(argument&&... arg) noexcept -> type& {
			return *emplace(end(), std::forward<argument>(arg)...);
		}
		inline auto erase(iterator iter) noexcept -> iterator {
			assert(_size > 0 && "called on empty");
			assert(iter._node != _head && "erase on sentinel");
			auto current = iter._node;
			auto prev = current->_prev;
			auto next = current->_next;

			prev->_next = next;
			next->_prev = prev;

			deallocate(current);
			--_size;
			return iterator(next);
		}
		inline void pop_front(void) noexcept {
			erase(begin());
		}
		inline void pop_back(void) noexcept {
			erase(--end());
		}

		inline auto front(void) const noexcept -> type& {
			assert(_size > 0 && "called on empty");
			return _head->_next->_value;
		}
		inline auto back(void) const noexcept -> type& {
			assert(_size > 0 && "called on empty");
			return _head->_prev->_value;
		}
		inline auto begin(void) const noexcept -> iterator {
			return iterator(_head->_next);
		}
		inline auto end(void) const noexcept -> iterator {
			return iterator(_head);
		}
		inline void swap(list& rhs) noexcept {
			library::swap(_head, rhs._head);
			library::swap(_size, rhs._size);
		}
		inline void clear(void) noexcept {
			auto current = _head->_next;
			while (current != _head) {
				if constexpr (true == placement)
					library::destruct<type>(current->_value);
				_allocator.deallocate(exchange(current, current->_next));
			}
			_head->_next = _head->_prev = _head;
			_size = 0;
		}
		inline auto size(void) const noexcept -> size_type {
			return _size;
		}
		inline bool empty(void) const noexcept {
			return 0 == _size;
		}
	private:
		template<typename... argument>
		inline auto allocate(argument&&... arg) noexcept -> node* {
			auto current = _allocator.allocate();
			if constexpr (true == placement)
				library::construct<type>(current->_value, std::forward<argument>(arg)...);
			return current;
		}
		inline void deallocate(node* current) noexcept {
			if constexpr (true == placement)
				library::destruct<type>(current->_value);
			_allocator.deallocate(current);
		}
		inline void link(node* next, node* current) noexcept {
			auto prev = next->_prev;

			prev->_next = current;
			current->_prev = prev;
			current->_next = next;
			next->_prev = current;

			++_size;
		}
		inline void splice(node* before, node* first, node* last) noexcept {
			node* first_prev = first->_prev;
			node* last_prev = last->_prev;
			node* before_prev = before->_prev;

			first_prev->_next = last;
			last_prev->_next = before;
			before_prev->_next = first;

			before->_prev = last_prev;
			last->_prev = first_prev;
			first->_prev = before_prev;
		}
	};
}