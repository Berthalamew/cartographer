#include "stdafx.h"
#include "panorama_friends.h"

/* public code */

c_panorama_friends* panorama_friends_get(void)
{
	return Memory::GetAddress<c_panorama_friends*>(0x517920, 0x541DB8);
}

bool c_panorama_friends::has_active_task(void)
{
	return m_current_task != nullptr;
}

void c_panorama_friends::initialize_startup(void)
{
	INVOKE_TYPE(0x1B3C1A, 0x1AF1E3, void(__thiscall*)(c_panorama_friends*), this);
	return;
}

void c_panorama_friends::cancel_task(void)
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

	m_current_task = NULL;
	m_field_48 = false;

	if (m_enumerator!=INVALID_HANDLE_VALUE)
	{
		XCloseHandle(m_enumerator);
		m_enumerator = INVALID_HANDLE_VALUE;
	}

	return;
}

void c_panorama_friends::start(void)
{
	INVOKE_TYPE(0x1B3C1A, 0x0, void(__thiscall*)(c_panorama_friends*), this);

	return;
}
