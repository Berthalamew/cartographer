#pragma once

struct time_globals
{
	bool initialized;
	bool paused;
	short ticks_per_second;
	real32 seconds_per_tick;
	DWORD tick_count;
	real32 game_speed;
	real32 game_ticks_leftover;
	real32 field_14;
	real32 field_18;
	real32 field_1C;
	real32 field_20;

	static bool available();
	static time_globals* get();
	static int get_game_time();
	static int get_tickrate();
	static real32 get_seconds_per_tick();
	static real32 seconds_to_ticks_real(real32 s);
	static int seconds_to_ticks_round(real32 s);
	static int get_ticks_difference();
	static real32 get_ticks_difference_real();
	static real32 get_ticks_fraction_leftover();
};
CHECK_STRUCT_SIZE(time_globals, 36);

real32 game_ticks_to_seconds(real32 ticks);

bool game_time_get_paused(void);

void game_time_discard(int32 desired_ticks, int32 actual_ticks, real32* elapsed_game_dt);

void game_time_apply_patches();
