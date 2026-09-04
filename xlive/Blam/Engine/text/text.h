#pragma once

/* enums */

enum e_text_justification
{
	_text_justification_left = 0,
	_text_justification_center,
	_text_justification_right,
	k_text_justification_count
};

enum e_font_index
{
    _font_index_defualt = 0,
    _font_index_number_font,
	k_font_index_count,

    _font_index_invalid = NONE
};

enum e_text_font
{
	_text_font_terminal_font = 0,
	_text_font_body_text_font,
	_text_font_title_font,
	_text_font_super_large_font,
	_text_font_large_body_text_font,
	_text_font_split_screen_hud_message_font,
	_text_font_full_screen_hud_message_font,
	_text_font_english_body_text_font,
	_text_font_hud_number_font,
	_text_font_subtitle_font,
	_text_font_main_menu_font,
	_text_font_text_chat_font,
	k_text_font_count
};
