#pragma once
#include "../array.h"
#include "../intrusive/pointer.h"
#include "../thread-local/pool.h"
#include "../serialize.h"
#include <cassert>

namespace iocp {
	class buffer final : public library::intrusive::pointer_hook<0>, public library::array<unsigned char, 1024> {
	public:
		buffer(void) noexcept = default;
		buffer(buffer const&) noexcept = delete;
		buffer(buffer&&) noexcept = delete;
		auto operator=(buffer const&) noexcept -> buffer & = delete;
		auto operator=(buffer&&) noexcept -> buffer & = delete;
		~buffer(void) noexcept = default;

		template<size_t index>
		inline static void deallocate(buffer* pointer) noexcept {};
	};
	class message final : public library::serialize_view {
		using base = library::serialize_view;
		library::intrusive::share_pointer<buffer, 0> _pointer;
	public:
		message(void) noexcept;
		message(buffer* pointer) noexcept;
		message(message& message, byte* array, size_type capacity) noexcept;
		message(message& message, byte* array, size_type front, size_type rear, size_type capacity) noexcept;
		message(message const&) noexcept = default;
		message(message&&) noexcept = default;
		auto operator=(message const&) noexcept -> message & = default;
		auto operator=(message&&) noexcept -> message & = default;
		~message(void) noexcept = default;
	};
	class message_pool final : public library::_thread_local::singleton<message_pool> {
		friend class library::_thread_local::singleton<message_pool>;
		using size_type = unsigned int;
		message _message;

		message_pool(void) noexcept;
		message_pool(message_pool const&) noexcept = delete;
		message_pool(message_pool&&) noexcept = delete;
		auto operator=(message_pool const&) noexcept -> message_pool & = delete;
		auto operator=(message_pool&&) noexcept -> message_pool & = delete;
		~message_pool(void) noexcept = default;
	public:
		auto allocate(size_type const size) noexcept -> message;
		void deallocate(buffer* value) noexcept;
	};
	template<>
	inline static void buffer::deallocate<0>(buffer* pointer) noexcept {
		message_pool::instance().deallocate(pointer);
	};

	struct header final {
		//unsigned char _magic_munber;
		unsigned short _size;
		unsigned short _check_sum;
		unsigned short _seed;

		auto initialize(unsigned short size) noexcept -> bool;
		void encrypt(unsigned long long fixed, unsigned char* buffer) noexcept;
		auto decrypt(unsigned long long fixed, unsigned char* buffer) const noexcept -> bool;
	};
}
template<>
inline constexpr bool library::memory_copy_safe<iocp::message> = true;