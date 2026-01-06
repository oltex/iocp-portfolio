#pragma once

#pragma once
#include <span>
#include <string>
#include <functional>
#include "library/string.h"

class command final {
public:
	class parameter final {
		std::string _value;
		using size_type = unsigned int;
	public:
		template<typename argument>
		inline parameter(argument&& arg) noexcept
			:_value(std::forward<argument>(arg)) {
		}
		inline parameter(parameter const&) noexcept = default;
		inline parameter(parameter&&) noexcept = default;
		inline auto operator=(parameter const&) noexcept -> parameter & = default;
		inline auto operator=(parameter&&) noexcept -> parameter & = default;
		inline ~parameter(void) noexcept = default;

		template<typename type>
		inline auto get(void) noexcept -> type {
			return library::string_to<type>(_value.data());
		}
	};
private:
	std::unordered_map<std::string, std::function<void(std::span<parameter> const)>> _function;
public:
	inline command(void) noexcept = default;
	inline command(command const&) noexcept = delete;
	inline command(command&&) noexcept = delete;
	inline auto operator=(command const&) noexcept -> command & = delete;
	inline auto operator=(command&&) noexcept -> command & = delete;
	inline ~command(void) noexcept = default;

	inline void add(std::string_view name, std::function<void(std::span<parameter> const)> function) noexcept {
		_function.emplace(name.data(), function);
	}
	inline auto execute(std::string_view name, std::span<parameter> const arg) noexcept {
		auto result = _function.find(name.data());
		if (result == _function.end())
			return false;
		result->second(arg);
		return true;
	}
};

class parser final {
	using size_type = unsigned int;
	std::unordered_map<std::string, std::vector<command::parameter>> _parsing;
public:
	inline parser(void) noexcept = default;
	inline parser(parser const&) noexcept = delete;
	inline parser(parser&&) noexcept = delete;
	inline auto operator=(parser const&) noexcept -> parser & = delete;
	inline auto operator=(parser&&) noexcept -> parser & = delete;
	inline ~parser(void) noexcept = default;

	inline void execute(std::string_view buffer) noexcept {
		std::vector<std::string> item;
		static constexpr char comment_char = '#';
		static constexpr char split_char[] = " ,:(){}[]=";
		static constexpr char end_char[] = { '\r', '\n', ';', 0 };
		bool encoding = true;

		char const* current = buffer.data();
		size_type begin_index = 0;
		for (size_type index = 0; index < buffer.size() + 1; ++index, ++current) {
			bool finish = false;

			// comment
			if (*current == comment_char) {
				encoding = false;
				finish = true;
			}
			// end char
			if (false == finish) {
				for (auto& iter : end_char) {
					if (*current == iter) {
						if (true == encoding && begin_index != index)
							item.emplace_back(std::string(buffer.data() + begin_index, index - begin_index));
						if (!item.empty()) {
							auto res = _parsing.emplace(std::piecewise_construct, std::forward_as_tuple(item[0]), std::forward_as_tuple());
							for (size_type i = 1; i < item.size(); ++i)
								res.first->second.emplace_back(item[i]);
							item.clear();
						}
						begin_index = index + 1;
						encoding = true;
						finish = true;
						break;
					}
				}
			}
			// split char
			if (false == finish && true == encoding) {
				for (auto& iter : split_char) {
					if (*current == iter) {
						if (begin_index != index)
							item.emplace_back(std::string(buffer.data() + begin_index, index - begin_index));
						begin_index = index + 1;
						finish = true;
						break;
					}
				}
			}
		}
	}
	inline auto begin(void) noexcept {
		return _parsing.begin();
	}
	inline auto end(void) noexcept {
		return _parsing.end();
	}
};
