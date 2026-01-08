#pragma once
#include "library/iocp/actor/entity.h"

class dummy : actor::entity {
public:
	virtual auto callback(actor::job& job) noexcept -> iocp::coroutine<bool> override;
};