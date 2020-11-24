#pragma once

class c_world : c_identifyable {
public:
	sdk::c_jvm_array* get_players() {
		auto offset = 0x54;

		return static_cast<sdk::c_jvm_array*>(ULongToPtr(*(int32_t*)((uintptr_t)this + offset)));
	}
};