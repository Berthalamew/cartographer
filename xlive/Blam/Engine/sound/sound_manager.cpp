#include "stdafx.h"
#include "sound_manager.h"

/* public code */

void __cdecl sound_initialize(void)
{
	INVOKE(0x2979E, 0x0, sound_initialize);
	return;
}

void __cdecl sound_render(void)
{
	INVOKE(0x2DF87, 0x0, sound_render);
	return;
}

void __cdecl sound_idle(void)
{
	INVOKE(0x2D9B2, 0, sound_idle);
	return;
}

void __cdecl sound_pause(e_sound_pause_state new_state)
{
	INVOKE(0x2B579, 0x0, sound_pause, new_state);
	return;
}
