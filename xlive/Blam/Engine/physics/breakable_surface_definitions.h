#pragma once
#include "tag_files/tag_reference.h"
#include "effects/particle_system_definition.h"

struct s_breakable_surface_definition
{
	real32 maximum_vitality;
	tag_reference effect; // effe
	tag_reference sound; // snd!
	tag_block<c_particle_system_definition> particle_effects;
	real32 particle_density;
};
ASSERT_STRUCT_SIZE(s_breakable_surface_definition, 32);