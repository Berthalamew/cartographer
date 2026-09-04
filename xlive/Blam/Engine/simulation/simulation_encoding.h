#pragma once

/* prototypes */

void __cdecl simulation_player_update_encode(class c_bitstream* packet, const struct simulation_player_update* player_update);

bool __cdecl simulation_player_update_decode(class c_bitstream* packet, struct simulation_player_update* player_update);
