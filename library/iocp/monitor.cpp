#include "monitor.h"
#include "timer.h"
#include "module/_pdh/query.h"

namespace iocp {
	monitor::monitor(void) noexcept {
		_stop_count.store(1);
		collect();
	}
	monitor::~monitor(void) noexcept {
		_stop_source.request_stop();
		for (int stop_count; 0 != (stop_count = _stop_count.load()); )
			_stop_count.wait(stop_count);
	}


	auto monitor::collect(void) noexcept -> coroutine<void> {
		pdh::query query;
		auto stop_token = _stop_source.get_token();
#pragma region processor
		//auto processor_time = query.add_counter(L"\\Processor Information(_Total)\\% Processor Time");
		//auto processor_utility = query.add_counter(L"\\Processor Information(_Total)\\% Processor Utility");
		auto processor_user_time = query.add_counter(L"\\Processor Information(_Total)\\% User Time");
		auto processor_privilege_time = query.add_counter(L"\\Processor Information(_Total)\\% Privileged Time");
		//auto processor_privilege_utility = query.add_counter(L"\\Processor Information(_Total)\\% Privileged Utility");

		auto processor_interrupt_time = query.add_counter(L"\\Processor Information(_Total)\\% Interrupt Time");
		//auto processor_interrupt_sec = query.add_counter(L"\\Processor Information(_Total)\\Interrupts/sec");
		//auto processor_clock_interrupt_sec = query.add_counter(L"\\Processor Information(_Total)\\Clock Interrupts/sec");
		auto processor_dpc_time = query.add_counter(L"\\Processor Information(_Total)\\% DPC Time");
		//auto processor_dpc_queue_sec = query.add_counter(L"\\Processor Information(_Total)\\DPCs Queued/sec");
		//auto processor_dpc_rate = query.add_counter(L"\\Processor Information(_Total)\\DPC Rate");
		//L"\\Processor Information(_Total)\\Processor Frequency"
		//L"\\Processor Information(_Total)\\% of Maximum Frequency"
#pragma endregion
#pragma region process
		//auto process_time = query.add_counter(L"\\Process(iocp)\\% Processor Time");
		auto process_user_time = query.add_counter(L"\\Process(iocp)\\% User Time");
		auto process_privilege_time = query.add_counter(L"\\Process(iocp)\\% Privileged Time");

		auto process_thread_count = query.add_counter(L"\\Process(iocp)\\Thread Count");
		auto process_handle_count = query.add_counter(L"\\Process(iocp)\\Handle Count");

		auto process_working_set_private = query.add_counter(L"\\Process(iocp)\\Working Set - Private");

		auto process_private_byte = query.add_counter(L"\\Process(iocp)\\Private Bytes");
		//auto process_pool_page_byte = query.add_counter(L"\\Process(iocp)\\Pool Paged Bytes");
		auto process_pool_nonpage_byte = query.add_counter(L"\\Process(iocp)\\Pool Nonpaged Bytes");

		auto process_page_fault = query.add_counter(L"\\Process(iocp)\\Page Faults/sec");
#pragma endregion
#pragma region system
		//auto system_process = query.add_counter(L"\\System\\Processes");
		//auto system_thread = query.add_counter(L"\\System\\Threads");
		auto system_processor_queue_len = query.add_counter(L"\\System\\Processor Queue Length");
		auto system_context_switch_sec = query.add_counter(L"\\System\\Context Switches/sec");
		auto system_system_call_sec = query.add_counter(L"\\System\\System Calls/sec");
		auto system_except_dispatch_sec = query.add_counter(L"\\System\\Exception Dispatches/sec");
		//auto system_file_control_operate_sec = query.add_counter(L"\\System\\File Control Operations/sec");
		//auto system_file_control_byte_sec = query.add_counter(L"\\System\\File Control Bytes/sec");
		//auto system_file_read_operate_sec = query.add_counter(L"\\System\\File Read Operations/sec");
		//auto system_file_read_byte_sec = query.add_counter(L"\\System\\File Read Bytes/sec");
		//auto system_file_write_operate_sec = query.add_counter(L"\\System\\File Write Operations/sec");
		//auto system_file_write_byte_sec = query.add_counter(L"\\System\\File Write Bytes/sec");
#pragma endregion
#pragma region memory
		auto _memory_avail_byte = query.add_counter(L"\\Memory\\Available Bytes");
		//auto _memory_standby_core_byte = query.add_counter(L"\\Memory\\Standby Cache Core Bytes");
		//auto _memory_standby_normal_byte = query.add_counter(L"\\Memory\\Standby Cache Normal Priority Bytes");
		//auto _memory_standby_reserve_byte = query.add_counter(L"\\Memory\\Standby Cache Reserve Bytes");
		//auto _memory_free_zero_byte = query.add_counter(L"\\Memory\\Free & Zero Page List Bytes");
		//auto _memory_cache_byte = query.add_counter(L"\\Memory\\Cache Bytes");
		//auto _memory_system_code_resident_byte = query.add_counter(L"\\Memory\\System Code Resident Bytes");
		//auto _memory_system_driver_resident_byte = query.add_counter(L"\\Memory\\System Driver Resident Bytes");
		//auto _memory_system_cache_resident_byte = query.add_counter(L"\\Memory\\System Cache Resident Bytes");
		//auto _memory_pool_page_resident_byte = query.add_counter(L"\\Memory\\Pool Paged Resident Bytes");
		//auto _memory_modify_page_list_byte = query.add_counter(L"\\Memory\\Modified Page List Bytes");

		auto _memory_commit_byte = query.add_counter(L"\\Memory\\Committed Bytes");
		auto _memory_commit_limit = query.add_counter(L"\\Memory\\Commit Limit");
		auto _memory_commit_byte_in_use = query.add_counter(L"\\Memory\\% Committed Bytes In Use");
		//auto _memory_pool_page_byte = query.add_counter(L"\\Memory\\Pool Paged Bytes");
		auto _memory_pool_nonpage_byte = query.add_counter(L"\\Memory\\Pool Nonpaged Bytes");

		auto _memory_page_sec = query.add_counter(L"\\Memory\\Pages/sec");
		//auto _memory_page_Input_sec = query.add_counter(L"\\Memory\\Pages Input/sec");
		//auto _memory_page_read_sec = query.add_counter(L"\\Memory\\Page Reads/sec");
		//auto _memory_page_output_sec = query.add_counter(L"\\Memory\\Pages Output/sec");
		//auto _memory_page_write_sec = query.add_counter(L"\\Memory\\Page Writes/sec");
		//auto _memory_page_fault_sec = query.add_counter(L"\\Memory\\Page Faults/sec");
		//auto _memory_transit_fault_sec = query.add_counter(L"\\Memory\\Transition Faults/sec");
		//auto _memory_demand_zero_faults_sec = query.add_counter(L"\\Memory\\Demand Zero Faults/sec");
		//auto _memory_write_copy_sec = query.add_counter(L"\\Memory\\Write Copies/sec");

		//L"\\Memory\\Transition Pages Repurposed/sec"
		//L"\\Memory\\Cache Faults/sec"
#pragma endregion
#pragma region tcpv4
		//auto tcpv4_connect_active = query.add_counter(L"\\TCPv4\\Connections Active");
		//auto tcpv4_connect_passive = query.add_counter(L"\\TCPv4\\Connections Passive");
		//auto tcpv4_connect_fail = query.add_counter(L"\\TCPv4\\Connection Failures");
		//auto tcpv4_connect_reset = query.add_counter(L"\\TCPv4\\Connections Reset");
		auto tcpv4_connect_establish = query.add_counter(L"\\TCPv4\\Connections Established");
		auto tcpv4_segment_receive_sec = query.add_counter(L"\\TCPv4\\Segments Received/sec");
		auto tcpv4_segment_sent_sec = query.add_counter(L"\\TCPv4\\Segments Sent/sec");
		auto tcpv4_segment_retransmit_sec = query.add_counter(L"\\TCPv4\\Segments Retransmitted/sec");
#pragma endregion
#pragma region ipv4
		auto ipv4_datagram_receive_sec = query.add_counter(L"\\IPv4\\Datagrams Received/sec");
		auto ipv4_datagram_receive_discard = query.add_counter(L"\\IPv4\\Datagrams Received Discarded");
		//auto ipv4_datagram_receive_header_error = query.add_counter(L"\\IPv4\\Datagrams Received Header Errors");
		//auto ipv4_datagram_receive_address_error = query.add_counter(L"\\IPv4\\Datagrams Received Address Errors");
		//auto ipv4_datagram_receive_unknown_protocol = query.add_counter(L"\\IPv4\\Datagrams Received Unknown Protocol");
		auto ipv4_datagram_sent_sec = query.add_counter(L"\\IPv4\\Datagrams Sent/sec");
		auto ipv4_datagram_outbound_discard = query.add_counter(L"\\IPv4\\Datagrams Outbound Discarded");
		//auto ipv4_datagram_outbound_no_route = query.add_counter(L"\\IPv4\\Datagrams Outbound No Route");
		auto ipv4_fragment_receive_sec = query.add_counter(L"\\IPv4\\Fragments Received/sec");
		//auto ipv4_fragment_reassemble_sec = query.add_counter(L"\\IPv4\\Fragments Re-assembled/sec");
		//auto ipv4_fragment_reassembly_fail = query.add_counter(L"\\IPv4\\Fragment Re-assembly Failures");
		auto ipv4_fragment_datagram_sec = query.add_counter(L"\\IPv4\\Fragmented Datagrams/sec");
		//auto ipv4_fragment_creat_sec = query.add_counter(L"\\IPv4\\Fragments Created/sec");
		//auto ipv4_fragment_fail = query.add_counter(L"\\IPv4\\Fragmentation Failures");
		//\\IPv4\\Datagrams Received Delivered / sec
		//\\IPv4\\Datagrams Forwarded / sec
#pragma endregion
#pragma region nic
		//auto nic_current_bandwidth = query.add_counter(L"\\Network Interface(*)\\Current Bandwidth");
		auto nic_output_queue_len = query.add_counter(L"\\Network Interface(*)\\Output Queue Length");
		auto nic_packet_receive_sec = query.add_counter(L"\\Network Interface(*)\\Packets Received/sec");
		auto nic_packet_receive_discard = query.add_counter(L"\\Network Interface(*)\\Packets Received Discarded");
		auto nic_packet_receive_error = query.add_counter(L"\\Network Interface(*)\\Packets Received Errors");
		//auto nic_packet_receive_unknown = query.add_counter(L"\\Network Interface(*)\\Packets Received Unknown");
		auto nic_packet_sent_sec = query.add_counter(L"\\Network Interface(*)\\Packets Sent/sec");
		auto nic_packet_outbound_discard = query.add_counter(L"\\Network Interface(*)\\Packets Outbound Discarded");
		auto nic_packet_outbound_error = query.add_counter(L"\\Network Interface(*)\\Packets Outbound Errors");
		auto nic_byte_receive_sec = query.add_counter(L"\\Network Interface(*)\\Bytes Received/sec");
		auto nic_byte_sent_sec = query.add_counter(L"\\Network Interface(*)\\Bytes Sent/sec");
		//\\Network Interface(*)\\Packets Received Unicast / sec
		//\\Network Interface(*)\\Packets Received Non - Unicast / sec
		//\\Network Interface(*)\\Packets Sent Unicast / sec
		//\\Network Interface(*)\\Packets Sent Non - Unicast / sec
		//\\Network Interface(*)\\Current Bandwidth
		//\\Network Interface(*)\\Offloaded Connections
#pragma endregion
		while (false == stop_token.stop_requested()) {
			query.collect_query_data();
#pragma region processor
			_processor_lock.write_start();
			//_processor._time = processor_time.get_format_value<float>();
			//_processor._utility = processor_utility.get_format_value<float>();
			_processor._user_time = processor_user_time.get_format_value<float>();
			_processor._privilege_time = processor_privilege_time.get_format_value<float>();
			//_processor._privilege_utility = processor_privilege_utility.get_format_value<float>();
			_processor._interrupt_time = processor_interrupt_time.get_format_value<float>();
			//_processor._interrupt_sec = processor_interrupt_sec.get_format_value<float>();
			//_processor._clock_interrupt_sec = processor_clock_interrupt_sec.get_format_value<float>();
			_processor._dpc_time = processor_dpc_time.get_format_value<float>();
			//_processor._dpc_queue_sec = processor_dpc_queue_sec.get_format_value<float>();
			//_processor._dpc_rate = processor_dpc_rate.get_format_value<long>();
			_processor_lock.write_end();
#pragma endregion
#pragma region process
			_process_lock.write_start();
			_process._user_time = process_user_time.get_format_value<float>(PDH_FMT_NOCAP100);
			_process._privilege_time = process_privilege_time.get_format_value<float>(PDH_FMT_NOCAP100);

			_process._thread_count = process_thread_count.get_format_value<long>();
			_process._handle_count = process_handle_count.get_format_value<long>();

			_process._physics._working_set_private = process_working_set_private.get_format_value<long long>();
			_process._virtual._private_byte = process_private_byte.get_format_value<long long>();
			//_process._virtual._pool_page_byte = process_pool_page_byte.get_format_value<long long>();
			_process._virtual._pool_nonpage_byte = process_pool_nonpage_byte.get_format_value<long long>();
			_process._page_fault_sec = process_page_fault.get_format_value<float>();
			_process_lock.write_end();
#pragma endregion
#pragma region system
			_system_lock.write_start();
			//_system._process = system_process.get_format_value<long>();
			//_system._thread = system_thread.get_format_value<long>();
			_system._processor_queue_len = system_processor_queue_len.get_format_value<long>();
			_system._context_switch_sec = system_context_switch_sec.get_format_value<float>();
			_system._system_call_sec = system_system_call_sec.get_format_value<float>();
			_system._except_dispatch_sec = system_except_dispatch_sec.get_format_value<float>();
			//_system._file._control_operate_sec = system_file_control_operate_sec.get_format_value<float>();
			//_system._file._control_byte_sec = system_file_control_byte_sec.get_format_value<double>();
			//_system._file._read_operate_sec = system_file_read_operate_sec.get_format_value<float>();
			//_system._file._read_byte_sec = system_file_read_byte_sec.get_format_value<double>();
			//_system._file._write_operate_sec = system_file_write_operate_sec.get_format_value<float>();
			//_system._file._write_byte_sec = system_file_write_byte_sec.get_format_value<double>();
			_system_lock.write_end();
#pragma endregion
#pragma region memory
			_memory_lock.write_start();
			_memory._physics._avail_byte = _memory_avail_byte.get_format_value<long long>();
			//_memory._physics._standby_core_byte = _memory_standby_core_byte.get_format_value<long long>();
			//_memory._physics._standby_normal_byte = _memory_standby_normal_byte.get_format_value<long long>();
			//_memory._physics._standby_reserve_byte = _memory_standby_reserve_byte.get_format_value<long long>();
			//_memory._physics._free_zero_byte = _memory_free_zero_byte.get_format_value<long long>();
			//_memory._physics._cache_byte = _memory_cache_byte.get_format_value<long long>();
			//_memory._physics._system_code_resident_byte = _memory_system_code_resident_byte.get_format_value<long long>();
			//_memory._physics._system_driver_resident_byte = _memory_system_driver_resident_byte.get_format_value<long long>();
			//_memory._physics._system_cache_resident_byte = _memory_system_cache_resident_byte.get_format_value<long long>();
			//_memory._physics._pool_page_resident_byte = _memory_pool_page_resident_byte.get_format_value<long long>();
			//_memory._physics._modify_page_list_byte = _memory_modify_page_list_byte.get_format_value<long long>();
			_memory._virtual._commit_byte = _memory_commit_byte.get_format_value<long long>();
			_memory._virtual._commit_limit = _memory_commit_limit.get_format_value<long long>();
			_memory._virtual._commit_byte_in_use = _memory_commit_byte_in_use.get_format_value<float>();
			//_memory._virtual._pool_page_byte = _memory_pool_page_byte.get_format_value<long long>();
			_memory._virtual._pool_nonpage_byte = _memory_pool_nonpage_byte.get_format_value<long long>();
			_memory._io._page_sec = _memory_page_sec.get_format_value<float>();
			//_memory._io._page_Input_sec = _memory_page_Input_sec.get_format_value<float>();
			//_memory._io._page_read_sec = _memory_page_read_sec.get_format_value<float>();
			//_memory._io._page_output_sec = _memory_page_output_sec.get_format_value<float>();
			//_memory._io._page_write_sec = _memory_page_write_sec.get_format_value<float>();
			//_memory._io._page_fault_sec = _memory_page_fault_sec.get_format_value<float>();
			//_memory._io._transit_fault_sec = _memory_transit_fault_sec.get_format_value<float>();
			//_memory._io._demand_zero_faults_sec = _memory_demand_zero_faults_sec.get_format_value<float>();
			//_memory._io._write_copy_sec = _memory_write_copy_sec.get_format_value<float>();
			_memory_lock.write_end();
#pragma endregion
#pragma region tcpv4
			_tcpv4_lock.write_start();
			//_tcpv4._connect._active = tcpv4_connect_active.get_format_value<long long>();
			//_tcpv4._connect._passive = tcpv4_connect_passive.get_format_value<long long>();
			//_tcpv4._connect._fail = tcpv4_connect_fail.get_format_value<long long>();
			//_tcpv4._connect._reset = tcpv4_connect_reset.get_format_value<long long>();
			_tcpv4._connect._establish = tcpv4_connect_establish.get_format_value<long>();
			_tcpv4._segment._receive_sec = tcpv4_segment_receive_sec.get_format_value<float>();
			_tcpv4._segment._sent_sec = tcpv4_segment_sent_sec.get_format_value<float>();
			_tcpv4._segment._retransmit_sec = tcpv4_segment_retransmit_sec.get_format_value<float>();
			_tcpv4_lock.write_end();
#pragma endregion
#pragma region ipv4
			_ipv4_lock.write_start();
			_ipv4._datagram._receive_sec = ipv4_datagram_receive_sec.get_format_value<float>();
			_ipv4._datagram._receive_discard = ipv4_datagram_receive_discard.get_format_value<long long>();
			//_ipv4._datagram._receive_header_error = ipv4_datagram_receive_header_error.get_format_value<long long>();
			//_ipv4._datagram._receive_address_error = ipv4_datagram_receive_address_error.get_format_value<long long>();
			//_ipv4._datagram._receive_unknown_protocol = ipv4_datagram_receive_unknown_protocol.get_format_value<long long>();
			_ipv4._datagram._sent_sec = ipv4_datagram_sent_sec.get_format_value<float>();
			_ipv4._datagram._outbound_discard = ipv4_datagram_outbound_discard.get_format_value<long long>();
			//_ipv4._datagram._outbound_no_route = ipv4_datagram_outbound_no_route.get_format_value<long>();
			_ipv4._fragment._receive_sec = ipv4_fragment_receive_sec.get_format_value<float>();
			//_ipv4._fragment._reassemble_sec = ipv4_fragment_reassemble_sec.get_format_value<float>();
			//_ipv4._fragment._reassembly_fail = ipv4_fragment_reassembly_fail.get_format_value<long long>();
			_ipv4._fragment._datagram_sec = ipv4_fragment_datagram_sec.get_format_value<float>();
			//_ipv4._fragment._creat_sec = ipv4_fragment_creat_sec.get_format_value<float>();
			//_ipv4._fragment._fail = ipv4_fragment_fail.get_format_value<long long>();
			_ipv4_lock.write_end();
#pragma endregion
#pragma region nic
			_nic_lock.write_start();
			//_nic.nic_current_bandwidth = nic_current_bandwidth.get_format_value<long long>();
			_nic.nic_output_queue_len = nic_output_queue_len.get_format_value<long>();
			_nic._packet._receive_sec = nic_packet_receive_sec.get_format_value<float>();
			_nic._packet._receive_discard = nic_packet_receive_discard.get_format_value<long long>();
			_nic._packet._receive_error = nic_packet_receive_error.get_format_value<long long>();
			//_nic._packet._receive_unknown = nic_packet_receive_unknown.get_format_value<long long>();
			_nic._packet._sent_sec = nic_packet_sent_sec.get_format_value<float>();
			_nic._packet._outbound_discard = nic_packet_outbound_discard.get_format_value<long long>();
			_nic._packet._outbound_error = nic_packet_outbound_error.get_format_value<long long>();
			_nic._byte._receive_sec = nic_byte_receive_sec.get_format_value<double>();
			_nic._byte._sent_sec = nic_byte_sent_sec.get_format_value<double>();
			_nic_lock.write_end();
#pragma endregion
			co_await sleep(1000);
		}
		_stop_count.fetch_sub(1);
		_stop_count.notify_all();
		co_return;
	}
	auto monitor::get_processor(void) noexcept -> processor {
		processor result;
		for (;;) {
			auto sequence = _processor_lock.read_start();
			result = _processor;
			if (_processor_lock.read_end(sequence))
				break;
		}
		return result;
	}
	auto monitor::get_process(void) noexcept -> process {
		process result;
		for (;;) {
			auto sequence = _process_lock.read_start();
			result = _process;
			if (_process_lock.read_end(sequence))
				break;
		}
		return result;
	}
	auto monitor::get_system(void) noexcept -> system {
		system result;
		for (;;) {
			auto sequence = _system_lock.read_start();
			result = _system;
			if (_system_lock.read_end(sequence))
				break;
		}
		return result;
	}
	auto monitor::get_memory(void) noexcept -> memory {
		memory result;
		for (;;) {
			auto sequence = _memory_lock.read_start();
			result = _memory;
			if (_memory_lock.read_end(sequence))
				break;
		}
		return result;
	}
	auto monitor::get_tcpv4(void) noexcept -> tcpv4 {
		tcpv4 result;
		for (;;) {
			auto sequence = _tcpv4_lock.read_start();
			result = _tcpv4;
			if (_tcpv4_lock.read_end(sequence))
				break;
		}
		return result;
	}
	auto monitor::get_ipv4(void) noexcept -> ipv4 {
		ipv4 result;
		for (;;) {
			auto sequence = _ipv4_lock.read_start();
			result = _ipv4;
			if (_ipv4_lock.read_end(sequence))
				break;
		}
		return result;
	}
	auto monitor::get_nic(void) noexcept -> nic {
		nic result;
		for (;;) {
			auto sequence = _nic_lock.read_start();
			result = _nic;
			if (_nic_lock.read_end(sequence))
				break;
		}
		return result;
	}
}
