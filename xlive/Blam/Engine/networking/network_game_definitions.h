#pragma once
#include "networking/transport/transport.h"
#include "saved_games/player_profile.h"

/* enums */

enum e_network_session_qos_status_data_type : int32
{
	_network_session_qos_status_data_none = 0,
	_network_session_qos_status_data_lobby_or_game,
	_network_session_qos_status_data_matchmaking,
	k_network_session_qos_status_data_type_count,
	k_network_session_qos_status_data_type_bits = 2,
};

/* structures */

struct s_player_identifier
{
	uint8 identifier[8];
};
ASSERT_STRUCT_SIZE(s_player_identifier, 8);

struct s_clan_identifier
{
	uint8 identifier[12];
};
ASSERT_STRUCT_SIZE(s_clan_identifier, 12);

struct s_player_configuration
{
	wchar_t player_name[XUSER_NAME_SIZE];
	int32 spawn_protection_time;
	int8 gap_24[28];

	s_player_appearance appearance;
	wchar_t clan_name[16];
	s_clan_identifier clan_identifiers;

	int8 team_index;
	int8 player_handicap_level;	// e_user_interface_controller_handicap
	int8 player_displayed_skill;
	int8 player_overall_skill;
	int8 player_is_griefer;
	int8 bungie_user_role;
	int8 achievement_flags;
	int8 unk2;
};
ASSERT_STRUCT_SIZE(s_player_configuration, 132);

#pragma pack(push, 1)
struct s_network_squad_status_data_game_details
{
	int16 field;
	int16 n2;
	wchar_t session_name[32];
	int8 gap[70];
	s_transport_session_description description;
	int8 gap1[14];
	int16 open_public_slot_count;
	int16 open_private_slot_count;
	int16 current_public_players;
	int16 current_private_players;
	int16 game_mode;
	int16 game_state;
	int16 network_game_type;
	int16 connection_quality;
	int8 gap2[176];
	int32 game_engine_type;
	int8 gap3[644];
	int16 player_count;
	int16 field_41E;
};
#pragma pack(pop)


struct s_network_squad_status_data
{
	struct s_header
	{
		int16 network_protocol_version;
		int16 network_platform;
		int32 executable_type;
		int32 executable_version;
		int32 compatible_version;
	};

	s_network_squad_status_data::s_header header;
	e_network_session_qos_status_data_type session_mode;
	s_network_squad_status_data_game_details game_details;
	int8 gap[1538];
};
ASSERT_STRUCT_SIZE(s_network_squad_status_data, 2616);

/* prototypes */

void player_appearance_encode(class c_bitstream* packet, struct s_player_appearance const* appearance);
bool player_appearance_decode(class c_bitstream* packet, struct s_player_appearance* appearance);
