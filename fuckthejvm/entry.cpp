#include "gasper/includes.h"

void start_thread()
{
	// Spawn a console
	g::spawn_console();

	if (!sdk::init()) {
		printf("SDK initialization failed...\n");
		return;
	}

	// Start the program
	g::start();
}

long __stdcall DllMain(HINSTANCE _, DWORD reason, void* _1)
{
	if (reason != DLL_PROCESS_ATTACH)
		return false;

	CreateThread(0, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(start_thread), 0, 0, 0);

	return true;
}