#include "application.h"
#include "library/iocp/monitor.h"
#include "library/iocp/timer.h"
#include <Windows.h>

application::application(void) noexcept
	: _network(200, 0xaabbccddeeff, 128) {
	_monitor_count.store(1);
	monitor();
}
application::~application(void) noexcept {
	_monitor_source.request_stop();
	for (int stop_count; 0 != (stop_count = _monitor_count.load()); )
		_monitor_count.wait(stop_count);
}

void application::execute(void) noexcept {
	_command.add("Listen_Start", [&](std::span<command::parameter> arg) {
		_network.listen_start(library::socket_address_ipv4("127.0.0.1", 6000), SOMAXCONN);
		});
	_command.add("Listen_Stop", [&](std::span<command::parameter> arg) {
		_network.listen_stop();
		});
	_command.add("Session_Clear", [&](std::span<command::parameter> arg) {
		_network.session_clear();
		});
	for (;;) {
		std::string buffer;
		if (!std::getline(std::cin, buffer))
			break;
		parser parser;
		parser.execute(buffer);
		for (auto& iter : parser)
			_command.execute(iter.first, iter.second);
	}
}

auto application::monitor(void) noexcept -> iocp::coroutine<void> {
	auto& monitor = iocp::monitor::instance();

	auto stop_token = _monitor_source.get_token();
	while (false == stop_token.stop_requested()) {
		auto processor = monitor.get_processor();
		auto process = monitor.get_process();
		auto system = monitor.get_system();
		auto memory = monitor.get_memory();
		auto tcpv4 = monitor.get_tcpv4();
		auto ipv4 = monitor.get_ipv4();
		auto nic = monitor.get_nic();
		auto metric = _network.network_metric();

		::system("cls");
		printf("----------------------------------------------------------------------------\n"\
			"[Processor Monitor]\n"\
			" CPU     User Time     : %.2f%%\n"\
			"         Privilege Time: %.2f%%\n"\
			" Memory  Available     : %.2f GB\n"\
			"         Private       : %.2f MB\n"\
			"         Non-Paged     : %.2f MB\n"\
			" Network Receive       : %.2f/sec\n"\
			"         Sent          : %.2f/sec\n"\
			"         Retransmit    : %.2f/sec\n"\
			"[Server Monitor]\n"\
			" Session Layer Active      : %lu\n"\
			"               Total Accept: %llu\n"\
			"               Total Conn  : %llu\n"\
			" Throughput    Accept      : %lu/sec\n"\
			"               Receive     : %lu/sec\n"\
			"               Send        : %lu/sec\n"\
			" Exception     Recv Timeout: %llu\n"\
			"               Recv Header : %llu\n"\
			"               Send Timeout: %llu\n"\
			"               Send BufFull: %llu\n",
			process._user_time,
			process._privilege_time,
			memory._physics._avail_byte / (double)0x40000000ull,
			process._virtual._private_byte / (double)0x100000ull,
			memory._virtual._pool_nonpage_byte / (double)0x100000ull,
			tcpv4._segment._receive_sec,
			tcpv4._segment._sent_sec,
			tcpv4._segment._retransmit_sec,

			metric._session_count,
			metric._accept_total,
			metric._connect_total,
			metric._accept_sec,
			metric._receive_sec,
			metric._send_sec,
			metric._receive_timeout_total,
			metric._header_bytelimit_total,
			metric._send_timeout_total,
			metric._send_bytelimit_total
		);
		co_await iocp::sleep(1000);
	}
	_monitor_count.fetch_sub(1);
	_monitor_count.notify_all();
	co_return;
}
