#include "stdafx.h"
#include "online_account_xbox.h"

#include "game/players.h"
#include "interface/user_interface_guide.h"
#include "H2MOD/Modules/Accounts/AccountLogin.h"
#include "H2MOD/Modules/Shell/Config.h"
#include "H2MOD/Utils/Utils.h"

/* public code */

bool online_xuid_is_guest_account(
	s_player_identifier* player_identifier)
{
	// return INVOKE(0x1AC4C0, 0 , online_xuid_is_guest_account, xuid);
	int64* id = (int64*)player_identifier->identifier;

	return TEST_FLAG(*id, 3ULL);
}

uint8 online_xuid_get_guest_account_number(
	s_player_identifier* player_identifier)
{
	// return INVOKE(0x1AC4C6, 0, online_xuid_get_guest_account_number,xuid);
	
	int64* id = (int64*)player_identifier->identifier;

	return TEST_FLAG(*id, 3ULL);
}

bool __cdecl online_connected_to_xbox_live()
{
	//return INVOKE(0x1AC4A3, 0x0, online_connected_to_xbox_live);
	return user_interface_guide_state_manager_get()->m_sign_in_state == eXUserSigninState_SignedInToLive;
}

void online_account_transition_to_offline(
	int32 user_index)
{
	BYTE abEnet[6];
	BYTE abOnline[20];
	XNetRandom(abEnet, sizeof(abEnet));
	XNetRandom(abOnline, sizeof(abOnline));
	ConfigureUserDetails("[Username]", "12345678901234567890123456789012", rand(), 0, H2Config_ip_lan, ByteToHexStr(abEnet, sizeof(abEnet)).c_str(), ByteToHexStr(abOnline, sizeof(abOnline)).c_str(), user_index, false);

	XUserSignInSetStatusChanged(0);
}