#pragma once
#include "handle.h"
#include "../memory.h"
#include "../interlock.h"
#include "../lockfree/free_list.h"

namespace grc {
	template<typename type, bool inplace>
	class arena {
	public:
		using size_type = unsigned int;
		using handle = unsigned long long;
		class self {
			friend struct node;
			handle _key;
		public:
			auto arena_handle(void) noexcept -> handle {
				return _key;
			}
		};
		struct node {
			handle _key;
			unsigned long _reference;
			type* _value;
			void (*_deleter)(type*);
		public:
			template<typename derive>
			void attach(derive* pointer, void(*deleter)(type*)) noexcept {
				_value = library::cast<type*>(pointer);
				_deleter = deleter;
				_key += 0x10000ull;
				if constexpr (std::is_base_of_v<self, derive>) 
					pointer->_key = _key;
				library::interlock_increment(_reference);
				library::interlock_and(_reference, 0x7FFFFFFFul);
			}
			auto acquire(void) noexcept -> bool {
				return !(0x80000000ul & library::interlock_increment(_reference));
			}
			auto acquire(handle key) noexcept -> bool {
				return acquire() && _key == key;
			}
			auto release(void) noexcept -> bool {
				if (0 == library::interlock_decrement(_reference) && 0 == library::interlock_compare_exchange(_reference, 0x80000000ul, 0)) {
					_deleter(_value);
					return true;
				}
				return false;
			}
		};
		using iterator = library::lockfree::free_list<node, false, false>::iterator;
	protected:
		library::lockfree::free_list<node, false, false> _slot;
	public:
		arena(size_type const capacity) noexcept
			: _slot(capacity) {
			for (auto index = 0u; index < _slot.capacity(); ++index) {
				_slot[index]._key = 0xffff & static_cast<unsigned long long>(index);
				_slot[index]._reference = 0x80000000;
			}
		}
		arena(arena const&) noexcept = delete;
		arena(arena&&) noexcept = delete;
		auto operator=(arena const&) noexcept -> arena & = delete;
		auto operator=(arena&&) noexcept -> arena & = delete;
		~arena(void) noexcept = default;

		auto allocate(void) noexcept -> node* {
			return _slot.allocate();
		}
		auto get(handle key) noexcept -> node& {
			return _slot[0xffff & key];
		}
		auto deallocate(node* node) noexcept {
			_slot.deallocate(node);
		}
		auto begin(void) const noexcept -> iterator {
			return _slot.begin();
		}
		auto end(void) const noexcept -> iterator {
			return _slot.end();
		}
	};

	template<typename type>
	class arena<type, true> {
	public:
		using size_type = unsigned int;
		using handle = unsigned long long;
		struct node {
			handle _key;
			unsigned long _reference;
			type _value;
		public:
			template<typename... argument>
			void allocate(argument&&... arg) noexcept {
				library::construct<type>(_value, std::forward<argument>(arg)...);
				_key += 0x10000ull;
				library::interlock_increment(_reference);
				library::interlock_and(_reference, 0x7FFFFFFFul);
			}
			auto acquire(void) noexcept -> bool {
				return !(0x80000000ul & library::interlock_increment(_reference));
			}
			auto acquire(handle key) noexcept -> bool {
				return acquire() && _key == key;
			}
			auto release(void) noexcept -> bool {
				if (0 == library::interlock_decrement(_reference) && 0 == library::interlock_compare_exchange(_reference, 0x80000000ul, 0)) {
					library::destruct<type>(_value);
					return true;
				}
				return false;
			}
			inline static auto recover(type& value) noexcept -> node& {
				return *reinterpret_cast<node*>(reinterpret_cast<unsigned char*>(&value) - offsetof(node, _value));
			}
		};
		using iterator = library::lockfree::free_list<node, false, false>::iterator;
	protected:
		library::lockfree::free_list<node, false, false> _slot;
	public:
		arena(size_type const capacity) noexcept
			: _slot(capacity) {
			for (auto index = 0u; index < _slot.capacity(); ++index) {
				_slot[index]._key = 0xffff & static_cast<unsigned long long>(index);
				_slot[index]._reference = 0x80000000;
			}
		}
		arena(arena const&) noexcept = delete;
		arena(arena&&) noexcept = delete;
		auto operator=(arena const&) noexcept -> arena & = delete;
		auto operator=(arena&&) noexcept -> arena & = delete;
		~arena(void) noexcept = default;

		auto allocate(void) noexcept -> node* {
			return _slot.allocate();
		}
		auto get(handle key) noexcept -> node& {
			return _slot[0xffff & key];
		}
		auto deallocate(node* node) noexcept {
			_slot.deallocate(node);
		}
		auto begin(void) const noexcept -> iterator {
			return _slot.begin();
		}
		auto end(void) const noexcept -> iterator {
			return _slot.end();
		}
	};
}