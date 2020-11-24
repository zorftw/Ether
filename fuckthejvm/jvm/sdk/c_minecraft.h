#pragma once

#include "c_entityplayersp.h"
#include "c_world.h"

class c_minecraft : c_identifyable {
private:
	sdk::c_instanceklass* instance_klass;
public:
	c_minecraft(sdk::c_instanceklass* instance_klass) : instance_klass(instance_klass) {
		assert(instance_klass);
	}

	std::uint32_t get_minecraft() const {
		return instance_klass->static_fields->instance;
	}

	std::int32_t get_display_height() const {
		if (!get_minecraft())
			return 1337;

		static std::uint64_t offset = { 0 };
		if (!offset)
			offset = sdk::find_offset(this->instance_klass, "d", "I");

		return get_value_direct<int>(get_minecraft() + offset);
	}

	std::int32_t get_display_width() const {
		if (!get_minecraft())
			return 1337;

		static std::uint64_t offset = { 0 };
		if (!offset)
			offset = sdk::find_offset(this->instance_klass, "e", "I");

		return get_value_direct<int>(get_minecraft() + offset);
	}

	c_world* get_world() const {
		if (!get_minecraft())
			return nullptr;

		static std::uint64_t offset = { 0 };
		if (!offset)
			offset = sdk::find_offset(this->instance_klass, "f", "Lbjf;");

		return static_cast<c_world*>(ULongToPtr(*(int32_t*)((uintptr_t)get_minecraft() + offset)));
	}

	c_entityplayersp* get_player() const {
		if (!get_minecraft())
			return nullptr;

		static std::uint64_t offset = { 0 };
		if (!offset)
			offset = sdk::find_offset(this->instance_klass, "h", "Lbjk;");

		return static_cast<c_entityplayersp*>(ULongToPtr(*(int32_t*)((uintptr_t)get_minecraft() + offset)));
	}

	std::int32_t get_player_addr() const {
		if (!get_minecraft())
			return 0;

		static std::uint64_t offset = { 0 };
		if (!offset)
			offset = sdk::find_offset(this->instance_klass, "h", "Lbjk;");

		return (*(int32_t*)((uintptr_t)get_minecraft() + offset));
	}
};