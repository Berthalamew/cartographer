#pragma once
#include "machine_id.h"
#include "networking/network_constants.h"

/* structures */

struct simulation_update;

struct simulation_machine_update
{
	uint32 machine_valid_mask;
	s_machine_identifier identifiers[k_network_maximum_machines_per_session];
};

/* prototypes */

void __cdecl simulation_player_update_encode(class c_bitstream* packet, const struct simulation_player_update* player_update);

bool __cdecl simulation_player_update_decode(class c_bitstream* packet, struct simulation_player_update* player_update);

void __cdecl synchronous_update_encode(c_bitstream* stream, struct simulation_update* update);
bool __cdecl synchronous_update_decode(c_bitstream* stream, struct simulation_update* update);
