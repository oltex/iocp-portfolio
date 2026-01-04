#pragma once
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <mmsystem.h>
#pragma comment(lib, "Winmm.lib")
#include <iomanip>

namespace library {

	//multimidia
	inline void time_begin_period(unsigned int const peroid) noexcept {
		::timeBeginPeriod(peroid);
	}
	inline void time_end_period(unsigned int const peroid) noexcept {
		::timeEndPeriod(peroid);
	}
	inline auto time_get_time(void) noexcept -> unsigned long {
		return ::timeGetTime();
	}

	//system
	inline auto get_tick_count(void) noexcept -> unsigned long {
#pragma warning(suppress: 28159)
		return ::GetTickCount();
	}
	inline auto get_tick_count64(void) noexcept -> unsigned long long {
		return ::GetTickCount64();
	}

	//unix timestamp
	inline auto time32(void) noexcept -> __time32_t {
		__time32_t time_t;
		::_time32(&time_t);
		return time_t;
	}
	inline auto time64(void) noexcept -> __time64_t {
		__time64_t time_t;
		::_time64(&time_t);
		return time_t;
	}
	inline auto local_time(__time64_t time_t) noexcept -> std::tm {
		std::tm tm;
		_localtime64_s(&tm, &time_t);
		return tm;
	}

	class date final {
		std::tm _tm;
	public:
		inline explicit date(void) noexcept = default;
		inline explicit date(std::tm tm) noexcept
			: _tm(tm) {
		};
		inline explicit date(date const& rhs) noexcept
			: _tm(rhs._tm) {
		};
		inline explicit date(date&& rhs) noexcept
			: _tm(rhs._tm) {
		}
		inline auto operator=(date const& rhs) noexcept -> date& {
			_tm = rhs._tm;
		}
		inline auto operator=(date&& rhs) noexcept -> date& {
			_tm = rhs._tm;
		}
		inline ~date(void) noexcept = default;

		inline auto put_time(char const* const format) const noexcept {
			return std::put_time(&_tm, format);
		}
		inline auto year(void) const noexcept -> int {
			return _tm.tm_year - 100;
		}
		inline auto month(void) const noexcept -> int {
			return _tm.tm_mon + 1;
		}
		inline auto day_of_year(void) const noexcept -> int {
			return _tm.tm_yday + 1;
		}
		inline auto day_of_month(void) const noexcept -> int {
			return _tm.tm_mday;
		}
		inline auto day_of_week(void) const noexcept -> int {
			return _tm.tm_wday;
		}
		inline auto hour(void) const noexcept -> int {
			return _tm.tm_hour;
		}
		inline auto minute(void) const noexcept -> int {
			return _tm.tm_min;
		}
		inline auto second(void) const noexcept -> int {
			return _tm.tm_sec;
		}
	};

	class time final {
		inline static LARGE_INTEGER _frequency = []() {
			LARGE_INTEGER frequency;
			::QueryPerformanceFrequency(&frequency);
			return frequency;
			}();
			LARGE_INTEGER _counter;
	public:
		inline explicit time(void) noexcept = default;
		inline explicit time(LARGE_INTEGER counter) noexcept
			: _counter(counter) {
		}
		inline explicit time(LONGLONG quad_part) noexcept
			: _counter{ .QuadPart = quad_part } {
		}
		inline explicit time(time const& rhs) noexcept
			: _counter(rhs._counter) {
		};
		inline explicit time(time&& rhs) noexcept
			: _counter(rhs._counter) {
		}
		inline auto operator=(time const& rhs) noexcept -> time& {
			_counter = rhs._counter;
			return *this;
		}
		inline auto operator=(time&& rhs) noexcept -> time& {
			_counter = rhs._counter;
			return *this;
		}
		inline ~time(void) noexcept = default;

		inline bool operator<(time const& rhs) const noexcept {
			return _counter.QuadPart < rhs._counter.QuadPart;
		}
		inline bool operator>(time const& rhs) const noexcept {
			return _counter.QuadPart > rhs._counter.QuadPart;
		}
		inline bool operator==(time const& rhs) const noexcept {
			return _counter.QuadPart == rhs._counter.QuadPart;
		}
		inline auto operator-(time const& rhs) noexcept -> double {
			return (_counter.QuadPart - rhs._counter.QuadPart) / static_cast<double>(_frequency.QuadPart);
		}
		inline auto operator-=(double duration) noexcept -> time& {
			_counter.QuadPart -= static_cast<LONGLONG>(duration * _frequency.QuadPart);
			return *this;
		}
		inline auto operator-(double duration) const noexcept -> time {
			return time(_counter.QuadPart - static_cast<LONGLONG>(duration * _frequency.QuadPart));
		}
		inline auto operator+=(double duration) noexcept -> time& {
			_counter.QuadPart += static_cast<LONGLONG>(duration * _frequency.QuadPart);
			return *this;
		}
		inline auto operator+(double duration) const noexcept -> time {
			return time(_counter.QuadPart + static_cast<LONGLONG>(duration * _frequency.QuadPart));
		}
		inline auto data(void) noexcept -> LARGE_INTEGER& {
			return _counter;
		}
	};
	inline auto query_performance_count(void) noexcept -> time {
		LARGE_INTEGER _counter;
		::QueryPerformanceCounter(&_counter);
		return time(_counter);
	}
}
