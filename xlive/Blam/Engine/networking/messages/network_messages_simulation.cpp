#include "stdafx.h"
#include "network_messages_simulation.h"

#include "network_message_type_collection.h"

/* public code */

void __cdecl network_message_types_register_simulation(c_network_message_type_collection* message_collection)
{
	INVOKE(0x1ED5C7, 0x1CDF80, network_message_types_register_simulation, message_collection);
	return;
}
