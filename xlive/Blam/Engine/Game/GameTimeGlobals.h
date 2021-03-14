#pragma once

#include "Blam\Common\Common.h"

//size: 0x24
struct time_globals
{
	enum GameState : bool
	{
		Running = false,
		Paused = true
	};

	bool initialized;
	GameState GameState;
	WORD ticks_per_second;
	float seconds_per_tick;
	DWORD tick_count;
	float game_speed;
	float update_time;
	float field_14;
	float field_18;
	float field_1C;
	float field_20;

	static time_globals* get();
	static float get_seconds_per_tick();
	static float seconds_to_ticks_precise(float s);
	static signed int seconds_to_ticks_impercise(float s);
};
CHECK_STRUCT_SIZE(time_globals, 0x24);