#include "includes.h"

#include "../jvm/sdk/c_minecraft.h"

void g::start()
{
	sdk::c_instanceklass* minecraft_klass;
	while ((minecraft_klass = sdk::find_klass("bao")) == nullptr)
		std::this_thread::sleep_for(std::chrono::seconds(5));

	while(true)
	{
		static auto instance = std::make_unique<c_minecraft>(minecraft_klass);

		static std::once_flag flag;
		if (instance->get_player())
		{
			// initialization 
			std::call_once(flag, [&]() {
				auto klass = sdk::find_klass("blk"); // lol
				c_entityliving::initialize(klass);
			});
		}

		if (instance->get_world())
		{
			printf("World: 0x%X\n", instance->get_world());
			if (instance->get_player())
				printf("Player: 0x%X\n", instance->get_player());
			auto player_list = instance->get_world()->get_players();

			// iterate playerlist
			for (int i = 0; i < player_list->length; ++i)
			{
				auto object = player_list->at<c_entityplayersp*>(i);
				printf("(%d) %.2f %.2f %.2f\n", i, object->get_position().x, object->get_position().y, object->get_position().z);
			}
			printf("\n"); // spacing
		}

		std::this_thread::sleep_for(std::chrono::seconds(10));
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
