#include "stdafx.h"
#include "game_preferences.h"

e_language get_current_language(void)
{
	typedef e_language(__cdecl* get_current_language_t)(void);
	auto p_get_current_language = Memory::GetAddress<get_current_language_t>(0x381FD, 0x2FAA6);
	return p_get_current_language();
}

void __cdecl global_preferences_initialize(void)
{
	INVOKE(0x325FD, 0x25E06, global_preferences_initialize);
	return;
}
