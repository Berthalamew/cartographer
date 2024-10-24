#pragma once

struct s_user_interface_guide_state_manager_string
{
	int field_0;
	wchar_t string[24];
	int field_34;
	int field_38;
};
ASSERT_STRUCT_SIZE(s_user_interface_guide_state_manager_string, 60);

// Not 100% on the size yet
#pragma pack(push, 1)
class c_user_interface_guide_state_manager
{
public:
	HANDLE m_xnotify_listener;
	bool m_unk_bool_4;
	char m_pad_5[3];
	XUSER_SIGNIN_STATE m_sign_in_state;
	char m_field_C;
	bool m_unk_bool_D;
	char m_pad_E;
	bool m_unk_bool_F;
	bool m_strings_initialized;
	XSESSION_INFO m_xsession_info;
	bool m_from_game_invite;
	char m_gamertag[XUSER_NAME_SIZE];
	uint8 gap_5E[2];
	void* m_callback_task;
	uint8 gap_64[1160];
	s_user_interface_guide_state_manager_string m_strings[4];
	char m_gap_5DC[12];
	XOVERLAPPED m_xoverlapped;
	int m_field_604;
	bool m_unk_bool_608;
	bool m_unk_bool_609;
	char m_pad_609[6];

	void add_user_signin_task(bool sign_to_live, void* signin_callback);
};
#pragma pack(pop)
ASSERT_STRUCT_SIZE(c_user_interface_guide_state_manager, 1552);

c_user_interface_guide_state_manager* user_interface_guide_state_manager_get(void);
