#include "jvm.h"

#include "../gasper/includes.h"

bool sdk::init()
{
	sdk::jvm_dll = reinterpret_cast<std::uintptr_t>(GetModuleHandleA("jvm.dll"));

	if (!sdk::jvm_dll)
		return false;

	return true;
}

// Used for finding a class without attaching the current thread to the JVM by stealing lol
sdk::c_instanceklass* sdk::find_klass(const char* name)
{
	if (!sdk::jvm_dll)
		return nullptr;

	auto thread = c_jvmthread::first();

	while (thread)
	{
		if (thread->get_environment())
		{
			auto original_thread = c_jvmthread::get_thread(); // get the current context
			c_jvmthread::set_thread(thread); // set the current thread to our target

			c_instanceklass* res = nullptr;

			thread->get_environment()->EnsureLocalCapacity(1); // make sure we can at least create 1 local reference
			if (!thread->get_environment()->PushLocalFrame(1)) { //returns 0 if true, make sure we create space for 1 reference 
				auto clazz = thread->get_environment()->FindClass(name);

				// check if found anything
				if (!clazz)
				{
					thread->get_environment()->ExceptionClear(); // clear classnotfound exception
					thread->get_environment()->PopLocalFrame(nullptr); // pop local frame
					thread = thread->next(); // continue
					continue;
				}

				// chad move
				std::uintptr_t* inner = *(std::uintptr_t**)clazz;
				std::uintptr_t offset = ((DWORD)inner + 0x48);
				std::int64_t ptr = *reinterpret_cast<std::int64_t*>(offset);
				res = reinterpret_cast<c_instanceklass*>(ptr);

				thread->get_environment()->ExceptionClear(); // clear possible exceptions left over
				thread->get_environment()->PopLocalFrame(nullptr); // pop local frame
				c_jvmthread::set_thread(original_thread); // set the old thread as context again


				return res;
			}
		}
		thread = thread->next();
	}

	return nullptr;
}

HINSTANCE get_jvm() {
	return reinterpret_cast<HINSTANCE>(sdk::jvm_dll);
}

__declspec(noinline) static auto find_get_thread_slow_function() {
	/*

	sig to containing function: (+0xF) \xE8\x00\x00\x00\x00\x83\xEB\x15, x????xxx

	*/
	auto jvm_exit = GetProcAddress(get_jvm(), "JVM_Exit");

	auto call_instruction = (uintptr_t)jvm_exit + 0x97; // call vm_exit

	auto offset = *reinterpret_cast<int32_t*>(call_instruction + 1); // offset

	auto next_instruction = call_instruction + 0x5; //ret or smth

	auto called_function = (next_instruction + offset); // vm_exit

	call_instruction = called_function + 0x15; // call    Thread__get_thread_slow

	offset = *reinterpret_cast<int32_t*>(call_instruction + 1); // offset

	next_instruction = call_instruction + 0x5; // mov rdi, rdx

	called_function = (next_instruction + offset); // Thread::get_thread_slow

	return called_function;
}

sdk::c_jvmthread* sdk::c_jvmthread::get_thread()
{
	static auto get_thread_addy = uint32_t{ 0 };
	if (!get_thread_addy) {
		get_thread_addy = find_get_thread_slow_function();

		if (!get_thread_addy)
			__fastfail(0);
	}
	using get_thread_fn = sdk::c_jvmthread* (__stdcall*)();
	return reinterpret_cast<get_thread_fn>(get_thread_addy)();
}

void sdk::c_jvmthread::set_thread(sdk::c_jvmthread* thread)
{
	static auto set_thread_function = uint64_t{0};
	if (!set_thread_function)
	{
		auto res = g::pattern_scan(get_jvm(), { 0xe9, 0xCC, 0xCC, 0xCC, 0xCC, 0x48, 0x8D, 0x35, 0xCC, 0xCC, 0xCC, 0xCC }, "x????xxx????");

		if (res)
		{
			/// Get base result
			auto base = res + 1;

			/// Parse the offset
			auto offset = *reinterpret_cast<const int32_t*>(base);

			/// Add addy size
			base += sizeof(int32_t);

			/// Actual fn addy
			base += offset;

			set_thread_function = reinterpret_cast<uint64_t>(base);
		}
		else
		{
			__fastfail(0);
		}
	}

	using set_thread_fn = void(__fastcall*)(sdk::c_jvmthread*);

	reinterpret_cast<set_thread_fn>(set_thread_function)(thread);
}

sdk::c_jvmthread* sdk::c_jvmthread::first()
{
	static sdk::c_jvmthread** thread_list = nullptr;
	if (!thread_list)
	{
		auto res = g::pattern_scan(get_jvm(), { 0x48, 0x8B, 0x05, 0xCC, 0xCC, 0xCC, 0xCC, 0x40, 0xB7, 0x01 }, "xxx????xxx");

		if (res)
		{
			const auto offset = *reinterpret_cast<const uint32_t*>(res + 3);
			thread_list = reinterpret_cast<sdk::c_jvmthread**>(const_cast<uint8_t*>(res) + 7 + offset);
		}
		else {
			__fastfail(0);
		}
	}

	return *thread_list;
}

sdk::c_jvmthread* sdk::c_jvmthread::next()
{
	static auto next_offset = uint32_t{ 0 };
	if (!next_offset)
	{
		auto res = g::pattern_scan(get_jvm(), { 0x48, 0x89, 0x81, 0xCC, 0xCC, 0xCC, 0xCC, 0xFF, 0x05, 0xCC, 0xCC, 0xCC, 0xCC }, "xxx????xx????");

		if (res)
			next_offset = *reinterpret_cast<const uint32_t*>(res + 3);
		else
			__fastfail(0);
	}

	return *reinterpret_cast<sdk::c_jvmthread**>(this + next_offset);
}

JNIEnv* sdk::c_jvmthread::get_environment()
{
	static auto env_offset = uint32_t{0};
	if (!env_offset)
	{
		auto res = g::pattern_scan(get_jvm(), {
			0x48, 0x8D, 0x83, 0xCC, 0xCC, 0xCC, 0xCC, 0x48, 0x3B, 0xF8, 0x74, 0x0F, 0x48, 0x8B, 0x15, 0xCC, 0xCC, 0xCC, 0xCC, 0x48, 0x8B, 0xCB, 0xE8, 0xCC, 0xCC, 0xCC, 0xCC, 0x83, 0xBB, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0x7E, 0x1A, 0x4C, 0x8B, 0x05, 0xCC, 0xCC, 0xCC, 0xCC, 0x48, 0x8B, 0x0D, 0xCC, 0xCC, 0xCC, 0xCC, 0x48, 0x8D, 0x15, 0xCC, 0xCC, 0xCC, 0xCC, 0xE8, 0xCC, 0xCC, 0xCC, 0xCC, 0x48, 0x8B, 0x05, 0xCC, 0xCC, 0xCC, 0xCC, 0x48, 0x8B, 0xCF
								   },
								   "xxx????xxxxxxxx????xxxx????xx?????xxxxx????xxx????xxx????x????xxx????xxx");

		if (res)
			env_offset = *reinterpret_cast<const uint32_t*>(res + 3);
		else
			__fastfail(0);
	}

	return reinterpret_cast<JNIEnv*>(this + env_offset);
}

bool sdk::c_jvmthread::is_compiler_thread()
{
	static auto offset = uint8_t{ 0 };
	if (!offset)
	{
		auto res = g::pattern_scan(get_jvm(), { 0xFF, 0x50, 0x28, 0x84, 0xC0, 0x74, 0x24 }, "xxxxxxx");

		if (res)
			offset = *reinterpret_cast<const uint8_t*>(res + 2);
		else
			__fastfail(0);
	}

	return *reinterpret_cast<bool*>(this + offset);
}

bool sdk::c_jvmthread::is_java_thread()
{
	static auto offset = uint32_t{ 0 };
	if (!offset)
	{
		auto res = g::pattern_scan(get_jvm(), {0xFF, 0x52, 0x20, 0x84, 0xC0, 0x75, 0x24}, "xxxxxxx");

		if (res)
			offset = *reinterpret_cast<const uint8_t*>(res + 2);
		else
			__fastfail(0);
	}

	return (*(unsigned __int8(__fastcall**)(LPVOID))(*(DWORD*)this + offset))(this);
}
