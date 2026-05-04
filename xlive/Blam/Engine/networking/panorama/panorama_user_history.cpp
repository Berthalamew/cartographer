#include "stdafx.h"
#include "panorama_user_history.h"

#include "networking/network_event.h"

/* public code */

c_panorama_user_history* panorama_user_history_get(void)
{
	return Memory::GetAddress<c_panorama_user_history*>(0x518210, 0x5426A8);
}

void c_panorama_user_history::handle_signin(
	XUSER_SIGNIN_STATE state)
{
	if (m_upload_pending)
	{
		ASSERT(!m_download_pending);

		abort_upload();

		event(_event_warning, "user history: upload aborted due to signin state change");
	}

	if (m_download_pending)
	{
		ASSERT(!m_upload_pending);

		abort_download();

		event(_event_warning, "user history: download aborted due to signin state change");
	}

	if (state == eXUserSigninState_SignedInToLive)
	{
		event(_event_verbose, "user history: new user logged in, starting download");

		m_field_1B40 = true;
		m_field_1B41 = false;

		csmemset(&m_download_buffer_0, 0, sizeof(m_download_buffer_0));
		csmemset(&m_download_buffer_1, 0, sizeof(m_download_buffer_1));

		start_download();
	}
	else
	{
		m_field_1B40 = false;
	}

	return;
}

bool c_panorama_user_history::start_download(void)
{
	bool result = false;

	if (m_upload_pending)
	{
		ASSERT(!m_download_pending);
	}
	else
	{
		abort_download();

		event(_event_verbose, "user history: state changed to 'start_download'");

		m_worker_proc = &c_panorama_user_history::start_download_worker;
		m_download_pending = true;
	}

	return result;
}

bool c_panorama_user_history::abort_download(void)
{
	bool result = false;

	if (m_download_pending)
	{
		ASSERT(!m_upload_pending);

		if (m_overlapped.InternalLow==ERROR_IO_PENDING)
		{
			XCancelOverlapped(&m_overlapped);
		}

		csmemset(&m_overlapped, 0, sizeof(m_overlapped));

		m_download_pending = false;

		event(_event_verbose, "user history: state changed to 'NONE'");

		m_worker_proc = NULL;
		m_field_1B41 = false;

		result = true;
	}

	return result;
}

bool c_panorama_user_history::abort_upload(void)
{
	bool result = false;

	if (m_upload_pending)
	{
		ASSERT(!m_download_pending);

		if (m_overlapped.InternalLow==ERROR_IO_PENDING)
		{
			XCancelOverlapped(&m_overlapped);
		}

		csmemset(&m_overlapped, 0, sizeof(m_overlapped));

		m_upload_pending = false;
		m_worker_proc = NULL;

		event(_event_verbose, "user history: state changed to 'NONE'");

		result = true;
	}

	return result;
}

/* private code */

void c_panorama_user_history::start_download_worker(void)
{
	uint32 path_length = NUMBEROF(m_server_path);
	uint32 result = XStorageBuildServerPath(0, XSTORAGE_FACILITY_PER_USER_TITLE, NULL, 0, L"user_history", m_server_path, &path_length);

	if (result==ERROR_SUCCESS)
	{
		csmemset(&m_download_buffer_1, 0, sizeof(m_download_buffer_1));
		csmemset(&m_overlapped, 0, sizeof(m_overlapped));

		result = XStorageDownloadToMemory(0, m_server_path, sizeof(m_download_buffer_1), (uint8*)&m_download_buffer_1, sizeof(m_downloaded_results), &m_downloaded_results, &m_overlapped);

		if (result==ERROR_IO_PENDING)
		{
			m_worker_proc = &c_panorama_user_history::pending_download_worker;


			event(_event_verbose, "user history: state changed to 'pending_download'");

		}
		else
		{
			ASSERT(result!=ERROR_SUCCESS);

			event(_event_error, "user history: failed to download history from Live: 0x%x", result);
		}
	}
	else
	{
		event(_event_error, "user history: failed to build server path on download: 0x%x", result);
	}

	if (m_worker_proc == &c_panorama_user_history::start_download_worker)
	{
		abort_download();
	}

	return;
}

void c_panorama_user_history::pending_download_worker(void)
{
	ASSERT(m_download_pending);
	ASSERT(!m_upload_pending);

	uint32 result = XGetOverlappedResult(&m_overlapped, NULL, FALSE);

    if (result==ERROR_SUCCESS)
	{
		finish_download();
	}
	else
	{
		if (result!=ERROR_IO_INCOMPLETE)
		{
			uint32 extended_result = XGetOverlappedExtendedError(&m_overlapped);

			if (extended_result==XONLINE_E_STORAGE_FILE_NOT_FOUND)
			{
				csmemset(&m_download_buffer_1, 0, sizeof(m_download_buffer_1));
				finish_download();
				abort_download();
			}
			else if (extended_result==0x8007007A)
			{
				event(_event_fatal, "User history block size is bad - did the struct definition change?");
				
				XStorageDelete(0, m_server_path, NULL);
				csmemset(&m_download_buffer_1, 0, sizeof(m_download_buffer_1));
				finish_download();
				abort_download();
			}
			else
			{
				event(_event_error, "user history: async download failed result=0x%08X, extended_error=0x%08X", result, extended_result);
				
				abort_download();
			}
		}
	}

	return;
}

void c_panorama_user_history::finish_download(void)
{
	event(_event_verbose, "user history: downloaded %u bytes", m_downloaded_results.dwBytesTotal);

	if (!m_downloaded_results.dwBytesTotal || m_downloaded_results.dwBytesTotal == (DWORD)(104 * m_download_buffer_1.field_88 + 144))
	{
		csmemcpy(&m_download_buffer_0, &m_download_buffer_1, sizeof(m_download_buffer_0));
	}
	else
	{
		event(_event_error, "User history block size is bad - did the struct definition change?");

		XStorageDelete(0, m_server_path, NULL);
		csmemset(&m_download_buffer_1, 0, sizeof(m_download_buffer_1));
		csmemset(&m_download_buffer_0, 0, sizeof(m_download_buffer_0));
	}
	
	ASSERT(m_download_pending==true);

	m_field_1B41 = true;
	m_download_pending = false;
	m_worker_proc = NULL;

	event(_event_verbose, "user history: state changed to 'NONE'");

	return;
}
