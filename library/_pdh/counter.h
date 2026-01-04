#pragma once
#include "library/template.h"
#include "library/memory.h"
#pragma comment(lib,"pdh.lib")
#define WIN32_LEAN_AND_MEAN
#include <Pdh.h>

namespace pdh {
	class counter final {
		PDH_HCOUNTER _counter;
	public:
		counter(PDH_HCOUNTER counter) noexcept;
		counter(counter const&) noexcept = default;
		counter(counter&& rhs) noexcept;
		auto operator=(counter const&) noexcept -> counter & = default;
		auto operator=(counter&& rhs) noexcept -> counter&;
		~counter(void) noexcept;

		template<typename type>
			requires library::any_of_type<type, long, long long, float, double>
		auto get_format_value(unsigned long format = 0) noexcept -> type {
			PDH_FMT_COUNTERVALUE value;
			if constexpr (library::same_type<type, long>) {
				::PdhGetFormattedCounterValue(_counter, PDH_FMT_LONG | format, nullptr, &value);
				return value.longValue;
			}
			else if constexpr (library::same_type<type, long long>) {
				::PdhGetFormattedCounterValue(_counter, PDH_FMT_LARGE | format, nullptr, &value);
				return value.largeValue;
			}
			else {
				::PdhGetFormattedCounterValue(_counter, PDH_FMT_DOUBLE | format, nullptr, &value);
				return library::cast<type>(value.doubleValue);
			}
		};
		auto data(void) noexcept -> PDH_HCOUNTER&;
	};
}