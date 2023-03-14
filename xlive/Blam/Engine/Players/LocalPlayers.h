#pragma once
#include "Blam/Cache/DataTypes/BlamPrimitiveType.h"
#include "Util/Memory.h"

#define ENGINE_MAX_LOCAL_PLAYERS (4)

static bool local_user_has_player(int user_index)
{
	typedef bool(__cdecl* local_user_has_player_t)(int user_index);
	local_user_has_player_t p_local_user_has_player = Memory::GetAddress<local_user_has_player_t>(0x5139B);
	return p_local_user_has_player(user_index);
}

static datum local_user_get_player_idx(int user_index)
{
	typedef datum(__cdecl* local_user_get_player_idx_t)(int user_index);
	local_user_get_player_idx_t p_local_user_has_player = Memory::GetAddress<local_user_get_player_idx_t>(0x5141D);
	return p_local_user_has_player(user_index);
}