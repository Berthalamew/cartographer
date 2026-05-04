#include "stdafx.h"
#include "panorama_achievements.h"

#include "interface/user_interface_controller.h"
#include "networking/network_event.h"

#include <XLive/achievements/XAchievements.h>

/* public code */

c_panorama_achievements* panorama_achievements_get(void)
{
	return Memory::GetAddress<c_panorama_achievements*>(0x519F70, 0x544408);
}

void c_panorama_achievements::cancel_task(void)
{
	if (m_op_pending)
	{
		if (m_overlapped.InternalLow==ERROR_IO_PENDING)
		{
			XCancelOverlapped(&m_overlapped);
		}
		
		csmemset(&m_overlapped, 0, sizeof(m_overlapped));
		m_op_pending = false;
	}

	if (m_enumerator!=INVALID_HANDLE_VALUE)
	{
		XCloseHandle(m_enumerator);
		m_enumerator = INVALID_HANDLE_VALUE;
	}

	return;
}

void c_panorama_achievements::dispose(void)
{
	if (m_op_pending)
	{
		cancel_task();
	}

	csmemset(&m_achievements, 0, sizeof(m_achievements));
	set_replicated_achievements();

	return;
}

void c_panorama_achievements::enumerate(void)
{
	uint32 result;

	if (m_op_pending)
	{
		cancel_task();

		ASSERT(!m_op_pending);
	}

	ASSERT(m_enumerator == INVALID_HANDLE_VALUE);

	result = XUserCreateAchievementEnumerator(0, 0, 0, 32, 0, NUMBEROF(m_achievements), (PDWORD)m_achievements, &m_enumerator);

	if (result==ERROR_SUCCESS)
	{
		result = XEnumerate(m_enumerator, (CHAR*)m_achievements, sizeof(m_achievements), NULL, &m_overlapped);
		
		if (result==ERROR_IO_PENDING)
		{
			m_op_pending = true;
		}
		else
		{
			event(_event_error, "Achievements: failed to enumerate achievements: 0x%x", result);
		}
	}
	else
	{
		event(_event_error, "Achievements: failed to create enumerator: 0x%x", result);
	}

	return;
}

void c_panorama_achievements::set_replicated_achievements(void) const
{
	int32 achievement_index;

	int8 replicated_achievements = 0;
	
	for (achievement_index = 0; achievement_index<NUMBEROF(m_achievements); ++achievement_index)
	{
		if (m_achievements[achievement_index].field_0 == 39)
		{
			break;
		}
	}
	
	SET_BIT(replicated_achievements, 0, TEST_FLAG(m_achievements[achievement_index].flags, FLAG(16) | FLAG(17)));
	
	for (achievement_index = 0; achievement_index < NUMBEROF(m_achievements); ++achievement_index)
	{
		if (m_achievements[achievement_index].field_0 == 16)
		{
			break;
		}
	}

	SET_BIT(replicated_achievements, 1, TEST_FLAG(m_achievements[achievement_index].flags, FLAG(16) | FLAG(17)));

	user_interface_controller_set_replicated_achievement(_controller0, replicated_achievements);

	return;
}