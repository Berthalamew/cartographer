#include "stdafx.h"
#include "network_session_interface.h"

#include "game/players.h"
#include "game/player_constants.h"
#include "networking/logic/life_cycle_manager.h"
#include "networking/session/network_session.h"
#include "networking/session/network_session_manager.h"
#include "networking/network_event.h"

/* structures */

struct s_network_session_interface_user
{
	bool user_exists;
	s_player_identifier player_identifier;
	e_controller_index controller_index;
	s_player_configuration player_data;
	int32 player_voice_exists;
	int32 player_text_chat_exists;
	uint32 user_update_timestamp[4];
	uint32 user_remove_timestamp[3];
};
ASSERT_STRUCT_SIZE(s_network_session_interface_user, 0xB8);

struct s_network_session_interface_globals
{
	bool initialised;
	uint8 gap_1;
	wchar_t machine_name[16];
	wchar_t session_name[32];
	uint8 qos_active;
	uint8 gap_63;
	uint8 gap_64[16];
	uint32 upstream_bandwidth_bps;
	uint32 downstream_bandwidth_bps;
	uint8 gap_7C[8];
	uint32 nat_type;
	uint32 field_88;
	uint32 field_8C;
	uint32 field_90;
	int32 current_map_progress_percentage;
	s_network_session_interface_user users[k_number_of_users];
	uint32 session_connection_identifiers[6];
	s_game_variant variants[2];
	uint8 gap_5F0[68];
	uint32 sessions_manager;
};
ASSERT_STRUCT_SIZE(s_network_session_interface_globals, 0x638);

/* prototypes */

static s_network_session_interface_globals* network_session_interface_globals_get(void);

/* public code */

bool __cdecl network_session_interface_initialize(
	c_network_session_manager* session_manager)
{
	return INVOKE(0x1B07B2, 0x1968DC, network_session_interface_initialize, session_manager);
}

const wchar_t* network_session_interface_get_session_name(void)
{
	s_network_session_interface_globals* session_interface_globals = network_session_interface_globals_get();

	return session_interface_globals->session_name;
}

bool network_group_session_get_membership(
	int32* update_number,
	int32* local_peer_index,
	int32* host_peer_index,
	int32* leader_peer_index,
	int32* peer_count,
	struct s_network_session_peer const** peers,
	int32* player_count,
	uint32* player_valid_flags,
	struct s_network_session_player const** players)
{
	bool success = false;
	c_network_session* session = NULL;

	if (network_life_cycle_in_squad_session(&session))
	{
		success = network_session_get_membership(
			session,
			update_number,
			local_peer_index,
			host_peer_index,
			leader_peer_index,
			peer_count,
			peers,
			player_count,
			player_valid_flags,
			players);
	}

	return success;
}


bool __cdecl network_session_interface_get_local_user_properties(
	int32 user_index,
	e_controller_index* controller_index,
	s_player_configuration* player_data,
	uint32* player_voice_settings,
	uint32* player_text_chat_settings)
{
	return INVOKE(0x1B10E0, 0x1970A8, network_session_interface_get_local_user_properties, user_index, controller_index, player_data, player_voice_settings, player_text_chat_settings);
}

bool network_session_interface_set_local_user_character_type(
	int32 user_index,
	e_character_type character_type)
{
	bool success = false;
	s_network_session_interface_user* user_properties = &network_session_interface_globals_get()->users[user_index];

	// Don't change the character type if the user doesn't exist
	if (user_properties->user_exists)
	{
		user_properties->player_data.appearance.player_character_type = character_type;
		success = true;
	}

	return success;
}

bool network_session_interface_get_local_user_identifier(
	int32 user_index,
	s_player_identifier* out_identifier)
{
	bool success = false;
	s_network_session_interface_user* user_properties = &network_session_interface_globals_get()->users[user_index];

	if (user_properties->user_exists)
	{
		*out_identifier = user_properties->player_identifier;
		success = true;
	}

	return success;
}

int32 network_session_interface_get_team_index(
	int32 user_index)
{
	s_network_session_interface_globals* session_interface_globals = network_session_interface_globals_get();

	ASSERT(user_index>=0 && user_index<k_number_of_users);
	ASSERT(session_interface_globals->users[user_index].user_exists);

	return session_interface_globals->users[user_index].player_data.team_index;
}

void network_session_interface_set_local_user_rank(
	int32 user_index,
	int8 rank)
{
	s_network_session_interface_globals* session_interface_globals = network_session_interface_globals_get();

	ASSERT(user_index>=0 && user_index<k_number_of_users);
	ASSERT(session_interface_globals->users[user_index].user_exists);

	session_interface_globals->users[user_index].player_data.player_displayed_skill = rank;
	session_interface_globals->users[user_index].player_data.player_overall_skill = rank;
	
	return;
}

int32 __cdecl network_session_interface_add_local_user(
	s_player_identifier const* user_identifier)
{
	return INVOKE(0x1B1031, 0x0, network_session_interface_add_local_user, user_identifier);
}

void network_session_interface_set_user_identifier(
	int32 user_index,
	s_player_identifier const* user_identifier)
{
	s_network_session_interface_user* user_properties;

	ASSERT(0<=user_index && user_index<k_number_of_users);
	ASSERT(user_identifier!=NULL);

	user_properties = &network_session_interface_globals_get()->users[user_index];

	csmemcpy(&user_properties->player_identifier, user_identifier, sizeof(user_properties->player_identifier));

	event(_event_message, "logic:session: local user %d set user identifier=%s", user_index, player_identifier_get_string(&user_properties->player_identifier));

	return;
}

void network_session_interface_set_local_user_properties(
	int32 user_index,
	e_controller_index controller_index,
	s_player_configuration const* p_data,
	int32 player_voice_exists,
	int32 player_text_chat_exists)
{
	s_network_session_interface_globals* session_interface_globals = network_session_interface_globals_get();

	ASSERT(user_index>=0 && user_index<k_number_of_users);
	ASSERT(controller_index>=0 && controller_index<k_number_of_controllers);
	ASSERT(p_data);
	
	ASSERT(ustrlen(p_data->name)<=NUMBEROF(p_data->name));

	ASSERT(player_appearance_valid(&p_data->appearance));

	ASSERT(session_interface_globals->users[user_index].user_exists);

	session_interface_globals->users[user_index].controller_index = controller_index;
	session_interface_globals->users[user_index].player_data = *p_data;
	session_interface_globals->users[user_index].player_voice_exists = player_voice_exists;
	session_interface_globals->users[user_index].player_text_chat_exists = player_text_chat_exists;
	
	return;
}

bool network_session_get_membership(
	class c_network_session* session,
	int32* update_number,
	int32* local_peer_index,
	int32* host_peer_index,
	int32* leader_peer_index,
	int32* peer_count,
	struct s_network_session_peer const **peers,
	int32* player_count,
	uint32* player_valid_flags,
	struct s_network_session_player const **players)
{
	bool success = false;
	
	ASSERT(session);

	if (session->established())
	{
		int32 current_local_peer_index;
		int32 current_host_peer_index;

		s_session_membership const* membership = session->get_session_membership(&current_local_peer_index, &current_host_peer_index);
		int32 current_update_number = session->get_local_session_membership_update_number();

		ASSERT(membership);
		ASSERT(current_local_peer_index>=0 && current_local_peer_index<membership->peer_count);
		ASSERT(current_host_peer_index>=0 && current_host_peer_index<membership->peer_count);
		ASSERT(membership->leader_peer_index>=0 && membership->leader_peer_index<membership->peer_count);
		ASSERT(current_update_number!=NONE);

		if (update_number)
		{
			*update_number = current_update_number;
		}
		if (local_peer_index)
		{
			*local_peer_index = current_local_peer_index;
		}
		if (host_peer_index)
		{
			*host_peer_index = current_host_peer_index;
		}
		if (leader_peer_index)
		{
			*leader_peer_index = membership->leader_peer_index;
		}
		if (peer_count)
		{
			*peer_count = membership->peer_count;
		}
		if (peers)
		{
			*peers = membership->peers;
		}
		if (player_count)
		{
			*player_count = membership->player_count;
		}
		if (player_valid_flags)
		{
			*player_valid_flags = membership->player_valid_flags;
		}
		if (players)
		{
			*players = membership->players;
		}
	}

	return success;
}

e_network_session_class network_squad_session_get_session_class(void)
{
	//return INVOKE(0x1B1643, 0x0, network_squad_session_get_session_class);

	e_network_session_class out_class = _network_session_class_unknown;
	c_network_session* session = NULL;

	if (network_life_cycle_in_squad_session(&session))
	{
		ASSERT(session);

		if (session->established())
		{
			out_class = session->m_session_class;
		}
	}

	return out_class;
}

bool network_session_is_online(
	e_network_session_class session_class)
{
	return session_class==_network_session_class_xbox_live;
}

bool network_squad_session_delegate_leadership(
	int32 player_index)
{
	int32 local_peer_index;
	int32 leader_peer_index;
	int32 peer_count;
	uint32 player_valid_flags;
	s_network_session_peer const* peers;
	s_network_session_player const* players;

	bool delegated = false;
	c_network_session* squad_session = NULL;

	ASSERT(player_index>=0 && player_index<k_maximum_players);

	if (network_life_cycle_in_squad_session(&squad_session) &&
		network_group_session_get_membership(NULL, &local_peer_index, NULL, &leader_peer_index, &peer_count, &peers, NULL, &player_valid_flags, &players))
	{
		if (local_peer_index == leader_peer_index)
		{
			if (TEST_BIT(player_valid_flags, player_index))
			{
				s_network_session_player const* player = &players[player_index];

				if (player->peer_index!=local_peer_index)
				{
					ASSERT(player->peer_index>=0 && player->peer_index<peer_count);
					
					if (squad_session->leader_request_delegate_leadership(&peers[player->peer_index].secure_address))
					{
						delegated = true;
					}
					else
					{
						event(
							_event_error,
							"logic:squad: failed to delegate leadership to player [#%d] peer [#%d]",
							player_index,
							player->peer_index
						);
					}
				}
				else
				{
					event(
						_event_error,
						"logic:squad: can't delegate leadership to player [#%d], they are on our local machine [peer #%d]",
						player_index,
						local_peer_index
					);
				}
			}
			else
			{
				event(_event_error, "logic:squad: can't delegate leadership to invalid player index [#%d]", player_index);
			}
		}
		else
		{
			event(_event_error, "logic:squad: can't delegate leadership, we are not leader of our squad");
		}
	}
	else
	{
		event(_event_error, "logic:squad: can't delegate leadership, no established squad session exists");
	}

	return delegated;
}

bool network_squad_session_boot_player(
	int32 player_index)
{
	int32 local_peer_index;
	int32 leader_peer_index;
	int32 peer_count;
	uint32 player_valid_flags;
	s_network_session_peer const* peers;
	s_network_session_player const* players;

	bool booted = false;
	c_network_session* squad_session = NULL;

	ASSERT(player_index>=0 && player_index<k_maximum_players);


	if (network_life_cycle_in_squad_session(&squad_session) &&
		network_group_session_get_membership(NULL, &local_peer_index, NULL, &leader_peer_index, &peer_count, &peers, NULL, &player_valid_flags, &players))
	{
		if (local_peer_index==leader_peer_index)
		{
			if (TEST_BIT(player_valid_flags, player_index))
			{
				s_network_session_player const* player = &players[player_index];
			
				if (player->peer_index!=local_peer_index)
				{
					ASSERT(player->peer_index>=0 && player->peer_index<peer_count);

					if (squad_session->leader_request_boot_machine(&peers[player->peer_index].secure_address))
					{
						booted = true;
					}
					else
					{
						event(
							_event_error,
							"logic:squad: failed to boot player [#%d] peer [#%d]",
							player_index,
							player->peer_index
						);
					}
				}
				else
				{
					event(
						_event_error,
						"logic:squad: failed to boot player [#%d], they are on our local machine peer [#%d]",
						player_index,
						local_peer_index
					);
				}
			}
			else
			{
				event(_event_error, "logic:squad: can't boot invalid player index [#%d]", player_index);
			}
		}
		else
		{
			event(
				_event_error,
				"logic:squad: can't boot player [#%d], we are not leader of our squad us [#%d] != leader [#%d]",
				player_index,
				local_peer_index,
				leader_peer_index
			);
		}
	}
	else
	{
		event(_event_error, "logic:squad: can't boot player, no squad session exists");
	}

	return booted;
}

/* private code */

static s_network_session_interface_globals* network_session_interface_globals_get(void)
{
	return Memory::GetAddress<s_network_session_interface_globals*>(0x51A590, 0x520408);
}
