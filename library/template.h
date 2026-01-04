#pragma once
#include <type_traits>

namespace detail {
	template <typename>
	inline constexpr bool const_type = false;
	template <typename type>
	inline constexpr bool const_type<type const> = true;
	template <typename _type>
	struct add_const {
		using type = _type const;
	};
	template <typename _type>
	struct remove_const {
		using type = _type;
	};
	template <typename _type>
	struct remove_const<_type const> {
		using type = _type;
	};
	template <typename type_1, typename type_2>
	struct copy_const {
		using type = std::conditional_t<const_type<type_1>, typename add_const<typename remove_const<type_2>::type>::type, typename remove_const<type_2>::type>;
	};

	template <typename>
	inline constexpr bool volatile_type = false;
	template <typename type>
	inline constexpr bool volatile_type<type volatile> = true;
	template <typename _type>
	struct add_volatile {
		using type = _type volatile;
	};
	template <typename _type>
	struct remove_volatile {
		using type = _type;
	};
	template <typename _type>
	struct remove_volatile<_type volatile> {
		using type = _type;
	};
	template <typename type_1, typename type_2>
	struct copy_volatile {
		using type = std::conditional_t<volatile_type<type_1>, typename add_volatile<typename remove_volatile<type_2>::type>::type, typename remove_volatile<type_2>::type>;
	};

	template <typename>
	inline constexpr bool pointer_type = false;
	template <typename type>
	inline constexpr bool pointer_type<type*> = true;
	template <typename type>
	inline constexpr bool pointer_type<type* const> = true;
	template <typename type>
	inline constexpr bool pointer_type<type* volatile> = true;
	template <typename type>
	inline constexpr bool pointer_type<type* const volatile> = true;
	template <typename _type>
	struct add_pointer {
		using type = _type*;
	};
	template <typename _type>
	struct remove_pointer {
		using type = _type;
	};
	template <typename _type>
	struct remove_pointer<_type*> {
		using type = _type;
	};
	template <typename _type>
	struct remove_pointer<_type* const> {
		using type = _type;
	};
	template <typename _type>
	struct remove_pointer<_type* volatile> {
		using type = _type;
	};
	template <typename _type>
	struct remove_pointer<_type* const volatile> {
		using type = _type;
	};

	template <typename>
	inline constexpr bool reference_type = false;
	template <typename type>
	inline constexpr bool reference_type<type&> = true;
	template <typename type>
	inline constexpr bool reference_type<type&&> = true;
	template <typename>
	inline constexpr bool lvalue_reference_type = false;
	template <typename type>
	inline constexpr bool lvalue_reference_type<type&> = true;
	template <typename>
	inline constexpr bool rvalue_reference_type = false;
	template <typename type>
	inline constexpr bool rvalue_reference_type<type&&> = true;
	template <typename _type>
	struct add_reference {
		using type = _type&;
	};
	template <typename _type>
	struct remove_reference {
		using type = _type;
	};
	template <typename _type>
	struct remove_reference<_type&> {
		using type = _type;
	};
	template <typename _type>
	struct remove_reference<_type&&> {
		using type = _type;
	};

	template <class _type>
	struct type_identity {
		using type = _type;
	};
}

namespace library {
	template <typename type>
	inline constexpr bool const_type = detail::const_type<type>;
	template <typename type>
	using add_const = typename detail::add_const<type>::type;
	template <typename type>
	using remove_const = typename detail::remove_const<type>::type;
	template <typename from, typename to>
	using copy_const = typename detail::copy_const<from, to>::type;

	template <typename type>
	inline constexpr bool volatile_type = detail::volatile_type<type>;
	template <typename type>
	using add_volatile = typename detail::add_volatile<type>::type;
	template <typename type>
	using remove_volatile = typename detail::remove_volatile<type>::type;
	template <typename from, typename to>
	using copy_volatile = typename detail::copy_volatile<from, to>::type;

	template <typename type>
	inline constexpr bool pointer_type = detail::pointer_type<type>;
	template<typename type>
	using add_pointer = typename detail::add_pointer<type>::type;
	template <typename type>
	using remove_pointer = typename detail::remove_pointer<type>::type;

	template <typename type>
	inline constexpr bool reference_type = detail::reference_type<type>;
	template<typename type>
	using add_reference = typename detail::add_reference<type>::type;
	template <typename type>
	using remove_reference = typename detail::remove_reference<type>::type;

	template <typename type>
	using remove_cv = remove_const<remove_volatile<type>>;
	template <typename type>
	using remove_cp = remove_const<remove_pointer<type>>;
	template <typename type>
	using remove_cr = remove_const<remove_reference<type>>;
	template <typename type>
	using remove_cvr = remove_cv<remove_reference<type>>;
	template <typename type>
	using remove_pr = remove_pointer<remove_reference<type>>;
	template <typename from, typename to>
	using copy_cv = copy_const<from, copy_volatile<from, to>>;

	template <class type>
	using type_identity = detail::type_identity<type>::type;

	template <typename, typename>
	inline constexpr bool same_type = false;
	template <typename type>
	inline constexpr bool same_type<type, type> = true;
	template <typename type, typename... rest>
	inline constexpr bool any_of_type = (same_type<type, rest> || ...);
	template <typename type>
	inline constexpr bool void_type = same_type<remove_cv<type>, void>;

	template <typename type>
	inline constexpr bool integral_type = any_of_type<remove_cv<type>, bool, char, signed char, unsigned char, wchar_t, char8_t, char16_t, char32_t, short, unsigned short, int, unsigned int, long, unsigned long, long long, unsigned long long>;
	template <typename type>
	inline constexpr bool floating_point_type = any_of_type<remove_cv<type>, float, double, long double>;
	template <typename type>
	inline constexpr bool arithmetic_type = integral_type<type> || floating_point_type<type>;
	//template <typename type>
	//inline constexpr bool fundamental_type = arithmetic_type<type> || void_type<type> || is_null_pointer_v<type>;


	template<typename F>
	struct first_arg;
	template<typename _class, typename _return, typename _argument>
	struct first_arg<_return(_class::*)(_argument) const> {
		using type = _argument;
	};
	template<typename _class, typename _return, typename _argument>
	struct first_arg<_return(_class::*)(_argument)> {
		using type = _argument;
	};
	template<typename F, typename = void>
	struct callable_first_arg;
	template<typename F>
	struct callable_first_arg<F, std::void_t<decltype(&F::operator())>> {
		using type = typename first_arg<decltype(&F::operator())>::type;
	};
	template<typename R, typename Arg, typename... Rest>
	struct callable_first_arg<R(Arg, Rest...), void> {
		using type = Arg;
	};
	template<typename R, typename Arg, typename... Rest>
	struct callable_first_arg<R(*)(Arg, Rest...), void> {
		using type = Arg;
	};
	template<typename function>
	using function_argument_type = typename callable_first_arg<decltype(&function::operator())>::type;


	template <typename type, typename... argument>
	struct key_exist_set {
		static constexpr bool able = false;
	};
	template <typename type>
	struct key_exist_set<type, type> {
		static constexpr bool able = true;
		static auto execute(type const& value) noexcept -> type const& {
			return value;
		}
	};

	template <typename key_type, typename... argument>
	struct key_exist_map {
		static constexpr bool able = false;
	};
	template <typename key_type, typename type>
	struct key_exist_map<key_type, key_type, type> {
		static constexpr bool able = true;
		static auto execute(key_type const& key, type const&) noexcept -> key_type const& {
			return key;
		}
	};

	/* Allow a specific type via explicit specialization */
	template<typename type>
	inline constexpr bool memory_move_safe = std::is_trivially_copyable_v<type>;
	template<typename type>
	inline constexpr bool memory_copy_safe = std::is_trivially_copyable_v<type>;
}