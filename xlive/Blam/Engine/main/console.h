#pragma once
#ifdef TERMINAL_ENABLED
#endif

/* enums */

#ifdef TERMINAL_ENABLED

enum e_status_line_flags
{
	_status_line_blink_bit = 0,
	_status_line_inhibit_drawing_bit,
	_status_line_left_justify_bit,
	_status_line_draw_once_bit,
	k_status_line_count,
};

#endif

/* structures */

#ifdef TERMINAL_ENABLED

struct s_status_line
{
	c_static_string<256> text;
	real_rgb_color color;
	real32 alpha;
	c_flags<e_status_line_flags, uint32, k_status_line_count> flags;
	const char* identifier;
	s_status_line* prev;
	s_status_line* next;
};

struct s_status_string
{
	c_static_string<256> format_string;
	int32 time_created;
	s_status_line line;
};

#endif

/* prototypes */

#ifdef TERMINAL_ENABLED

void status_printf(const char* format, ...);

void console_initialize(void);

void console_dispose(void);

bool console_is_active(void);

void console_execute_initial_commands(void);

void console_open(bool open_debug_menu);

void console_close(void);

void console_clear(void);

void console_printf(const char* format, ...);

void console_warning(const char* format, ...);

void console_update(real32 dt);

#else

#define console_initialize() (void)0 

#define console_dispose() (void)0

#define console_is_active() (void)0

#define console_execute_initial_commands() (void)0

#define console_open(...) (void)0

#define console_close() (void)0

#define console_clear() (void)0

#define console_printf(...) (void)0

#define console_warning(...) (void)0

#define console_update(...) (void)0

#endif

/* globals */

#ifdef TERMINAL_ENABLED

extern bool console_dump_to_file;
extern bool console_dump_to_debug_display;

extern s_status_line* g_status_line_head;

extern s_status_line* g_status_line_tail;

extern s_status_string g_status_strings[k_error_category_count];

#endif
