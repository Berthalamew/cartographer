#pragma once

#ifdef TERMINAL_ENABLED

#include "dialogs/edit_text.h"
#include "input/input_abstraction.h"

/* constants */

enum
{
	TERMINAL_PRINTF_MAXIMUM_LINE_LENGTH = 255,
	TERMINAL_GETS_MAXIMUM_KEYS = 32,
	TERMINAL_GETS_MAXIMUM_LINE_LENGTH = 255,
	TERMINAL_GETS_MAXIMUM_PROMPT_LENGTH = 31,
	TERMINAL_TAB_STOP_COUNT = 3,
};

/* structures */

struct terminal_gets_state
{
	int16 key_count;
	key_stroke keys[TERMINAL_GETS_MAXIMUM_KEYS];
	real_argb_color color;
	char prompt[TERMINAL_GETS_MAXIMUM_PROMPT_LENGTH+1];
	char result[TERMINAL_PRINTF_MAXIMUM_LINE_LENGTH+1];
	int16 horizontal_scroll_amount;
	edit_text edit;
};

/* globals */

extern bool g_terminal_render_enable;

/* prototypes */

void terminal_initialize(void);

void terminal_dispose(void);

void terminal_output_to_console(bool enabled);

void terminal_suppress_output(bool suppress_output);

void terminal_clear(void);

bool terminal_gets_begin(terminal_gets_state* state);

void terminal_gets_end(terminal_gets_state* state);

bool terminal_update_input(real32 dt);

void terminal_update_output(real32 dt);

datum terminal_new_line(void);

bool terminal_update(real32 dt);

void __cdecl terminal_draw(void);

void terminal_printf(const real_argb_color* color, const char* format, ...);

#else

#define terminal_initialize() (void)0

#define terminal_dispose() (void)0

#define terminal_output_to_console(...) (void)0 

#define terminal_suppress_output(...) (void)0

#define terminal_clear() (void)0

#define terminal_gets_begin(...) (void)0

#define terminal_gets_end(...) (void)0

#define terminal_update_input(...) (void)0

#define terminal_update_output(...) (void)0

#define terminal_new_line() (void)0

#define terminal_update(...) (void)0

#define terminal_draw() (void)0

#define terminal_printf(...) (void)0

#endif
