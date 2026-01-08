#pragma once
#include "task.h"
#include "scheduler.h"
#include "../coroutine.h"

namespace iocp {
	class finalize : public library::awaiter {
		std::coroutine_handle<void> _parent;
	public:
		finalize(std::coroutine_handle<void> parent) noexcept;
		auto await_ready(void) noexcept -> bool;
		auto await_suspend(std::coroutine_handle<void> handle) const noexcept -> std::coroutine_handle<void>;
	};
	template<typename result>
	class yield : public library::awaiter {
		std::coroutine_handle<void> _parent;
		result& _value;
	public:
		yield(std::coroutine_handle<void> parent, result& value) noexcept
			: _parent(parent), _value(value) {
		}
		auto await_suspend(std::coroutine_handle<void>) const noexcept -> std::coroutine_handle<void> {
			return _parent;
		}
		auto await_resume(void) noexcept -> result {
			return std::move(_value);
		}
	};

	template<typename result>
	class promise final : public library::promise<promise<result>>, public task {
		using base = library::promise<promise<result>>;
		template<typename result>
		friend class coroutine;
		std::coroutine_handle<void> _parent = std::noop_coroutine();
		result _value;
	public:
		promise(void) noexcept = default;
		promise(promise const&) noexcept = delete;
		promise(promise&&) noexcept = delete;
		auto operator=(promise const&) noexcept -> promise & = delete;
		auto operator=(promise&&) noexcept -> promise & = delete;
		~promise(void) noexcept = default;

		auto final_suspend(void) noexcept -> finalize {
			return finalize(_parent);
		}
		void return_value(result const& value) noexcept {
			_value = value;
		}
		void return_value(result&& value) noexcept {
			_value = std::move(value);
		}
		auto yield_value(result const& value) noexcept -> yield<result> {
			_value = value;
			return yield<result>(_parent, _value);
		}
		auto yield_value(result&& value) noexcept -> yield<result> {
			_value = std::move(value);
			return yield<result>(_parent, _value);
		}
		virtual void execute(void) noexcept override {
			base::handle().resume();
		}
	};
	template<>
	class promise<void> : public library::promise<promise<void>>, public task {
		template<typename result>
		friend class coroutine;
		std::coroutine_handle<void> _parent = std::noop_coroutine();
	public:
		promise(void) noexcept = default;
		promise(promise const&) noexcept = delete;
		promise(promise&&) noexcept = delete;
		auto operator=(promise const&) noexcept -> promise & = delete;
		auto operator=(promise&&) noexcept -> promise & = delete;
		~promise(void) noexcept = default;

		auto final_suspend(void) noexcept -> finalize;
		void return_void(void) noexcept {};
		virtual void execute(void) noexcept override;
	};

	template<typename result>
	class coroutine : public library::coroutine<iocp::promise<result>>, public library::awaiter {
		using base = library::coroutine<iocp::promise<result>>;
		using base::base;
		bool _start = true;
	public:
		~coroutine(void) noexcept {
			if (true == _start) {
				if constexpr (!std::is_void_v<result>)
					assert(false);
				auto& promise = base::_handle.promise();
				promise._parent = std::noop_coroutine();
				scheduler::instance().post(promise);
			}
		}

		void auto_start(bool enable) noexcept {
			_start = enable;
		}
		auto await_suspend(std::coroutine_handle<void> handle) noexcept -> std::coroutine_handle<void> {
			auto& current = base::_handle.promise();
			current._parent = handle;
			_start = false;
			return base::_handle;
		}
		auto await_resume(void) noexcept -> result {
			if constexpr (std::is_void_v<result>) {
				base::_handle.destroy();
				return;
			}
			else {
				auto out = std::move(base::_handle.promise()._value);
				if (base::_handle.done())
					base::_handle.destroy();
				return out;
			}
		}
		template<typename type>
			requires (!library::void_type<result>)
		auto operator()(type&& value) noexcept -> coroutine& {
			base::_handle.promise()._value = std::forward<type>(value);
			return *this;
		}
	};
}