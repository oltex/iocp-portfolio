#pragma once

namespace iocp {
	class task {
	public:
		inline task(void) noexcept = default;
		inline task(task const&) noexcept = default;
		inline task(task&&) noexcept = default;
		inline auto operator=(task const&) noexcept -> task & = default;
		inline auto operator=(task&&) noexcept -> task & = default;
		inline virtual ~task(void) noexcept = default;

		virtual void execute(void) noexcept = 0;
	};
}