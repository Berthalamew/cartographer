#pragma once

// max count: 1
struct s_game_globals_multiplayer_information
{
	tag_reference flag;         // item
	tag_reference unit;         // unit

	// max count: 20
	s_tag_block vehicles;		// s_game_globals_tag_reference   

	tag_reference hill_shader;  // shad
	tag_reference flag_shader;  // shad
	tag_reference ball;         // item

	// max count: 60
	s_tag_block sounds;			// s_game_globals_tag_reference     

	tag_reference in_game_text; // unic
	int32 pad[10];
	s_tag_block general_events;	// struct: s_multiplayer_event_response_definition
	s_tag_block slayer_events;	// struct: s_multiplayer_event_response_definition
	s_tag_block ctf_events;		// struct: s_multiplayer_event_response_definition
	s_tag_block oddball_events;	// struct: s_multiplayer_event_response_definition
	s_tag_block unk_block;
	s_tag_block king_events;	// struct: s_multiplayer_event_response_definition
};
ASSERT_STRUCT_SIZE(s_game_globals_multiplayer_information, 152);
