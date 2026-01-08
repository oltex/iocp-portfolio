#include "dummy.h"

auto dummy::callback(actor::job& job) noexcept -> iocp::coroutine<bool> {


	co_return true;
}
