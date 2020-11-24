#pragma once

// standard stuff
#include <thread>
#include <memory>
#include <algorithm>
#include <vector>
#include <Windows.h>
#include <Psapi.h>
#include <regex>
#include <string>
#include <mutex>

// jvm related stuff
#include "../jvm/jvm.h"
#include "../jvm/sdk/c_minecraft.h"

// Wheter we are debugging or not
#define DEBUG_MODE true

namespace g {

	// Spawn an epic console
	static auto spawn_console() {
		AllocConsole(); // allocate a console
		FILE* in;
		FILE* out;

		freopen_s(&in, "conin$", "r", stdin);
		freopen_s(&out, "conout$", "w", stdout);
		freopen_s(&out, "conout$", "w", stderr);
	}

	// Begin hackalacking
	void start();

	// Scan for pattern in entirety of the memory
	unsigned char* pattern_scan_memory(std::vector<std::uint8_t> pattern, const char* mask, std::int32_t offset = 0);

	// Scan for pattern in JVM.dll
	unsigned char* pattern_scan(HINSTANCE module, std::vector<std::uint8_t> pattern, const char* mask, std::int32_t offset = 0);

	// Scan for a constmethod
	sdk::c_constmethod* pattern_scan_method(std::vector<std::uint8_t> pattern, const char* mask);
}