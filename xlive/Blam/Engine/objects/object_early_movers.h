#pragma once
#include "Blam/Engine/math/matrix_math.h"

#define MAXIMUM_OBJECT_EARLY_MOVERS_PER_MAP 32

struct s_early_mover_data
{
	real_point3d origin;
	real_point3d position;
	real_vector3d linear_velocity_copy;
	real_vector3d linear_velocity;
	real_vector3d angular_velocity_copy;
	real_vector3d angular_velocity;
	real_matrix4x3 transform_copy;
	real_matrix4x3 transform;
	real_matrix4x3 inverse_transform;
	bool some_bool;
	bool another_bool;
};

struct s_object_early_movers_globals
{
	s_early_mover_data early_mover_data[MAXIMUM_OBJECT_EARLY_MOVERS_PER_MAP];
	datum early_mover_objects[MAXIMUM_OBJECT_EARLY_MOVERS_PER_MAP];
	int32 object_index_count;
	bool map_initialized;
};
CHECK_STRUCT_SIZE(s_object_early_movers_globals, 7560);

s_object_early_movers_globals* object_early_movers_globals_get(void);

void object_early_mover_new(datum object_index);
