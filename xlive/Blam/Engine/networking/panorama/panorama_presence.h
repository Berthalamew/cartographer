#pragma once
#include <XLive/xbox/xbox.h>

/* enums */

enum e_context_id
{
	_context_id_variant = 2,
	_context_id_difficulty = 3,
	_context_id_singleplayer_map = 4,
	_context_id_map = 5,
	_context_id_presence = X_CONTEXT_PRESENCE,
	_context_id_game_type = X_CONTEXT_GAME_TYPE,
	_context_id_game_mode = X_CONTEXT_GAME_MODE
};

enum e_context_variant
{
	_context_variant_ctf = 0,
	_context_variant_slayer,
	_context_variant_oddball,
	_context_variant_koth,
	_context_variant_juggernaut,
	_context_variant_territories,
	_context_variant_assault,
	k_context_variant_count
};

enum e_context_presence
{
	_context_presence_mainmenu = 0,
	_context_presence_settings,
	_context_presence_server_browser,
	_context_presence_singleplayer,
	_context_presence_lobby,
	_context_presence_results,
	_context_presence_live_in_game,
	_context_presence_public_game,
	_context_presence_invite_only_game,
	_context_presence_network_in_game
};


/* prototypes */

void __cdecl panorama_presence_set(uint32 context);
