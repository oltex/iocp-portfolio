#include "singleton.h"

namespace library {
	template<>
	type* singleton<type, true, true>::_instance = nullptr;
	template<>
	template<>
	declspec_dll auto singleton<type, true, true>::construct<int>(int&& value) noexcept -> type& {
		_instance = library::allocate<type>();
		::new(reinterpret_cast<void*>(_instance)) type(value);
		return *_instance;
	}
	template<>
	declspec_dll auto singleton<type, true, true>::instance(void) noexcept -> type& {
		return *_instance;
	}
	template<>
	declspec_dll void singleton<type, true, true>::destruct(void) noexcept {
		_instance->~device();
		library::deallocate(_instance);
	}
	template class declspec_dll singleton<type, true, true>;
}

//namespace library {
//	template<>
//	d3d11::device* singleton<d3d11::device, true, true>::_instance = nullptr;
//	template<>
//	template<>
//	declspec_dll auto singleton<d3d11::device, true, true>::construct<D3D_DRIVER_TYPE, unsigned int>(D3D_DRIVER_TYPE&& driver_type, unsigned int&& flag) noexcept -> d3d11::device& {
//		_instance = library::allocate<d3d11::device>();
//		::new(reinterpret_cast<void*>(_instance)) d3d11::device(driver_type, flag);
//		return *_instance;
//	}
//	template<>
//	declspec_dll auto singleton<d3d11::device, true, true>::instance(void) noexcept -> d3d11::device& {
//		return *_instance;
//	}
//	template<>
//	declspec_dll void singleton<d3d11::device, true, true>::destruct(void) noexcept {
//		_instance->~device();
//		library::deallocate(_instance);
//	}
//	template class declspec_dll singleton<d3d11::device, true, true>;
//}