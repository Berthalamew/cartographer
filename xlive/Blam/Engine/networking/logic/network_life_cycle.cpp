#include "stdafx.h"
#include "network_life_cycle.h"

/* public code */

bool __cdecl network_life_cycle_initialize(
	c_network_message_gateway* message_gateway,
	c_network_observer* observer,
	c_network_session_manager* session_manager,
	c_network_session* squad_session_one,
	c_network_session* squad_session_two)
{
	ASSERT(squad_session_one && squad_session_two);

	return INVOKE(0x1AD494, 0x1A6411, network_life_cycle_initialize, message_gateway, observer, session_manager, squad_session_one, squad_session_two);
}

bool __cdecl network_life_cycle_in_game_session(
	c_network_session** session)
{
	return INVOKE(0x1AD75C, 0x0, network_life_cycle_in_game_session, session);
}

bool __cdecl network_life_cycle_get_observer(
	c_network_observer** observer)
{
	return INVOKE(0x1AD7B7, 0x0, network_life_cycle_get_observer, observer);
}
