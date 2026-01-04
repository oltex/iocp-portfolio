#pragma once
#include "task.h"
#include "library/container/list.h"
#include "library/container/span.h"
#include "library/container/vector.h"
#include "library/system/coroutine.h"
#include "library/system/lock.h"

namespace iocp {
	struct mutex {
		friend class lock;
		enum class mode : unsigned char { exclude, share };
		struct node final {
			lock* _lock;
			mode _mode;
		};
		library::spin_lock _spin;
		bool _exclusive = false;
		unsigned long _shared = 0;
		library::list<node> _waiter;
	};
	struct proxy {
		mutex* _mutex;
		mutex::mode _mode;
	};
	inline auto exclude(mutex& mutex) noexcept {
		return proxy{ &mutex, mutex::mode::exclude };
	}
	inline auto share(mutex& mutex) noexcept {
		return proxy{ &mutex, mutex::mode::share };
	}
	class lock : public library::awaiter, public task {
	public:
		std::coroutine_handle<void> _handle;
		library::vector<proxy> _proxy;
		unsigned long _dependency = 0;
	public:
		template <typename... argument>
		lock(argument... arg) noexcept {
			_proxy.emplace_back(arg...);
		}
		lock(library::vector<proxy>& arg) noexcept {
			for (auto& iter : arg)
				_proxy.emplace_back(iter);
		}
		auto await_suspend(std::coroutine_handle<void> handle) noexcept -> bool;
		virtual void execute(void) noexcept override;
	};
	extern void unlock(library::vector<proxy>& arg) noexcept;
	class fork : public library::awaiter, public task {
		using size_type = unsigned int;
		std::coroutine_handle<void> _handle;
		size_type _count;
	public:
		fork(size_type count) noexcept;
		auto await_suspend(std::coroutine_handle<void> handle) noexcept -> bool;
		void join(void) noexcept;
		virtual void execute(void) noexcept override;
	};
}