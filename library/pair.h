#pragma once
#include <utility>

namespace detail {
    template<typename type_1, typename type_2, bool = std::is_same_v<type_1, type_2>, bool = std::is_empty_v<type_1> && !std::is_final_v<type_1>, bool = std::is_empty_v<type_2> && !std::is_final_v<type_2>>
    struct compress_pair {
    };
    template <typename type_1, typename type_2, bool same>
    struct compress_pair<type_1, type_2, same, true, false> : private type_1 {
        type_2 _second;
        inline auto first(void) noexcept -> type_1& {
            return *this;
        }
        inline auto second(void) noexcept -> type_2& {
            return _second;
        }
    };
    template <typename type_1, typename type_2, bool same>
    struct compress_pair<type_1, type_2, same, false, true> : private type_2 {
        type_1 _first;
        inline auto first(void) noexcept -> type_1& {
            return _first;
        }
        inline auto second(void) noexcept -> type_2& {
            return *this;
        }
    };
    template <typename type_1, typename type_2, bool same>
    struct compress_pair<type_1, type_2, same, false, false> {
        type_1 _first;
        type_2 _second;
        inline auto first(void) noexcept -> type_1& {
            return _first;
        }
        inline auto second(void) noexcept -> type_2& {
            return _second;
        }
    };
    template<typename type_1, typename type_2>
    struct compress_pair<type_1, type_2, true, true, true> : private type_1 {
        type_2 _second;
        inline auto first(void) noexcept -> type_1& {
            return *this;
        }
        inline auto second(void) noexcept -> type_2& {
            return _second;
        }
    };
    template<typename type_1, typename type_2>
    struct compress_pair<type_1, type_2, false, true, true> : private type_1, private type_2 {
        inline auto first(void) noexcept -> type_1& {
            return *this;
        }
        inline auto second(void) noexcept -> type_2& {
            return *this;
        }
    };
}

namespace library {
	struct piecewise_construct {
		inline explicit piecewise_construct(void) noexcept = default;
	};
	inline constexpr piecewise_construct _piecewise_construct{};

	template<typename... type>
	class tuple;
	template <typename type_1, typename type_2>
	struct pair final {
		type_1 _first;
		type_2 _second;

		inline constexpr pair(void) noexcept = default;
		template <typename type_1, typename type_2>
		inline constexpr pair(type_1&& first, type_2&& second) noexcept
			: _first(std::forward<type_1>(first)), _second(std::forward<type_2>(second)) {
		}
		template <class... type_1, class... type_2>
		inline constexpr pair(piecewise_construct, tuple<type_1...> first, tuple<type_2...> second) noexcept
			: pair(first, second, std::index_sequence_for<type_1...>{}, std::index_sequence_for<type_2...>{}) {
		}
		template <class tuple_1, class tuple_2, size_t... index_1, size_t... index_2>
		inline constexpr pair(tuple_1& first, tuple_2& second, std::index_sequence<index_1...>, std::index_sequence<index_2...>)
			: _first(first.template move<index_1>()...), _second(second.template move<index_2>()...) {
		}
	};

    template<typename type_1, typename type_2>
    struct compress_pair final : public detail::compress_pair<type_1, type_2> {
    };
}