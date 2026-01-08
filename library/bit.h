#pragma once
//bit_set
#include "memory.h"
#include "array.h"
#include <type_traits>

//bit_grid
#include "vector.h"
#include "pair.h"
#include <memory>

namespace library {
	template<typename type>
	inline constexpr auto bit_ceil(type const number) noexcept -> type {
		type result = 1;
		while (result < number)
			result <<= 1;
		return result;
	}
	template<typename type>
	inline constexpr auto bit_floor(type const number) noexcept -> type {
		type result = 1;
		while ((result << 1) <= number)
			result <<= 1;
		return result;
	}
	template<typename type>
		requires library::any_of_type<type, unsigned int, int, unsigned long, long, unsigned long long, long long>
	inline constexpr auto bit_scan_forward(type const mask) noexcept -> unsigned long {
		unsigned long index;
		if constexpr (4 == sizeof(type))
			::_BitScanForward(&index, mask);
		else
			::_BitScanForward64(&index, mask);
		return index;
	}
	template<typename type>
		requires library::any_of_type<type, unsigned int, int, unsigned long, long, unsigned long long, long long>
	inline constexpr auto bit_scan_reverse(type const mask) noexcept -> unsigned long {
		unsigned long index;
		if constexpr (4 == sizeof(type))
			::_BitScanReverse(&index, mask);
		else
			::_BitScanReverse64(&index, mask);
		return index;
	}
	template<typename type>
		requires library::any_of_type<type, unsigned int, int, unsigned long, long, unsigned long long, long long>
	inline constexpr auto bit_mask_forward(type const mask) noexcept -> unsigned long {
		return 1 << bit_scan_forward(mask);
	}
	template<typename type>
		requires library::any_of_type<type, unsigned int, int, unsigned long, long, unsigned long long, long long>
	inline constexpr auto bit_mask_reverse(type const mask) noexcept -> unsigned long {
		return 1 << bit_scan_reverse(mask);
	}
}

namespace detail {
	template<size_t _size>
	class bit_set {
	protected:
		using size_type = unsigned int;
		using type = std::conditional_t<_size <= sizeof(unsigned long) * 8, unsigned long, unsigned long long>;
		inline static constexpr size_type _bit = sizeof(type) * 8;
		library::array<type, (_size - 1) / _bit + 1> _array;
	public:
		inline bit_set(void) noexcept
			: _array{} {
		};
		inline bit_set(bit_set const&) noexcept = default;
		inline bit_set(bit_set&&) noexcept = default;
		inline auto operator=(bit_set const&) noexcept -> bit_set & = default;
		inline auto operator=(bit_set&&) noexcept -> bit_set & = default;
		inline ~bit_set(void) noexcept = default;

		inline constexpr auto size(void) const noexcept -> size_type {
			return _size;
		}
	};

	template<>
	class bit_set<0> {
	protected:
		using size_type = unsigned int;
		using type = unsigned long long;
		inline static constexpr size_type _bit = sizeof(type) * 8;
		library::array<type, 0> _array;
		size_type _size;
	public:
		inline bit_set(size_type const size) noexcept
			: _size(size), _array((size - 1) / _bit + 1, 0) {
		};
		inline bit_set(bit_set const&) noexcept = default;
		inline bit_set(bit_set&&) noexcept = default;
		inline auto operator=(bit_set const&) noexcept -> bit_set & = default;
		inline auto operator=(bit_set&&) noexcept -> bit_set & = default;
		inline ~bit_set(void) noexcept = default;

		inline auto size(void) const noexcept -> size_type {
			return _size;
		}
	};
}

namespace library {
	template<size_t size>
	class bit_set : public detail::bit_set<size> {
		using base = detail::bit_set<size>;
		using base::type;
		using base::size_type;
		using base::_array;
		using base::_bit;
	public:
		using base::base;

		inline void set(size_type const position, bool const value) noexcept {
			if (false == value)
				_array[position / _bit] &= ~(static_cast<type>(1) << position % _bit);
			else
				_array[position / _bit] |= static_cast<type>(1) << position % _bit;
		}
		inline void flip(size_type const position) noexcept {
			_array[position / _bit] ^= static_cast<type>(1) << position % _bit;
		}
		inline void reset(void) noexcept {
			library::memory_set(_array.data(), 0, sizeof(type) * _array.size());
		}
		inline auto test(size_type const position) const noexcept -> bool {
			return 0 != (_array[position / _bit] & static_cast<type>(1) << position % _bit);
		}
		inline auto operator==(bit_set const& rhs) const noexcept -> bool {
			return library::memory_compare<type>(_array.data(), rhs._array.data(), _array.size());
		}
		inline auto operator&=(bit_set const& rhs) noexcept -> bit_set& {
			for (auto index = 0u; index < _array.size(); ++index)
				_array[index] &= rhs._array[index];
			return *this;
		}
		inline auto operator|=(bit_set const& rhs) noexcept -> bit_set& {
			for (auto index = 0u; index < _array.size(); ++index)
				_array[index] |= rhs._array[index];
			return *this;
		}
		inline auto operator^=(bit_set const& rhs) noexcept  -> bit_set& {
			for (auto index = 0u; index < _array.size(); ++index)
				_array[index] ^= rhs._array[index];
			return *this;
		}
		inline auto operator&(bit_set const& rhs) const noexcept -> bit_set {
			bit_set result(*this);
			result &= rhs;
			return result;
		}
		inline auto operator|(bit_set const& rhs) const noexcept -> bit_set {
			bit_set result(*this);
			result |= rhs;
			return result;
		}
		inline auto operator^(bit_set const& rhs) const noexcept -> bit_set {
			bit_set result(*this);
			result ^= rhs;
			return result;
		}
		inline auto operator~(void) const noexcept -> bit_set {
			bit_set result(*this);
			for (auto index = 0u; index < _array.size(); ++index)
				result._array[index] = ~result._array[index];

			auto extra_bit = base::size() % _bit;
			if (0 != extra_bit)
				result._array[_array.size() - 1] &= (static_cast<type>(1) << extra_bit) - 1;
			return result;
		}
		inline explicit operator bool(void) const noexcept {
			for (size_type index = 0; index < _array.size(); ++index)
				if (_array[index] != 0)
					return true;
			return false;
		}
	};

	class bit_grid final {
	private:
		using size_type = unsigned int;
		using div_type = div_t;
	public:
		inline explicit bit_grid(void) noexcept
			: _width(0), _height(0) {
		}
		inline explicit bit_grid(size_type const width, size_type const height) noexcept {
			set_size(width, height);
		}
		inline explicit bit_grid(bit_grid const& rhs) noexcept;
		inline explicit bit_grid(bit_grid&& rhs) noexcept;
		inline auto operator=(bit_grid const& rhs) noexcept -> bit_grid&;
		inline auto operator=(bit_grid&& rhs) noexcept -> bit_grid&;
		inline ~bit_grid(void) noexcept = default;

		inline void set_size(size_type const width, size_type const height) noexcept {
			_width = width;
			_height = height;
			div_type _div = ::div(width * height, 64);
			if (_div.rem)
				++_div.quot;
			_vector.assign(_div.quot, 0);
		}
		inline void set_bit(size_type const x, size_type const y, bool const flag) noexcept {
			div_type _div = get_div(x, y);
			switch (flag) {
			case 0:
				_vector[_div.quot] &= ~(static_cast<unsigned long long>(1) << _div.rem);
				break;
			case 1:
				_vector[_div.quot] |= static_cast<unsigned long long>(1) << _div.rem;
				break;
			}
		}
		inline bool get_bit(size_type const x, size_type const y) const noexcept {
			div_type _div = get_div(x, y);
			return _vector[_div.quot] & static_cast<unsigned long long>(1) << _div.rem;
		}
		//inline auto get_word(size_type const x, size_type const y) const noexcept -> type {
		//	div_type _div = get_div(x, y);
		//	return _vector[_div.quot];
		//}
		//inline auto get_word(size_type const index) const noexcept -> type {
		//	return _vector[index];
		//}
		//inline auto get_pos(div_type const& _div) const noexcept -> pair<size_type, size_type> {
		//	size_type index = _div.quot * _bit + _div.rem;
		//	return pair<size_type, size_type>{index% _width, index / _width};
		//}
		inline auto get_width(void) const noexcept -> size_type {
			return _width;
		}
		inline auto get_height(void) const noexcept -> size_type {
			return _height;
		}

		inline void clear(void) noexcept {
			memset(_vector.data(), 0, sizeof(unsigned long long) * _vector.capacity());
		}
		inline bool in_bound(size_type const x, size_type const y) const noexcept {
			if (x >= _width || y >= _height)
				return false;
			return true;
		}
	private:
		inline auto get_div(size_type const x, size_type const y) const noexcept -> div_type {
			size_type index = x + _width * y;
			return ::div(index, 64);
		}

		size_type _width, _height;
		vector<unsigned long long> _vector;
	};
}