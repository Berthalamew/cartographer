#pragma once
#include "game/game.h"
#include "input/controllers.h"

#include <Xlive/xbox/xbox.h>

/* enums */

enum e_user_interface_guide_state_type
{
	_user_interface_guide_state_type_boot = 0,
	_user_interface_guide_state_type_mute,
	_user_interface_guide_state_type_bring,
	_user_interface_guide_state_type_leave_and_designate,
	
	k_user_interface_guide_state_type_count,
	_user_interface_guide_state_type_invalid = NONE
};

/* structures */

struct s_user_interface_guide_state_manager_button
{
	bool field_0;
	int8 pad[3];
	XPLAYERLIST_BUTTON button;
	uint32 field_38;
};
ASSERT_STRUCT_SIZE(s_user_interface_guide_state_manager_button, 60);

/* classes */

#pragma pack(push, 1)
class c_user_interface_guide_state_manager
{
public:
	HANDLE m_xnotify_listener;
	bool m_block_game_input;
	int8 m_pad_5[3];
	XUSER_SIGNIN_STATE m_sign_in_state;
	bool m_update_sign_in_state;
	bool m_field_D;
	bool m_started_custom_player_list_action;
	bool m_field_F;
	bool m_buttons_initialized;
	XSESSION_INFO m_xsession_info;
	bool m_from_game_invite;
	char m_gamertag[XUSER_NAME_SIZE];
	uint8 gap_5E[2];
	bool(*m_callback_task)(void*);
	int32 m_field_64;
	XPLAYERLIST_USER m_player_data[k_maximum_players];
	s_user_interface_guide_state_manager_button m_buttons[k_user_interface_guide_state_type_count];
	XPLAYERLIST_RESULT m_player_list_result;
	XOVERLAPPED m_xoverlapped;
	e_user_interface_guide_state_type m_custom_action_type;
	bool m_field_608;
	bool m_clear_actions;
	int8 gap_609[6];

	void add_user_signin_task(bool sign_to_live, bool(*signin_callback)(void*));
	void update(void);
	void update_dedicated_server(void);

private:
	void initialize_buttons(void);
	bool joining_separate_game_after_delegation(void) const;
	void clear_custom_actions(void);
	bool set_custom_action(int32 action_index, const wchar_t* action_text, uint32 flags, e_user_interface_guide_state_type custom_action_type);
	void show_player_list(e_user_interface_guide_state_type type1, e_user_interface_guide_state_type type2, int32 a3, string_id title, string_id description);
	void update_sign_in_state(e_controller_index controller_index, int32 user_index, XUSER_SIGNIN_STATE sign_in_state);
	void update_sign_in_state_dedicated_server(XUSER_SIGNIN_STATE sign_in_state);
	int32 get_player_count(class c_network_session* session, int32 a3);
	void update_option(e_user_interface_guide_state_type type, struct s_player_identifier* player_identifier);

	inline void clear_invite_flags(void)
	{
		m_from_game_invite = false;
		m_field_D = false;

		return;
	}

};
ASSERT_STRUCT_SIZE(c_user_interface_guide_state_manager, 1552);
#pragma pack(pop)

void user_interface_guide_apply_patches(void);

class c_user_interface_guide_state_manager* user_interface_guide_state_manager_get(void);
class c_panorama_friends* user_interface_guide_friends_get(void);
class c_panorama_achievements* user_interface_guide_achievements_get(void);
class c_panorama_user_profile* user_interface_guide_profile_get(void);

void user_interface_guide_string_get(string_id id, c_maximum_interface_text* text);
