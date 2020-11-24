#pragma once

#include "c_entityliving.h"

class c_entityplayersp : public c_entityliving {
public:

	double get_distance_to(c_entityliving* entity)
	{
		auto pos = this->get_position();
		auto entity_pos = entity->get_position();

		return sdk::distance(pos.x, pos.y, pos.z, entity_pos.x, entity_pos.y, entity_pos.z);
	}
};