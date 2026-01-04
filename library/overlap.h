#pragma once
#include "memory.h"
//#include "../multi/event.h"
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace library {
	class overlap final {
		_OVERLAPPED _overlapped;
	public:
		inline overlap(void) noexcept = default;
		inline overlap(overlap const&) noexcept = delete;
		inline overlap(overlap&&) noexcept = delete;
		inline auto operator=(overlap const&) noexcept -> overlap & = delete;
		inline auto operator=(overlap&&) noexcept -> overlap & = delete;
		inline ~overlap(void) noexcept = default;

		//inline void set_event(multi::event& event) noexcept {
		//	_overlapped.hEvent = event.data();
		//}
		inline auto get_result(HANDLE handle, unsigned long* byte, bool const wait) noexcept -> bool {
			return ::GetOverlappedResult(handle, &_overlapped, byte, wait);
		}
		inline auto has_completed(void) const noexcept -> bool {
			return HasOverlappedIoCompleted(&_overlapped);
		}
		inline void clear(void) noexcept {
			library::memory_set(&_overlapped, 0, sizeof(_OVERLAPPED));
		}
		inline auto data(void) noexcept -> _OVERLAPPED& {
			return _overlapped;
		}
		inline static auto recover(OVERLAPPED& overlapped) noexcept -> library::overlap& {
			return *reinterpret_cast<overlap*>(reinterpret_cast<unsigned char*>(&overlapped) - offsetof(library::overlap, _overlapped));
		}
	};
}