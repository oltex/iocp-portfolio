#pragma once
#include "memory.h"
#include "function.h"
#include "interlock.h"
#include "pool.h"
#include <malloc.h>
#include <utility>
#include <type_traits>
#include <iostream>
#include <cassert>

namespace detail {
	struct reference {
	private:
		using size_type = unsigned int;
	public:
		size_type _use, _weak;

		inline reference(void) noexcept
			: _use(1), _weak(1) {
		};
		inline reference(reference const&) noexcept = delete;
		inline reference(reference&&) noexcept = delete;
		inline auto operator=(reference const&) noexcept -> reference & = delete;
		inline auto operator=(reference&&) noexcept -> reference & = delete;
		inline virtual ~reference(void) noexcept = default;

		inline virtual void destroy_object(void) noexcept = 0;
		inline virtual void destroy_reference(void) noexcept = 0;
	};
	template <typename type, typename deleter, typename allocator>
	struct reference_external final : public reference {
		type* _pointer;
		[[no_unique_address]] deleter _deleter;
		[[no_unique_address]] allocator _allocator;
	public:
		template<typename other>
			requires std::is_convertible_v<other*, type*>
		inline reference_external(other* const pointer, deleter functor, allocator alloc) noexcept
			: _pointer(static_cast<type*>(pointer)), _deleter(functor), _allocator(alloc) {
		};
		inline reference_external(reference_external const&) noexcept = delete;
		inline reference_external(reference_external&&) noexcept = delete;
		inline auto operator=(reference_external const&) noexcept -> reference_external & = delete;
		inline auto operator=(reference_external&&) noexcept -> reference_external & = delete;
		inline virtual ~reference_external(void) noexcept override = default;

		inline virtual void destroy_object(void) noexcept override {
			_deleter(_pointer);
		}
		inline virtual void destroy_reference(void) noexcept override {
			typename allocator::template rebind<reference_external> rebind_allocator(_allocator);
			rebind_allocator.deallocate(this);
		}
	};
	template <typename type, typename allocator>
	struct reference_inplace final : public reference {
		union {
			type _value;
		};
		[[no_unique_address]] allocator _allocator;
	public:
		template<typename... argument>
		inline reference_inplace(allocator const& alloc, argument&&... arg) noexcept
			: _allocator(alloc) {
			library::construct(_value, std::forward<argument>(arg)...);
		};
		inline reference_inplace(reference_inplace const&) noexcept = delete;
		inline reference_inplace(reference_inplace&&) noexcept = delete;
		inline auto operator=(reference_inplace const&) noexcept -> reference_inplace & = delete;
		inline auto operator=(reference_inplace&&) noexcept -> reference_inplace & = delete;
		inline virtual ~reference_inplace(void) noexcept override {
		};

		inline virtual void destroy_object(void) noexcept override {
			library::destruct<type>(_value);
		}
		inline virtual void destroy_reference(void) noexcept override {
			typename allocator::template rebind<reference_inplace> rebind_allocator(_allocator);
			rebind_allocator.deallocate(this);
		}
	};
}

namespace library {
	template<typename type>
	class share_pointer final {
		template <typename other>
		friend class share_pointer;
		template<typename type>
		friend class weak_pointer;
		template<typename type, typename... argument>
		friend inline auto make_share(argument&&... arg) noexcept -> share_pointer<type>;
		template<typename type, typename allocator, typename... argument>
		friend inline auto allocate_share(allocator const& alloc, argument&&... arg) noexcept -> share_pointer<type>;

		using size_type = unsigned int;
		type* _pointer;
		detail::reference* _reference;
	public:
		inline constexpr share_pointer(void) noexcept
			: _pointer(nullptr), _reference(nullptr) {
		}
		inline constexpr share_pointer(std::nullptr_t) noexcept
			: _pointer(nullptr), _reference(nullptr) {
		};
		inline share_pointer(share_pointer const& rhs) noexcept
			: _pointer(rhs._pointer), _reference(rhs._reference) {
			if (nullptr != _reference)
				library::interlock_increment(_reference->_use);
		};
		inline share_pointer(share_pointer&& rhs) noexcept
			: _pointer(library::exchange(rhs._pointer, nullptr)), _reference(library::exchange(rhs._reference, nullptr)) {
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
			if (nullptr != _reference && 0 == library::interlock_decrement(_reference->_use)) {
				_reference->destroy_object();
				if (0 == library::interlock_decrement(_reference->_weak))
					_reference->destroy_reference();
			}
		}
		template<typename other, typename deleter = decltype([](other* pointer) noexcept -> void { delete pointer; }) >
			requires std::is_convertible_v<other*, type*>
		inline share_pointer(other* const pointer, deleter functor = {}) noexcept
			: _pointer(static_cast<type*>(pointer)) {
			if (nullptr == pointer) {
				_reference = nullptr;
				return;
			}
			library::allocator<other> allocator;
			typename library::allocator<other>::template rebind<detail::reference_external<other, deleter, library::allocator<other>>> rebind_allocator(allocator);
			auto reference = rebind_allocator.allocate(pointer, functor, allocator);
			_reference = reference;
		}
		template<typename other>
			requires std::is_convertible_v<other*, type*>
		inline share_pointer(share_pointer<other> const& rhs) noexcept
			: _pointer(static_cast<type*>(rhs._pointer)), _reference(rhs._reference) {
			if (nullptr != _reference)
				library::interlock_increment(_reference->_use);
		};
		template<typename other>
			requires std::is_convertible_v<other*, type*>
		inline share_pointer(share_pointer<other>&& rhs) noexcept
			: _pointer(static_cast<type*>(library::exchange(rhs._pointer, nullptr))), _reference(library::exchange(rhs._reference, nullptr)) {
		};
		template<typename other>
			requires std::is_convertible_v<other*, type*>
		inline auto operator=(share_pointer<other> const& rhs) noexcept -> share_pointer& {
			share_pointer(rhs).swap(*this);
			return *this;
		}
		template<typename other>
			requires std::is_convertible_v<other*, type*>
		inline auto operator=(share_pointer<other>&& rhs) noexcept -> share_pointer& {
			share_pointer(std::move(rhs)).swap(*this);
			return *this;
		};
		template<typename other>
		inline share_pointer(share_pointer<other> const& rhs, type* pointer) noexcept
			: _pointer(pointer), _reference(rhs._reference) {
			if (nullptr != _reference)
				library::interlock_increment(_reference->_use);
		}
		template<typename other>
		inline share_pointer(share_pointer<other>&& rhs, type* pointer) noexcept
			: _pointer(pointer), _reference(library::exchange(rhs._reference, nullptr)) {
			rhs._pointer = nullptr;
		}

		[[nodiscard]] inline explicit operator bool(void) const noexcept {
			return nullptr != _pointer;
		}
		[[nodiscard]] inline auto operator==(std::nullptr_t) const noexcept -> bool {
			return nullptr == _pointer;
		}
		[[nodiscard]] inline auto operator*(void) const noexcept -> type& {
			assert(nullptr != _pointer);
			return *_pointer;
		}
		[[nodiscard]] inline auto operator->(void) const noexcept -> type* const {
			assert(nullptr != _pointer);
			return _pointer;
		}
		[[nodiscard]] inline auto use_count(void) const noexcept -> size_type {
			return nullptr == _reference ? 0 : _reference->_use;
		}
		[[nodiscard]] inline auto get(void) const noexcept -> type* {
			return _pointer;
		}
		inline void swap(share_pointer& rhs) noexcept {
			library::swap(_pointer, rhs._pointer);
			library::swap(_reference, rhs._reference);
		}
	};
	template<typename type, typename allocator, typename... argument>
	[[nodiscard]] inline auto allocate_share(allocator const& alloc, argument&&... arg) noexcept -> share_pointer<type> {
		share_pointer<type> pointer;
		typename allocator::template rebind<detail::reference_inplace<type, allocator>> rebind_allocator(alloc);
		auto reference = rebind_allocator.allocate(alloc, std::forward<argument>(arg)...);
		pointer._pointer = &reference->_value;
		pointer._reference = reference;
		return pointer;
	}
	template <typename type, typename... argument>
	[[nodiscard]] inline auto make_share(argument&&... arg) noexcept -> share_pointer<type> {
		share_pointer<type> pointer;
		library::allocator<type> allocator;
		return library::allocate_share<type, library::allocator<type>>(allocator, std::forward<argument>(arg)...);
	}
	template <typename to, typename from>
	[[nodiscard]] inline auto static_cast_share(share_pointer<from> const& rhs) noexcept -> share_pointer<to> {
		auto pointer = static_cast<to*>(rhs.get());
		return share_pointer<to>(rhs, pointer);
	}
	template <typename to, typename from>
	[[nodiscard]] inline auto dynamic_cast_share(share_pointer<from> const& rhs) noexcept -> share_pointer<to> {
		auto pointer = dynamic_cast<to*>(rhs.get());
		return share_pointer<to>(rhs, pointer);
	}
	template<typename type>
	class weak_pointer final {
		template<typename type>
		friend class weak_pointer;
		using size_type = unsigned int;
		type* _pointer;
		detail::reference* _reference;
	public:
		inline constexpr weak_pointer(void) noexcept
			: _pointer(nullptr), _reference(nullptr) {
		}
		inline constexpr weak_pointer(std::nullptr_t) noexcept
			: _pointer(nullptr), _reference(nullptr) {
		}
		inline weak_pointer(weak_pointer const& rhs) noexcept
			: _pointer(rhs._pointer), _reference(rhs._reference) {
			if (nullptr != _reference)
				library::interlock_increment(_reference->_weak);
		};
		inline weak_pointer(weak_pointer&& rhs) noexcept
			: _pointer(library::exchange(rhs._pointer, nullptr)), _reference(library::exchange(rhs._reference, nullptr)) {
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
			if (nullptr != _reference && 0 == library::interlock_decrement(_reference->_weak))
				_reference->destroy_reference();
		}
		template<typename other>
			requires std::is_convertible_v<other*, type*>
		inline weak_pointer(share_pointer<other> const& shared_ptr) noexcept
			: _pointer(static_cast<type*>(shared_ptr._pointer)), _reference(shared_ptr._reference) {
			if (nullptr != _reference)
				library::interlock_increment(_reference->_weak);
		}
		template<typename other>
			requires std::is_convertible_v<other*, type*>
		inline weak_pointer(weak_pointer<other> const& rhs) noexcept
			: _pointer(static_cast<type*>(rhs._pointer)), _reference(rhs._reference) {
			if (nullptr != _reference)
				library::interlock_increment(_reference->_weak);
		};
		template<typename other>
			requires std::is_convertible_v<other*, type*>
		inline weak_pointer(weak_pointer<other>&& rhs) noexcept
			: _pointer(static_cast<type*>(library::exchange(rhs._pointer, nullptr))), _reference(library::exchange(rhs._reference, nullptr)) {
		};
		template<typename other>
			requires std::is_convertible_v<other*, type*>
		inline auto operator=(weak_pointer<other> const& rhs) noexcept -> weak_pointer& {
			weak_pointer(rhs).swap(*this);
			return *this;
		}
		template<typename other>
			requires std::is_convertible_v<other*, type*>
		inline auto operator=(weak_pointer<other>&& rhs) noexcept -> weak_pointer& {
			weak_pointer(std::move(rhs)).swap(*this);
			return *this;
		};

		[[nodiscard]] inline auto use_count(void) const noexcept -> size_type {
			return nullptr == _reference ? 0 : _reference->_use;
		}
		[[nodiscard]] inline auto expired(void) const noexcept -> bool {
			return nullptr == _reference || 0 == _reference->_use;
		}
		[[nodiscard]] inline auto lock(void) noexcept -> share_pointer<type> {
			share_pointer<type> result;
			if (nullptr == _reference)
				return result;
			for (size_type current = _reference->_use, prev; 0 != current; current = prev)
				if (prev = library::interlock_compare_exchange(_reference->_use, current + 1, current), current == prev) {
					result._pointer = _pointer;
					result._reference = _reference;
					break;
				}
			return result;
		}
		inline void swap(weak_pointer& rhs) noexcept {
			library::swap(_pointer, rhs._pointer);
			library::swap(_reference, rhs._reference);
		}
	};

	namespace intrusive {
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

			size_type _use, _weak;
		public:
			inline pointer_hook(void) noexcept = default;
			inline pointer_hook(pointer_hook const&) noexcept = default;
			inline pointer_hook(pointer_hook&&) noexcept = default;
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
					for (auto use = _pointer->_use, prev; 0 != use; use = prev)
						if (prev = library::interlock_compare_exchange(_pointer->_use, use + 1, use), use == prev)
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
}
//template<typename type>
//class unique_pointer final {
//	template <typename other>
//	friend class unique_pointer;
//	template<typename type, typename... argument>
//	inline friend auto make_unique(argument&&... arg) noexcept ->unique_pointer<type>;
//	type* _pointer;
//public:
//	inline constexpr unique_pointer(void) noexcept
//		: _pointer(nullptr) {
//	};
//	inline constexpr unique_pointer(nullptr_t) noexcept
//		: _pointer(nullptr) {
//	};
//	template<typename other>
//		requires std::is_convertible_v<other*, type*>
//	inline explicit unique_pointer(other* const pointer) noexcept
//		: _pointer(pointer) {
//	}
//	inline explicit unique_pointer(unique_pointer&) noexcept = delete;
//	inline unique_pointer(unique_pointer&& rhs) noexcept
//		: _pointer(library::exchange(rhs._pointer, nullptr)) {
//	};
//	template<typename other>
//		requires std::is_convertible_v<other*, type*>
//	inline unique_pointer(unique_pointer<other>&& rhs) noexcept
//		: _pointer(static_cast<type*>(library::exchange(rhs._pointer, nullptr))) {
//	};
//	inline auto operator=(unique_pointer const&) noexcept -> unique_pointer & = delete;
//	inline auto operator=(unique_pointer&& rhs) noexcept -> unique_pointer& {
//		unique_pointer(std::move(rhs)).swap(*this);
//		return *this;
//	};
//	template<typename other>
//		requires std::is_convertible_v<other*, type*>
//	inline auto operator=(unique_pointer<other>&& rhs) noexcept -> unique_pointer& {
//		unique_pointer(std::move(rhs)).swap(*this);
//		return *this;
//	};
//	inline ~unique_pointer(void) noexcept {
//		if (nullptr != _pointer) {
//			library::destruct<type>(*_pointer);
//			library::deallocate<type>(_pointer);
//		}
//	}
//
//	inline auto operator*(void) noexcept -> type& {
//		return *_pointer;
//	}
//	inline auto operator->(void) noexcept -> type* {
//		return _pointer;
//	}
//	inline bool operator==(nullptr_t) noexcept {
//		return nullptr == _pointer;
//	}
//	inline explicit operator bool() const noexcept {
//		return nullptr != _pointer;
//	}
//	inline void swap(unique_pointer& rhs) noexcept {
//		library::swap(_pointer, rhs._pointer);
//	}
//	inline auto get(void) const noexcept -> type* {
//		return _pointer;
//	}
//	inline void set(type* value) noexcept {
//		_pointer = value;
//	}
//	inline void reset(void) noexcept {
//		_pointer = nullptr;
//	}
//};
//template<typename type>
//class unique_pointer<type[]> final {
//	using size_type = unsigned int;
//	type* _pointer;
//public:
//	inline constexpr unique_pointer(void) noexcept
//		: _pointer(nullptr) {
//	};
//	inline constexpr unique_pointer(nullptr_t) noexcept
//		: _pointer(nullptr) {
//	};
//	inline explicit unique_pointer(type* const pointer) noexcept
//		: _pointer(pointer) {
//	}
//	inline explicit unique_pointer(unique_pointer&) noexcept = delete;
//	inline explicit unique_pointer(unique_pointer&& rhs) noexcept
//		: _pointer(library::exchange(rhs._pointer, nullptr)) {
//	}
//	inline auto operator=(unique_pointer const&) noexcept -> unique_pointer & = delete;
//	inline auto operator=(unique_pointer&& rhs) noexcept -> unique_pointer& {
//		unique_pointer(std::move(rhs)).swap(*this);
//		return *this;
//	};
//	inline ~unique_pointer(void) noexcept {
//		if (nullptr != _pointer) {
//			library::destruct<type>(*_pointer);
//			library::deallocate<type>(_pointer);
//		}
//	}
//
//	inline auto operator[](size_type const index) noexcept -> type& {
//		return _pointer[index];
//	}
//	inline bool operator==(nullptr_t) noexcept {
//		return nullptr == _pointer;
//	}
//	inline explicit operator bool() const noexcept {
//		return nullptr != _pointer;
//	}
//	inline void swap(unique_pointer& rhs) noexcept {
//		library::swap(_pointer, rhs._pointer);
//	}
//	inline auto get(void) const noexcept -> type* {
//		return _pointer;
//	}
//	inline void set(type* value) noexcept {
//		_pointer = value;
//	}
//	inline void reset(void) noexcept {
//		_pointer = nullptr;
//	}
//};
//template <typename type, typename... argument>
//inline auto make_unique(argument&&... arg) noexcept -> unique_pointer<type> {
//	unique_pointer<type> pointer;
//	pointer._pointer = library::allocate<type>();
//	library::construct<type>(*pointer._pointer, std::forward<argument>(arg)...);
//	return pointer;
//}