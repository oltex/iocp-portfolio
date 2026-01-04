#pragma once
#include "library/iocp/tcp/network.h"

class login_network : public tcp::network {
public:
	using tcp::network::network;
};