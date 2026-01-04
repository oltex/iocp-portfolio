#include "application.h"
#include "library/iocp/monitor.h"
#include "library/iocp/timer.h"

application::application(void) noexcept
	: _echo_network(200) {
	_echo_network.listen_start(library::socket_address_ipv4("127.0.0.1", 6000), 65535);

	_monitor_count.store(1);
	monitor();
}
application::~application(void) noexcept {
	_monitor_source.request_stop();
	for (int stop_count; 0 != (stop_count = _monitor_count.load()); )
		_monitor_count.wait(stop_count);

	_echo_network.listen_stop();
	_echo_network.session_clear();
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
		auto metric = _echo_network.network_metric();

		printf("----------------------------------------------------------------------------\n"\
			"[Processor Monitor]\n"\
			" CPU Layer User Time     : %.2f%%\n"\
			"           Privilege Time: %.2f%%\n"\
			"           Interrupt Time: %.2f%%\n"\
			"           DPC Time      : %.2f%%\n"\
			" System Layer Processor Queue Length: %ld\n"\
			"              Context Switch: %.2f/sec\n"\
			"              System Call: %.2f/sec\n"\
			"              Exception Dispatch: %.2f/sec\n"\
			" Physical Layer Available: %.2f GB\n"\
			" Virtual Layer Commit    : %.2f MB\n"\
			"               Non-Paged : %.2f MB\n"\
			" I/O Layer Page: %.2f/sec\n"
			" TCP Layer Connect Establish : %ld\n"\
			"           Segment Receive   : %.2f/sec\n"\
			"                   Sent      : %.2f\n"\
			"                   Retransmit: %.2f\n"\
			" IP Layer Datagram Receive: %.2f/sec (Discard: %lld)\n"\
			"                   Sent   : %.2f (Discard: %lld)\n"\
			"          Fragment Receive : %.2f/sec\n"\
			"                   Datagram: %.2f\n"\
			" NIC Layer Packet Receive: %.2f/sec (Discard: %lld Error: %lld)\n"\
			"                  Sent   : %.2f (Discard: %lld Error: %lld)\n"\
			"           Byte Receive: %.2f/sec\n"\
			"                Sent   : %.2f\n"\
			"           Output Queue Length: %ld\n"\
			"[Process Moniter]\n"\
			" Thread Count: %ld\n"\
			" Handle Count: %ld\n"\
			" CPU Layer User Time     : %.2f%%\n"\
			"            Privilege Time: %.2f%%\n"\
			" Physical Layer Working Set: %.2f MB \n"\
			" Virtual Layer Private   : %.2f MB\n"\
			"               Non-Paged : %.2f MB\n"\
			" I/O Layer Page Fault: %.2f/sec\n"\
			"[Network Monitor]\n"\
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
			processor._user_time,
			processor._privilege_time,
			processor._interrupt_time,
			processor._dpc_time,

			system._processor_queue_len,
			system._context_switch_sec,
			system._system_call_sec,
			system._except_dispatch_sec,

			memory._physics._avail_byte / (double)0x40000000ull,
			memory._virtual._commit_byte / (double)0x100000ull,
			memory._virtual._pool_nonpage_byte / (double)0x100000ull,
			memory._io._page_sec,

			tcpv4._connect._establish,
			tcpv4._segment._receive_sec,
			tcpv4._segment._sent_sec,
			tcpv4._segment._retransmit_sec,

			ipv4._datagram._receive_sec, ipv4._datagram._receive_discard,
			ipv4._datagram._sent_sec, ipv4._datagram._outbound_discard,
			ipv4._fragment._receive_sec,
			ipv4._fragment._datagram_sec,

			nic._packet._receive_sec, nic._packet._receive_discard, nic._packet._receive_error,
			nic._packet._sent_sec, nic._packet._outbound_discard, nic._packet._outbound_error,
			nic._byte._receive_sec,
			nic._byte._sent_sec,
			nic.nic_output_queue_len,

			process._thread_count,
			process._handle_count,
			process._user_time,
			process._privilege_time,
			process._physics._working_set_private / (double)0x100000ull,
			process._virtual._private_byte / (double)0x100000ull,
			process._virtual._pool_nonpage_byte / (double)0x100000ull,
			process._page_fault_sec,

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
