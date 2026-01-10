#include "library/iocp/scheduler.h"
#include "library/iocp/timer.h"
#include "library/iocp/monitor.h"
#include "library/iocp/actor/system.h"
#include "library/debug.h"
#pragma comment(lib, "library/iocp.lib")
#pragma comment(lib, "library/_pdh.lib")
#include "application.h"

int main(void) noexcept {
	library::crt_set_debug_flag();
	iocp::scheduler::construct(8, 8);
	iocp::timer::construct();
	iocp::monitor::construct(L"echo-dummy");
	{
		application& app = application::construct();
		app.execute();
		app.destruct();
	}
	iocp::monitor::destruct();
	iocp::timer::destruct();
	iocp::scheduler::destruct();
	return 0;
}