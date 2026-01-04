#include "worker.h"

namespace iocp {
	worker::worker(void) noexcept 
		: _scheduler(scheduler::instance()){
	}
}
