#pragma once

/* constants */

// 1 hour offset
#define k_process_system_time_startup_offset_sec (1 * 60 * 60)

#define k_shell_time_sec_denominator 1
#define k_shell_time_msec_denominator 1000
#define k_shell_time_usec_denominator 1000000

/* public code */

HWND* shell_windows_get_hwnd(void);
bool* should_initilize_xlive_get(void);
bool* xlive_initilized_get(void);

uint32 __cdecl system_milliseconds();

void shell_windows_initialize();
void shell_windows_apply_patches();

unsigned long long shell_time_now_sec();
unsigned long long shell_time_now_msec();
unsigned long long shell_time_now(unsigned long long denominator);

void shell_windows_throttle_framerate(int desired_framerate);

bool __cdecl game_is_minimized(void);

bool __cdecl gfwl_gamestore_initialize(void);

int32* fatal_error_id_get(void);
