#pragma once
#include "../template.h"
#include <intrin.h>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace library {
	inline void read_barrier(void) noexcept {
		::_ReadBarrier();
	}
	inline void write_barrier(void) noexcept {
		::_WriteBarrier();
	}
	inline void read_write_barrier(void) noexcept {
		::_ReadWriteBarrier();
	}
	inline void load_fence(void) noexcept {
		::_mm_lfence();
	}
	inline void store_fence(void) noexcept {
		::_mm_sfence();
	}
	inline void memory_fence(void) noexcept {
		::_mm_mfence();
	}
	inline void fast_store_fence(void) noexcept {
		::__faststorefence();
	}
	inline void flush(void const* address) noexcept {
		::_mm_clflush(address);
	}
	inline void pause(void) noexcept {
		::_mm_pause();
	}

	template<typename type>
		requires library::any_of_type<library::remove_volatile<type>, unsigned int, int, unsigned long, long, unsigned long long, long long>
	inline auto interlock_bit_test_and_set(type& base, library::type_identity<type> offset) noexcept -> bool {
		if constexpr (4 == sizeof(type))
			return ::_interlockedbittestandset(reinterpret_cast<volatile long*>(&base), static_cast<long>(offset));
		else
			return ::_interlockedbittestandset64(reinterpret_cast<volatile __int64*>(&base), static_cast<__int64>(offset));
	}
	template<typename type>
		requires library::any_of_type<library::remove_volatile<type>, unsigned int, int, unsigned long, long, unsigned long long, long long>
	inline auto interlock_bit_test_and_reset(type& base, library::type_identity<type> offset) noexcept -> bool {
		if constexpr (4 == sizeof(type))
			return ::_interlockedbittestandreset(reinterpret_cast<volatile long*>(&base), static_cast<long>(offset));
		else
			return ::_interlockedbittestandreset64(reinterpret_cast<volatile __int64*>(&base), static_cast<__int64>(offset));
	}

	template<typename type>
		requires library::any_of_type<library::remove_volatile<type>, unsigned char, char, unsigned short, short, unsigned int, int, unsigned long, long, unsigned long long, long long>
	inline auto interlock_and(type& destine, library::type_identity<type> value) noexcept {
		if constexpr (1 == sizeof(type))
			return ::_InterlockedAnd8(reinterpret_cast<volatile char*>(&destine), static_cast<char>(value));
		else if constexpr (2 == sizeof(type))
			return ::_InterlockedAnd16(reinterpret_cast<volatile short*>(&destine), static_cast<short>(value));
		else if constexpr (4 == sizeof(type))
			return ::_InterlockedAnd(reinterpret_cast<volatile long*>(&destine), static_cast<long>(value));
		else
			return ::_InterlockedAnd64(reinterpret_cast<volatile long long*>(&destine), static_cast<long long>(value));
	}
	template<typename type>
		requires library::any_of_type<library::remove_volatile<type>, unsigned char, char, unsigned short, short, unsigned int, int, unsigned long, long, unsigned long long, long long>
	inline auto interlock_or(type& destine, library::type_identity<type> value) noexcept {
		if constexpr (1 == sizeof(type))
			return ::_InterlockedOr8(reinterpret_cast<volatile char*>(&destine), static_cast<char>(value));
		else if constexpr (2 == sizeof(type))
			return ::_InterlockedOr16(reinterpret_cast<volatile short*>(&destine), static_cast<short>(value));
		else if constexpr (4 == sizeof(type))
			return ::_InterlockedOr(reinterpret_cast<volatile long*>(&destine), static_cast<long>(value));
		else
			return ::_InterlockedOr64(reinterpret_cast<volatile long long*>(&destine), static_cast<long long>(value));
	}
	template<typename type>
		requires library::any_of_type<library::remove_volatile<type>, unsigned char, char, unsigned short, short, unsigned int, int, unsigned long, long, unsigned long long, long long>
	inline auto interlock_xor(type& destine, library::type_identity<type> value) noexcept {
		if constexpr (1 == sizeof(type))
			return ::_InterlockedXor8(reinterpret_cast<volatile char*>(&destine), static_cast<char>(value));
		else if constexpr (2 == sizeof(type))
			return ::_InterlockedXor16(reinterpret_cast<volatile short*>(&destine), static_cast<short>(value));
		else if constexpr (4 == sizeof(type))
			return ::_InterlockedXor(reinterpret_cast<volatile long*>(&destine), static_cast<long>(value));
		else
			return ::_InterlockedXor64(reinterpret_cast<volatile long long*>(&destine), static_cast<long long>(value));
	}

	template<typename type>
		requires library::any_of_type<library::remove_volatile<type>, unsigned short, short, unsigned int, int, unsigned long, long, unsigned long long, long long>
	inline auto interlock_increment(type& addend) noexcept -> type {
		if constexpr (2 == sizeof(type))
			return ::_InterlockedIncrement16(reinterpret_cast<volatile short*>(&addend));
		else if constexpr (4 == sizeof(type))
			return ::_InterlockedIncrement(reinterpret_cast<volatile long*>(&addend));
		else
			return ::_InterlockedIncrement64(reinterpret_cast<volatile long long*>(&addend));
	}
	template<typename type>
		requires library::any_of_type<library::remove_volatile<type>, unsigned short, short, unsigned int, int, unsigned long, long, unsigned long long, long long>
	inline auto interlock_decrement(type& addend) noexcept -> type {
		if constexpr (2 == sizeof(type))
			return ::_InterlockedDecrement16(reinterpret_cast<volatile short*>(&addend));
		else if constexpr (4 == sizeof(type))
			return ::_InterlockedDecrement(reinterpret_cast<volatile long*>(&addend));
		else
			return ::_InterlockedDecrement64(reinterpret_cast<volatile long long*>(&addend));
	}
	template<typename type>
		requires (library::any_of_type<library::remove_volatile<type>, unsigned char, char, unsigned short, short, unsigned int, int, unsigned long, long, unsigned long long, long long> || library::pointer_type<type>)
	inline auto interlock_exchange(type& target, library::type_identity<type> value) noexcept -> type {
		if constexpr (library::pointer_type<type>)
			return ::_InterlockedExchangePointer(reinterpret_cast<void* volatile*>(&target), static_cast<void*>(value));
		else if constexpr (1 == sizeof(type))
			return ::_InterlockedExchange8(reinterpret_cast<volatile char*>(&target), static_cast<char>(value));
		else if constexpr (2 == sizeof(type))
			return ::_InterlockedExchange16(reinterpret_cast<volatile short*>(&target), static_cast<short>(value));
		else if constexpr (4 == sizeof(type))
			return ::_InterlockedExchange(reinterpret_cast<volatile long*>(&target), static_cast<long>(value));
		else
			return ::_InterlockedExchange64(reinterpret_cast<volatile long long*>(&target), static_cast<long long>(value));
	}
	template<typename type>
		requires library::any_of_type<library::remove_volatile<type>, unsigned int, int, unsigned long, long, unsigned long long, long long>
	inline auto interlock_exchange_add(type& addend, library::type_identity<type> value) noexcept {
		if constexpr (4 == sizeof(type))
			return ::_InterlockedExchangeAdd(reinterpret_cast<long volatile*>(&addend), static_cast<long>(value));
		else
			return ::_InterlockedExchangeAdd64(reinterpret_cast<long long volatile*>(&addend), static_cast<long long>(value));
	}
	template<typename type>
		requires (library::any_of_type<library::remove_volatile<type>, unsigned short, short, unsigned int, int, unsigned long, long, unsigned long long, long long> || library::pointer_type<type>)
	inline auto interlock_compare_exhange(type& destine, library::type_identity<type> exchange, library::type_identity<type> compare) noexcept -> type {
		if constexpr (library::pointer_type<type>)
			return reinterpret_cast<type>(::_InterlockedCompareExchangePointer(reinterpret_cast<void* volatile*>(&destine), reinterpret_cast<void*>(exchange), reinterpret_cast<void*>(compare)));
		else if constexpr (2 == sizeof(type))
			return ::_InterlockedCompareExchange16(reinterpret_cast<short volatile*>(&destine), static_cast<short>(exchange), static_cast<short>(compare));
		else if constexpr (4 == sizeof(type))
			return ::_InterlockedCompareExchange(reinterpret_cast<long volatile*>(&destine), static_cast<long>(exchange), static_cast<long>(compare));
		else
			return ::_InterlockedCompareExchange64(reinterpret_cast<long long volatile*>(&destine), static_cast<long long>(exchange), static_cast<long long>(compare));
	}
	template<typename type>
	inline auto interlock_compare_exhange128(type& destine, library::type_identity<type>& exchange, library::type_identity<type> compare) noexcept -> bool {
		return ::_InterlockedCompareExchange128(reinterpret_cast<__int64 volatile*>(&destine), reinterpret_cast<__int64 const*>(&exchange)[1], reinterpret_cast<__int64 const*>(&exchange)[0], reinterpret_cast<__int64*>(&compare));
	}
}