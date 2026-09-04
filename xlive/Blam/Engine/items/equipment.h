#pragma once
#include "units/units.h"

/* enums */

enum e_powerup_type : int16
{
	_powerup_type_none = 0,
	_powerup_type_double_speed = 1,
	_powerup_type_over_shield = 2,
	_powerup_type_active_camouflage = 3,
	_powerup_type_fullspectrum_vision = 4,
	_powerup_type_health = 5,
	_powerup_type_grenade = 6,
	k_powerup_type_count
}

/* structures */

struct _equipment_definition
{
	e_powerup_type powerup_type;
	e_unit_grenade_type grenade_type;
	real32 powerup_time;	// seconds
	tag_reference pickup_sound;
};
ASSERT_STRUCT_SIZE(_equipment_definition, 20)

struct equipment_definition
{
	_object_definition object;
	_item_definition item;
	_equipment_definition equipment;
};
ASSERT_STRUCT_SIZE(equipment_definition, 316)
