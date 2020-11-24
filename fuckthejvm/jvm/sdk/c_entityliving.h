#pragma once

#include "c_identifyable.h"

class c_entityliving : c_identifyable {
protected:
	static inline std::uint32_t position_offset;
	static inline std::uint32_t motion_offset;
	static inline std::uint32_t yaw_offset;
	static inline std::uint32_t pitch_offset;
public:

	static void initialize(sdk::c_instanceklass* klass) {
		position_offset = sdk::find_offset(klass, "s", "D"); // they're stored in a vector
		motion_offset = sdk::find_offset(klass, "v", "D");

		yaw_offset = sdk::find_offset(klass, "y", "F");
		pitch_offset = sdk::find_offset(klass, "z", "F");
	}


	sdk::c_vec3d get_position() const
	{
		return get_value<sdk::c_vec3d>(position_offset);
	}

	float get_yaw() const
	{
		return get_value<float>(yaw_offset);
	}

	float get_pitch() const
	{
		return get_value<float>(pitch_offset);
	}
};