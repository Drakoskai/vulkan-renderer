#include "Util.h"
#include <stdio.h>
#include <windows.h>

std::wstring StringToWString(const std::string& s) {
	int len;
	int slength = static_cast<int>(s.length()) + 1;
	len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, nullptr, 0);
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
