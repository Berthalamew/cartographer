#include "stdafx.h"
#include "panorama_user_profile.h"

#include "networking/network_event.h"

/* public code */

c_panorama_user_profile* panorama_user_profile_get(void)
{
	return Memory::GetAddress<c_panorama_user_profile*>(0x51A558, 0x0);
}

bool c_panorama_user_profile::start_download(void)
{
	bool result = true;

	cancel_download();

	event(_event_verbose, "Panorama Profile: state changed to 'start_download'");

	m_current_task = &c_panorama_user_profile::start_download_worker;
	m_download_started = true;

	return result;
}

bool c_panorama_user_profile::cancel_download(void)
{
	bool result = false;

	if (m_download_pending)
	{
		if (m_overlapped.InternalLow==ERROR_IO_PENDING)
		{
			XCancelOverlapped(&m_overlapped);
		}

		m_download_pending = false;

		event(_event_verbose, "Panorama Profile: state changed to 'NONE'");

		m_current_task = NULL;
		m_download_started = false;

		result = true;
	}

	return result;
}

/* private code */

void c_panorama_user_profile::start_download_worker(void)
{
	INVOKE_TYPE(0x1E24B3, 0x0, void(__thiscall*)(c_panorama_user_profile*), this);

	return;
}
