#include "includes.h"

void g::start()
{
	/* NOTE: This method is now overdue */
	std::vector<uint8_t> bytecode_clickmd = { 0x2A, 0xB4, 0xCC, 0xCC, 0x9E, 0x00, 0x04, 0xB1, 0x2A, 0xB4, 0xCC };
	auto click_constmethod = g::pattern_scan_method(bytecode_clickmd, "xx??xxxxxx?"); //0x38 is the size of the constmethod class

	if (!click_constmethod)
	{
		printf("Unable to parse constmethod...\n");
		return;
	}

	auto test = sdk::find_klass("blk"); //player class

	const auto dump_class = [](sdk::c_instanceklass* klass) {
		printf("\n");
		printf("Dumping klass: (0x%X)\n", klass);
		for (int i = 0; i < klass->fields->length; ++i)
		{
			auto field = klass->fields->at(i);

			auto field_info = sdk::c_fieldinfo::from_field_array(klass->fields, i);

			if (!field_info)
				return;

			auto name = field_info->name(klass->constant_pool);
			auto sig = field_info->sig(klass->constant_pool);

			if (!name)
				return;

			if (!sig)
				return;

			printf("Field %s(%s) (offset: 0x%X)\n", std::string(name->text, name->length).c_str(), std::string(sig->text, sig->length).c_str(), field_info->offset());
		}
		printf("\n");
	};

	sdk::c_instanceklass* clazz = test;
	while (clazz) // dump EntityPlayerSP chain
	{
		dump_class(clazz);
		clazz = clazz->super;
	}
}


unsigned char* g::pattern_scan_memory(std::vector<std::uint8_t> pattern, const char* mask, std::int32_t offset)
{
	SYSTEM_INFO sys_info;
	GetSystemInfo(&sys_info);

	// Check the max amount we can iterate
	auto end = sys_info.lpMaximumApplicationAddress;

	char* chunk = 0;
	char* match = nullptr;

	// Iterate
	while (chunk < (char*)end)
	{
		MEMORY_BASIC_INFORMATION mbi;
		
		// Query page information
		if (!VirtualQuery(chunk, &mbi, sizeof(mbi)))
			return 0;
		
		// Make sure we aren't gonna find our own vector (lol)
		DWORD pointer_to_pattern = (DWORD)&pattern;
		if (pointer_to_pattern > (DWORD) mbi.AllocationBase && pointer_to_pattern < (DWORD)mbi.AllocationBase + mbi.RegionSize)
		{
			chunk = chunk + mbi.RegionSize;
			continue;
		}

		// Check if we can access it in the first place
		if (mbi.State == MEM_COMMIT && mbi.Protect != PAGE_NOACCESS)
		{
			DWORD old_protection = 0;
			// Change protection so that we can actually read it
			if (VirtualProtect(mbi.BaseAddress, mbi.RegionSize, PAGE_EXECUTE_READWRITE, &old_protection)) {

				// Scan for the epic pattern
				const auto in_scan = [&](unsigned int size) {
					std::uint32_t pattern_length = strlen(mask);
					auto based = reinterpret_cast<std::uint8_t*>(mbi.BaseAddress);

					for (int i = 0; i < size - pattern_length; ++i)
					{
						//yadadadad
						bool found = true;
						for (int j = 0; j < pattern_length; j++)
						{
							if (mask[j] != '?' && based[i + j] != pattern[j])
							{
								found = false;
								break;
							}
						}

						// Found it, epic.
						if (found)
							return ((char*)&based[i]);
					}

					// Sadly didn't find it.
					return (char*)0;
				};
			
				// Scan for pattern
				auto internal_addy = in_scan(mbi.RegionSize);

				// Reset the protection
				VirtualProtect(mbi.BaseAddress, mbi.RegionSize, old_protection, &old_protection);

				if (internal_addy != 0)
				{
					match = internal_addy;
					break;
				}
			}
		}


		chunk = chunk + mbi.RegionSize;
	}

	return (unsigned char*)(match + offset);
}

unsigned char* g::pattern_scan(HINSTANCE mod, std::vector<std::uint8_t> pattern, const char* mask, std::int32_t offset)
{
	auto dos = reinterpret_cast<PIMAGE_DOS_HEADER>(mod);
	auto nt = reinterpret_cast<PIMAGE_NT_HEADERS>(mod + dos->e_lfanew);
	auto module_size = nt->OptionalHeader.SizeOfImage;

	// Scan for the epic pattern
	const auto in_scan = [&]() {
		std::uint32_t pattern_length = strlen(mask);
		auto based = reinterpret_cast<std::uint8_t*>(mod);

		for (int i = 0; i < module_size - pattern_length; ++i)
		{
			//yadadadad
			bool found = true;
			for (int j = 0; j < pattern_length; j++)
			{
				if (mask[j] != '?' && based[i + j] != pattern[j])
				{
					found = false;
					break;
				}
			}

			// Found it, epic.
			if (found)
				return ((char*)&based[i]);
		}

		// Sadly didn't find it.
		return (char*)0;
	};

	auto address = in_scan();
	return address ? (unsigned char*)address : 0;
}

sdk::c_constmethod* g::pattern_scan_method(std::vector<std::uint8_t> pattern, const char* mask)
{
	auto address = g::pattern_scan_memory(pattern, mask, 0); //0x38 is the size of the constmethod class

	if (!address)
		return nullptr; // Unable to be found

	sdk::c_constmethod* constmethod = reinterpret_cast<sdk::c_constmethod*>(address - 0x30);

	return constmethod?constmethod:nullptr;
}
