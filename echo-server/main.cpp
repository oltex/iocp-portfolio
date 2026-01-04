#include "library/iocp/scheduler.h"
#include "library/iocp/timer.h"
#include "library/iocp/monitor.h"
#pragma comment(lib, "library/iocp.lib")
#pragma comment(lib, "library/_pdh.lib")
#include "application.h"

int main(void) noexcept {
	iocp::scheduler::construct(8, 8);
	iocp::timer::construct();
	iocp::monitor::construct();
	{
		application app;
		system("pause");
	}
	iocp::monitor::destruct();
	iocp::timer::destruct();
	iocp::scheduler::destruct();
	return 0;
}