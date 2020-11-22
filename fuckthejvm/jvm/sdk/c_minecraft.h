#pragma once

#include "c_entityplayersp.h"

class c_minecraft {
private:
	sdk::c_instanceklass* instance_klass;
public:
	c_minecraft(sdk::c_instanceklass* instance_klass) : instance_klass(instance_klass) {
		assert(instance_klass);
	}

	std::uint32_t get_minecraft() {
		return instance_klass->static_fields->instance;
	}

	std::int32_t get_display_height() {
		if (!get_minecraft())
			return 1337;

		return *reinterpret_cast<int*>(get_minecraft() + 0xC);
	}

	c_entityplayersp* get_player() {
		if (!get_minecraft())
			return nullptr;

		return static_cast<c_entityplayersp*>(ULongToPtr(*(int32_t*)((uintptr_t)get_minecraft() + 0x84)));
	}
};