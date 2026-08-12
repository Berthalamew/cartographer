#pragma once

/* structures */

// max count: MAXIMUM_RECORDED_ANIMATIONS_PER_MAP 1024
struct recorded_animation_definition
{
	char name[k_tag_string_length];
	byte version;
	byte raw_animation_data;
	byte unit_control_data_version;
	byte pad;
	short length_of_animation_ticks;
	short pad1[3];
	tag_data recorded_animation_event_stream; // DataSize: 2097152
};
ASSERT_STRUCT_SIZE(recorded_animation_definition, 52);
