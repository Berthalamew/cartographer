#pragma once

// 1 hour offset
#define k_process_system_time_startup_offset_sec (1 * 60 * 60)

#define k_shell_time_sec_denominator 1
#define k_shell_time_msec_denominator 1000
#define k_shell_time_usec_denominator 1000000

uint32 __cdecl system_milliseconds();

void shell_windows_initialize();
void shell_windows_apply_patches();

long long shell_time_now_sec();
long long shell_time_now_msec();
long long shell_time_now(long long denominator);

void shell_windows_throttle_framerate(int desired_framerate);

bool __cdecl game_is_minimized(void);
