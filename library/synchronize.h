#pragma once
#include "handle.h"
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace library {
	template <typename type>
	inline bool wait_on_address(type& address, type& compare, unsigned long const milli_second) noexcept {
		static_assert(std::is_trivially_copyable_v<type>, "type must be trivially copyable");
		static_assert(sizeof(type) == 1 || sizeof(type) == 2 || sizeof(type) == 4 || sizeof(type) == 8, "type size must be 1/2/4/8 bytes");
		return ::WaitOnAddress(reinterpret_cast<void*>(&address), reinterpret_cast<void*>(&compare), sizeof(type), milli_second);
	}
	template <typename type>
	inline void wake_by_address_single(type& address) noexcept {
		::WakeByAddressSingle(reinterpret_cast<void*>(&address));
	}
	template <typename type>
	inline void wake_by_address_all(type& address) noexcept {
		::WakeByAddressAll(reinterpret_cast<void*>(&address));
	}

	class event final : public handle {
	public:
		inline event(void) noexcept = default;
		inline event(bool const manual, bool const initial_state) noexcept
			: handle(::CreateEventW(nullptr, manual, initial_state, nullptr)) {
		};
		inline event(event const&) noexcept = delete;
		inline event(event&& rhs) noexcept
			: handle(std::move(rhs)) {
		};
		inline auto operator=(event const&) noexcept -> event & = delete;
		inline auto operator=(event&& rhs) noexcept -> event& {
			handle::operator=(std::move(rhs));
			return *this;
		};
		inline virtual ~event(void) noexcept override = default;

		inline void create(bool const manual, bool const initial_state) noexcept {
			_handle = ::CreateEventW(nullptr, manual, initial_state, nullptr);
		}
		inline void set(void) noexcept {
			::SetEvent(_handle);
		}
		inline void reset(void) noexcept {
			::ResetEvent(_handle);
		}
		inline void pulse(void) noexcept {
			::PulseEvent(_handle);
		}
	};
	class mutex final : public handle {
	public:
		inline mutex(bool const initial_owner) noexcept
			: handle(::CreateMutexW(nullptr, initial_owner, nullptr)) {
		};
		inline mutex(mutex const&) noexcept = delete;
		inline mutex(mutex&& rhs) noexcept
			: handle(std::move(rhs)) {
		};
		inline auto operator=(mutex const&) noexcept -> mutex & = delete;
		inline auto operator=(mutex&& rhs) noexcept -> mutex& {
			handle::operator=(std::move(rhs));
		};
		inline virtual ~mutex(void) noexcept override = default;
	};
}