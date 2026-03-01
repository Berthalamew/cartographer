#include "stdafx.h"
#include "user_interface_text.h"

#include "user_interface_utilities.h"

#include "interface/hud.h"
#include "input/input_windows.h"
#include "main/game_preferences.h"

/* prototypes */

static void __cdecl ui_get_text_bounds_and_position_hook(int32 a1, wchar_t* string, int32 a3, int32 a4, real32 scale);

/* globals */

// Separated scale for the text labels (carto addition)
static real32 g_ui_text_label_scaling = 0.0f;

/* public code */

void user_interface_text_apply_hooks(
	void)
{
	// Replace the ui_scale_factor with our own scaling for text labels
	WritePointer(Memory::GetAddress(0x2305AC) + 4, &g_ui_text_label_scaling);
	WritePointer(Memory::GetAddress(0x23066A) + 4, &g_ui_text_label_scaling);

	PatchCall(Memory::GetAddress(0x22CFFD), ui_get_text_bounds_and_position_hook);
	PatchCall(Memory::GetAddress(0x22FCDC), user_interface_get_key_character);
	return;
}

c_user_interface_text::c_user_interface_text(
	void)
{
	m_custom_font_type = 0;
	m_text_is_pulsating = 0;
	field_16 = 1;
	field_18 = NONE;
	m_text_justification = 2;
	field_20 = 1.f;
	field_24 = NONE;
	m_tab_stop_count = 0;
	field_3C = NONE;
	text_length = 0;
	m_ui_start_time = 0;
	m_text_color.blue = 1.f;
	m_text_color.green = 1.f;
	m_text_color.red = 1.f;
	return;
}

const real_rgb_color* c_user_interface_text::get_color(
	void) const
{
	return &m_text_color;
}

int32 c_user_interface_text::get_font(
	void) const
{
	return m_custom_font_type;
}

void c_user_interface_text::set_font(
	int32 font_type)
{
	m_custom_font_type = font_type;
	return;
}

void c_user_interface_text::set_pulsating(
	bool pulsating)
{
	m_text_is_pulsating = pulsating;
	return;
}

void c_user_interface_text::set_color(
	real_rgb_color* color)
{
	m_text_color = *color;
	return;
}

void c_user_interface_text::set_color(
	const real_rgb_color* color)
{
	m_text_color = *color;
	return;
}

void c_user_interface_text::set_tab_stop_count(
	const int16* tab_stops,
	int32 tab_stop_count)
{
	ASSERT(tab_stop_count <= k_ui_text_max_tab_stops);
	csmemcpy(m_tab_stops, tab_stops, sizeof(*m_tab_stops)*tab_stop_count);
	m_tab_stop_count = tab_stop_count;
	return;
}

bool c_user_interface_text::is_private_use_character(
	wchar_t character)
{
	return IN_RANGE(character, k_first_unicode_private_use_character, k_last_unicode_private_use_character);
}

real32 get_ui_text_label_scale(
	void)
{
	return g_ui_text_label_scaling;
}

void set_ui_text_label_scale(
	real32 scale)
{
	g_ui_text_label_scaling = scale;
	return;
}

bool __cdecl user_interface_parse_string(
	wchar_t* string, 
	size_t max_length,
	char a3)
{
	return INVOKE(0x22F712, 0x0, user_interface_parse_string, string, max_length, a3);
}

void user_interface_get_key_character(
	e_input_key_code key_code, 
	c_maximum_interface_text *string)
{
	c_maximum_interface_text src;

	ascii_key* key = &ascii_to_key_table[input_windows_get_remapped_key(key_code)];
	switch (key_code)
	{
	case _key_oem_8:
		if (get_current_language() == _language_french)
		{
			user_interface_global_string_get(_string_id_key_exclamation, &src);
			string->set(src.get_string());
		}
		break;
	case _key_tilde:
		if (get_current_language() == _language_italian)
		{
			user_interface_global_string_get(_string_id_key_grave_o, &src);
			string->set(src.get_string());
		}
		else if (get_current_language() == _language_french)
		{
			user_interface_global_string_get(_string_id_key_grave_u, &src);
			string->set(src.get_string());
		}
		break;
	case _key_left_square_bracket:
		if (get_current_language() == _language_german)
		{
			user_interface_global_string_get(_string_id_key_beta, &src);
			string->set(src.get_string());
		}
		break;
	case _key_right_square_bracket:
		if (get_current_language() == _language_french)
		{
			user_interface_global_string_get(_string_id_key_acute, &src);
			string->set(src.get_string());
		}
		break;
	case _key_backslash:
		if (get_current_language() == _language_japanese)
		{
			user_interface_global_string_get(_string_id_key_yen, &src);
			string->set(src.get_string());
		}
		else if (get_current_language() == _language_french)
		{
			user_interface_global_string_get(_string_id_key_asterisk, &src);
			string->set(src.get_string());
		}
		break;
	case _key_forwardslash:
		if (get_current_language() == _language_italian)
		{
			user_interface_global_string_get(_string_id_key_grave_u, &src);
			string->set(src.get_string());
		}
		else if (get_current_language() == _language_french)
		{
			user_interface_global_string_get(_string_id_key_colon, &src);
			string->set(src.get_string());
		}
		break;
	case _key_colon:
		if (get_current_language() == _language_french)
		{
			user_interface_global_string_get(_string_id_key_dollar, &src);
			string->set(src.get_string());
		}
		else if (get_current_language() == _language_italian)
		{
			user_interface_global_string_get(_string_id_key_grave_e, &src);
			string->set(src.get_string());
		}
		break;
	case _key_quotation_mark:
		if (get_current_language() == _language_italian)
		{
			user_interface_global_string_get(_string_id_key_grave_a, &src);
			string->set(src.get_string());
		}
		break;
	default:
		bool is_remapped_character = false;
		if (key->remapped)
		{
			if (key->string == _string_id_invalid)
			{
				if (key->remapped_character)
				{
					wchar_t str[2] = { key->remapped_character, L'\0' };
					string->set(str);
					is_remapped_character = true;
				}
				else
				{
					user_interface_global_string_get(_string_id_invalid, &src);
				}
			}
			else
			{
				user_interface_global_string_get(key->string, &src);
			}
		}
	   

		if (!is_remapped_character)
		{
			string->set(src.get_string());
		}
		break;
	}
	return;
}

// this seems to compute the required space to display the text?
static void __cdecl ui_get_text_bounds_and_position_hook(
	int32 a1,
	wchar_t* string,
	int32 a3,
	int32 a4,
	real32 scale)
{
	INVOKE(0x99D97, 0x0, ui_get_text_bounds_and_position_hook, a1, string, a3, a4, *get_secondary_hud_scale());
	return;
}
