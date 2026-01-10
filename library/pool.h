#pragma once
#include "memory.h"
#include "function.h"

#include "singleton.h"
#include "interlock.h"

#include <memory>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace library {
	template<typename type>
	class allocator {
	public:
		inline constexpr allocator(void) noexcept = default;
		inline constexpr allocator(allocator const&) noexcept = default;
		template<typename other>
		inline constexpr allocator(allocator<other> const&) noexcept {
		};

		template <typename other>
		using rebind = allocator<other>;
		template<typename... argument>
		[[nodiscard]] inline auto allocate(argument&&... arg) noexcept -> type* {
			return library::create<type>(std::forward<argument>(arg)...);
		}
		[[nodiscard]] inline void deallocate(type* value) noexcept {
			library::destroy<type>(value);
		}
	};

	template<typename type, bool placement = true, bool compress = true>
	class pool {
		union union_node final {
			union_node* _next;
			type _value;
			inline explicit union_node(void) noexcept = delete;
			inline explicit union_node(union_node const&) noexcept = delete;
			inline explicit union_node(union_node&&) noexcept = delete;
			inline auto operator=(union_node const&) noexcept -> union_node & = delete;
			inline auto operator=(union_node&&) noexcept -> union_node & = delete;
			inline ~union_node(void) noexcept = delete;
		};
		struct strcut_node final {
			strcut_node* _next;
			type _value;
			inline explicit strcut_node(void) noexcept = delete;
			inline explicit strcut_node(strcut_node const&) noexcept = delete;
			inline explicit strcut_node(strcut_node&&) noexcept = delete;
			inline auto operator=(strcut_node const&) noexcept -> strcut_node & = delete;
			inline auto operator=(strcut_node&&) noexcept -> strcut_node & = delete;
			inline ~strcut_node(void) noexcept = delete;
		};
		using node = typename std::conditional<compress, union union_node, struct strcut_node>::type;
		node* _head;
	public:
		template <typename other, bool _placement = placement, bool _compress = compress>
		using rebind = pool<other, _placement, _compress>;
		inline pool(void) noexcept
			: _head(nullptr) {
		};
		inline pool(pool const&) noexcept = delete;
		inline pool(pool&& rhs) noexcept
			: _head(library::exchange(rhs._head, nullptr)) {
		};
		inline auto operator=(pool const&) noexcept = delete;
		inline auto operator=(pool&& rhs) noexcept -> pool& {
			while (nullptr != _head)
#pragma warning(suppress: 6001)
				library::deallocate<node>(library::exchange(_head, _head->_next));
			_head = library::exchange(rhs._head, nullptr);
			return *this;
		}
		inline ~pool(void) noexcept {
			while (nullptr != _head)
#pragma warning(suppress: 6001)
				library::deallocate<node>(library::exchange(_head, _head->_next));
		}

		template<typename... argument>
		[[nodiscard]] inline auto allocate(argument&&... arg) noexcept -> type* {
			node* current;
			if (nullptr == _head)
				current = library::allocate<node>();
			else
				current = library::exchange(_head, _head->_next);
			if constexpr (true == placement)
				library::construct<type, argument...>(current->_value, std::forward<argument>(arg)...);
			return &current->_value;
		}
		[[nodiscard]] inline void deallocate(type* const value) noexcept {
			if constexpr (true == placement)
				library::destruct<type>(*value);
			auto current = reinterpret_cast<node*>(reinterpret_cast<unsigned char*>(value) - offsetof(node, _value));
			current->_next = library::exchange(_head, current);
		}
	};

	namespace lockfree {
		template<typename type, bool placement = true, bool compress = true>
		class pool final {
			union union_node final {
				union_node* _next;
				type _value;
				inline explicit union_node(void) noexcept = delete;
				inline explicit union_node(union_node const&) noexcept = delete;
				inline explicit union_node(union_node&&) noexcept = delete;
				inline auto operator=(union_node const&) noexcept -> union_node & = delete;
				inline auto operator=(union_node&&) noexcept -> union_node & = delete;
				inline ~union_node(void) noexcept = delete;
			};
			struct strcut_node final {
				strcut_node* _next;
				type _value;
				inline explicit strcut_node(void) noexcept = delete;
				inline explicit strcut_node(strcut_node const&) noexcept = delete;
				inline explicit strcut_node(strcut_node&&) noexcept = delete;
				inline auto operator=(strcut_node const&) noexcept -> strcut_node & = delete;
				inline auto operator=(strcut_node&&) noexcept -> strcut_node & = delete;
				inline ~strcut_node(void) noexcept = delete;
			};
			using node = typename std::conditional<compress, union union_node, struct strcut_node>::type;
			alignas(64) unsigned long long _head;
		public:
			inline pool(void) noexcept
				: _head(0) {
			};
			inline pool(pool const&) noexcept = delete;
			inline pool(pool&& rhs) noexcept
				: _head(library::exchange(rhs._head, nullptr)) {
			};
			inline auto operator=(pool const&) noexcept = delete;
			inline auto operator=(pool&& rhs) noexcept -> pool& {
				node* head = reinterpret_cast<node*>(0x00007fffffffffffull & _head);
				while (nullptr != head)
					library::deallocate<node>(library::exchange(head, head->_next));
				_head = library::exchange(rhs._head, nullptr);
				return *this;
			}
			inline ~pool(void) noexcept {
				node* head = reinterpret_cast<node*>(0x00007fffffffffffull & _head);
				while (nullptr != head)
					library::deallocate<node>(library::exchange(head, head->_next));
			}

			template<typename... argument>
			[[nodiscard]] inline auto allocate(argument&&... arg) noexcept -> type* {
				node* current;
				for (unsigned long long head = _head, prev;; head = prev) {
					current = reinterpret_cast<node*>(0x00007fffffffffffull & head);
					if (nullptr == current) {
						current = library::allocate<node>();
						break;
					}
					unsigned long long next = reinterpret_cast<unsigned long long>(current->_next) + (0xffff800000000000ull & head);
					if (prev = _InterlockedCompareExchange(reinterpret_cast<unsigned long long volatile*>(&_head), next, head); prev == head)
						break;
				}
				if constexpr (true == placement)
					library::construct<type, argument...>(current->_value, std::forward<argument>(arg)...);
				return &current->_value;
			}
			[[nodiscard]] inline void deallocate(type* value) noexcept {
				if constexpr (true == placement)
					library::destruct<type>(*value);
				auto current = reinterpret_cast<node*>(reinterpret_cast<unsigned char*>(value) - offsetof(node, _value));
				for (unsigned long long head = _head, prev;; head = prev) {
					current->_next = reinterpret_cast<node*>(0x00007fffffffffffull & head);
					unsigned long long next = reinterpret_cast<unsigned long long>(current) + (0xffff800000000000ull & head) + 0x0000800000000000ull;
					if (prev = _InterlockedCompareExchange(reinterpret_cast<unsigned long long volatile*>(&_head), next, head); prev == head)
						break;
				}
			}
		};

		template<typename type, bool placement = true, bool compress = true>
		class fixed_pool {
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
				inline ~iterator(void) noexcept = default;

				[[nodiscard]] inline auto operator*(void) const noexcept -> type& {
					return _node->_value;
				}
				[[nodiscard]] inline auto operator->(void) const noexcept -> type* {
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
				[[nodiscard]] inline auto operator==(iterator const& rhs) const noexcept -> bool {
					return _node == rhs._node;
				}
			};

			inline fixed_pool(size_type const capacity)noexcept
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
			inline fixed_pool(fixed_pool const&) noexcept = delete;
			inline fixed_pool(fixed_pool&&) noexcept = delete;
			inline auto operator=(fixed_pool const&) noexcept -> fixed_pool & = delete;
			inline auto operator=(fixed_pool&&) noexcept -> fixed_pool & = delete;
			inline ~fixed_pool(void) noexcept {
				library::deallocate<node>(_array);
			};

			template<typename... argument>
			[[nodiscard]] inline auto allocate(argument&&... arg) noexcept -> type* {
				for (unsigned long long head = _head, prev;; head = prev) {
					node* current = reinterpret_cast<node*>(0x00007fffffffffffull & head);
					if (nullptr == current)
						return nullptr;
					auto next = reinterpret_cast<unsigned long long>(current->_next) + (0xffff800000000000ull & head) + 0x0000800000000000ull;
					if (prev = library::interlock_compare_exchange(_head, next, head); prev == head) {
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
					current->_next = reinterpret_cast<node*>(head & 0x00007fffffffffffull);
					unsigned long long next = reinterpret_cast<unsigned long long>(current) + (head & 0xffff800000000000ull);
					if (prev = library::interlock_compare_exchange(_head, next, head); prev == head)
						break;
				}
			}
			[[nodiscard]] inline auto begin(void) const noexcept -> iterator {
				return iterator(_array);
			}
			[[nodiscard]] inline auto end(void) const noexcept -> iterator {
				return iterator(_array + _capacity);
			}
			[[nodiscard]] inline auto operator[](size_type const index) noexcept -> type& {
				return _array[index]._value;
			}
			[[nodiscard]] inline auto capacity(void) const noexcept -> size_type {
				return _capacity;
			}
			[[nodiscard]] inline auto data(void) noexcept {
				return _array;
			}
#pragma region function
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
#pragma endregion
		};
	}

	namespace _thread_local {
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

				inline context(void) noexcept
					: _size(0), _head(nullptr), _break(nullptr) {
				};
				inline context(context const&) noexcept = delete;
				inline context(context&&) noexcept = delete;
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

			inline pool(void) noexcept = default;
			inline pool(pool const&) noexcept = delete;
			inline pool(pool&&) noexcept = delete;
			inline auto operator=(pool const&) noexcept -> pool & = delete;
			inline auto operator=(pool&&) noexcept -> pool & = delete;
			inline ~pool(void) noexcept {
				auto stack = _stack;
				while (nullptr != stack) {
					library::deallocate<node>(stack->_value);
					_pool.deallocate(library::exchange(stack, stack->_next));
				}

				auto head = reinterpret_cast<bucket*>(0x00007fffffffffffull & _head);
				while (nullptr != head)
					_pool.deallocate(library::exchange(head, head->_next));
			};

			[[nodiscard]] inline void deallocate(node* value, size_type const size) noexcept {
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
			[[nodiscard]] inline auto allocate(argument&&... arg) noexcept -> type* {
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
						unsigned long long next = reinterpret_cast<unsigned long long>(address->_next) + (0xffff800000000000ull & head) + 0x0000800000000000ull;
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
			[[nodiscard]] inline void deallocate(type* value) noexcept {
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

		template<typename type, size_t bucket_size = 1024, bool placement = true, bool compress = true>
		class allocator {
		public:
			template <typename other, size_t _bucket_size = bucket_size, bool _placement = placement, bool _compress = compress>
			using rebind = allocator<other, _bucket_size, _placement, _compress>;
			template<typename... argument>
			inline auto allocate(argument&&... arg) noexcept -> type* {
				return library::_thread_local::pool<type, bucket_size, placement, compress>::instance().allocate(std::forward<argument>(arg)...);
			}
			inline void deallocate(type* value) noexcept {
				library::_thread_local::pool<type, bucket_size, placement, compress>::instance().deallocate(value);
			}
		};
	}
}