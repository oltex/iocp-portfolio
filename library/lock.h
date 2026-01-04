#pragma once
#include "interlock.h"
#include "synchronize.h"
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <atomic>

namespace library {
	class spin_lock final {
		unsigned char _lock;
	public:
		inline spin_lock(void) noexcept
			: _lock(0) {
		};
		inline spin_lock(spin_lock const&) noexcept = delete;
		inline spin_lock(spin_lock&&) noexcept = delete;
		inline auto operator=(spin_lock const&) noexcept -> spin_lock & = delete;
		inline auto operator=(spin_lock&&) noexcept -> spin_lock & = delete;
		inline ~spin_lock(void) noexcept = default;

		inline void lock(void) noexcept {
			while (1 == _lock || 1 == library::interlock_exchange(_lock, 1))
				YieldProcessor();
		}
		inline void unlock(void) noexcept {
			library::interlock_exchange(_lock, 0);
			//_lock = 0;
		}
	};

	class seq_lock final {
		unsigned long long _sequence;
	public:
		inline seq_lock(void) noexcept
			: _sequence(0) {
		};
		inline seq_lock(seq_lock const&) noexcept = delete;
		inline seq_lock(seq_lock&&) noexcept = delete;
		inline auto operator=(seq_lock const&) noexcept -> seq_lock & = delete;
		inline auto operator=(seq_lock&&) noexcept -> seq_lock & = delete;
		inline ~seq_lock(void) noexcept = default;

		[[nodiscard]] auto read_start(void) const noexcept -> unsigned long long {
			auto sequence = _sequence;
			library::read_barrier();
			return sequence;
		}
		auto read_end(unsigned long long const sequence) const noexcept -> bool {
			library::read_barrier();
			return sequence == _sequence && !(sequence & 1);
		}
		void write_start(void) noexcept {
			++_sequence;
			library::write_barrier();
		}
		void write_end(void) noexcept {
			library::write_barrier();
			++_sequence;
		}
	};

	class wait_on_address_lock final {
		long _address = 0;
	public:
		inline wait_on_address_lock(void) noexcept = default;
		inline wait_on_address_lock(wait_on_address_lock const&) noexcept = delete;
		inline wait_on_address_lock(wait_on_address_lock&&) noexcept = delete;
		inline auto operator=(wait_on_address_lock const&) noexcept -> wait_on_address_lock & = delete;
		inline auto operator=(wait_on_address_lock&&) noexcept -> wait_on_address_lock & = delete;
		inline ~wait_on_address_lock(void) noexcept = default;

		inline void lock(void) noexcept {
			long compare = 1;
			while (1 == _address || 1 == library::interlock_exchange(_address, 1))
				wait_on_address(_address, compare, INFINITE);
		}
		inline void unlock(void) noexcept {
			_address = 0;
			wake_by_address_single(_address);
		}
	};

	class critical_section final {
		CRITICAL_SECTION _critical_section;
	public:
		inline critical_section(void) noexcept {
			::InitializeCriticalSection(&_critical_section);
		};
		inline critical_section(critical_section const&) noexcept = delete;
		inline critical_section(critical_section&&) noexcept = delete;
		inline auto operator=(critical_section const&) noexcept -> critical_section & = delete;
		inline auto operator=(critical_section&&) noexcept -> critical_section & = delete;
		inline ~critical_section(void) noexcept {
			::DeleteCriticalSection(&_critical_section);
		};

		inline void enter(void) noexcept {
			EnterCriticalSection(&_critical_section);
		}
		inline bool try_enter(void) noexcept {
			return TryEnterCriticalSection(&_critical_section);
		}
		inline void leave(void) noexcept {
			LeaveCriticalSection(&_critical_section);
		}
		inline auto data(void) noexcept -> CRITICAL_SECTION& {
			return _critical_section;
		}
	};

	class slim_read_write_lock final {
		SRWLOCK _srwlock;
	public:
		inline slim_read_write_lock(void) noexcept {
			::InitializeSRWLock(&_srwlock);
		};
		inline slim_read_write_lock(slim_read_write_lock const&) noexcept = delete;
		inline slim_read_write_lock(slim_read_write_lock&&) noexcept = delete;
		inline auto operator=(slim_read_write_lock const&) noexcept -> slim_read_write_lock & = delete;
		inline auto operator=(slim_read_write_lock&&) noexcept -> slim_read_write_lock & = delete;
		inline ~slim_read_write_lock(void) noexcept = default;

		inline void acquire_exclusive(void) noexcept {
			::AcquireSRWLockExclusive(&_srwlock);
		}
		inline bool try_acquire_exclusive(void) noexcept {
			return ::TryAcquireSRWLockExclusive(&_srwlock);
		}
		inline void acquire_shared(void) noexcept {
			::AcquireSRWLockShared(&_srwlock);
		}
		inline bool try_acquire_shared(void) noexcept {
			return ::TryAcquireSRWLockShared(&_srwlock);
		}
		inline void release_exclusive(void) noexcept {
#pragma warning(suppress: 26110)
			::ReleaseSRWLockExclusive(&_srwlock);
		}
		inline void release_shared(void) noexcept {
			::ReleaseSRWLockShared(&_srwlock);
		}
		inline auto data(void) noexcept -> SRWLOCK& {
			return _srwlock;
		}
	};
}