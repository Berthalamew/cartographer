#pragma once
#include "machine_id.h"
#include "simulation_players.h"

#include "memory/bitstream.h"
#include "networking/network_constants.h"

/* structures */

struct simulation_machine_update
{
	uint32 machine_valid_mask;
	s_machine_identifier identifiers[k_network_maximum_machines_per_session];
};

/* prototypes */

void __cdecl simulation_player_update_encode(c_bitstream* packet, const simulation_player_update* player_update);

bool __cdecl simulation_player_update_decode(c_bitstream* packet, simulation_player_update* player_update);
