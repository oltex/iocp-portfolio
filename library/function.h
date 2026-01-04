#pragma once
#include "template.h"
#include <utility>
#include <type_traits>
#include <cmath>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace library {
	template<typename type, typename size_type = unsigned int>
	struct fnv_hash {
		inline static constexpr size_type _offset_basis = sizeof(size_type) == 4 ? 2166136261U : 14695981039346656037ULL;
		inline static constexpr size_type _prime = sizeof(size_type) == 4 ? 16777619U : 1099511628211ULL;

		inline auto operator()(type const& key) const noexcept -> size_type {
			auto value = _offset_basis;
			auto byte = reinterpret_cast<unsigned char const*>(&key);
			for (size_type index = 0; index < sizeof(type); ++index) {
				value ^= static_cast<size_type>(byte[index]);
				value *= _prime;
			}
			return value;
		}
	};
	template <typename type, std::size_t size, typename size_type>
	struct fnv_hash<type[size], size_type> {
		inline static constexpr size_type _offset_basis = sizeof(size_type) == 4 ? 2166136261U : 14695981039346656037ULL;
		inline static constexpr size_type _prime = sizeof(size_type) == 4 ? 16777619U : 1099511628211ULL;

		inline auto operator()(type(&key)[size]) const noexcept -> size_type {
			auto value = _offset_basis;
			auto byte = reinterpret_cast<unsigned char const*>(key);
			for (size_t index = 0; index < sizeof(type) * size; ++index) {
				value ^= static_cast<unsigned char>(byte[index]);
				value *= _prime;
			}
			return value;
		}
	};

	template<typename type = void>
	struct less {
		inline auto operator()(type const& left, type const& right) const noexcept -> bool {
			return left < right;
		}
	};
	template<>
	struct less<void> {
		template <typename type_1, typename type_2>
		inline auto operator()(type_1&& left, type_2&& right) const noexcept -> bool {
			return static_cast<type_1&&>(left) < static_cast<type_2&&>(right);
		}
	};
	template<typename type = void>
	struct great {
		inline auto operator()(type const& left, type const& right) const noexcept -> bool {
			return left > right;
		}
	};
	template<>
	struct great<void> {
		template <typename type_1, typename type_2>
		inline auto operator()(type_1&& left, type_2&& right) const noexcept -> bool {
			return static_cast<type_1&&>(left) > static_cast<type_2&&>(right);
		}
	};
	template <typename type = void>
	struct equal {
		inline auto operator()(type const& left, type const& right) const noexcept -> bool {
			return left == right;
		}
	};
	template <>
	struct equal<void> {
		using transparent = int;
		template <typename type_1, typename type_2>
		inline auto operator()(type_1&& left, type_2&& right) const noexcept -> bool {
			return static_cast<type_1&&>(left) == static_cast<type_2&&>(right);
		}
	};
	template<typename type>
	inline constexpr auto ordering(type const& left, type const& right) noexcept {
		return left <=> right;
	}

	template <class type, class other = type>
	inline constexpr auto exchange(type& value, other&& new_value) noexcept -> type {
		type old_value = static_cast<type&&>(value);
		value = static_cast<other&&>(new_value);
		return old_value;
	}
	template<typename type>
	inline constexpr void swap(type& left, type& right) noexcept {
		type temp = std::move(left);
		left = std::move(right);
		right = std::move(temp);
	}

	template<typename type>
	inline constexpr auto maximum(type const& first, type const& second) noexcept -> type const& {
		if (first < second)
			return second;
		else
			return first;
	}
	template<typename type, typename... argument>
	inline constexpr auto maximum(type const& first, argument const&... second) noexcept -> type const& {
		return maximum(first, maximum(second...));
	}
	template<typename type>
	inline constexpr auto minimum(type const& first, type const& second) noexcept -> type const& {
		if (first > second)
			return second;
		else
			return first;
	}
	template<typename type, typename... argument>
	inline constexpr auto minimum(type const& first, argument const&... second) noexcept -> type const& {
		return maximum(first, maximum(second...));
	}

	template<typename type>
	inline constexpr auto absolute(type const value) noexcept {
		if constexpr (std::floating_point<type>)
			return std::signbit(value) ? -value : value;
		else
			return value < type(0) ? -value : value;
	}

	template <typename type>
	inline constexpr auto greatest_common_divisor(type first, type second) noexcept {
		while (second != 0) {
			type temp = second;
			second = first % second;
			first = temp;
		}
		return first;
	}
	template <typename type_1, typename... type_2>
	inline constexpr auto greatest_common_divisor(type_1 first, type_2... second) noexcept {
		return greatest_common_divisor(first, greatest_common_divisor(second...));
	}
	template <typename type>
	inline constexpr auto least_common_multiple(type first, type second) noexcept {
		return first * second / greatest_common_divisor(first, second);
	}
	template <typename type_1, typename... type_2>
	inline constexpr auto least_common_multiple(type_1 first, type_2... second) noexcept {
		return least_common_multiple(first, least_common_multiple(second...));
	}

	template <typename iterator, class _Diff>
	inline constexpr void advance(iterator& iter, _Diff offset) noexcept {
		if constexpr (std::_Is_ranges_random_iter_v<iterator>)
			iter += offset;
		else {
			for (; offset < 0; ++offset)
				--iter;
			for (; 0 < offset; --offset)
				++iter;
		}
	}
	template <typename iterator>
	inline constexpr auto next(iterator iter) noexcept -> iterator {
		library::advance(iter, 1);
		return iter;
	}
	template <typename iterator>
	inline constexpr auto prev(iterator iter) noexcept -> iterator {
		library::advance(iter, -1);
		return iter;
	}
}