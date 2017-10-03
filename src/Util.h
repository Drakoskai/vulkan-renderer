#ifndef UTIL_H_
#define UTIL_H_
#include "stdafx.h"
#include <vector>

const std::string MODELS_PATH = "assets/models/";
const std::string MATERIALS_PATH = "assets/materials/";
const std::string TEXTURES_PATH = "assets/";

template< typename T >
void SwapValues(T & a, T & b) {
	T c = a;
	a = b;
	b = c;
}

template<typename T>
void SafeDelete(T* ptr) {
	if (ptr) {
		delete ptr;
		ptr = nullptr;
	}
}

template<typename T>
void SafeDeleteArr(T* ar) {
	if (ar) {
		delete[] ar;
		ar = nullptr;
	}
}

std::vector<uint32_t> ReadFileInts(const std::string& filename);

std::vector<char> ReadFile(const std::string& filename);

std::wstring StringToWString(const std::string& s);

int VDebugPrintF(const char* format, va_list argList);

int DebugPrintF(const char* format, ...);

int DebugPrintFIf(bool test, const char* format, ...);

#endif
