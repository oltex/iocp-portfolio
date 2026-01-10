#pragma once
#include "promise.h"
#include "../lock.h"
#include "../singleton.h"
#include <stop_token>
#include <atomic>

namespace iocp {
	class monitor : public library::singleton<monitor, true> {
		friend class library::singleton<monitor, true>;
		std::wstring _process_name;
		std::stop_source _stop_source;
		std::atomic<int> _stop_count;

		struct processor {
			//float _time;
			//float _utility;
			float _user_time;
			float _privilege_time;
			//float _privilege_utility;

			float _interrupt_time;
			//float _interrupt_sec;
			//float _clock_interrupt_sec;
			float _dpc_time;
			//float _dpc_queue_sec;
			//long _dpc_rate;
		} _processor;
		struct process {
			float _user_time;
			float _privilege_time;
			long _thread_count;
			long _handle_count;
			struct {
				long long _working_set_private;
			} _physics;
			struct {
				long long _private_byte;
				//long long _pool_page_byte;
				long long _pool_nonpage_byte;
			} _virtual;
			float _page_fault_sec;
		}_process;
		struct system {
			//long _process;
			//long _thread;
			long _processor_queue_len;
			float _context_switch_sec;
			float _system_call_sec;
			float _except_dispatch_sec;
			//struct {
			//	float _control_operate_sec;
			//	double _control_byte_sec;
			//	float _read_operate_sec;
			//	double _read_byte_sec;
			//	float _write_operate_sec;
			//	double _write_byte_sec;
			//} _file;
		} _system;
		struct memory {
			struct {
				long long _avail_byte;
				//long long _standby_core_byte;
				//long long _standby_normal_byte;
				//long long _standby_reserve_byte;
				//long long _free_zero_byte;
				//long long _cache_byte;
				//long long _system_code_resident_byte;
				//long long _system_driver_resident_byte;
				//long long _system_cache_resident_byte;
				//long long _pool_page_resident_byte;
				//long long _modify_page_list_byte;
			} _physics;
			struct {
				long long _commit_byte;
				long long _commit_limit;
				float _commit_byte_in_use;
				//long long _pool_page_byte;
				long long _pool_nonpage_byte;
			} _virtual;
			struct {
				float _page_sec;
				//float _page_Input_sec;
				//float _page_read_sec;
				//float _page_output_sec;
				//float _page_write_sec;

				//float _page_fault_sec;
				//float _transit_fault_sec;
				//float _demand_zero_faults_sec;
				//float _write_copy_sec;
			} _io;
		} _memory;
		struct tcpv4 {
			struct {
				//long long _active;
				//long long _passive;
				//long long _fail;
				//long long _reset;
				long _establish;
			} _connect;
			struct {
				float _receive_sec;
				float _sent_sec;
				float _retransmit_sec;
			} _segment;
		} _tcpv4;
		struct ipv4 {
			struct {
				float _receive_sec;
				long long _receive_discard;
				//long long _receive_header_error;
				//long long _receive_address_error;
				//long long _receive_unknown_protocol;

				float _sent_sec;
				long long _outbound_discard;
				//long long _outbound_no_route;
			} _datagram;
			struct {
				float _receive_sec;
				//float _reassemble_sec;
				//long long _reassembly_fail;

				float _datagram_sec;
				//float _creat_sec;
				//long long _fail;
			} _fragment;
		} _ipv4;
		struct nic {
			//long long nic_current_bandwidth;
			long nic_output_queue_len;
			struct {
				float _receive_sec;
				long long _receive_discard;
				long long _receive_error;
				//long long _receive_unknown;
				float _sent_sec;
				long long _outbound_discard;
				long long _outbound_error;
			} _packet;
			struct {
				double _receive_sec;
				double _sent_sec;
			} _byte;
		} _nic;
		library::seq_lock _processor_lock;
		library::seq_lock _process_lock;
		library::seq_lock _system_lock;
		library::seq_lock _memory_lock;
		library::seq_lock _tcpv4_lock;
		library::seq_lock _ipv4_lock;
		library::seq_lock _nic_lock;

		monitor(std::wstring_view process_name) noexcept;
		monitor(monitor const&) noexcept = delete;
		monitor(monitor&&) noexcept = delete;
		auto operator=(monitor const&) noexcept -> monitor & = delete;
		auto operator=(monitor&&) noexcept -> monitor & = delete;
		~monitor(void) noexcept;

		auto collect(void) noexcept -> coroutine<void>;
	public:
		auto get_processor(void) noexcept -> processor;
		auto get_process(void) noexcept -> process;
		auto get_system(void) noexcept -> system;
		auto get_memory(void) noexcept -> memory;
		auto get_tcpv4(void) noexcept -> tcpv4;
		auto get_ipv4(void) noexcept -> ipv4;
		auto get_nic(void) noexcept -> nic;
	};
}