#pragma once


// Unsure about the size for this struct...
struct firing_position_ref
{
	datum dynamic_firing_set_index;
	datum field_3F8;
	int16 field_3FC;
	uint8 gap_3F8[26];
	int32 current_position_index;
};
