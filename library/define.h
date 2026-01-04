#pragma once

#ifdef define_dll
#define declspec_dll _declspec(dllexport)
#else
#define declspec_dll _declspec(dllimport)
#endif