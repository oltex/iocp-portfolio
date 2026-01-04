#pragma once
#include "counter.h"
#pragma comment(lib,"pdh.lib")
#define WIN32_LEAN_AND_MEAN
#include <Pdh.h>

namespace pdh {
	class query final {
		PDH_HQUERY _qurey;
	public:
		query(void) noexcept;
		query(query const&) noexcept = delete;
		query(query&&) noexcept = delete;
		auto operator=(query const&) noexcept -> query & = delete;
		auto operator=(query&&) noexcept -> query & = delete;
		~query(void) noexcept;

		auto add_counter(wchar_t const* path) noexcept -> counter;
		void collect_query_data(void) noexcept;
		inline void collect_query_data_ex(void) noexcept {
			//PdhCollectQueryDataEx()
		}
		//inline auto expand_counter_path(std::wstring_view const path) noexcept {
		//}
	};
}