#pragma once
#include "../memory.h"
#include "../function.h"
#include <memory>
#include <cassert>

namespace library {
	template<typename type, size_t _size>
	class array {
	protected:
		using size_type = unsigned int;
		using iterator = type*;
	public:
		type _array[_size];

		inline auto operator[](size_type const index) const noexcept -> type const& {
			assert(index < _size && "index out of range");
			return _array[index];
		}
		inline auto operator[](size_type const index) noexcept ->type& {
			return library::cast<type&>(library::cast<array const&>(*this).operator[](index));
		}
		inline auto front(void) const noexcept ->type const& {
			return _array[0];
		}
		inline auto front(void) noexcept ->type& {
			return library::cast<type&>(library::cast<array const&>(*this).front());
		}
		inline auto back(void) const noexcept ->type const& {
			return _array[_size - 1];
		}
		inline auto back(void) noexcept ->type& {
			return library::cast<type&>(library::cast<array const&>(*this).back());
		}
		inline auto begin(void) noexcept -> iterator {
			return _array;
		}
		inline auto end(void) noexcept -> iterator {
			return _array + _size;
		}
		inline void fill(type const& value) noexcept {
			for (auto& iter : _array)
				iter = value;
		}
		inline auto size(void) const noexcept -> size_type {
			return _size;
		}
		inline auto data(void) const noexcept -> type const* {
			return _array;
		}
		inline auto data(void) noexcept -> type* {
			return library::cast<type*>(library::cast<array const&>(*this).data());
		}
	};
	template<typename type>
	class array<type, 0> {
	protected:
		using size_type = unsigned int;
		using iterator = type*;
		size_type _size;
		type* _array;
	public:
		template<typename... argument>
		inline array(size_type const size, argument&&... arg) noexcept
			: _size(size), _array(library::allocate<type>(size)) {
			for (auto index = 0u; index < size; ++index)
				library::construct(_array[index], arg...);
		};
		inline array(array const& rhs) noexcept
			: _size(rhs._size), _array(library::allocate<type>(_size)) {
			for (auto index = 0u; index < _size; ++index)
				library::construct(_array[index], rhs._array[index]);
		};
		inline array(array&& rhs) noexcept
			:_size(library::exchange(rhs._size, 0)), _array(library::exchange(rhs._array, nullptr)) {
		};
		inline auto operator=(array const& rhs) noexcept -> array& {
			assert(this != &rhs && "self-assignment");
			for (auto index = 0u; index < _size; ++index)
				library::destruct(_array[index]);
			library::deallocate<type>(_array);

			_size = rhs._size;
			_array = library::allocate<type>(_size);
			for (auto index = 0u; index < _size; ++index)
				library::construct(_array[index], rhs._array[index]);
			return *this;
		};
		inline auto operator=(array&& rhs) noexcept -> array& {
			assert(this != &rhs && "self-assignment");
			for (auto index = 0u; index < _size; ++index)
				library::destruct(_array[index]);
			library::deallocate<type>(_array);

			_size = library::exchange(rhs._size, 0);
			_array = library::exchange(rhs._array, nullptr);
			return *this;
		};
		inline ~array(void) noexcept {
			for (auto index = 0u; index < _size; ++index)
				library::destruct(_array[index]);
			library::deallocate<type>(_array);
		};

		inline auto operator[](size_type const index) const noexcept ->type const& {
			assert(index < _size && "index out of range");
			return _array[index];
		}
		inline auto operator[](size_type const index) noexcept ->type& {
			return library::cast<type&>(library::cast<array const&>(*this).operator[](index));
		}
		[[nodiscard]] inline auto front(void) const noexcept ->type& {
			return _array[0];
		}
		[[nodiscard]] inline auto back(void) const noexcept ->type& {
			return _array[_size - 1];
		}
		inline auto begin(void) const noexcept -> iterator {
			return _array;
		}
		inline auto end(void) const noexcept -> iterator {
			return _array + _size;
		}
		inline void fill(type const& value) noexcept {
			for (auto& iter : _array)
				iter = value;
		}
		inline void swap(array& rhs) noexcept {
			library::swap(_size, rhs._size);
			library::swap(_array, rhs._array);
		}
		[[nodiscard]] inline auto size(void) const noexcept -> size_type {
			return _size;
		}
		inline auto data(void) const noexcept -> type* {
			return _array;
		}
	};
}