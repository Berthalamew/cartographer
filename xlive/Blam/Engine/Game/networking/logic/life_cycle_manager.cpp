#include "stdafx.h"
#include "life_cycle_manager.h"
#include "Util/Memory.h"

e_game_life_cycle get_game_life_cycle()
{
	typedef e_game_life_cycle(__cdecl get_lobby_state_t)();
	auto p_get_game_life_cycle = Memory::GetAddress<get_lobby_state_t*>(0x1AD660, 0x1A65DD);

	return p_get_game_life_cycle();
}
