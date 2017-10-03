#include "Util.h"
#include <stdio.h>
#include <windows.h>
#include <fstream>

std::vector<uint32_t> ReadFileInts(const std::string& filename) {
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		throw std::runtime_error("failed to open file!");
	}
	const size_t fileSize = static_cast<size_t>(file.tellg());
	std::vector<uint32_t> buffer(fileSize);
	file.seekg(0);
	uint32_t data;
	uint32_t i = 0;

	while (file >> data) {
		buffer[i] = data;
		i++;
	}
	file.close();
	return buffer;
}

std::vector<char> ReadFile(const std::string& filename) {

	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		throw std::runtime_error("failed to open file!");
	}
	const size_t fileSize = static_cast<size_t>(file.tellg());
	std::vector<char> buffer(fileSize);
	file.seekg(0);
	file.read(buffer.data(), fileSize);
	file.close();

	return buffer;
}

std::wstring StringToWString(const std::string& s) {
	int slength = static_cast<int>(s.length()) + 1;
	int len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, nullptr, 0);
	wchar_t* buf = new wchar_t[len];
	MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
	std::wstring r(buf);
	delete[] buf;

	return r;
}

int VDebugPrintF(const char* format, va_list argList) {
	const unsigned int MAX_CHARS = 1024;
	static char s_buffer[MAX_CHARS];
	int charsWritten = vsnprintf_s(s_buffer, MAX_CHARS, format, argList);
	OutputDebugString(StringToWString(s_buffer).c_str());

	return charsWritten;
}

int DebugPrintF(const char* format, ...) {
	va_list argList;
	va_start(argList, format);
	int charsWritten = VDebugPrintF(format, argList);
	va_end(argList);

	return charsWritten;
}

int DebugPrintFIf(bool test, const char* format, ...) {
	if (test) {
		va_list argList;
		va_start(argList, format);
		const int charsWritten = VDebugPrintF(format, argList);
		va_end(argList);

		return charsWritten;
	}
	return 0;
}
