#pragma once
#include "input/input_constants.h"

/* constants */

enum
{
	k_ui_text_max_tab_stops = 8,
};

/* classes */

class c_user_interface_text
{
protected:
	//void* __vtable;
	int32 m_custom_font_type;
	real_rgb_color m_text_color;
	int16 m_text_is_pulsating;
	int16 field_16;
	datum field_18;
	int32 m_text_justification;
	real32 field_20;
	datum field_24;
	int16 m_tab_stops[k_ui_text_max_tab_stops];
	int32 m_tab_stop_count;
	int16 field_3C;
	int16 text_length;
	int32 m_ui_start_time;

public:
	c_user_interface_text();

	const real_rgb_color* get_color(void) const;
	int32 get_font(void) const;

	void set_font(int32 font_type);
	void set_pulsating(bool pulsating);
	void set_color(real_rgb_color* color);
	void set_color(const real_rgb_color* color);
	void set_tab_stop_count(const int16 *tab_stops, int32 tab_stop_count);

	static bool is_private_use_character(wchar_t character);

	// c_user_interface_text virtual functions

	virtual ~c_user_interface_text() = default;
	virtual void  set_text(const wchar_t* initial_text) = 0;
	virtual void  append_text(const wchar_t* update_text) = 0;
	virtual const wchar_t* get_raw_string() = 0;
};
ASSERT_STRUCT_SIZE(c_user_interface_text, 0x44);


/* public methods */

void user_interface_text_apply_hooks(void);

real32 get_ui_text_label_scale(void);

void set_ui_text_label_scale(real32 scale);

bool __cdecl user_interface_parse_string(wchar_t* string, size_t max_length, char a3);

void user_interface_get_key_character(e_input_key_code key_code, c_maximum_interface_text* string);
