#include "stdafx.h"
#include "user_interface_player_widget.h"

// c_player_widget virtual functions


void c_player_widget::setup_children()
{
	//return INVOKE_TYPE(0x220441, 0x0, int32(__thiscall*)(c_player_widget*), this);
	
	c_user_interface_widget::setup_children();
	m_visible = false;

	return;
}


c_player_widget_representation::c_player_widget_representation(void) : m_flags()
{
	return;
}

void c_player_widget_representation::set_player_name(
	wchar_t const* name)
{
	//INVOKE_TYPE(0x2205BD, 0x0, void(__thiscall*)(c_player_widget_representation*, wchar_t const*), this, configuration);

	m_player_name = name;
	m_flags.set(_player_widget_name_bit, true);

	return;
}

void c_player_widget_representation::set_appearance(
	s_player_appearance const* appearance)
{
	//INVOKE_TYPE(0x2205CA, 0x0, void(__thiscall*)(c_player_widget_representation*, s_player_appearance*), this, appearance);

	ASSERT(appearance);

	m_appearance = *appearance;
	m_flags.set(_player_widget_appearance_bit, true);

	return;
}

void c_player_widget_representation::set_player_team_name(
	string_id team_name)
{
	//INVOKE_TYPE(0x2205EB, 0x0, void(__thiscall*)(c_player_widget_representation*, string_id), this, team_name);

	m_team_name = team_name;
	m_flags.set(_player_widget_team_name_bit, true);

	return;
}

void c_player_widget_representation::set_player_custom_name(
	wchar_t const* player_custom_name)
{
	ustrncpy(m_player_custom_name, player_custom_name, NUMBEROF(m_player_custom_name));
	m_flags.set(_player_widget_player_custom_name_bit, true);

	return;
}

void c_player_widget_representation::set_player_team(
	e_game_team team)
{
	//INVOKE_TYPE(0x2205F8, 0x0, void(__thiscall*)(c_player_widget_representation*, e_game_team), this, team);

	m_player_team = team;
	m_flags.set(_player_widget_player_team_bit, true);

	return;
}

void c_player_widget_representation::set_player_is_observer(
	bool observer)
{
	//INVOKE_TYPE(0x220607, 0x0, void(__thiscall*)(c_player_widget_representation*, bool), this, observer);

	m_player_is_observing = observer;
	m_flags.set(_player_widget_player_is_observing_bit, true);

	return;
}

void c_player_widget_representation::set_fake_player(
	bool fake)
{
	//INVOKE_TYPE(0x220617, 0x0, void(__thiscall*)(c_player_widget_representation*, bool), this, fake);

	m_fake_player = fake;
	m_flags.set(_player_widget_fake_player_bit, true);

	return;
}

void c_player_widget_representation::set_player_rank(
	int32 rank)
{
	//INVOKE_TYPE(0x220A12, 0x0, void(__thiscall*)(c_player_widget_representation*, int32), this, rank);

	m_player_rank = (int16)rank;
	m_flags.set(_player_widget_player_rank_bit, rank!=NONE);

	return;
}

void c_player_widget_representation::set_user_role(
	int32 role)
{
	//INVOKE_TYPE(0x220624, 0x0, void(__thiscall*)(c_player_widget_representation*, int32), this, role);

	m_bungie_role = role;
	m_flags.set(_player_widget_bungie_role_bit, true);

	return;
}

void c_player_widget_representation::set_change_color(
	real_rgb_color const* color)
{
	//INVOKE_TYPE(0x220634, 0x0, void(__thiscall*)(c_player_widget_representation*, real_rgb_color const*), this, color);

	m_change_color = *color;
	m_flags.set(_player_widget_appearance_bit, true);

	return;
}
