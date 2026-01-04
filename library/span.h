#pragma once
#include <utility>

namespace detail {
	template <typename type, size_t size>
	class span {
	protected:
		type* _pointer;
		inline static constexpr size_t _size = size;
	public:
		inline constexpr span(void) noexcept = default;
		inline constexpr span(type(&pointer)[size]) noexcept
			: _pointer(pointer) {
		}
		inline constexpr span(type* pointer, size_t const) noexcept
			: _pointer(pointer) {
		}
		template<typename container, size_t container_size>
			requires(size == container_size)
		inline constexpr span(container& arg)
			: _pointer(arg.data()) {
		}
	};

	template <typename type>
	struct span<type, 0xffffffffffffffffull> {
	protected:
		type* _pointer;
		size_t _size;
	public:
		inline constexpr span(void) noexcept
			: _pointer(nullptr), _size(0) {
		};
		inline constexpr span(type* pointer, size_t const size) noexcept
			: _pointer(pointer), _size(size) {
		}
		template<typename container>
		inline constexpr span(container& arg)
			: _pointer(arg.data()), _size(arg.size()) {
		}
	};
}

namespace library {
	template <typename type, size_t size = 0xffffffffffffffffull>
	class span : public detail::span<type, size> {
		using base = detail::span<type, size>;
		using size_type = unsigned int;
		using iterator = type*;
	public:
		using base::base;

		inline auto operator[](size_type const index) const noexcept ->type& {
			return base::_pointer[index];
		}
		inline auto begin(void) noexcept -> iterator {
			return base::_pointer;
		}
		inline auto end(void) noexcept -> iterator {
			return base::_pointer + base::_size;
		}
		inline constexpr auto size(void) const noexcept {
			return base::_size;
		}
		inline auto data(void) noexcept -> type* {
			return base::_pointer;
		}
	};
}