#pragma once
#include "Blam/Common/Common.h"
#include "Blam/Engine/Game/saved_games/game_variant.h"

enum e_game_simulation : __int8
{
	_game_simulation_none = 0x0,
	_game_simulation_local = 0x1,
	_game_simulation_synchronous_client = 0x2,
	_game_simulation_synchronous_server = 0x3,
	_game_simulation_distributed_client = 0x4,
	_game_simulation_distributed_server = 0x5,
	k_game_simulation_count = 0x6,
};

enum e_engine_type : int
{
	_single_player = 1,
	_multiplayer,
	_main_menu,
	_mutiplayer_shared,
	_single_player_shared
};

#pragma pack(push,1)
struct s_game_options
{
	e_engine_type m_engine_type;
	e_game_simulation m_simulation_type;
	char network_protocol_related;
	bool session_host_is_dedicated;
	__int16 tickrate;
	byte gap_C[4];
	byte gap_E[2];
	char rand_bytes[8];
	int random_seed;
	BYTE scenario_custom;
	__int16 field_1C;
	char custom_map_name[96];
	PAD(2);
	int campaign_map_id;
	int map_id;
	wchar_t map_tag_name[260];
	__int16 initial_sbsp;
	PAD(6);
	char playtest;
	PAD(1);
	__int16 difficulty;
	char coop;
	char field_29D;
	PAD(2);
	s_game_variant m_game_variant;
	DWORD menu_context;
	DWORD machine_flags;
	DWORD machines;
	DWORD field_3DC;
	PAD(94);
	WORD player_count;
	char gap_440[3];
	char field_443;
	char gap_444[4];
	char player_data[3392];
};
CHECK_STRUCT_SIZE(s_game_options, 0x1188);
#pragma pack(pop)
