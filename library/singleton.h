#pragma once
#include "define.h"
#include "memory.h"
#include <type_traits>

namespace library {
	template<typename type, bool placement = false, bool dll = false>
	class singleton {
		inline singleton(singleton const&) noexcept = delete;
		inline singleton(singleton&&) noexcept = delete;
		inline auto operator=(singleton const&) noexcept -> singleton & = delete;
		inline auto operator=(singleton&&) noexcept -> singleton & = delete;
	protected:
		inline singleton(void) noexcept = default;
		inline ~singleton(void) noexcept = default;
	public:
		inline static auto instance(void) noexcept -> type& {
			static type _instance;
			return _instance;
		}
	};
	template<typename type>
	class singleton<type, true, false> {
		inline static type* _instance;

		inline singleton(singleton const&) noexcept = delete;
		inline singleton(singleton&&) noexcept = delete;
		inline auto operator=(singleton const&) noexcept -> singleton & = delete;
		inline auto operator=(singleton&&) noexcept -> singleton & = delete;
	protected:
		inline singleton(void) noexcept = default;
		inline ~singleton(void) noexcept = default;
	public:
		template<typename... argument>
		inline static auto construct(argument&&... arg) noexcept -> type& {
			_instance = library::allocate<type>();
			if constexpr (std::is_class_v<type>)
				if constexpr (sizeof...(argument) == 0)
					::new(reinterpret_cast<void*>(_instance)) type;
				else
					::new(reinterpret_cast<void*>(_instance)) type(std::forward<argument>(arg)...);
			else if constexpr (0 < sizeof...(argument))
#pragma warning(suppress: 6011)
				* _instance = type(std::forward<argument>(arg)...);
			return *_instance;
		}
		inline static auto instance(void) noexcept -> type& {
			return *_instance;
		}
		inline static void destruct(void) noexcept {
			_instance->~type();
			library::deallocate(_instance);
		}
	};
	template<typename type>
	class declspec_dll singleton<type, false, true> {
		inline singleton(singleton const&) noexcept = delete;
		inline singleton(singleton&&) noexcept = delete;
		inline auto operator=(singleton const&) noexcept -> singleton & = delete;
		inline auto operator=(singleton&&) noexcept -> singleton & = delete;
	protected:
		inline singleton(void) noexcept = default;
		inline ~singleton(void) noexcept = default;
	public:
		inline static auto instance(void) noexcept -> type&;
	};
	template<typename type>
	class declspec_dll singleton<type, true, true> {
		static type* _instance;

		inline singleton(singleton const&) noexcept = delete;
		inline singleton(singleton&&) noexcept = delete;
		inline auto operator=(singleton const&) noexcept -> singleton & = delete;
		inline auto operator=(singleton&&) noexcept -> singleton & = delete;
	protected:
		inline singleton(void) noexcept = default;
		inline ~singleton(void) noexcept = default;
	public:
		template<typename... argument>
#pragma warning(suppress: 4506)
		inline static auto construct(argument&&... arg) noexcept -> type&;
#pragma warning(suppress: 4506)
		inline static auto instance(void) noexcept -> type&;
#pragma warning(suppress: 4506)
		inline static void destruct(void) noexcept;
#pragma warning(suppress: 4661)
	};

	namespace _thread_local {
		template<typename type>
		class singleton {
			inline singleton(singleton const&) noexcept = delete;
			inline singleton(singleton&&) noexcept = delete;
			inline auto operator=(singleton const&) noexcept -> singleton & = delete;
			inline auto operator=(singleton&&) noexcept -> singleton & = delete;
		protected:
			inline singleton(void) noexcept = default;
			inline ~singleton(void) noexcept = default;
		public:
			inline static auto instance(void) noexcept -> type& {
				thread_local type _instance;
				return _instance;
			}
		};
	}
}