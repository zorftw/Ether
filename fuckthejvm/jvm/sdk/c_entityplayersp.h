#pragma once

class c_entityplayersp {
public:
	sdk::c_vec3d get_position()
	{
		return *reinterpret_cast<sdk::c_vec3d*>((std::uintptr_t)this + 0x30);
	}
};