#pragma once
#include "game/players.h"
#include "networking/network_constants.h"

/* structures */

struct simulation_machine_update
{
	uint32 machine_valid_mask;
	s_machine_identifier identifiers[k_network_maximum_machines_per_session];
};
