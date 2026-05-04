#pragma once

/* enums */

enum e_session_game_mode
{
	_session_game_mode_none = 0,
	_session_game_mode_browsing,
	_session_game_mode_pregame,
	_session_game_mode_ingame,
	_session_game_mode_unknown,
	_session_game_mode_postgame,
	_session_game_mode_joining,
	_session_game_mode_matchmaking,
	k_number_of_session_game_modes,
	k_number_of_session_game_bits = 3,
};

enum e_session_protocol
{
	_session_protocol_splitscreen_coop = 0,
	_session_protocol_splitscreen_custom,
	_session_protocol_system_link_coop,
	_session_protocol_system_link_custom,
	_session_protocol_xbox_live_coop,
	_session_protocol_xbox_live_custom,
	_session_protocol_xbox_live_optimatch,
};

/* structures */

struct s_game_auto_join_globals
{
	bool do_auto_join;
	XSESSION_INFO auto_join_session;
};

/* public methods */

bool session_protocol_has_coop(e_session_protocol protocol);
bool __cdecl user_interface_create_new_squad(bool a1, bool online);
bool user_interface_squad_local_peer_is_host(void);
bool __cdecl user_interface_squad_local_peer_is_leader();
bool __cdecl user_interface_session_get_map(uint32* campaign_id, uint32* map_id, uint32* custom_map_id);
bool __cdecl user_interface_squad_session_is_xbox_live(void);
bool user_interface_squad_is_booting_allowed(void);
e_session_game_mode user_interface_get_session_game_mode(void);

int16 __cdecl user_interface_session_get_campaign_difficulty(void);
int16 __cdecl user_interface_squad_get_player_count();
e_session_protocol __cdecl user_interface_squad_get_active_protocol();
struct s_game_variant* __cdecl user_interface_session_get_game_variant(void);
void user_interface_networking_leave_squad(bool immediate);
bool user_interface_squad_delegate_leadership(int32 player_index);
void user_interface_networking_set_globals(bool a1, XSESSION_INFO* session, int32 unused, bool from_game_invite);
void __cdecl user_interface_networking_reset_player_counts(void);
void __cdecl user_interface_squad_clear_match_playlist(void);
void __cdecl user_interface_squad_clear_game_settings();
void __cdecl user_interface_squad_set_campaign_difficulty(int32 difficulty);
void __cdecl user_interface_set_desired_multiplayer_mode(int32 desired_mode);
bool user_interface_squad_is_player_valid(int32 player_index);
bool user_interface_squad_is_local_player(int32 player_index);
int32 user_interface_squad_get_player_index(struct s_player_identifier const* player_identifier);
bool user_interface_squad_boot_player(int32 player_index);

void user_interface_networking_join_game(XSESSION_INFO* session, int32 a2, bool from_game_invite);
void user_interface_networking_join_game_direct(XNKID kid, XNKEY key, const XNADDR* addr, int8 exe_type, int32 exe_version, int32 comp_version);
void user_interface_networking_update_auto_join();

/* globals */

extern s_game_auto_join_globals g_game_auto_join;
