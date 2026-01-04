#pragma once
#include "../memory.h"
#include "../function.h"
#include "../template.h"
#include <memory>
#include <cassert>

namespace detail {
	template <size_t _capacity = 128>
	class serialize_buffer {
	protected:
		using byte = unsigned char;
		using size_type = unsigned int;
		size_type _front;
		size_type _rear;
		byte _array[_capacity];
	public:
		inline serialize_buffer(void) noexcept
			: _front(0), _rear(0) {
		};
		inline serialize_buffer(serialize_buffer const&) noexcept = default;
		inline serialize_buffer(serialize_buffer&&) noexcept = default;
		inline auto operator=(serialize_buffer const&) noexcept -> serialize_buffer & = default;
		inline auto operator=(serialize_buffer&&) noexcept -> serialize_buffer & = default;
		inline ~serialize_buffer(void) noexcept = default;

		template<typename type>
			requires library::arithmetic_type<type>
		inline auto operator<<(type const value) noexcept -> serialize_buffer& {
			assert(sizeof(type) + _rear <= _capacity && "not enough capacity");
			reinterpret_cast<type&>(_array[_rear]) = value;
			_rear += sizeof(type);
			return *this;
		}
		inline void push(byte* const buffer, size_type const length) noexcept {
			assert(length + _rear <= _capacity && "not enough capacity");
			library::memory_copy(_array + _rear, buffer, length);
			_rear += length;
		}
		inline auto capacity(void) const noexcept -> size_type {
			return _capacity;
		}
	};
	template<>
	class serialize_buffer<0> {
	protected:
		using byte = unsigned char;
		using size_type = unsigned int;
		size_type _front;
		size_type _rear;
		size_type _capacity;
		byte* _array;
	public:
		inline serialize_buffer(void) noexcept
			: _front(0), _rear(0), _capacity(0), _array(nullptr) {
		};
		inline serialize_buffer(serialize_buffer const& rhs) noexcept
			: _front(rhs._front), _rear(rhs._rear), _capacity(rhs._capacity), _array(reinterpret_cast<byte*>(library::allocate(_capacity))) {
			library::memory_copy(_array + _front, rhs._array + _front, _rear - _front);
		}
		inline serialize_buffer(serialize_buffer&& rhs) noexcept
			: _front(library::exchange(rhs._front, 0)), _rear(library::exchange(rhs._rear, 0)), _capacity(library::exchange(rhs._capacity, 0)), _array(library::exchange(rhs._array, nullptr)) {
		}
		inline auto operator=(serialize_buffer const& rhs) noexcept -> serialize_buffer&;
		inline auto operator=(serialize_buffer&& rhs) noexcept -> serialize_buffer&;
		inline ~serialize_buffer(void) noexcept {
			library::deallocate<byte>(_array);
		};

		template<typename type>
			requires library::arithmetic_type<type>
		inline auto operator<<(type const value) noexcept -> serialize_buffer& {
			//if (sizeof(type) + _rear > _capacity) {
			//	reserve(library::maximum(static_cast<size_type>(_capacity * 1.5f), _size + 1));
			//}
			reinterpret_cast<type&>(_array[_rear]) = value;
			_rear += sizeof(type);
			return *this;
		}
		inline void push(byte* const buffer, size_type const length) noexcept {
			library::memory_copy(_array + _rear, buffer, length);
			_rear += length;
		}
		inline void reserve(size_type const& capacity) noexcept {
			if (_capacity < capacity) {
#pragma warning(suppress: 6308)
				_array = reinterpret_cast<byte*>(realloc(_array, capacity));
				_capacity = capacity;
			}
		}
		inline auto capacity(void) const noexcept -> size_type {
			return _capacity;
		}
	};
}

namespace library {
	template<size_t _capacity = 128>
	class serialize_buffer : public detail::serialize_buffer<_capacity> {
		using base = detail::serialize_buffer<_capacity>;
		using base::size_type;
		using base::byte;
		using iterator = byte*;
		using base::_front;
		using base::_rear;
		using base::_array;
	public:
		template<typename type>
			requires library::arithmetic_type<type>
		inline auto operator>>(type& value) noexcept -> serialize_buffer& {
			assert(sizeof(type) + _front <= _rear && "not enough data");
			value = reinterpret_cast<type&>(_array[_front]);
			_front += sizeof(type);
			return *this;
		}
		inline void peek(byte* const buffer, size_type const length) const noexcept {
			assert(length + _front <= _rear && "not enough data");
			library::memory_copy(buffer, _array + _front, length);
		}
		inline void pop(size_type const length) noexcept {
			assert(length + _front <= _rear && "not enough data");
			_front += length;
		}
		inline auto begin(void) noexcept -> iterator {
			return _array + _front;
		}
		inline auto end(void) noexcept -> iterator {
			return _array + _rear;
		}
		inline void clear(void) noexcept {
			_front = _rear = 0;
		}
		inline auto size(void) const noexcept -> size_type {
			return _rear - _front;
		}
		inline auto front(void) const noexcept -> size_type {
			return _front;
		}
		inline auto rear(void) const noexcept -> size_type {
			return _rear;
		}
		inline void move_front(size_type const length) noexcept {
			_front += length;
		}
		inline void move_rear(size_type const length) noexcept {
			_rear += length;
		}
		inline auto data(void) const noexcept -> byte* {
			return _array;
		}
	};

	class serialize_view {
	protected:
		using byte = unsigned char;
		using size_type = unsigned int;
		using iterator = byte*;

		size_type _front;
		size_type _rear;
		size_type _capacity;
		byte* _array;
	public:
		inline serialize_view(byte* buffer, size_type capacity) noexcept
			: _array(buffer), _front(0), _rear(0), _capacity(capacity) {
		};
		inline serialize_view(byte* buffer, size_type front, size_type rear, size_type capacity) noexcept
			: _array(buffer), _front(front), _rear(rear), _capacity(capacity) {
		};
		inline serialize_view(serialize_view const& rhs) noexcept
			: _array(rhs._array), _front(rhs._front), _rear(rhs._rear), _capacity(rhs._capacity) {
		}
		inline serialize_view(serialize_view&& rhs) noexcept
			: _array(library::exchange(rhs._array, nullptr)), _front(library::exchange(rhs._front, 0)), _rear(library::exchange(rhs._rear, 0)), _capacity(library::exchange(rhs._capacity, 0)) {
		}
		inline auto operator=(serialize_view const& rhs) noexcept -> serialize_view& {
			_array = rhs._array;
			_front = rhs._front;
			_rear = rhs._rear;
			_capacity = rhs._capacity;
			return *this;
		}
		inline auto operator=(serialize_view&& rhs) noexcept -> serialize_view& {
			_array = library::exchange(rhs._array, nullptr);
			_front = library::exchange(rhs._front, 0);
			_rear = library::exchange(rhs._rear, 0);
			_capacity = library::exchange(rhs._capacity, 0);
			return *this;
		}
		inline ~serialize_view(void) noexcept = default;

		template<typename type>
			requires library::arithmetic_type<type>
		inline auto operator<<(type const& value) noexcept -> serialize_view& requires std::is_arithmetic_v<type> {
			assert(sizeof(type) + _rear <= _capacity && "not enough capacity");
			reinterpret_cast<type&>(_array[_rear]) = value;
			_rear += sizeof(type);
			return *this;
		}
		inline void push(byte* const buffer, size_type const length) noexcept {
			assert(length + _rear <= _capacity && "not enough capacity");
			library::memory_copy(_array + _rear, buffer, length);
			_rear += length;
		}
		template<typename type>
			requires library::arithmetic_type<type>
		inline auto operator>>(type& value) noexcept -> serialize_view& requires std::is_arithmetic_v<type> {
			assert(sizeof(type) + _front <= _rear && "not enough data");
			value = reinterpret_cast<type&>(_array[_front]);
			_front += sizeof(type);
			return *this;
		}
		inline void peek(byte* const buffer, size_type const length) const noexcept {
			assert(length + _front <= _rear && "not enough data");
			library::memory_copy(buffer, _array + _front, length);
		}
		inline void pop(size_type const length) noexcept {
			assert(length + _front <= _rear && "not enough data");
			_front += length;
		}
		inline auto begin(void) noexcept -> iterator {
			return _array + _front;
		}
		inline auto end(void) noexcept -> iterator {
			return _array + _rear;
		}
		inline void clear(void) noexcept {
			_front = _rear = 0;
		}
		inline auto size(void) const noexcept -> size_type {
			return _rear - _front;
		}
		inline auto remain(void) const noexcept -> size_type {
			return _capacity - _rear;
		}
		inline auto capacity(void) const noexcept -> size_type {
			return _capacity;
		}
		inline auto front(void) const noexcept -> size_type {
			return _front;
		}
		inline auto rear(void) const noexcept -> size_type {
			return _rear;
		}
		inline bool empty(void) const noexcept {
			return _front == _rear;
		}
		inline void move_front(size_type const length) noexcept {
			_front += length;
		}
		inline void move_rear(size_type const length) noexcept {
			_rear += length;
		}
		inline auto data(void) noexcept -> byte* {
			return _array;
		}
	};
}

//template<typename... argument>
//inline void test(argument&&... arg) noexcept {
//	((reinterpret_cast<argument&>(_array[_rear]) = std::forward<argument>(arg), _rear += sizeof(argument)), ...);
//}

//template<typename type>
//inline auto push(std::string const& value) noexcept -> size_type {
//	operator<<(static_cast<type>(value.size()));
//	return push((unsigned char*)value.c_str(), int(sizeof(std::string::value_type) * value.size()));
//}