#pragma once
#include "library/inputoutput.h"
#include "library/thread.h"
#include "library/array.h"
#include "library/singleton.h"

namespace iocp {
	class worker;
	class task;
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