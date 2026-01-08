#pragma once
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#include <intrin.h>
#include <eh.h>

#include <exception>
#include <new>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
//#ifdef _DEBUG
//	#define new new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
//#endif
//#include <DbgHelp.h>
//#pragma comment(lib, "dbghelp.lib")

namespace library {
	inline void debug_break(void) noexcept {
		::__debugbreak();
	}

	inline auto crt_set_debug_flag(void) noexcept {
		_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	}
	inline auto crt_dump_memory_leak(void) noexcept {
		_CrtDumpMemoryLeaks();
	}
	inline auto crt_set_break_allocate(long const new_value) noexcept {
		_CrtSetBreakAlloc(new_value);
	}
	inline auto crt_set_report_mode(int const type, int const mode) noexcept {
		_CrtSetReportMode(type, mode);
	}
	inline auto crt_set_report_hook(_CRT_REPORT_HOOK hook) noexcept {
		_CrtSetReportHook(hook);
	}
	inline auto crt_check_memory(void) noexcept -> int {
		return _CrtCheckMemory();
	}
	//inline auto crt_is_valid_heap_pointer(void const* pointer) noexcept -> int {
	//	return ::_CrtIsValidHeapPointer(pointer);
	//}
	//_CrtSetReportHook2();
	//_CrtSetReportHookW2();

	inline auto set_invalid_parameter_handle(_invalid_parameter_handler handle) noexcept {
		::_set_invalid_parameter_handler(handle);
	}
	inline auto set_thread_local_invalid_parameter_handle(_invalid_parameter_handler handle) noexcept {
		::_set_thread_local_invalid_parameter_handler(handle);
	}
	inline auto set_purecall_handler(_purecall_handler handle) noexcept {
		::_set_purecall_handler(handle);
	}
	inline auto set_abort_behavior(unsigned int const flag, unsigned int const mask) noexcept {
		::_set_abort_behavior(flag, mask);
	}
	inline auto set_se_translator(_se_translator_function function) noexcept {
		::_set_se_translator(function);
	}

	inline auto set_terminate(std::terminate_handler handle) noexcept {
		std::set_terminate(handle);
	}
	inline auto set_new_handler(std::new_handler handle) noexcept {
		std::set_new_handler(handle);
	}

	inline auto set_unhandle_except_filter(LPTOP_LEVEL_EXCEPTION_FILTER filer) noexcept {
		::SetUnhandledExceptionFilter(filer);
	}
}