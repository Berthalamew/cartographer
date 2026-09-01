#pragma once
#include "objects/objects.h"

/* macros */

#define projectile_get(index) ((struct projectile_datum*)(object_get_and_verify_type((index), _object_mask_projectile)))
#define projectile_try_and_get(index) ((struct projectile_datum*)(object_try_and_get_and_verify_type((index), _object_mask_projectile)))

/* enums */


enum e_projectile_flags
{
	_projectile_has_nonzero_angular_velocity_bit = 0,
	_projectile_tracer_bit,

	// TODO: finish remaining flags
};

/* structures */

struct _projectile_datum
{
	int32 flags;
	int8 action;
	int8 consecutive_collision_counter;
	int16 hit_global_material;
	real_vector3d hit_normal;
	datum ignore_object_index;
	datum target_object_index;
	int32 target_model_target_index;
	datum contrail_attachment_index;
	int32 field_24;
	real32 detonation_timer;
	int32 field_2C;
	real32 detonation_timer_delta;
	real32 arming_time;
	real32 arming_time_delta;
	real32 odometer;
	real32 velocity_scale;
	real32 maximum_damage_distance;
	real_vector3d rotation_axis;
	real32 rotation_sine;
	real32 rotation_cosine;
	real32 damage_scale;
	real32 guided_angular_velocity;
	real32 last_thrust_speed;
	real32 thrust_odometer;
	real_point3d start_point;
	int32 field_78;
	uint16 shot_id;
	uint16 time_of_flight_ticks;
};

struct projectile_datum
{
	datum definition_index;
	_object_datum object;
	_projectile_datum projectile;
};
ASSERT_STRUCT_SIZE(projectile_datum, 0x1AC);