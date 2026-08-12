#pragma once
#include "animations/animation_channel.h"
#include "objects/object_definition.h"
#include "objects/objects.h"

/* enums */

enum e_device_definition_flags : int32
{
	_device_definition_position_loops = FLAG(0),
	_device_definition_unused = FLAG(1),
	_device_definition_allow_interpolation = FLAG(2)
};

enum e_device_lightmap_flags : int16
{
	_lightmap_dont_use_in_lightmap = FLAG(0),
	_lightmap_dont_use_in_lightprobe = FLAG(1),
};

enum e_device_group_flags
{
	_device_group_can_change_only_once_bit = 0,
	_device_group_changed_once_bit,
	_device_group_runtime_bit,
	NUMBER_OF_DEVICE_GROUP_FLAGS,
};

/* structures */

struct _device_definition
{
	// Explaination("$$$ DEVICE $$$", "")

	e_device_definition_flags flags;
	real32 power_transition_time_seconds;
	real32 power_acceleration_time_seconds;
	real32 position_transition_time_seconds;
	real32 position_acceleration_time_seconds;
	real32 depowered_position_transition_time_seconds;
	real32 depowered_position_acceleration_time_seconds;

	e_device_lightmap_flags lightmap_flags;
	int16 pad;

	// Sounds
	tag_reference open;     // up
	tag_reference close;    // down
	tag_reference opened;
	tag_reference closed;
	tag_reference depowered;
	tag_reference repowered;

	real32 delay_time_seconds;
	tag_reference delay_effect;
	real32 automatic_activation_radius; // World units
};

struct device_definition
{
	_object_definition object;
	_device_definition device;
};
ASSERT_STRUCT_SIZE(device_definition, 284);

struct s_device_animation_args
{
	real32 current_position;
	real32 start_velocity;
	real32 target_position1;
	real32 target_position2;
	real32 current_displacement;
	real32 displacement_to_travel;
	real32 initial_velocity;
	real32 total_time;
};
ASSERT_STRUCT_SIZE(s_device_animation_args, 32);

struct device_datum
{
	object_datum object;
	uint32 flags;
	datum power_group_index;
	real32 power_ratio;
	real32 power;
	datum position_group_index;
	real32 position_ratio;
	real32 device_position;
	int16 ticks;
	int16 pad;
	s_device_animation_args position_args;
	s_device_animation_args overlay_args;
	c_animation_channel position_channel;
	c_animation_channel power_and_overlay_track_channel;
};
ASSERT_STRUCT_SIZE(device_datum, 460);

struct device_group_datum
{
	int16 identifier;
	uint16 flags;
	real32 desired_value;
	real32 initial_value;
};
ASSERT_STRUCT_SIZE(device_group_datum, 12);

/* macros */

#define device_group_get(index) (device_group_datum*)datum_get(device_groups_data_get(), index)


/* prototypes */

struct data_array* device_groups_data_get(void);

int32 device_group_get_from_scenario_index(int16 scenario_device_group_index);

