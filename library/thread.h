#pragma once
#include "handle.h"
#include <process.h>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <tuple>
#include <type_traits>
#include <memory>
#include <cassert>

namespace library {
	inline void sleep(unsigned long milli_second) noexcept {
		::Sleep(milli_second);
	}

	class token final : public handle {
	public:
		inline explicit token(void) noexcept = default;
		inline explicit token(token const&) noexcept = delete;
		inline token(token&& rhs) noexcept
			: handle(std::move(rhs)) {
		};
		inline auto operator=(token const&) noexcept -> token & = delete;
		inline auto operator=(token&& rhs) noexcept -> token& {
			handle::operator=(std::move(rhs));
			return *this;
		};
		inline virtual ~token(void) noexcept override = default;

		inline void adjust_privileges_se_lock_memory_privilege(void) noexcept {
			LUID luid;
			if (0 == LookupPrivilegeValueW(nullptr, L"SeLockMemoryPrivilege", &luid)) {
				switch (GetLastError()) {
				default:
					__debugbreak();
#pragma warning(suppress: 4065)
				}
			}

			TOKEN_PRIVILEGES privileges;
			privileges.PrivilegeCount = 1;
			privileges.Privileges[0].Luid = luid;
			privileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
			if (0 == AdjustTokenPrivileges(_handle, false, &privileges, sizeof(TOKEN_PRIVILEGES), nullptr, nullptr))
				__debugbreak();
			else {
				switch (GetLastError()) {
				case ERROR_SUCCESS:
					break;
				case ERROR_NOT_ALL_ASSIGNED:
				default:
					__debugbreak();
				}
			}
		}
	};

	class thread final : public handle {
		template <typename tuple, size_t... index>
		inline static unsigned int __stdcall invoke(void* arg) noexcept {
			const std::unique_ptr<tuple> value(reinterpret_cast<tuple*>(arg));
			tuple& tuple = *value.get();
			std::invoke(std::move(std::get<index>(tuple))...);
			return 0;
		}
		template <typename tuple, size_t... index>
		inline static constexpr auto make(std::index_sequence<index...>) noexcept {
			return &invoke<tuple, index...>;
		}
	public:
		inline explicit thread(void) noexcept = default;
		template <typename function, typename... argument>
		inline explicit thread(function&& func, unsigned int flag, argument&&... arg) noexcept {
			using tuple = std::tuple<std::decay_t<function>, std::decay_t<argument>...>;
			auto copy = std::make_unique<tuple>(std::forward<function>(func), std::forward<argument>(arg)...);
			constexpr auto proc = make<tuple>(std::make_index_sequence<1 + sizeof...(argument)>());
			_handle = reinterpret_cast<HANDLE>(::_beginthreadex(nullptr, 0, proc, copy.get(), flag, 0));

			if (_handle)
				copy.release();
			else
				::__debugbreak();
		}
		inline explicit thread(thread const&) noexcept = delete;
		inline explicit thread(thread&& rhs) noexcept
			: handle(std::move(rhs)) {
		};
		inline auto operator=(thread const&) noexcept -> thread & = delete;
		inline auto operator=(thread&& rhs) noexcept -> thread& {
			handle::operator=(std::move(rhs));
			return *this;
		};
		inline virtual ~thread(void) noexcept override = default;

		template <typename function, typename... argument>
		inline void begin(function&& func, unsigned int flag, argument&&... arg) noexcept {
			using tuple = std::tuple<std::decay_t<function>, std::decay_t<argument>...>;
			auto copy = std::make_unique<tuple>(std::forward<function>(func), std::forward<argument>(arg)...);
			constexpr auto proc = make<tuple>(std::make_index_sequence<1 + sizeof...(argument)>());
			_handle = reinterpret_cast<HANDLE>(::_beginthreadex(nullptr, 0, proc, copy.get(), flag, 0));

			if (_handle)
				copy.release();
			else
				::__debugbreak();
		}
		inline void suspend(void) noexcept {
			::SuspendThread(_handle);
		}
		inline void resume(void) noexcept {
			::ResumeThread(_handle);
		}
		inline void terminate(void) noexcept {
#pragma warning(suppress: 6258)
			::TerminateThread(_handle, 0);
		}
		inline auto get_id(void) noexcept -> unsigned long {
			::GetThreadId(_handle);
		}
		inline auto get_exit_code(void) noexcept -> unsigned long {
			unsigned long code;
			::GetExitCodeThread(_handle, &code);
			return code;
		}
		inline void set_affinity_mask(DWORD_PTR mask) noexcept {
			::SetThreadAffinityMask(_handle, mask);
		}
		inline void set_priority(int const priority) noexcept {
			::SetThreadPriority(_handle, priority);
		}
		inline auto set_thread_descript(wchar_t const* const descript) noexcept {
			auto result = ::SetThreadDescription(_handle, descript);
			assert(SUCCEEDED(result));
		}

		inline static void switch_to(void) noexcept {
			::SwitchToThread();
		}
		inline static auto get_current(void) noexcept -> HANDLE {
			return ::GetCurrentThread();
		}
		inline static auto get_current_id(void) noexcept -> unsigned long {
			return ::GetCurrentThreadId();
		}
	};

	class process final : public handle {
	public:
		inline explicit process(void) noexcept
			: handle(get_current()) {
		};
		inline explicit process(process const&) noexcept = delete;
		inline explicit process(process&& rhs) noexcept
			: handle(std::move(rhs)) {
		};
		inline auto operator=(process const&) noexcept -> process & = delete;
		inline auto operator=(process&& rhs) noexcept -> process& {
			handle::operator=(std::move(rhs));
			return *this;
		};
		inline virtual ~process(void) noexcept override = default;

		inline auto open_token(void) noexcept -> token {
			token token_;
			if (0 == ::OpenProcessToken(_handle, TOKEN_ADJUST_PRIVILEGES, &token_.data())) {
				switch (GetLastError()) {
				default:
					__debugbreak();
#pragma warning(suppress: 4065)
				}
			}
			return token_;
		}
		inline static auto get_current(void) noexcept -> HANDLE {
			return ::GetCurrentProcess();
		}
		inline static auto get_current_id(void) noexcept -> unsigned long {
			return ::GetCurrentProcessId();
		}
	};
}