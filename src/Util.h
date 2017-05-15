#pragma once

#include <functional>
#include <vector>
#include <fstream>

template<typename T>
void SafeDelete(T* ptr) {
	if(ptr) {
		delete ptr;
		ptr = nullptr;
	}
}

template<typename T>
void SafeDeleteArr(T* ar) {
	if(ar) {
		delete[] ar;
		ar = nullptr;
	}
}

static std::vector<char> ReadFile(const std::string& filename) {
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if(!file.is_open()) {
		throw std::runtime_error("failed to open file!");
	}
	size_t fileSize = static_cast<size_t>(file.tellg());
	std::vector<char> buffer(fileSize);
	file.seekg(0);
	file.read(buffer.data(), fileSize);
	file.close();

	return buffer;
}

std::wstring StringToWString(const std::string& s);

int VDebugPrintF(const char* format, va_list argList);

int DebugPrintF(const char* format, ...);
