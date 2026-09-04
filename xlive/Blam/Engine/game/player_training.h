#pragma once

/* constants */

enum
{
	k_player_training_count = 32,
};

/* enums */

enum
{
	_player_training_flag_not_in_multiplayer_bit = 0,
	k_player_training_flag_count
};

/* structures */

struct s_player_training_entry_data
{
	string_id display_string;		// comes out of the HUD text globals
	string_id display_string2;		// comes out of the HUD text globals, used for grouped prompt
	string_id display_string3;		// comes out of the HUD text globals, used for ungrouped prompt
	int16 max_display_time;			// how long the message can be on screen before being hidden
	int16 display_count;			// how many times a training message will get displayed (0-3 only!)
	int16 dissapear_delay;			// how long a displayed but untriggered message stays up
	int16 redisplay_delay;			// how long after display this message will stay hidden
	real32 display_delays;			// how long the event can be triggered before it's displayed

	int16 flags;					// k_player_training_flag_count
	int16 pad;
};
ASSERT_STRUCT_SIZE(s_player_training_entry_data, 28);

