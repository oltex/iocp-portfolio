#pragma once
#include "memory.h"
#include "function.h"
#include "template.h"
#include "array.h"
#include <cassert>
#include <cstdlib>
#include <cstring>

namespace library {
	template <typename type>
		requires (library::any_of_type<type, char, wchar_t>)
	inline auto string_length(type const* const character) noexcept -> size_t {
		if constexpr (library::same_type<type, char>)
			return ::strlen(character);
		else
			return ::wcslen(character);
	}
	template <typename type>
		requires (library::any_of_type<type, char, wchar_t>)
	inline auto string_copy(type* const destine, size_t const size, type const* const source) noexcept -> type* {
		if constexpr (library::same_type<type, char>)
			::strcpy_s(destine, size, source);
		else
			::wcscpy_s(destine, size, source);
		return destine;
	}
	template <typename type, size_t size>
		requires (library::any_of_type<type, char, wchar_t>)
	inline auto string_copy(type(&destine)[size], type const* const source) noexcept -> type* {
		if constexpr (library::same_type<type, char>)
			::strcpy_s(destine, size, source);
		else
			::wcscpy_s(destine, size, source);
		return destine; 
	}
	template <typename type>
		requires (library::any_of_type<type, char, wchar_t>)
	inline auto string_compare(type const* const string1, type const* const string2) noexcept -> int {
		if constexpr (library::same_type<type, char>)
			return ::strcmp(string1, string2);
		else
			return ::wcscmp(string1, string2);
	}
	template <typename type>
		requires (library::any_of_type<type, char, wchar_t>)
	inline auto string_string(type const* const string, type const* const sub_string) noexcept {
		if constexpr (library::same_type<type, char>)
			return ::strstr(string, sub_string);
		else
			return ::wcsstr(string, sub_string);
	}
	template <typename type>
		requires (library::any_of_type<type, char, wchar_t>)
	inline auto string_print(type* buffer, size_t const count, type const* format, ...) noexcept {
		va_list arg;
		va_start(arg, format);
		if constexpr (library::same_type<type, char>)
			return ::vsprintf_s(buffer, count, format, arg);
		else
			return ::vswprintf_s(buffer, count, format, arg);
		va_end(arg);
	}
	template <typename type, size_t size>
		requires (library::any_of_type<type, char, wchar_t>)
	inline constexpr auto string_print(type(&buffer)[size], type const* format, ...) noexcept {
		va_list arg;
		va_start(arg, format);
		if constexpr (library::same_type<type, char>)
			return ::vsprintf_s(buffer, format, arg);
		else
			return ::vswprintf_s(buffer, format, arg);
		va_end(arg);
	}
	template <typename to, typename from>
		requires (library::any_of_type<from, char, wchar_t>)
	inline auto string_to(from const* const string, int const radix = 10) noexcept -> to {
		if constexpr (std::is_same_v<to, bool>) {
			return(0 == library::string_compare(string, "1") ||
				0 == library::string_compare(string, "true") ||
				0 == library::string_compare(string, "TRUE"));
		}
		else if constexpr (library::same_type<char, from>) {
			if constexpr (library::same_type<to, int>)
				return static_cast<int>(std::strtol(string, nullptr, radix));
			else if constexpr (library::same_type<to, long>)
				return std::strtol(string, nullptr, radix);
			else if constexpr (library::same_type<to, long long>)
				return std::strtoll(string, nullptr, radix);
			else if constexpr (library::same_type<to, unsigned int>)
				return static_cast<unsigned int>(std::strtoul(string, nullptr, radix));
			else if constexpr (library::same_type<to, unsigned long>)
				return std::strtoul(string, nullptr, radix);
			else if constexpr (library::same_type<to, unsigned long long>)
				return std::strtoull(string, nullptr, radix);
			else if constexpr (library::same_type<to, float>)
				return std::strtof(string, nullptr);
			else if constexpr (library::same_type<to, double>)
				return std::strtod(string, nullptr);
		}
		else {
			if constexpr (library::same_type<int, to>)
				return static_cast<int>(std::wcstol(string, nullptr, radix));
			else if constexpr (library::same_type<long, to>)
				return std::wcstol(string, nullptr, radix);
			else if constexpr (library::same_type<long long, to>)
				return std::wcstoll(string, nullptr, radix);
			else if constexpr (library::same_type<unsigned int, to>)
				return static_cast<unsigned int>(std::wcstoul(string, nullptr, radix));
			else if constexpr (library::same_type<unsigned long, to>)
				return std::wcstoul(string, nullptr, radix);
			else if constexpr (library::same_type<unsigned long long, to>)
				return std::wcstoull(string, nullptr, radix);
			else if constexpr (library::same_type<float, to>)
				return std::wcstof(string, nullptr);
			else if constexpr (library::same_type<double, to>)
				return std::wcstod(string, nullptr);
		}
	}

	inline auto multibyte_to_widechar(char const* source, int const source_size, wchar_t* destine, int const destine_size) noexcept {
		return ::MultiByteToWideChar(CP_ACP, 0, source, source_size, destine, destine_size);
	}
	inline auto widechar_to_multibyte(wchar_t const* source, int const source_size, char* destine, int const destine_size = 0) noexcept {
		return ::WideCharToMultiByte(CP_ACP, 0, source, source_size, destine, destine_size, nullptr, nullptr);
	}

	class guid;
}

namespace detail {
	template<typename word>
	struct string_extract {
		using type = word;
	};
	template<typename word>
	struct string_extract<word*> {
		using type = library::remove_cv<word>;
	};
	template<class word, std::size_t size>
	struct string_extract<word[size]> {
		using type = library::remove_cv<word>;
	};

	template<typename type, size_t size>
	struct string_literal {
		type _value[size];
		inline constexpr string_literal(const type(&str)[size]) noexcept {
			std::copy_n(str, size, _value);
		}
		inline constexpr explicit string_literal(string_literal const&) noexcept = default;
		inline constexpr explicit string_literal(string_literal&&) noexcept = default;
		inline auto operator=(string_literal const&) noexcept -> string_literal & = default;
		inline auto operator=(string_literal&&) noexcept -> string_literal & = default;
		inline ~string_literal(void) noexcept = default;
	};

	template<typename type>
	class string_view;
	template<typename type, size_t sso = 16>
		requires (library::any_of_type<type, char, wchar_t>)
	class string final {
		using size_type = unsigned int;
		union buffer {
			library::array<type, sso> _array;
			type* _pointer;
		};

		size_type _size;
		size_type _capacity;
		buffer _buffer;
	public:
		using iterator = type*;

		inline string(void) noexcept
			: _size(0), _capacity(sso), _buffer() {
		};
		template<typename argument>
		inline string(argument&& arg) noexcept
			: string() {
			insert(end(), std::forward<argument>(arg));
		}
		inline string(string const& rhs) noexcept
			: string(const_cast<string&>(rhs).data()) {
		};
		inline string(string&& rhs) noexcept
			: _size(library::exchange(rhs._size, 0)), _capacity(library::exchange(rhs._capacity, static_cast<size_type>(sso))), _buffer(rhs._buffer) {
			rhs.null();
		};
		inline auto operator=(string const& rhs) noexcept -> string& {
			assign(const_cast<string&>(rhs).data());
			return *this;
		};
		inline auto operator=(string&& rhs) noexcept -> string& {
			if (sso < _capacity)
				library::deallocate(_buffer._pointer);

			_size = library::exchange(rhs._size, 0);
			_capacity = library::exchange(rhs._capacity, static_cast<size_type>(sso));
			_buffer = rhs._buffer;
			rhs.null();
			return *this;
		};
		inline ~string(void) noexcept {
			if (sso < _capacity)
				library::deallocate(_buffer._pointer);
		};

		template<typename argument>
		inline auto insert(iterator iter, argument&& arg) noexcept -> iterator {
			size_type char_size;
			if constexpr (library::same_type<type, typename string_extract<library::remove_cvr<argument>>::type>) {
				if constexpr (library::any_of_type<library::remove_cvr<argument>, string<type>, string_view<type>>)
					char_size = arg.size();
				else if constexpr (library::same_type<library::remove_cvr<argument>, type>)
					char_size = 1;
				else
					char_size = static_cast<size_type>(library::string_length(arg));
			}
			else if constexpr (library::same_type<char, type>) {
				if constexpr (library::any_of_type<library::remove_cvr<argument>, string<wchar_t>, string_view<wchar_t>>)
					char_size = library::widechar_to_multibyte(arg.data(), static_cast<int>(arg.size()), nullptr, 0);
				else if constexpr (library::same_type<library::remove_cvr<argument>, wchar_t>)
					char_size = library::widechar_to_multibyte(&arg, 1, nullptr, 0);
				else
					char_size = library::widechar_to_multibyte(arg, static_cast<int>(library::string_length(arg)), nullptr, 0);
			}
			else {
				if constexpr (library::any_of_type<library::remove_cvr<argument>, string<char>, string_view<char>>)
					char_size = library::multibyte_to_widechar(arg.data(), static_cast<int>(arg.size()), nullptr, 0);
				else if constexpr (library::same_type<library::remove_cvr<argument>, char>)
					char_size = library::multibyte_to_widechar(&arg, 1, nullptr, 0);
				else
					char_size = library::multibyte_to_widechar(arg, static_cast<int>(library::string_length(arg)), nullptr, 0);
			}

			if (_size + char_size >= _capacity) {
				auto index = iter - begin();
				reserve(library::maximum(static_cast<size_type>(_capacity * 1.5f), _size + char_size + 1));
				iter = begin() + index;
			}
			library::memory_move(iter + char_size, iter, end() - iter);

			if constexpr (library::same_type<type, typename string_extract<library::remove_cvr<argument>>::type>) {
				if constexpr (library::any_of_type<library::remove_cvr<argument>, string<type>, string_view<type>>)
					library::memory_copy(iter, arg.data(), char_size);
				else if constexpr (library::same_type<library::remove_cvr<argument>, type>)
					*iter = arg;
				else
					library::memory_copy(iter, arg, char_size);
			}
			else if constexpr (library::same_type<char, type>) {
				if constexpr (library::any_of_type<library::remove_cvr<argument>, string<wchar_t>, string_view<wchar_t>>)
					library::widechar_to_multibyte(arg.data(), static_cast<int>(arg.size()), iter, char_size);
				else if constexpr (library::same_type<library::remove_cvr<argument>, wchar_t>)
					library::widechar_to_multibyte(&arg, 1, iter, char_size);
				else
					library::widechar_to_multibyte(arg, static_cast<int>(library::string_length(arg)), iter, char_size);
			}
			else {
				if constexpr (library::any_of_type<library::remove_cvr<argument>, string<char>, string_view<char>>)
					library::multibyte_to_widechar(arg.data(), static_cast<int>(arg.size()), iter, char_size);
				else if constexpr (library::same_type<library::remove_cvr<argument>, char>)
					library::multibyte_to_widechar(&arg, 1, iter, char_size);
				else
					library::multibyte_to_widechar(arg, static_cast<int>(library::string_length(arg)), iter, char_size);
			}
			_size += char_size;
			null();
			return iter;
		}
		template<typename guid>
			requires (library::same_type<library::remove_cvr<guid>, library::guid>)
		inline auto insert(iterator iter, guid& arg) noexcept -> iterator {
			if constexpr (library::same_type<char, type>) {
				char buffer[37];
				library::string_print(buffer, "%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
					arg._guid.Data1, arg._guid.Data2, arg._guid.Data3,
					arg._guid.Data4[0], arg._guid.Data4[1], arg._guid.Data4[2], arg._guid.Data4[3],
					arg._guid.Data4[4], arg._guid.Data4[5], arg._guid.Data4[6], arg._guid.Data4[7]);
				return insert(iter, buffer);
			}
			else {
				wchar_t buffer[37];
				library::string_print(buffer, L"%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
					arg._guid.Data1, arg._guid.Data2, arg._guid.Data3,
					arg._guid.Data4[0], arg._guid.Data4[1], arg._guid.Data4[2], arg._guid.Data4[3],
					arg._guid.Data4[4], arg._guid.Data4[5], arg._guid.Data4[6], arg._guid.Data4[7]);
				return insert(iter, buffer);
			}
		}
		inline auto push_back(type character) noexcept -> type& {
			return *insert(end(), character);
		}
		inline auto erase(iterator iter) noexcept -> iterator {
			assert(_size > 0 && "called on empty");
			library::memory_move(iter, iter + 1, end() - (iter + 1));
			--_size;
			null();
			return iter;
		}
		inline void pop_back(void) noexcept {
			erase(end() - 1);
		}
		template<typename argument>
		inline auto append(argument&& arg) noexcept -> string& {
			insert(end(), std::forward<argument>(arg));
			return *this;
		}
		template<typename argument>
		inline auto assign(argument arg) noexcept -> string& {
			clear();
			return append(arg);
		}
		template<typename argument>
		inline auto operator+=(argument&& arg) noexcept -> string& {
			return append(std::forward<argument>(arg));
		}
		template<typename argument>
		inline auto operator=(argument arg) noexcept -> string& {
			return assign(arg);
		}

		inline bool operator==(string const& rhs) const noexcept {
			return size() == rhs.size() && 0 == library::memory_compare(data(), rhs.data(), size());
		}
		template<typename argument>
			requires(!library::any_of_type<library::remove_cvr<argument>, string<char>, string<wchar_t>>)
		inline auto operator==(argument&& rhs) const noexcept -> bool {
			size_type char_size;
			if constexpr (library::any_of_type<library::remove_cvr<argument>, string<type>, string_view<type>>)
				char_size = rhs.size();
			else if constexpr (library::same_type<library::remove_reference<argument>, type>)
				char_size = 1;
			else
				char_size = static_cast<size_type>(library::string_length(std::forward<argument>(rhs)));

			if (char_size != size())
				return false;
			if constexpr (library::any_of_type<library::remove_cvr<argument>, string<type>, string_view<type>>)
				return 0 == library::memory_compare(data(), rhs.data(), _size);
			else if constexpr (library::same_type<library::remove_reference<argument>, type>)
				return data()[0] == rhs;
			else
				return 0 == library::memory_compare(data(), std::forward<argument>(rhs), _size);
		};
		template<typename argument>
		inline friend auto operator+(string const& lhs, argument&& rhs) noexcept -> string {
			string result;
			size_type char_size;

			if constexpr (library::same_type<type, typename string_extract<library::remove_cvr<argument>>::type>) {
				if constexpr (library::any_of_type<library::remove_cvr<argument>, string<type>, string_view<type>>)
					char_size = rhs.size();
				else if constexpr (library::same_type<library::remove_cvr<argument>, type>)
					char_size = 1;
				else
					char_size = static_cast<size_type>(library::string_length(rhs));
			}
			else if constexpr (library::same_type<char, type>) {
				if constexpr (library::any_of_type<library::remove_cvr<argument>, string<wchar_t>, string_view<wchar_t>>)
					char_size = library::widechar_to_multibyte(rhs.data(), static_cast<int>(rhs.size()), nullptr, 0);
				else if constexpr (library::same_type<library::remove_cvr<argument>, wchar_t>)
					char_size = library::widechar_to_multibyte(&rhs, 1, nullptr, 0);
				else
					char_size = library::widechar_to_multibyte(rhs, static_cast<int>(library::string_length(rhs)), nullptr, 0);
			}
			else {
				if constexpr (library::any_of_type<library::remove_cvr<argument>, string<char>, string_view<char>>)
					char_size = library::multibyte_to_widechar(rhs.data(), static_cast<int>(rhs.size()), nullptr, 0);
				else if constexpr (library::same_type<library::remove_cvr<argument>, char>)
					char_size = library::multibyte_to_widechar(&rhs, 1, nullptr, 0);
				else
					char_size = library::multibyte_to_widechar(rhs, static_cast<int>(library::string_length(rhs)), nullptr, 0);
			}

			result.reserve(lhs.size() + char_size + 1);
			result.append(lhs).append(std::forward<argument>(rhs));
			return result;
		}

		inline auto begin(void) noexcept -> iterator {
			return data();
		}
		inline auto end(void) noexcept -> iterator {
			return data() + _size;
		}
		inline auto front(void) const noexcept -> type& {
			assert(_size > 0 && "called on empty");
			return data()[0];
		}
		inline auto back(void) const noexcept -> type& {
			assert(_size > 0 && "called on empty");
			return data()[_size - 1];
		}
		inline auto operator[](size_type const index) const noexcept ->type& {
			assert(index < _size && "index out of range");
			return data()[index];
		}
		inline void reserve(size_type const capacity) noexcept {
			if (_capacity < capacity) {
				if (sso >= _capacity)
					_buffer._pointer = reinterpret_cast<type*>(library::memory_copy(library::allocate<type>(capacity), _buffer._array.data(), _size + 1));
				else
					_buffer._pointer = reinterpret_cast<type*>(library::reallocate(_buffer._pointer, capacity));
				_capacity = capacity;
			}
		}
		inline void resize(size_type const size) noexcept {
			if (size >= _capacity)
				reserve(library::maximum(static_cast<size_type>(_capacity * 1.5f), size + 1));
			_size = size;
			null();
		}
		inline void clear(void) noexcept {
			_size = 0;
			null();
		}
		inline void swap(string& rhs) noexcept {
			library::swap(_size, rhs._size);
			library::swap(_capacity, rhs._capacity);
			library::swap(_buffer, rhs._buffer);
		}
		inline bool empty(void) const noexcept {
			return 0 == _size;
		}
		inline auto size(void) const noexcept -> size_type {
			return _size;
		}
		inline auto capacity(void) const noexcept -> size_type {
			if (sso < _capacity)
				return _capacity - 1;
			else
				return sso - 1;

		}
		inline auto data(void) noexcept -> type* {
			return const_cast<type*>(static_cast<string const&>(*this).data());
		}
		inline auto data(void) const noexcept -> type const* {
			if (sso >= _capacity)
				return _buffer._array.data();
			else
				return _buffer._pointer;
		}
	private:
		inline void null(void) noexcept {
			if constexpr (library::same_type<type, char>)
				data()[_size] = '\0';
			else
				data()[_size] = L'\0';
		}
	};
	template<typename type>
	class string_view {
		using size_type = unsigned int;
		using iterator = type*;
		type const* _pointer;
		size_type _size;
	public:
		inline string_view(void) noexcept
			: _pointer(nullptr), _size(0) {
		};
		inline string_view(string<type> const& string) noexcept
			: _pointer(string.data()), _size(string.size()) {
		};
		template<typename argument>
			requires library::any_of_type<std::decay_t<argument>, type*, type const*>
		inline string_view(argument&& arg) noexcept
			: _pointer(std::forward<argument>(arg)), _size(static_cast<size_type>(library::string_length(std::forward<argument>(arg)))) {
		}
		inline string_view(string_view const&) noexcept = default;
		inline string_view(string_view&&) noexcept = default;
		inline auto operator=(string_view const&) noexcept -> string_view & = default;
		inline auto operator=(string_view&&) noexcept -> string_view & = default;
		inline ~string_view(void) noexcept = default;

		inline auto size(void) const noexcept -> size_type {
			return _size;
		}
		inline auto begin(void) noexcept -> iterator {
			return _pointer;
		}
		inline auto end(void) noexcept -> iterator {
			return _pointer + _size;
		}
		inline auto operator[](size_type const index) const noexcept ->type& {
			assert(index < _size && "index out of range");
			return _pointer[index];
		}
		inline auto data(void) const noexcept -> type const* {
			return _pointer;
		}
	};

	template<typename word>
	struct string_extract<string<word>> {
		using type = word;
	};
	template<typename word>
	struct string_extract<string_view<word>> {
		using type = word;
	};

	template<typename type, typename size_type = unsigned int>
		requires (library::any_of_type<type, string<char>, string<wchar_t>, string_view<char>, string_view<wchar_t>>)
	struct string_hash {
		inline static constexpr size_type _offset_basis = sizeof(size_type) == 4 ? 2166136261U : 14695981039346656037ULL;
		inline static constexpr size_type _prime = sizeof(size_type) == 4 ? 16777619U : 1099511628211ULL;

		template <typename argument>
		inline auto operator()(argument&& key) const noexcept -> size_type {
			size_type size;
			if constexpr (library::any_of_type<library::remove_cvr<argument>, string<char>, string<wchar_t>, string_view<char>, string_view<wchar_t>>)
				size = key.size();
			else if constexpr (library::any_of_type<library::remove_reference<argument>, char, wchar_t>)
				size = 1;
			else
				size = static_cast<size_type>(library::string_length(key));
			unsigned char const* byte;
			if constexpr (library::any_of_type<library::remove_cvr<argument>, string<char>, string<wchar_t>, string_view<char>, string_view<wchar_t>>)
				byte = reinterpret_cast<unsigned char const*>(key.data());
			else if constexpr (library::any_of_type<library::remove_reference<argument>, char, wchar_t>)
				byte = reinterpret_cast<unsigned char const*>(&key);
			else
				byte = reinterpret_cast<unsigned char const*>(key);

			auto value = _offset_basis;
			for (size_type index = 0; index < size; ++index) {
				value ^= static_cast<size_type>(byte[index]);
				value *= _prime;
			}
			return value;
		}
	};
}

namespace library {
	using string = typename detail::string<char>;
	using wstring = typename detail::string<wchar_t>;
	using string_view = typename detail::string_view<char>;
	using wstring_view = typename detail::string_view<wchar_t>;
	template<size_t size>
	using string_literal = typename detail::string_literal<char, size>;
	template<size_t size>
	using wstring_literal = typename detail::string_literal<wchar_t, size>;

	template <typename size_type>
	struct fnv_hash<string, size_type> : public detail::string_hash<string, size_type> {
	};
	template <typename size_type>
	struct fnv_hash<wstring, size_type> : public detail::string_hash<wstring, size_type> {
	};
	template <typename size_type>
	struct fnv_hash<string_view, size_type> : public detail::string_hash<string_view, size_type> {
	};
	template <typename size_type>
	struct fnv_hash<wstring_view, size_type> : public detail::string_hash<wstring_view, size_type> {
	};
}