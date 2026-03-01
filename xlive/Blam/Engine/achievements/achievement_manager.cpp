#include "stdafx.h"
#include "achievement_manager.h"

#include "networking/online/online_account_xbox.h"

/* public code */

void c_achievement_manager::start_upload(
	void)
{
	INVOKE_TYPE(0x4A808, 0x0, void(__thiscall*)(c_achievement_manager*), this);
	return;
}

void c_achievement_manager::submit_event(
	int32* event)
{
	INVOKE_TYPE(0x4B553, 0x0, void(__thiscall*)(c_achievement_manager*, int32*), this, event);
	return;
}

void c_achievement_manager::disable_all_achievements(
	void)
{
	INVOKE_TYPE(0x4A6C6, 0x0, void(__thiscall*)(c_achievement_manager*), this);
	return;
}


void c_achievement_manager::start_level_chosen(
	int8 level)
{
	m_live = online_connected_to_xbox_live();
	error(_error_delayed, "achievements: start_level_chosen: live: %d", m_live);
	if (m_live)
	{
		m_level = level;
		m_achievement = 1;
	}
	else
	{
		m_level = 0;
		m_achievement = 0;
	}
	m_field_0 = m_level;
	m_field_1 = m_achievement;
	error(
		_error_delayed,
		"achievements: set_state: saving checkpoints: %d, earning achievements: %d",
		m_level,
		m_achievement);
	disable_all_achievements();
	m_field_B0 = true;

	return;
}

c_achievement_manager* achievement_manager_get(void)
{
	return *Memory::GetAddress<c_achievement_manager**>(0x482D48);
}
