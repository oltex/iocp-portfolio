#pragma once
#include "../inputoutput.h"
#include "../thread.h"
#include "../array.h"
#include "../singleton.h"

namespace iocp {
	class scheduler;
	class task {
	public:
		inline task(void) noexcept = default;
		inline task(task const&) noexcept = default;
		inline task(task&&) noexcept = default;
		inline auto operator=(task const&) noexcept -> task & = default;
		inline auto operator=(task&&) noexcept -> task & = default;
		inline virtual ~task(void) noexcept = default;

		virtual void task_execute(void) noexcept = 0;
	};
	class worker {
	protected:
		scheduler& _scheduler;
	public:
		worker(void) noexcept;
		inline worker(worker const&) noexcept = delete;
		inline worker(worker&&) noexcept = delete;
		inline auto operator=(worker const&) noexcept -> worker & = delete;
		inline auto operator=(worker&&) noexcept -> worker & = delete;
		inline ~worker(void) noexcept = default;

		virtual void worker_execute(bool result, unsigned long transferred, uintptr_t key, OVERLAPPED* overlapped) noexcept = 0;
	};

	class scheduler : public library::singleton<scheduler, true> {
		friend class library::singleton<scheduler, true>;
		using size_type = unsigned int;
		enum class key_type : unsigned char {
			close = 0, worker, task
		};
		library::inputoutput_complet_port _io_complet_port;
		library::array<library::thread, 0> _worker_thread;
	public:
		scheduler(size_type concurrent, size_type worker) noexcept;
		inline scheduler(scheduler const&) noexcept = delete;
		inline scheduler(scheduler&&) noexcept = delete;
		inline auto operator=(scheduler const&) noexcept -> scheduler & = delete;
		inline auto operator=(scheduler&&) noexcept -> scheduler & = delete;
		~scheduler(void) noexcept;

		void connect(worker& worker, library::socket& socket, uintptr_t key) noexcept;
		void post(worker& worker, unsigned long transferred, uintptr_t key, OVERLAPPED* overlapped) noexcept;
		void post(task& arg) noexcept;
		void execute(void) noexcept;
	};
}