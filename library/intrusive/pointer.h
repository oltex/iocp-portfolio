#pragma once
#include "../function.h"
#include "../memory.h"
#include "../interlock.h"
#include <malloc.h>
#include <utility>
#include <type_traits>

namespace library::intrusive {
	template<typename type, size_t>
	class share_pointer;
	template<typename type, size_t>
	class weak_pointer;
	template<size_t index>
	class pointer_hook {
		using size_type = unsigned int;
		template<typename type, size_t>
		friend class share_pointer;
		template<typename type, size_t>
		friend class weak_pointer;

		template<size_t index>
		inline void destruct(void) noexcept {};
		template<size_t index>
		inline static void deallocate(void* pointer) noexcept {};

		size_type _use;
		size_type _weak;
	public:
		inline explicit pointer_hook(void) noexcept = default;
		inline explicit pointer_hook(pointer_hook const&) noexcept = default;
		inline explicit pointer_hook(pointer_hook&&) noexcept = default;
		inline auto operator=(pointer_hook const&) noexcept -> pointer_hook & = default;
		inline auto operator=(pointer_hook&&) noexcept -> pointer_hook & = default;
		inline ~pointer_hook(void) noexcept = default;

		template<typename type>
		inline auto share_pointer_this(void) noexcept -> share_pointer<type, index>;
	};

	template<typename type, size_t index>
	class share_pointer {
		template<typename other, size_t>
		friend class share_pointer;
		template<typename other, size_t>
		friend class weak_pointer;
		using size_type = unsigned int;
		using hook = pointer_hook<index>;
		//static_assert(std::is_base_of<hook, type>::value);
		hook* _pointer;
	public:
		inline constexpr share_pointer(void) noexcept
			: _pointer(nullptr) {
		}
		inline constexpr share_pointer(nullptr_t) noexcept
			: _pointer(nullptr) {
		};
		inline share_pointer(share_pointer const& rhs) noexcept
			: _pointer(rhs._pointer) {
			if (nullptr != _pointer)
				library::interlock_increment(_pointer->_use);
		};
		inline share_pointer(share_pointer&& rhs) noexcept
			: _pointer(library::exchange(rhs._pointer, nullptr)) {
		};
		inline auto operator=(share_pointer const& rhs) noexcept -> share_pointer& {
			share_pointer(rhs).swap(*this);
			return *this;
		}
		inline auto operator=(share_pointer&& rhs) noexcept -> share_pointer& {
			share_pointer(std::move(rhs)).swap(*this);
			return *this;
		};
		inline ~share_pointer(void) noexcept {
			if (nullptr != _pointer && 0 == library::interlock_decrement(_pointer->_use)) {
				static_cast<type*>(_pointer)->template destruct<index>();
				if (0 == library::interlock_decrement(_pointer->_weak))
					type::deallocate<index>(static_cast<type*>(_pointer));
			}
		}
		template<typename other>
		//requires std::is_convertible_v<other*, type*>
		inline share_pointer(other* value) noexcept
			: _pointer(static_cast<hook*>(value)) {
			library::interlock_exchange(_pointer->_use, 1);
			library::interlock_exchange(_pointer->_weak, 1);
		}
		template<typename other>
		//requires std::is_convertible_v<other*, type*>
		inline share_pointer(weak_pointer<other, index> const& weak_ptr) noexcept
			: _pointer(weak_ptr._pointer) {
			if (nullptr != _pointer) {
				for (size_type use = _pointer->_use, prev; 0 != use; use = prev)
					if (prev = library::interlock_compare_exhange(_pointer->_use, use + 1, use), use == prev)
						return;
				_pointer = nullptr;
			}
		}
		template<typename other>
		//requires std::is_convertible_v<other*, type*>
		inline share_pointer(share_pointer<other, index> const& rhs) noexcept
			: _pointer(rhs._pointer) {
			if (nullptr != _pointer)
				library::interlock_increment(_pointer->_use);
		};
		template<typename other>
		//requires std::is_convertible_v<other*, type*>
		inline share_pointer(share_pointer<other, index>&& rhs) noexcept
			: _pointer(library::exchange(rhs._pointer, nullptr)) {
		};
		template<typename other>
		//requires std::is_convertible_v<other*, type*>
		inline auto operator=(share_pointer<other, index> const& rhs) noexcept -> share_pointer& {
			share_pointer(rhs).swap(*this);
			return *this;
		}
		template<typename other>
		//requires std::is_convertible_v<other*, type*>
		inline auto operator=(share_pointer<other, index>&& rhs) noexcept -> share_pointer& {
			share_pointer(std::move(rhs)).swap(*this);
			return *this;
		};

		inline auto operator*(void) const noexcept -> type& {
			return static_cast<type&>(*_pointer);
		}
		inline auto operator->(void) const noexcept -> type* const {
			return static_cast<type*>(_pointer);
		}
		inline bool operator==(nullptr_t) noexcept {
			return nullptr == _pointer;
		}
		inline explicit operator bool() const noexcept {
			return nullptr != _pointer;
		}
		inline void swap(share_pointer& rhs) noexcept {
			library::swap(_pointer, rhs._pointer);
		}
		inline auto use_count(void) const noexcept -> size_type {
			return _pointer._use;
		}
		inline auto get(void) const noexcept -> type* {
			return static_cast<type*>(_pointer);
		}
		inline void set(type* value) noexcept {
			_pointer = static_cast<hook*>(value);
		}
		inline void reset(void) noexcept {
			_pointer = nullptr;
		}

	};
	template<typename type, size_t index>
	class weak_pointer {
		template<typename type, size_t>
		friend class share_pointer;
		using size_type = unsigned int;
		using hook = pointer_hook<index>;
		//static_assert(std::is_base_of<hook, type>::value);
		hook* _pointer;
	public:
		inline constexpr explicit weak_pointer(void) noexcept
			: _pointer(nullptr) {
		}
		inline weak_pointer(weak_pointer const& rhs) noexcept
			: _pointer(rhs._pointer) {
			if (nullptr != _pointer)
				library::interlock_increment(_pointer->_weak);
		};
		inline weak_pointer(weak_pointer&& rhs) noexcept
			: _pointer(library::exchange(rhs._pointer, nullptr)) {
		};
		inline auto operator=(weak_pointer const& rhs) noexcept -> weak_pointer& {
			weak_pointer(rhs).swap(*this);
			return *this;
		}
		inline auto operator=(weak_pointer&& rhs) noexcept -> weak_pointer& {
			weak_pointer(std::move(rhs)).swap(*this);
			return *this;
		};
		inline ~weak_pointer(void) noexcept {
			if (nullptr != _pointer && 0 == library::interlock_decrement(_pointer->_weak))
				type::deallocate<index>(static_cast<type*>(_pointer));
		}
		inline constexpr explicit weak_pointer(nullptr_t) noexcept
			: _pointer(nullptr) {
		}
		template<typename other>
		//requires std::is_convertible_v<other*, type*>
		inline weak_pointer(share_pointer<other, index> const& share_ptr) noexcept
			: _pointer(share_ptr._pointer) {
			if (nullptr != _pointer)
				library::interlock_increment(_pointer->_weak);
		}
		template<typename other>
		//requires std::is_convertible_v<other*, type*>
		inline weak_pointer(weak_pointer<other, index> const& rhs) noexcept
			: _pointer(rhs._pointer) {
			if (nullptr != _pointer)
				library::interlock_increment(_pointer._weak);
		};
		template<typename other>
		//requires std::is_convertible_v<other*, type*>
		inline weak_pointer(weak_pointer<other, index>&& rhs) noexcept
			: _pointer(library::exchange(rhs._pointer, nullptr)) {
		};
		template<typename other>
		//requires std::is_convertible_v<other*, type*>
		inline auto operator=(weak_pointer<other, index> const& rhs) noexcept -> weak_pointer& {
			weak_pointer(rhs).swap(*this);
			return *this;
		}
		template<typename other>
		//requires std::is_convertible_v<other*, type*>
		inline auto operator=(weak_pointer<other, index>&& rhs) noexcept -> weak_pointer& {
			weak_pointer(std::move(rhs)).swap(*this);
			return *this;
		};

		inline bool expire(void) const noexcept {
			return nullptr != _pointer && 0 != _pointer->_use;
		}
		inline auto lock(void) noexcept {
			return share_pointer<type, 0>(*this);
		}
		template<typename other>
		inline void swap(weak_pointer<other, index>& rhs) noexcept {
			library::swap(_pointer, rhs._pointer);
		}
	};

	template<size_t index>
	template<typename type>
	inline auto pointer_hook<index>::share_pointer_this(void) noexcept -> share_pointer<type, index> {
		share_pointer<type, index> pointer;
		pointer.set(static_cast<type*>(this));
		library::interlock_increment(_use);
		return pointer;
	}
}