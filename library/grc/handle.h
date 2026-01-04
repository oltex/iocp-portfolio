#pragma once
#include <utility>

namespace grc {
	//template<typename type = void>
	//class handle;

	//template<>
	//class handle<void> {
	//	template<typename type, bool inplace>
	//	friend class arena;
	//	unsigned long long _key;
	//public:
	//	handle(unsigned long long key = 0) noexcept;
	//	~handle(void) noexcept = default;
	//	auto operator==(handle const& rhs) const noexcept -> bool;
	//};

	//template<typename type>
	//class handle : public handle<void> {
	//	using base = handle<void>;
	//	template<typename other>
	//	friend class handle;
	//public:
	//	using base::base;
	//	template<typename other>
	//	handle(handle<other> const& rhs) noexcept
	//		: handle<void>(rhs) {
	//	}
	//	template<typename other>
	//	handle(handle<other>&& rhs) noexcept
	//		: handle<void>(std::move(rhs)) {
	//	}
	//	template<typename other>
	//	auto operator=(handle<other> const& rhs) noexcept -> handle& {
	//		base::operator=(rhs);
	//		return *this;
	//	}
	//	template<typename other>
	//	auto operator=(handle<other>&& rhs) noexcept -> handle& {
	//		base::operator=(std::move(rhs));
	//		return *this;
	//	}
	//};

	//class self {
	//	template<typename type, bool inplace>
	//	friend class arena;
	//	handle<> _handle;
	//public:
	//	auto handle(void) const noexcept -> grc::handle<>;
	//};
}