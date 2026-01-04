#include "tcp/network.h"
#include "rudp/network.h"
#include "monitor.h"
#include "timer.h"
//#include "scheduler.h"
//#include "promise.h"
//#include "timer.h"
//#include "lock.h"
#include "library/debug.h"
#include "library/system/event.h"
#include <mutex>

//#define MAX_TRHEAD 4
//#define MAX_LOOP 300000
//#define MIN_OVERLAPPED 1
//#define MAX_OVERLAPPED 5
//#define MAX_SOURCE 10 //under 32
//#define ACTIVE_TIME 1/*00000*/
//static unsigned int _count = 0;
//alignas(64) int _source[MAX_SOURCE];
//alignas(64) int _source_share[MAX_SOURCE];
//iocp::mutex _mutex[MAX_SOURCE];
//std::mutex std_mutex[MAX_SOURCE];
//library::event _event;

#pragma region resource
//void func(int thread_index) noexcept {
//	int sleeptime = 0;
//	srand(thread_index);
//	for (auto loop = 0; loop < MAX_LOOP; ++loop) {
//		unsigned int use_count = library::maximum(MIN_OVERLAPPED, (1 + rand() % MAX_OVERLAPPED));
//		library::vector<iocp::proxy> proxys;
//		library::vector<unsigned long long> indices;
//		indices.reserve(use_count);
//		unsigned int used_mask = 0;
//		while (static_cast<unsigned int>(indices.size()) < use_count) {
//			auto index = rand() % MAX_SOURCE;
//			unsigned int bit = 1u << index;
//			if (used_mask & bit)
//				continue;
//			used_mask |= bit;
//			proxys.emplace_back(rand() % 2 == 0 ? iocp::exclude(_mutex[index]) : iocp::share(_mutex[index]));
//			indices.emplace_back(index);
//		}
//		[](library::vector<iocp::proxy> proxys, library::vector<unsigned long long> indices) noexcept -> iocp::coroutine<void> {
//			co_await iocp::lock(proxys);
//			for (auto index = 0u; index < indices.size(); ++index) {
//				if (iocp::mutex::mode::exclude == proxys[index]._mode) {
//					auto result = ++_source[indices[index]];
//					if (result != 1)
//						__debugbreak();
//				}
//				else {
//					++_source_share[indices[index]];
//				}
//			}
//			for (auto active = 0; active < ACTIVE_TIME; ++active) {
//			}
//			for (auto index = 0; index < indices.size(); ++index) {
//				if (iocp::mutex::mode::exclude == proxys[index]._mode) {
//					auto result = --_source[indices[index]];
//					if (result != 0)
//						__debugbreak();
//				}
//				else {
//					--_source_share[indices[index]];
//				}
//			}
//			iocp::unlock(proxys);
//
//			auto res = co_await[]() -> iocp::coroutine<int> {
//				auto sum = 0;
//				auto fork = iocp::fork(3);
//				for (auto i = 0; i < 3; ++i) {
//					[](iocp::fork& fork, int& sum) noexcept -> iocp::coroutine<void> {
//						library::interlock_increment(sum);
//						fork.join();
//						co_return;
//						}(fork, sum);
//				}
//				co_await fork;
//				if (4 != library::interlock_increment(sum))
//					__debugbreak();
//				co_return 10;
//			}();
//
//
//			char buffer[256];
//			auto n = library::interlock_increment(_count);
//			if (MAX_TRHEAD * MAX_LOOP == n)
//				_event.set();
//			int len = std::snprintf(buffer, sizeof(buffer), "%d:", n);
//			for (auto index2 : indices)
//				len += std::snprintf(buffer + len, sizeof(buffer) - len, " %lld", index2);
//			std::snprintf(buffer + len, sizeof(buffer) - len, "\n");
//			std::fputs(buffer, stdout);
//			co_return;
//			}(proxys, indices);
//	}
//}
#pragma endregion

#pragma region enter func
//auto func2(void) noexcept -> iocp::coroutine {
//	int a = 10;
//	co_await iocp::lock(1);
//	co_return;
//}
//
//auto func1(void) noexcept -> iocp::coroutine {
//	int a = 10;
//
//	co_await iocp::lock(1);
//	co_await func2();
//	int b = 10;
//	co_return;
//}
#pragma endregion 

#pragma region fork join
//auto fork_function(void) noexcept -> iocp::coroutine {
//	auto wait = iocp::wait(1);
//	int test = 0;
//	std::printf("Hello\n");
//	[](iocp::wait& wait) noexcept -> iocp::coroutine {
//		std::printf("World\n");
//		wait.execute();
//		co_return;
//		}(wait);
//
//		co_await wait;
//		if (1 != ++test)
//			__debugbreak();
//		std::printf("Good\n");
//		co_return;
//}
#pragma endregion

#pragma region performance
//void multi(int thread_index) noexcept {
//	srand(thread_index);
//	for (auto loop = 0; loop < MAX_LOOP; ++loop) {
//		unsigned int use_count = library::maximum(MIN_OVERLAPPED, (1 + rand() % MAX_OVERLAPPED));
//		library::vector<iocp::proxy> proxys;
//		std::list<unsigned long> indices;
//		unsigned int used_mask = 0;
//		while (static_cast<unsigned int>(indices.size()) < use_count) {
//			auto index = rand() % MAX_SOURCE;
//			unsigned int bit = 1u << index;
//			if (used_mask & bit)
//				continue;
//			used_mask |= bit;
//			proxys.emplace_back(iocp::exclude(_mutex[index]));
//			indices.emplace_back(index);
//		}
//		indices.sort();
//
//		[](library::vector<iocp::proxy> proxys, std::list<unsigned long> indices) noexcept -> iocp::coroutine {
//			//for (auto iter: indices) 
//			//	std_mutex[iter].lock();
//			co_await iocp::lock(proxys);
//			for (auto index2 : indices) {
//				auto result = ++_source[index2];
//				if (result != 1)
//					__debugbreak();
//			}
//			for (auto active = 0; active < ACTIVE_TIME; ++active) {
//			}
//			for (auto index2 : indices) {
//				auto result = --_source[index2];
//				if (result != 0)
//					__debugbreak();
//			}
//			//for (auto iter : indices)
//				//std_mutex[iter].unlock();
//			co_await iocp::unlock();
//
//			if (MAX_TRHEAD * MAX_LOOP == library::interlock_increment(_count))
//				_event.set();
//			co_return;
//			}(proxys, indices);
//	}
//}
//void single(int thread_index) noexcept {
//	srand(thread_index);
//	for (auto loop = 0; loop < MAX_LOOP; ++loop) {
//		unsigned int use_count = library::maximum(MIN_OVERLAPPED, (1 + rand() % MAX_OVERLAPPED));
//		library::vector<unsigned long long> indices;
//		indices.reserve(use_count);
//		unsigned int used_mask = 0;
//		while (static_cast<unsigned int>(indices.size()) < use_count) {
//			auto index = rand() % MAX_SOURCE;
//			unsigned int bit = 1u << index;
//			if (used_mask & bit)
//				continue;
//			used_mask |= bit;
//			indices.emplace_back(index);
//		}
//
//		for (auto index2 : indices) {
//			auto result = ++_source[index2];
//			if (result != 1)
//				__debugbreak();
//		}
//		for (auto active = 0; active < ACTIVE_TIME; ++active) {
//		}
//		for (auto index2 : indices) {
//			auto result = --_source[index2];
//			if (result != 0)
//				__debugbreak();
//		}
//		if (MAX_TRHEAD * MAX_LOOP == library::interlock_increment(_count))
//			_event.set();
//	}
//}
#pragma endregion

#pragma region timer
//void timer(int thread_index) noexcept {
//	[]() ->iocp::coroutine<void> {
//		for (;;) {
//			printf("1000 time\n");
//			co_await iocp::sleep(1000);
//		}
//		co_return;
//		}();
//
//	[]() ->iocp::coroutine<void> {
//		for (;;) {
//			printf("500 time\n");
//			co_await iocp::sleep(500);
//		}
//		co_return;
//		}();
//
//	[]() ->iocp::coroutine<void> {
//		for (;;) {
//			printf("100 time\n");
//			co_await iocp::sleep(100);
//		}
//		co_return;
//		}();
//
//}
#pragma endregion


int main(void) noexcept {
	library::crt_set_debug_flag();
	iocp::scheduler::construct(8, 8);
	iocp::timer::construct();
	iocp::monitor::construct();


	{
		rudp::network network(library::socket_address_ipv4("127.0.0.1", 7000), 65535);
		rudp::network network2(library::socket_address_ipv4("127.0.0.1", 8000), 65535);
		system("pause");
		network2.socket_connect(library::socket_address_ipv4("127.0.0.1", 7000));
		system("pause");
	}

	{
		tcp::network network;
		system("pause");
		network.listen_start(library::socket_address_ipv4("127.0.0.1", 6000), 65535);
		system("pause");

		network.socket_connect(library::socket_address_ipv4("127.0.0.1", 6000));
		system("pause");
		network.session_cancel((65536));
		system("pause");
		network.listen_stop();
	}


	iocp::monitor::destruct();
	Sleep(1000);
	iocp::timer::destruct();
	iocp::scheduler::destruct();
	return 0;
}

//_event.create(false, false);
//auto rdtsc = __rdtsc();
//library::vector<library::thread> _thread;
//for (auto index = 0; index < MAX_TRHEAD; ++index)
//	_thread.emplace_back(func, 0, index);
//_event.wait_for_single(INFINITE);
//rdtsc = __rdtsc() - rdtsc;
//printf("%lld\n", rdtsc);
//system("pause");
//library::handle::wait_for_multiple<library::thread>(_thread, true, INFINITE);


	//library::wsa_start_up();

	//library::socket from_socket;
	//from_socket.create(AF_INET, SOCK_DGRAM, IPPROTO_UDP, WSA_FLAG_OVERLAPPED);
	//from_socket.bind(library::socket_address_ipv4("127.0.0.1", 5000));


	//rudp::network network;
	//system("pause");

	//library::socket to_socket;
	//to_socket.create(AF_INET, SOCK_DGRAM, IPPROTO_UDP, WSA_FLAG_OVERLAPPED);
	//to_socket.bind(library::socket_address_ipv4("127.0.0.1", 5000));

	//char buffer[2000]{};
	//to_socket.send_to(buffer, 1472, 0, library::socket_address_ipv4("127.0.0.1", 6000));

	//system("pause");