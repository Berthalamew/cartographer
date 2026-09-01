#include "stdafx.h"
#include "kablam_config.h"

#include "networking/logic/life_cycle_manager.h"
#include "networking/session/network_session.h"

#include "H2MOD/Modules/EventHandler/EventHandler.hpp"
#include "H2MOD/Modules/Shell/Config.h"
#include "H2MOD/Modules/Shell/Startup/Startup.h"

/* prototypes */

static void kablam_config_cartographer_initialize(void);

static void vip_lock(e_game_life_cycle state);

static int32 server_console_add_vip(const wchar_t* player_name);

static int32 server_console_clear_vip(void);

/* public code */

void kablam_config_apply_patches(void)
{
	PatchCall(Memory::GetAddress(0x0, 0xC7B6), kablam_config_initialize);
	return;
}

void __cdecl kablam_config_initialize(HKEY key, LPCWSTR subkey)
{
	INVOKE(0x0, 0x8EFA, kablam_config_initialize, key, subkey);

	c_kablam_config_live* live_config = kablam_config_live_get();

	if (live_config)
	{
		wcsncpy_s(live_config->m_live_id, L"live bypass", NUMBEROF(live_config->m_live_id));
	}

	kablam_config_cartographer_initialize();
	return;
}

c_kablam_config_live* kablam_config_live_get()
{
	return INVOKE(0, 0x8A29, kablam_config_live_get);
}

/* private code */

static void kablam_config_cartographer_initialize(void)
{
	// TODO: initialize the ini config in the same place between client and dedi 
	InitH2Config();
	PostH2Config();

	// Overwrite the playlist config entry from the registry key with the one in h2config if the entry is defined
	// TODO: figure out a better option for handling this other than just duplicating the entry in the registry
	if (csstrnlen(H2Config_dedi_server_playlist, NUMBEROF(H2Config_dedi_server_playlist)) > 0)
	{
		wchar_t* server_playlist = Memory::GetAddress<wchar_t*>(0, 0x3B3704);
		usnprintf(server_playlist, MAX_PATH, L"%hs", H2Config_dedi_server_playlist);
	}

	// set the additional post-game carnage report time
	WriteValue<uint8>(Memory::GetAddress(0, 0xE590) + 2, (uint8)H2Config_additional_pcr_time);

	if (H2Config_vip_lock)
	{
		EventHandler::register_callback(vip_lock, EventType::gamelifecycle_change, EventExecutionType::execute_after);
	}
	return;
}

static void vip_lock(e_game_life_cycle state)
{
	c_network_session* session;

	if (network_life_cycle_in_squad_session(&session))
	{
		if (state == _life_cycle_post_game)
		{
			server_console_clear_vip();
			session->get_session_parameters()->party_privacy = _network_game_privacy_open;
		}
		if (state == _life_cycle_in_game)
		{
			for (int32 i = 0; i < k_maximum_players; i++)
			{
				if (NetworkSession::PlayerIsActive(i))
				{
					server_console_add_vip(NetworkSession::GetPlayerName(i));
				}
			}
			session->get_session_parameters()->party_privacy = _network_game_privacy_invite;
		}
	}

	return;
}

static int32 __cdecl server_console_add_vip(const wchar_t* player_name)
{
	return INVOKE(0, 0x1D932, server_console_add_vip, player_name);
}

static int32 __cdecl server_console_clear_vip(void)
{
	return INVOKE(0, 0x1DB16, server_console_clear_vip);
}
