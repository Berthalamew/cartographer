#include "stdafx.h"
#include "network_game_definitions.h"

#include "memory/bitstream.h"
#include "game/players.h"

/* public code */

void player_appearance_encode(
	c_bitstream* packet,
	s_player_appearance const* appearance)
{
	packet->write_integer("primary-color", appearance->change_color_index[0] + 1, 5);
	packet->write_integer("secondary-color", appearance->change_color_index[1]+1, 5);
	packet->write_integer("tertiary-color", appearance->change_color_index[2]+1, 5);
	packet->write_integer("quaternary-color", appearance->change_color_index[3]+1, 5);
	packet->write_integer("player character type", appearance->player_character_type+1, 3);
	packet->write_integer("foreground-emblem", appearance->emblem_info.foreground_emblem, 6);
	packet->write_integer("background-emblem", appearance->emblem_info.background_emblem, 6);
	packet->write_integer("emblem-flags", appearance->emblem_info.emblem_flags.get_unsafe(), 4);

	return;
}

bool player_appearance_decode(
	c_bitstream* packet,
	s_player_appearance* appearance)
{
	csmemset(appearance, 0, sizeof(*appearance));
	
	appearance->change_color_index[0].set_raw_value((int8)packet->read_integer("primary-color", 5)-1);
	appearance->change_color_index[1].set_raw_value((int8)packet->read_integer("secondary-color", 5)-1);
	appearance->change_color_index[2].set_raw_value((int8)packet->read_integer("tertiary-color", 5)-1);
	appearance->change_color_index[3].set_raw_value((int8)packet->read_integer("quaternary-color", 5)-1);

	appearance->player_character_type.set_raw_value((int8)packet->read_integer("player character type", 3)-1);
	appearance->emblem_info.foreground_emblem = (e_emblem_foreground)packet->read_integer("foreground-emblem", 6);
	appearance->emblem_info.background_emblem = (e_emblem_background)packet->read_integer("background-emblem", 6);
	
	appearance->emblem_info.emblem_flags.set_unsafe((uint8)packet->read_integer("emblem-flags", 4));
	
	return player_appearance_valid(appearance);
}
