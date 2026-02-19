#include "stdafx.h"
#include "simulation_encoding.h"

#include "simulation/simulation_players.h"

/* public code */

void __cdecl simulation_player_update_encode(c_bitstream* packet, const simulation_player_update* player_update)
{
	ASSERT(packet);
	ASSERT(player_update);
	ASSERT(VALID_INDEX(player_update->update_type, k_simulation_player_update_type_count));

	INVOKE(0x1E06AB, 0x1C7B6B, simulation_player_update_encode, packet, player_update);
	return;
}

bool __cdecl simulation_player_update_decode(c_bitstream* packet, simulation_player_update* player_update)
{
	ASSERT(packet);
	ASSERT(player_update);

	return INVOKE(0x1E078A, 0x1C7C4A, simulation_player_update_decode, packet, player_update);
}
