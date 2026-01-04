#include "application.h"

application::application(void) noexcept 
	: _login_network(100) {
	_login_network.listen_start(library::socket_address_ipv4("127.0.0.1", 6000), 65535);
}
