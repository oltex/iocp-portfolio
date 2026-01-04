#pragma once
#include "../memory.h"
#include <coroutine>
#include <iostream>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace library {
	class awaiter {
	public:
		inline awaiter(void) noexcept = default;
		inline awaiter(awaiter const&) noexcept = delete;
		inline awaiter(awaiter&&) noexcept = delete;
		inline auto operator=(awaiter const&) noexcept -> awaiter & = delete;
		inline auto operator=(awaiter&&) noexcept -> awaiter & = delete;
		inline ~awaiter(void) noexcept = default;

		inline bool await_ready(void) noexcept {
			return false;
		}
		inline void await_suspend(std::coroutine_handle<void> handle) noexcept {
		}
		inline int await_resume(void) noexcept {
			return 0;
		}
	};

	template<typename type>
	class promise {
	public:
		inline promise(void) noexcept = default;
		inline promise(promise const&) noexcept = delete;
		inline promise(promise&&) noexcept = delete;
		inline auto operator=(promise const&) noexcept -> promise & = delete;
		inline auto operator=(promise&&) noexcept -> promise & = delete;
		inline ~promise(void) noexcept = default;

		inline auto initial_suspend(void) noexcept -> std::suspend_always {
			return std::suspend_always();
		}
		inline auto final_suspend(void) noexcept -> std::suspend_always {
			return std::suspend_always();
		}
		//inline auto yield_value(int result) noexcept -> std::suspend_always {
		//	return std::suspend_always();
		//}
		//inline auto await_transform(int result) noexcept -> suspend {
		//	printf("await transform\n");
		//	return suspend();
		//}
		//inline void return_void(void) noexcept {
		//}
		//inline void return_value(int result) noexcept {
		//}
		inline void unhandled_exception(void) noexcept {
			__debugbreak();
		}
		inline auto get_return_object(void) noexcept -> std::coroutine_handle<type> {
			return std::coroutine_handle<type>::from_promise(static_cast<type&>(*this));
		}
		inline auto handle(void) noexcept -> std::coroutine_handle<type> {
			return std::coroutine_handle<type>::from_promise(static_cast<type&>(*this));
		}
		inline static void* operator new(size_t size) noexcept {
			return library::allocate(size);
		}
		inline static void operator delete(void* pointer, size_t size) noexcept {
			library::deallocate(pointer);
		}
	};

	template<typename type>
	class coroutine {
	protected:
		std::coroutine_handle<type> _handle;
	public:
		using promise_type = type;
		inline coroutine(std::coroutine_handle<type> handle = nullptr) noexcept
			: _handle(handle) {
		}
		inline coroutine(coroutine const&) noexcept = delete;
		inline coroutine(coroutine&& rhs) noexcept
			: _handle(rhs._handle) {
			rhs._handle = nullptr;
		};
		inline auto operator=(coroutine const&) noexcept -> coroutine & = delete;
		inline auto operator=(coroutine&& rhs) noexcept -> coroutine& {
			_handle = rhs._handle;
			rhs._handle = nullptr;
			return *this;
		};
		inline ~coroutine(void) noexcept = default;

		inline auto data(void) noexcept -> std::coroutine_handle<type>& {
			return _handle;
		}
	};
}