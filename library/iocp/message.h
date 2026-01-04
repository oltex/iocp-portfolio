#pragma once
#include "../array.h"
#include "../intrusive/pointer.h"
#include "../thread-local/pool.h"
#include "../serialize.h"
#include <cassert>

namespace iocp {
	class buffer final : public library::intrusive::pointer_hook<0>, public library::array<unsigned char, 1024> {
	public:
		inline buffer(void) noexcept = default;
		inline buffer(buffer const&) noexcept = delete;
		inline buffer(buffer&&) noexcept = delete;
		inline auto operator=(buffer const&) noexcept -> buffer & = delete;
		inline auto operator=(buffer&&) noexcept -> buffer & = delete;
		inline ~buffer(void) noexcept = default;

		template<size_t index>
		inline static void deallocate(buffer* pointer) noexcept {};
	};

	class message final : public library::serialize_view {
		using base = library::serialize_view;
		library::intrusive::share_pointer<buffer, 0> _pointer;
	public:
		inline message(void) noexcept
			: _pointer(), base(nullptr, 0) {
		}
		inline message(buffer* pointer) noexcept
			: _pointer(pointer), base(pointer->data(),pointer->size()) {
		}
		inline message(message const& rhs) noexcept = default;
		inline message(message& message, byte* array, size_type capacity) noexcept
			: _pointer(message._pointer), base(array, capacity) {
		};
		inline message(message& message, byte* array, size_type front, size_type rear, size_type capacity) noexcept
			: _pointer(message._pointer), base(array, front, rear, capacity) {
		};
		inline message(message&& rhs) noexcept = default;
		inline auto operator=(message const& rhs) noexcept -> message& = default;
		inline auto operator=(message&& rhs) noexcept -> message & = default;
		inline ~message(void) noexcept = default;
	};

	class message_pool final : public library::_thread_local::singleton<message_pool> {
		friend class library::_thread_local::singleton<message_pool>;
		using size_type = unsigned int;
		inline static size_type _size = 0;
		message _message;

		inline message_pool(void) noexcept
			: _message(library::_thread_local::pool<buffer>::instance().allocate()) {
		};
		inline message_pool(message_pool const&) noexcept = delete;
		inline message_pool(message_pool&&) noexcept = delete;
		inline auto operator=(message_pool const&) noexcept -> message_pool & = delete;
		inline auto operator=(message_pool&&) noexcept -> message_pool & = delete;
		inline ~message_pool(void) noexcept = default;
	public:
		inline auto allocate(size_type const size) noexcept -> message {
			if (_message.remain() < size) {
				library::interlock_increment(_size);
				_message = library::_thread_local::pool<buffer>::instance().allocate();
			}
			message message(_message, _message.data() + _message.rear(), size);
			_message.move_rear(size);
			return message;
		}
		inline void deallocate(buffer* value) noexcept {
			library::interlock_decrement(_size);
			library::_thread_local::pool<buffer>::instance().deallocate(value);
		}
		inline static auto size(void) noexcept -> size_type {
			return _size;
		}
	};

	template<>
	inline static void buffer::deallocate<0>(buffer* pointer) noexcept {
		message_pool::instance().deallocate(pointer);
	};
}
template<>
inline constexpr bool library::memory_copy_safe<iocp::message> = true;