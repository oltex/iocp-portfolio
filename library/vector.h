#pragma once
#include "memory.h"
#include "function.h"
#include "template.h"
#include <initializer_list>
#include <utility>
#include <stdlib.h>
#include <cassert>

namespace library {
	template<typename type>
	class vector {
	protected:
		using size_type = unsigned int;
		size_type _size;
		size_type _capacity;
		type* _array;
	public:
		using iterator = type*;

		inline vector(void) noexcept
			: _size(0), _capacity(0), _array(nullptr) {
		};
		inline vector(std::initializer_list<type> const& list) noexcept
			: vector() {
			reserve(static_cast<size_type>(list.size()));
			for (auto& iter : list)
				emplace_back(iter);
		}
		inline vector(iterator const& begin, iterator const& end) noexcept
			: vector() {
			auto size = static_cast<size_type>(end - begin);
			reserve(size);
			if constexpr (library::memory_copy_safe<type>) {
				library::memory_copy(_array, begin, size);
				_size = size;
			}
			else {
				for (auto iter = begin; iter != end; ++iter)
					emplace_back(*iter);
			}
		}
		inline vector(vector const& rhs) noexcept
			: vector(rhs.begin(), rhs.end()) {
		}
		inline vector(vector&& rhs) noexcept
			: _size(library::exchange(rhs._size, 0)), _capacity(library::exchange(rhs._capacity, 0)), _array(library::exchange(rhs._array, nullptr)) {
		}
		inline auto operator=(vector const& rhs) noexcept -> vector& {
			assert(this != &rhs && "self-assignment");
			vector(rhs).swap(*this);
			return *this;
		};
		inline auto operator=(vector&& rhs) noexcept -> vector& {
			assert(this != &rhs && "self-assignment");
			vector(std::move(rhs)).swap(*this);
			return *this;
		}
		inline ~vector(void) noexcept {
			clear();
			library::deallocate<type>(_array);
		}

		template<typename... argument>
		inline auto emplace(iterator iter, argument&&... arg) noexcept -> iterator {
			if (_size >= _capacity) {
				auto index = iter - begin();
				reserve(library::maximum(static_cast<size_type>(_capacity * 1.5f), _size + 1));
				iter = begin() + index;
			}
			if constexpr (library::memory_move_safe<type>)
				library::memory_move(iter + 1, iter, end() - iter);
			else {
				auto start = end();
				for (; start != iter; --start)
					library::construct<type>(*start, std::move(*(start - 1)));
			}
			library::construct(*iter, std::forward<argument>(arg)...);
			++_size;
			return iter;
		}
		template<typename... argument>
		inline auto emplace_back(argument&&... arg) noexcept -> type& {
			return *emplace(end(), std::forward<argument>(arg)...);
		}
		inline auto erase(iterator iter) noexcept -> iterator {
			assert(_size > 0 && "called on empty");

			library::destruct<type>(*iter);
			if constexpr (library::memory_move_safe<type>)
				library::memory_move(iter, iter + 1, end() - (iter + 1));
			else {
				auto start = iter;
				for (; start != end() - 1; ++start)
					library::construct<type>(*start, std::move(*(start + 1)));
			}
			--_size;
			return iter;
		}
		inline void pop_back(void) noexcept {
			erase(end() - 1);
		}
		inline auto begin(void) const noexcept -> iterator {
			return _array;
		}
		inline auto end(void) const noexcept -> iterator {
			return _array + _size;
		}
		inline auto front(void) const noexcept ->type& {
			assert(_size > 0 && "called on empty");
			return _array[0];
		}
		inline auto back(void) const noexcept ->type& {
			assert(_size > 0 && "called on empty");
			return _array[_size - 1];
		}
		inline auto operator[](size_type const index) const noexcept ->type& {
			assert(index < _size && "index out of range");
			return _array[index];
		}
		inline void reserve(size_type const capacity) noexcept {
			if (_capacity < capacity) {
				if constexpr (library::memory_move_safe<type>)
#pragma warning(suppress: 6308)
					_array = library::reallocate<type>(_array, capacity);
				else {
					auto array = library::allocate<type>(capacity);
					for (auto index = 0u; index < _size; ++index) {
						library::construct<type>(array[index], std::move(_array[index]));
						library::destruct<type>(_array[index]);
					}
					library::deallocate<type>(library::exchange(_array, array));
				}
				_capacity = capacity;
			}
		}
		template<typename... argument>
		inline void resize(size_type const size, argument&&... arg) noexcept {
			if (size > _capacity)
				reserve(size);
			while (size > _size)
				emplace_back(arg...);
			while (size < _size)
				pop_back();
		}
		template<typename... argument>
		inline void assign(size_type const size, argument&&... arg) noexcept {
			// NOTE: This simple clear()+resize() implementation can be optimized by reusing existing elements (assigning to the first min(size, _size) and only popping/pushing the tail).
			clear();
			resize(size, std::forward<argument>(arg)...);
		}
		inline void swap(vector& rhs) noexcept {
			library::swap(_size, rhs._size);
			library::swap(_capacity, rhs._capacity);
			library::swap(_array, rhs._array);
		}
		inline void clear(void) noexcept {
			if constexpr (!std::is_trivially_destructible_v<type>)
				while (0 != _size)
					pop_back();
			else
				_size = 0;
		}
		inline bool empty(void) const noexcept {
			return 0 == _size;
		}
		inline auto size(void) const noexcept -> size_type {
			return _size;
		}
		inline auto capacity(void) const noexcept -> size_type {
			return _capacity;
		}
		template<typename predicate>
		inline auto find(predicate&& pred) noexcept -> iterator {
			for (auto iter = begin(); iter != end(); ++iter)
				if (pred(*iter))
					return iter;
			return end();
		}
		inline auto data(void) noexcept -> type* {
			return _array;
		}
	};
}