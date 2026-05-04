#include "stdafx.h"
#include "user_interface_guide.h"

#include "user_interface_networking.h"
#include "user_interface_shared_globals.h"

#include "achievements/achievement_manager.h"
#include "game/game_time.h"
#include "game/players.h"
#include "interface/screens/screen_cartographer_account_manager.h"
#include "main/main.h"
#include "main/main_game.h"
#include "networking/logic/network_life_cycle.h"
#include "networking/logic/network_session_interface.h"
#include "networking/panorama/panorama_achievements.h"
#include "networking/panorama/panorama_favorites.h"
#include "networking/panorama/panorama_friends.h"
#include "networking/panorama/panorama_presence.h"
#include "networking/panorama/panorama_user_profile.h"
#include "networking/panorama/panorama_user_history.h"
#include "networking/session/network_session.h"
#include "networking/network_event.h"
#include "scenario/scenario.h"
#include "scenario/scenario_definitions.h"
#include "sound/sound_manager.h"
#include "text/text_group.h"

#include <XLive/XAM/xam.h>
#include <XLive/XUser/XCustomAction.h>

/* prototypes */

CLASS_HOOK_DECLARE_LABEL(c_user_interface_guide_state_manager__update, c_user_interface_guide_state_manager::update);
static void __declspec(naked) jmp_c_user_interface_guide_state_manager__update(void)
{
	CLASS_HOOK_JMP(c_user_interface_guide_state_manager__update, c_user_interface_guide_state_manager::update);
}

static bool user_interface_guide_type_allowed(e_user_interface_guide_state_type type);
static void user_interface_guide_restart_for_update(void);

/* public code */

void user_interface_guide_apply_patches(void)
{
	PatchCall(Memory::GetAddress(0x20CAB3, 0x0), jmp_c_user_interface_guide_state_manager__update);

	return;
}

c_user_interface_guide_state_manager* user_interface_guide_state_manager_get(void)
{
	return Memory::GetAddress<c_user_interface_guide_state_manager*>(0x9712C8, 0x4504D0);
}

c_panorama_favorites* user_interface_guide_favorites_get(void)
{
	return panorama_favorites_get();
}

c_panorama_friends* user_interface_guide_friends_get(void)
{
	return panorama_friends_get();
}

c_panorama_achievements* user_interface_guide_achievements_get(void)
{
	return panorama_achievements_get();
}

c_panorama_user_profile* user_interface_guide_profile_get(void)
{
	return panorama_user_profile_get();
}

void c_user_interface_guide_state_manager::add_user_signin_task(
	bool sign_to_live,
	bool(*signin_callback)(void*))
{
	//INVOKE_TYPE(0xDD7550, 0x0, int(__thiscall*)(c_user_interface_guide_state_manager*, bool, void*), this, sign_to_live, signin_callback);
	
	m_callback_task = signin_callback;
	
	cartographer_account_manager_open_list();
	
	return;
}

void c_user_interface_guide_state_manager::update(void)
{
	if (m_xnotify_listener)
	{
		bool run_callback;
		uint32 id;
		uint32 param;
		
		e_scenario_type scenario_type;
		struct scenario* scenario;

		e_controller_index controller_index = g_controller_logging_in;
		int32 user_index = user_interface_controller_get_user_index(controller_index);
		
		if (user_index==NONE)
		{
			user_index = 0;
		}

		if (m_update_sign_in_state)
		{
			update_sign_in_state(controller_index, user_index, XUserGetSigninState(user_index));

			m_update_sign_in_state = false;
		}

		if (m_started_custom_player_list_action && m_xoverlapped.InternalLow != ERROR_IO_PENDING)
		{
			uint32 result = 0;

			XGetOverlappedResult(&m_xoverlapped, &result, FALSE);

			for (int32 type = 0; type<NUMBEROF(m_buttons); ++type)
			{
				s_user_interface_guide_state_manager_button* button = &m_buttons[type];
				if (button->field_0 && button->field_38 == m_player_list_result.dwKeyCode)
				{
					m_started_custom_player_list_action = false;
					update_option((e_user_interface_guide_state_type)type, (s_player_identifier*)&m_player_list_result.xuidSelected);
					button->field_0 = false;
				}
			}

			csmemset(&m_xoverlapped, 0, sizeof(m_xoverlapped));
		}

		id = 0;
		param = 0;
		run_callback = false;

		if (XNotifyGetNext(m_xnotify_listener, 0, &id, &param))
		{
			switch (id)
			{
			case XN_SYS_UI:
				break;
			case XN_SYS_SIGNINCHANGED:
				update_sign_in_state(controller_index, user_index, XUserGetSigninState(user_index));
				break;
			case XN_SYS_PROFILESETTINGCHANGED:
				break;
			default:
				if (id > 20)
				{
					if (id <= 22)
					{
						user_interface_guide_restart_for_update();
					}
					else if (id == XN_LIVE_INVITE_ACCEPTED)
					{
						int32 invite_result;
						XINVITE_INFO invite_info;

						ASSERT(param == 0);

						csmemset(&invite_info, 0, sizeof(invite_info));
						invite_result = XInviteGetAcceptedInfo(param, &invite_info);

						if (invite_result!=ERROR_SUCCESS)
						{
							event(_event_error, "c_user_interface_guide_state_manager::update: XInviteGetAcceptedInfo failed (%d)", invite_result);
						}
						else
						{
							achievement_manager_get()->start_level_chosen(0);
							
							m_xsession_info = invite_info.hostInfo;
							m_from_game_invite = invite_info.fFromGameInvite;
							m_field_D = true;
						}
					}
					else if (id==XN_CUSTOM_ACTIONPRESSED)
					{
						// TODO: finish code
						// Custom actions aren't implemented properly in cartographer so we don't really need it.....
					}
				}
				break;
			}
		}

		scenario = global_scenario_try_and_get();

		if (scenario)
		{
			scenario_type = scenario->type;
		}
		else
		{
			scenario_type = _scenario_type_invalid;
		}


		if (run_callback)
		{
			if (m_callback_task && !m_block_game_input)
			{
				m_callback_task(NULL);
				m_callback_task = NULL;
			}

			if (scenario_type == _scenario_type_solo)
			{
				if (m_block_game_input)
				{
					game_time_set_paused(true);
					sound_pause(false);
				}
				else if (!user_interface_channel_is_active())
				{
					game_time_set_paused(false);
					sound_pause(true);
				}
			}
		}

		if (m_field_D && id==XN_LIVE_INVITE_ACCEPTED && !m_field_F)
		{
			if (scenario_type == _scenario_type_solo)
			{
				main_save_and_exit_campaign_immediately();
			}
			else if (scenario_type != _scenario_type_multiplayer)
			{
				if (scenario_type != _scenario_type_main_menu)
				{

				}
				else if (user_interface_globals_get_edit_player_profile())
				{
					user_interface_globals_save_profile_changes_to_disk();
				}
			}
			else
			{
				if (m_sign_in_state)
				{
					int32 player_count = 0;

					network_group_session_get_membership(NULL, NULL, NULL, NULL, NULL, NULL, &player_count, NULL, NULL);
					
					if (user_interface_squad_local_peer_is_host() && user_interface_networking_squad_is_online() && player_count > 1)
					{
						m_field_F = true;
					}
					else if (joining_separate_game_after_delegation())
					{
						user_interface_networking_leave_squad(true);
					}
				}
			}
			
			if (!m_field_F)
			{
				user_interface_networking_join_game(&m_xsession_info, 0, m_from_game_invite);
				clear_invite_flags();
			}
		}

		if (!m_block_game_input)
		{
			clear_custom_actions();

			if (!m_block_game_input && m_field_F)
			{
				c_maximum_interface_text text;

				user_interface_guide_string_get(_string_id_guide_party_management_leave_and_designate_button_caption, &text);
				set_custom_action(0, text.get_string(), FLAG(0), _user_interface_guide_state_type_leave_and_designate);
				show_player_list(
					_user_interface_guide_state_type_bring,
					_user_interface_guide_state_type_invalid,
					0,
					_string_id_guide_party_management_leaving_game_title,
					_string_id_guide_party_management_leaving_game_description
				);
				
				m_field_F = false;
			}
		}
	}

	return;
}

void c_user_interface_guide_state_manager::update_dedicated_server(void)
{
	if (m_xnotify_listener)
	{
		uint32 id = 0;
		uint32 param = 0;

		if (XNotifyGetNext(m_xnotify_listener, 0, &id, &param))
		{
			switch (id)
			{
			case XN_SYS_SIGNINCHANGED:
				update_sign_in_state_dedicated_server(XUserGetSigninState(0));
				break;
			}
		}
	}

	return;
}

void user_interface_guide_string_get(
	string_id id,
	c_maximum_interface_text* text)
{
	ASSERT(text);

	s_user_interface_shared_globals* user_interface_shared_globals = user_interface_shared_globals_get();

	if (user_interface_shared_globals)
	{
		string_list_get_normal_string(user_interface_shared_globals->unicode_string_list_tag.index, id, text);
	}
	else
	{
		text->set(L"Default");
	}

	return;
}

/* private code */

void c_user_interface_guide_state_manager::initialize_buttons(void)
{
	c_maximum_interface_text text;

	user_interface_guide_string_get(_string_id_guide_party_management_boot_button_caption, &text);
	m_buttons[0].button.dwType = 6;
	ustrncpy(m_buttons[0].button.wszCustomText, text.get_string(), NUMBEROF(m_buttons[0].button.wszCustomText));

	user_interface_guide_string_get(_string_id_guide_party_management_mute_button_caption, &text);
	m_buttons[0].button.dwType = 6;
	ustrncpy(m_buttons[1].button.wszCustomText, text.get_string(), NUMBEROF(m_buttons[1].button.wszCustomText));
	
	user_interface_guide_string_get(_string_id_guide_party_management_bring_button_caption, &text);
	m_buttons[2].button.dwType = 5;
	ustrncpy(m_buttons[2].button.wszCustomText, text.get_string(), NUMBEROF(m_buttons[2].button.wszCustomText));

	user_interface_guide_string_get(_string_id_guide_party_management_leave_and_designate_button_caption, &text);
	m_buttons[3].button.dwType = 6;
	ustrncpy(m_buttons[3].button.wszCustomText, text.get_string(), NUMBEROF(m_buttons[3].button.wszCustomText));
	
	m_buttons_initialized = 1;

	return;
}

bool c_user_interface_guide_state_manager::joining_separate_game_after_delegation(void) const
{
	return
		user_interface_get_session_game_mode() != _session_game_mode_postgame ||
		m_custom_action_type == _user_interface_guide_state_type_leave_and_designate;
}

void c_user_interface_guide_state_manager::clear_custom_actions(void)
{
	if (m_clear_actions)
	{
		XCustomSetAction(_user_interface_guide_state_type_boot, NULL, 0);
		XCustomSetAction(_user_interface_guide_state_type_mute, NULL, 0);
		XCustomSetAction(_user_interface_guide_state_type_bring, NULL, 0);
		m_custom_action_type = _user_interface_guide_state_type_invalid;
		m_clear_actions = false;
	}

	return;
}

bool c_user_interface_guide_state_manager::set_custom_action(
	int32 action_index,
	const wchar_t* action_text,
	uint32 flags,
	e_user_interface_guide_state_type custom_action_type)
{
	XCustomSetAction(action_index, action_text, flags);
	m_custom_action_type = custom_action_type;
	m_clear_actions = true;

	return true;
}

void c_user_interface_guide_state_manager::show_player_list(
	e_user_interface_guide_state_type type_x,
	e_user_interface_guide_state_type type_y,
	int32 a3,
	string_id title,
	string_id description)
{
	c_network_session* session = NULL;

	network_life_cycle_in_session(&session);

	for (int32 button_index = 0; button_index<NUMBEROF(m_buttons); ++button_index)
	{
		m_buttons[button_index].field_0 = false;
	}

	if (m_xoverlapped.InternalLow == ERROR_IO_PENDING)
	{
		XCancelOverlapped(&m_xoverlapped);
	}

	csmemset(&m_player_list_result, 0, sizeof(m_player_list_result));

	if (session && session->session_class()== _network_session_class_xbox_live)
	{
		int32 player_count = get_player_count(session, 1);

		if (player_count>0)
		{
			XPLAYERLIST_BUTTON* button_x = NULL;
			XPLAYERLIST_BUTTON* button_y = NULL;

			if (!m_buttons_initialized)
			{
				initialize_buttons();
			}

			if (type_x!=_user_interface_guide_state_type_invalid && user_interface_guide_type_allowed(type_x))
			{
				button_x = &m_buttons[type_x].button;

				m_buttons[type_x].field_0 = true;
				m_buttons[type_x].field_38 = 0x5802;
			}

			if (type_y!=_user_interface_guide_state_type_invalid && user_interface_guide_type_allowed(type_y))
			{
				button_y = &m_buttons[type_y].button;
				
				m_buttons[type_y].field_0 = true;
				m_buttons[type_y].field_38 = 0x5803;
			}

			c_maximum_interface_text title_string;
			c_maximum_interface_text description_string;

			user_interface_guide_string_get(title, &title_string);
			user_interface_guide_string_get(description, &description_string);

			if (XShowCustomPlayerListUI(
				0,
				FLAG(0),
				title_string.get_string(),
				description_string.get_string(),
				NULL,
				NULL,
				m_player_data,
				player_count,
				button_x,
				button_y,
				&m_player_list_result,
				&m_xoverlapped) == ERROR_IO_PENDING)
			{
				m_started_custom_player_list_action = true;
			}
			else
			{
				error(_error_delayed, "Failed to bring up guide in player list mode");

				for (int32 button_index = 0; button_index < NUMBEROF(m_buttons); ++button_index)
				{
					m_buttons[button_index].field_0 = false;
				}
			}
		}
	}

	return;
}

void c_user_interface_guide_state_manager::update_sign_in_state(
	e_controller_index controller_index,
	int32 user_index,
	XUSER_SIGNIN_STATE sign_in_state)
{
	XUSER_SIGNIN_STATE previous_sign_in_state = m_sign_in_state;

	ASSERT(user_index>=0 && user_index<=k_number_of_users);

	if (sign_in_state==eXUserSigninState_SignedInLocally)
	{
		m_sign_in_state = eXUserSigninState_SignedInLocally;
		
		// From signed into live -> locally signed in
		if (previous_sign_in_state== eXUserSigninState_SignedInToLive)
		{
			user_interface_controller_switch_to_offline(controller_index);
			user_interface_controller_update_network_properties(controller_index);
			
			user_interface_guide_friends_get()->cancel_task();
			user_interface_guide_achievements_get()->dispose();

			if (user_interface_force_load_mainmenu())
			{
				network_life_cycle_end();
				user_interface_dispose_all_active_ui();
				main_menu_launch_force();
			}
		}

	}
	else if (sign_in_state==eXUserSigninState_SignedInToLive)
	{
		XUID xuid;
		uint32 name_result;

		m_sign_in_state = eXUserSigninState_SignedInToLive;

		name_result = XUserGetName(user_index, m_gamertag, NUMBEROF(m_gamertag));

		if (name_result)
		{
			error(_error_delayed, "XUserGetName status: %d", name_result);
			
#ifdef ASSERTS_ENABLED
			vassert(false, "", NULL);
#endif

			m_gamertag[0] = '\0';
		}

		panorama_presence_set(_context_presence_mainmenu);

		user_interface_guide_friends_get()->cancel_task();
		user_interface_guide_achievements_get()->dispose();
		user_interface_guide_friends_get()->start();
		user_interface_guide_favorites_get()->start_download();
		user_interface_guide_achievements_get()->cancel_task();
		user_interface_guide_achievements_get()->enumerate();
		user_interface_guide_profile_get()->start_download();

		if (user_interface_controller_is_player_profile_valid(controller_index) &&
			!XUserGetXUID(user_index, &xuid))
		{
			user_interface_controller_set_xbox_live_account(controller_index, &xuid);
			user_interface_controller_xbox_live_account_set_signed_in(controller_index, true);
		}
	}
	else
	{
		m_sign_in_state = eXUserSigninState_NotSignedIn;
		m_gamertag[0] = '\0';

		user_interface_controller_sign_out_all_controllers();

		// From signed in at all -> signing out
		if (previous_sign_in_state)
		{
			network_life_cycle_end();
			main_menu_launch_force();
		}
	}

	achievement_history_get()->handle_signin(m_sign_in_state);
	achievement_manager_get()->handle_live_signin_notification(m_sign_in_state==eXUserSigninState_SignedInToLive);

	//g_controller_logging_in = k_no_controller;

	return;
}

void c_user_interface_guide_state_manager::update_sign_in_state_dedicated_server(
	XUSER_SIGNIN_STATE sign_in_state)
{
	if (sign_in_state==eXUserSigninState_SignedInLocally)
	{
		m_sign_in_state = eXUserSigninState_SignedInLocally;
	}
	else if (sign_in_state==eXUserSigninState_SignedInToLive)
	{
		m_sign_in_state = eXUserSigninState_SignedInToLive;
	}
	else
	{
		m_sign_in_state = eXUserSigninState_NotSignedIn;
	}

	return;
}

int32 c_user_interface_guide_state_manager::get_player_count(
	c_network_session* session,
	int32 a3)
{
	int32 player_count;

	s_session_membership const* membership = session->get_session_membership(NULL, NULL);

	csmemset(m_player_data, 0, sizeof(m_player_data));
	
	player_count = 0;
	
	if (membership)
	{
		int32 current_player = 0;
		
		c_maximum_interface_text muted;
		c_maximum_interface_text not_muted;

		player_count = membership->player_count;

		user_interface_guide_string_get(_string_id_is_muted, &muted);
		user_interface_guide_string_get(_string_id_not_muted, &not_muted);

		for (int32 player_index = 0; player_index < NUMBEROF(membership->players); ++player_index)
		{
			s_network_session_player const* session_player = &membership->players[player_index];
			XPLAYERLIST_USER *playerlist_user = &m_player_data[player_index];

			ASSERT(&membership->player_valid_flags);

			if (BIT_VECTOR_TEST_FLAG(&membership->player_valid_flags, player_index))
			{
				uint32 player_text_chat_settings;

				playerlist_user->xuid= *(XUID*)session_player->identifier.identifier;
				
				if (!network_session_interface_get_local_user_properties(0, 0, 0, 0, &player_text_chat_settings))
				{
					vassert(false, "network_session_interface_get_local_user_properties failed", NULL);
				}

				ustrncpy(
					playerlist_user->wszCustomText, 
					TEST_BIT(player_text_chat_settings, player_index) ? not_muted.get_string() : muted.get_string(),
					NUMBEROF(playerlist_user->wszCustomText)
				);
			}

		}

		ASSERT(current_player == player_count);
	}
	
	return player_count;
}

void c_user_interface_guide_state_manager::update_option(
	e_user_interface_guide_state_type type,
	s_player_identifier* player_identifier)
{
	int32 player_index = user_interface_squad_get_player_index(player_identifier);

	switch (type)
	{
	case _user_interface_guide_state_type_boot:
		user_interface_squad_boot_player(player_index);
		break;
	case _user_interface_guide_state_type_mute:
	{
		e_controller_index controller_index;
		s_player_configuration player_data;
		uint32 player_voice;
		uint32 player_text;

		if (!network_session_interface_get_local_user_properties(NULL, &controller_index, &player_data, &player_voice, &player_text))
		{
			vassert(false, "network_session_interface_get_local_user_properties failed", NULL);
		}

		SET_BIT(player_text, player_index, !TEST_BIT(player_text, player_index));
		SET_BIT(player_voice, player_index, !TEST_BIT(player_voice, player_index));

		network_session_interface_set_local_user_properties(0, controller_index, &player_data, player_voice, player_text);
		break;
	}
	case _user_interface_guide_state_type_bring:
		if (!user_interface_squad_local_peer_is_host())
		{
			user_interface_networking_leave_squad(true);
		}

		if (joining_separate_game_after_delegation())
		{
			user_interface_networking_join_game(&m_xsession_info, 0, m_from_game_invite);
		}
		
		clear_invite_flags();
		m_field_F = false;

		break;
	case _user_interface_guide_state_type_leave_and_designate:
		user_interface_squad_delegate_leadership(player_index);
		user_interface_networking_leave_squad(true);

		if (joining_separate_game_after_delegation())
		{
			user_interface_networking_join_game(&m_xsession_info, 0, m_from_game_invite);
		}
		
		clear_invite_flags();
		m_field_F = false;
		
		break;
	default:
		unreachable();
		break;
	}

	return;
}

static bool user_interface_guide_type_allowed(
	e_user_interface_guide_state_type type)
{
	bool allowed = true;

	if (type==_user_interface_guide_state_type_boot)
	{
		allowed = user_interface_squad_is_booting_allowed();
	}

	return allowed;
}

static void user_interface_guide_restart_for_update(void)
{
	XLIVEUPDATE_INFORMATION update_information;

	csmemset(&update_information, 0, sizeof(update_information));

	if (SUCCEEDED(XLiveGetUpdateInformation(&update_information)))
	{
		// We do not update from here anymore so I didn't bother rewritting this code - Berthalamew
		// TODO: rewrite the code here if we want it for reference since it's a warbird function?
	}

	return;
}
