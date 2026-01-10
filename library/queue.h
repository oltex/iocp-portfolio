#pragma once
#include "memory.h"
#include "function.h"
#include "template.h"
#include "bit.h"
#include "vector.h"
#include "pool.h"
#include <cassert>
#include <memory>
#include <type_traits>

#include "storage.h"
#include <optional>


namespace library {
	template<typename type, typename allocator = pool<type>, bool placement = true>
	class queue final {
		using size_type = unsigned int;
		struct node final {
			node* _next;
			type _value;
			inline explicit node(void) noexcept = delete;
			inline explicit node(node const&) noexcept = delete;
			inline explicit node(node&&) noexcept = delete;
			inline auto operator=(node const&) noexcept = delete;
			inline auto operator=(node&&) noexcept = delete;
			inline ~node(void) noexcept = delete;
		};
		size_type _size = 0;
		node* _head;
		node* _tail;
		allocator::template rebind<node> _allocator;
	public:
		inline explicit queue(void) noexcept {
			auto current = _allocator.allocate();
			current->_next = nullptr;
			_head = _tail = current;
		};
		inline explicit queue(queue const&) noexcept = delete;
		inline explicit queue(queue&& rhs) noexcept
			: queue() {
			swap(rhs);
		};
		inline auto operator=(queue const&) noexcept -> queue & = delete;
		inline auto operator=(queue&& rhs) noexcept -> queue& {
			assert(this != &rhs && "self-assignment");
			queue(std::move(rhs)).swap(*this);
			return *this;
		};
		inline ~queue(void) noexcept {
			auto current = _head->_next;
			while (nullptr != current) {
				if constexpr (true == placement)
					library::destruct(current->_value);
				_allocator.deallocate(exchange(current, current->_next));
			}
			deallocate(reinterpret_cast<void*>(_head));
		};

		template<typename... argument>
		inline void emplace(argument&&... arg) noexcept {
			auto current = _allocator.allocate();
			current->_next = nullptr;
			if constexpr (true == placement)
				library::construct<type>(current->_value, std::forward<argument>(arg)...);
			library::exchange(_tail, current)->_next = current;
			++_size;
		}
		inline void pop(void) noexcept {
			assert(_size > 0 && "called on empty");
			auto current = library::exchange(_head->_next, _head->_next->_next);
			if (_tail == current)
				_tail = _head;
			if constexpr (true == placement)
				library::destruct(current->_value);
			_allocator.deallocate(current);
			--_size;
		}
		inline auto front(void) noexcept -> type& {
			assert(_size > 0 && "called on empty");
			return _head->_next->_value;
		}
		inline auto back(void) noexcept -> type& {
			assert(_size > 0 && "called on empty");
			return _tail->_value;
		}
		inline void swap(queue& rhs) noexcept {
			library::swap(_size, rhs._size);
			library::swap(_head, rhs._head);
			library::swap(_tail, rhs._tail);
		}
		inline auto size(void) const noexcept -> size_type {
			return _size;
		}
		inline auto empty(void) const noexcept -> bool {
			return 0 == _size;
		}
		inline void clear(void) noexcept {
			while (0 != _size)
				pop();
		}
	};

	template<typename type, typename predicate = library::less<type>, typename allocator = std::nullptr_t>
	class priority_queue final {
		using size_type = unsigned int;
		struct node final {
			node* _left, * _right, * _parent;
			type _value;
			inline explicit node(void) noexcept = delete;
			inline explicit node(node const&) noexcept = delete;
			inline explicit node(node&&) noexcept = delete;
			inline auto operator=(node const&) noexcept = delete;
			inline auto operator=(node&&) noexcept = delete;
			inline ~node(void) noexcept = delete;
		};
		size_type _size;
		node* _root;
		allocator::template rebind<node> _allocator;
	public:
		inline explicit priority_queue(void) noexcept
			: _size(0), _root(reinterpret_cast<node*>(library::allocate(sizeof(node*) * 3))) {
			//_last = _root->_left = _root->_right = _root->_parent = _root;
		};
		inline explicit priority_queue(priority_queue const&) noexcept = default;
		inline explicit priority_queue(priority_queue&&) noexcept = default;
		inline auto operator=(priority_queue const&) noexcept -> priority_queue & = default;
		inline auto operator=(priority_queue&&) noexcept -> priority_queue & = default;
		inline ~priority_queue(void) noexcept {
			library::deallocate(reinterpret_cast<void*>(_root));
		};

		template<typename... argument>
		inline void emplace(argument&&... arg) noexcept {
			auto current = _allocator.allocate();
			library::construct<type>(current->_value, std::forward<argument>(arg)...);

			auto mask = library::bit_mask_reverse(++_size);
			auto parent = _root;
			while (mask > 1) {
				if (_size & mask)
					parent = parent->_right;
				else
					parent = parent->_left;
				mask >>= 1;
			}
			current->_parent = parent;
			current->_left = current->_right = _root;
			if (_size & mask)
				parent->_right = current;
			else
				parent->_left = current;

			while (current->_parent != _root) {
				auto parent = current->_parent;
				if (predicate::execute(parent->_value, current->_value))
					break;
				//swap(current, parent);
			}
		};
	};
	template<typename type, typename predicate>
	class priority_queue<type, predicate, std::nullptr_t> final {
		using size_type = unsigned int;
		vector<type> _vector;
		[[no_unique_address]] predicate _predicate;
	public:
		inline explicit priority_queue(void) noexcept = default;
		inline explicit priority_queue(priority_queue const&) noexcept = default;
		inline explicit priority_queue(priority_queue&&) noexcept = default;
		inline auto operator=(priority_queue const&) noexcept -> priority_queue & = default;
		inline auto operator=(priority_queue&&) noexcept -> priority_queue & = default;
		inline ~priority_queue(void) noexcept = default;

		template<typename... argument>
		inline void emplace(argument&&... arg) noexcept {
			_vector.emplace_back(std::forward<argument>(arg)...);
			auto leaf = _vector.back();
			auto child = _vector.size() - 1;
			while (0 < child) {
				auto parent = (child - 1) / 2;

				if (_predicate(_vector[parent], leaf))
					break;
				_vector[child] = _vector[parent];
				child = parent;
			}
			_vector[child] = leaf;
		};
		inline void pop(void) noexcept {
			auto leaf = _vector.back();
			auto size = _vector.size() - 1;

			size_type parent = 0;
			for (;;) {
				auto left = parent * 2 + 1;
				if (size <= left)
					break;
				auto right = left + 1;

				if (size > right && _predicate(_vector[right], _vector[left]))
					left = right;
				if (_predicate(leaf, _vector[left]))
					break;

				_vector[parent] = _vector[left];
				parent = left;
			}
			_vector[parent] = leaf;
			_vector.pop_back();
		}
		inline auto top(void) const noexcept -> type& {
			return _vector.front();
		};
		inline void clear(void) noexcept {
			_vector.clear();
		}
		inline void swap(priority_queue& rhs) noexcept {
			library::swap(_vector, rhs._vector);
		}
		inline auto size(void) const noexcept -> size_type {
			return _vector.size();
		}
		inline bool empty(void) const noexcept {
			return _vector.empty();
		}
	};

	template<typename type, bool resize = true, bool placement = true>
	class circle_queue final {
		using size_type = unsigned int;
		size_type _capacity;
		size_type _front, _rear;
		type* _array;
	public:
#pragma region iterator
		//class iterator final {
		//public:
		//	inline explicit iterator(type* const array_, size_type current, size_type capacity) noexcept
		//		: _array(array_), _current(current), _capacity(capacity) {
		//	}
		//	inline iterator(iterator const& rhs) noexcept
		//		: _array(rhs._array), _current(rhs._current), _capacity(rhs._capacity) {
		//	}
		//	inline auto operator=(iterator const& rhs) noexcept -> iterator& {
		//		_array = rhs._array;
		//		_current = rhs._current;
		//		_capacity = rhs._capacity;
		//		return *this;
		//	}
		//public:
		//	inline auto operator*(void) const noexcept -> type& {
		//		return _array[_current];
		//	}
		//	inline auto operator->(void) const noexcept -> type* {
		//		return _array + _current;
		//	}
		//	inline auto operator++(void) noexcept -> iterator& {
		//		_current = (_current + 1) % _capacity;
		//		return *this;
		//	}
		//	inline auto operator++(int) noexcept -> iterator {
		//		iterator iter(*this);
		//		_current = (_current + 1) % _capacity;
		//		return iter;
		//	}
		//	inline auto operator--(void) noexcept -> iterator& {
		//		_front = (_front + 1) % _capacity;
		//		return *this;
		//	}
		//	inline auto operator--(int) noexcept -> iterator {
		//		iterator iter(*this);
		//		_front = (_front + 1) % _capacity;
		//		return iter;
		//	}
		//	inline bool operator==(iterator const& rhs) const noexcept {
		//		return _current == rhs._current;
		//	}
		//	inline bool operator!=(iterator const& rhs) const noexcept {
		//		return _current != rhs._current;
		//	}
		//public:
		//	size_type _capacity = 0;
		//	size_type _current = 0;
		//	type* _array;
		//};
		//inline auto begin(void) noexcept -> iterator {
		//	return iterator(_array, _front, _capacity);
		//}
		//inline auto end(void) noexcept -> iterator {
		//	return iterator(_array, _rear, _capacity);
		//}
#pragma endregion
		inline circle_queue(void) noexcept
			: _capacity(1), _front(0), _rear(0), _array(library::allocate<type>(1)) {
		};
		inline circle_queue(circle_queue const&) noexcept = delete;
		inline circle_queue(circle_queue&& rhs) noexcept
			: circle_queue() {
			swap(rhs);
		};
		inline auto operator=(circle_queue const&) noexcept -> circle_queue & = delete;
		inline auto operator=(circle_queue&& rhs) noexcept -> circle_queue& {
			circle_queue(std::move(rhs)).swap(*this);
			return *this;
		};
		inline ~circle_queue(void) noexcept {
			clear();
			library::deallocate<type>(_array);
		}

		template<typename... argument>
		inline auto emplace(argument&&... arg) noexcept -> type& {
			if constexpr (true == resize) {
				if (full())
					reserve(maximum(static_cast<size_type>(_capacity * 1.5f), size() + 2));
			}
			else
				assert(full() && "called on full");
			auto& element = _array[_rear];
			if constexpr (true == placement)
				library::construct<type>(element, std::forward<argument>(arg)...);
			_rear = (_rear + 1) % _capacity;
			return element;
		}
		inline void pop(void) noexcept {
			assert(!empty() && "called on empty");
			if constexpr (true == placement)
				library::destruct<type>(_array[_front]);
			_front = (_front + 1) % _capacity;
		}
		inline auto top(void) const noexcept -> type& {
			assert(!empty() && "called on empty");
			return _array[_front];
		};
		inline void clear(void) noexcept {
			if constexpr (true == placement && std::is_destructible_v<type> && !std::is_trivially_destructible_v<type>)
				while (!empty())
					pop();
			else
				_front = _rear;
		}
		inline void swap(circle_queue& rhs) noexcept {
			library::swap(_capacity, rhs._capacity);
			library::swap(_front, rhs._front);
			library::swap(_rear, rhs._rear);
			library::swap(_array, rhs._array);
		}
		inline void reserve(size_type const capacity) noexcept {
			if (_capacity < capacity) {
				auto size_ = size();
				auto once = _capacity - _front;
				auto array_ = library::allocate<type>(capacity);
				if (size_ <= once)
					library::memory_copy(array_, _array + _front, size_);
				else {
					library::memory_copy(array_, _array + _front, once);
					library::memory_copy(array_ + once, _array, size_ - once);
				}
				library::deallocate<type>(_array);

				_capacity = capacity;
				_front = 0;
				_rear = size_;
				_array = array_;
			}
		}
		inline auto size(void) const noexcept -> size_type {
			return (_rear + _capacity - _front) % _capacity;
		}
		inline auto remain(void) const noexcept -> size_type {
			return (_front + _capacity - (_rear + 1)) % _capacity;
		}
		inline auto capacity(void) const noexcept -> size_type {
			return _capacity;
		}
		inline bool empty(void) const noexcept {
			return _front == _rear;
		}
		inline bool full(void) const noexcept {
			return (_rear + 1) % _capacity == _front;
		}
	};

	namespace lockfree {
		template <typename type, bool multi_pop = true>
			requires library::memory_copy_safe<type> || (std::is_trivially_copy_constructible_v<type> && std::is_trivially_destructible_v<type>)
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
					_node = reinterpret_cast<node*>(0x00007ffffffffffeull & _node->_next);
					return *this;
				}
				inline bool operator==(iterator const& rhs) const noexcept {
					return _node == rhs._node;
				}
			};

			inline explicit queue(void) noexcept {
				node* current = _pool::instance().allocate();
				current->_next = 0x0000800000000000ull + reinterpret_cast<unsigned long long>(this);
				_head = _tail = reinterpret_cast<unsigned long long>(current) + 1;
			}
			inline explicit queue(queue const&) noexcept = delete;
			inline explicit queue(queue&&) noexcept = delete;
			inline auto operator=(queue const&) noexcept -> queue & = delete;
			inline auto operator=(queue&&) noexcept -> queue & = delete;
			inline ~queue(void) noexcept {
				auto head = reinterpret_cast<node*>(0x00007ffffffffffeull & _head);
				for (;;) {
					node* current = library::exchange(head, reinterpret_cast<node*>(0x00007ffffffffffeull & head->_next));
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
					unsigned long long count = 0xffff800000000000ull & tail;
					auto address = reinterpret_cast<node*>(0x00007ffffffffffeull & tail);
					unsigned long long next = address->_next;

					unsigned long long next_count = count + 0x0000800000000000ull;
					if (next_count == (0xffff800000000000ull & next)) {
						if (1 == (0x1ULL & next)) {
							tail = _InterlockedCompareExchange(reinterpret_cast<unsigned long long volatile*>(&_tail), next, tail);
							continue;
						}
						else if (reinterpret_cast<unsigned long long>(this) == (0x00007fffffffffffull & next)) {
							unsigned long long next_tail = next_count + reinterpret_cast<unsigned long long>(current) + 1;
							current->_next = next_count + 0x0000800000000000ull + reinterpret_cast<unsigned long long>(this);
							if (next == _InterlockedCompareExchange(reinterpret_cast<unsigned long long volatile*>(&address->_next), next_tail, next))
								break;
						}
					}
					tail = *reinterpret_cast<volatile unsigned long long*>(&_tail);

				}
			}
			inline auto pop(void) noexcept -> std::optional<type> requires (true == multi_pop) {
				for (unsigned long long head = _head, prev;;) {
					unsigned long long count = 0xffff800000000000ull & head;
					auto address = reinterpret_cast<node*>(0x00007ffffffffffeull & head);
					unsigned long long next = address->_next;

					if (count + 0x0000800000000000ull == (0xffff800000000000ull & next)) {
						if (reinterpret_cast<unsigned long long>(this) == (0x00007fffffffffffull & next))
							return std::nullopt;
						else if (1 == (0x1ull & next)) {
							unsigned long long tail = _tail;
							if (tail == head)
								_InterlockedCompareExchange(reinterpret_cast<unsigned long long volatile*>(&_tail), next, tail);
							library::storage<type> result;
							result.relocate(reinterpret_cast<node*>(0x00007ffffffffffeull & next)->_value);
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
				auto address = reinterpret_cast<node*>(0x00007ffffffffffeull & head);
				unsigned long long next = address->_next;

				if (reinterpret_cast<unsigned long long>(this) == (0x00007fffffffffffull & next))
					__debugbreak();
				else if (1 != (0x1ULL & next))
					__debugbreak();

				unsigned long long tail = _tail;
				if (tail == head)
					_InterlockedCompareExchange(reinterpret_cast<unsigned long long volatile*>(&_tail), next, tail);

				library::storage<type> result;
				result.relocate(reinterpret_cast<node*>(0x00007ffffffffffeull & next)->_value);
				_head = next;
				_pool::instance().deallocate(address);
				return std::move(result.get());
			}
			inline auto empty(void) const noexcept requires (false == multi_pop) {
				unsigned long long head = _head;
				auto address = reinterpret_cast<node*>(0x00007ffffffffffeull & head);
				unsigned long long next = address->_next;

				if (reinterpret_cast<unsigned long long>(this) == (0x00007fffffffffffull & next))
					return true;
				return false;
			}
			inline auto begin(void) noexcept -> iterator requires(false == multi_pop) {
				return iterator(reinterpret_cast<node*>(0x00007ffffffffffeull & reinterpret_cast<node*>(0x00007ffffffffffeull & _head)->_next));
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
}
