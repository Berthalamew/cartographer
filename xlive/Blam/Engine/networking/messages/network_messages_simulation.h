#pragma once
#include "simulation/simulation_view.h"

/* structures */

struct s_network_message_view_establishment
{
	e_simulation_view_establishment_mode establishment_mode;
	int32 establishment_identifier;
};

/* prototypes */

void __cdecl network_message_types_register_simulation(class c_network_message_type_collection* message_collection);
