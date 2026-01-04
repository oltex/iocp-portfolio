#include "scheduler.h"
#include "task.h"
#include "worker.h"

namespace iocp {
	scheduler::scheduler(size_type concurrent, size_type worker) noexcept
		: _io_complet_port(concurrent), _worker_thread(worker, &scheduler::execute, 0, this) {
	}
	scheduler::~scheduler(void) noexcept {
		for (auto& iter : _worker_thread)
			_io_complet_port.post_queue_state(0, 0, nullptr);
		library::handle::wait_for_multiple<library::thread>(_worker_thread, true, INFINITE);
	}

	void scheduler::connect(worker& worker, library::socket& socket, uintptr_t key) noexcept {
		_io_complet_port.connect(socket, (static_cast<unsigned long long>(key_type::worker) << 56) | (key << 47) | reinterpret_cast<uintptr_t>(&worker));
	}
	void scheduler::post(worker& worker, unsigned long transferred, uintptr_t key, OVERLAPPED* overlapped) noexcept {
		_io_complet_port.post_queue_state(transferred, (key << 47) | reinterpret_cast<uintptr_t>(&worker), overlapped);
	}
	void scheduler::post(task& arg) noexcept {
		_io_complet_port.post_queue_state(0, static_cast<unsigned long long>(key_type::task) << 56, reinterpret_cast<OVERLAPPED*>(&arg));
	}
	void scheduler::execute(void) noexcept {
		for (;;) {
			auto [result, transferred, key, overlapped] = _io_complet_port.get_queue_state(INFINITE);
			switch (static_cast<key_type>(key >> 56)) {
			case key_type::close:
				return;
			case key_type::worker:
				reinterpret_cast<worker*>(0x00007FFFFFFFFFFFULL & key)->execute(result, transferred, (0x00FF800000000000ULL & key) >> 47, overlapped);
				break;
			case key_type::task: {
				auto result = reinterpret_cast<task*>(overlapped);
				result->execute();
			} break;
			}
		}
	}
};