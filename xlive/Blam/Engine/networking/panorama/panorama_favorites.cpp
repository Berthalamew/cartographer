#include "stdafx.h"
#include "panorama_favorites.h"

#include "networking/network_event.h"

#include <XLive/XStorage/XStorage.h>

/* public code */

c_panorama_favorites* panorama_favorites_get(void)
{
	return Memory::GetAddress<c_panorama_favorites*>(0x517970, 0x541E08);
}

bool c_panorama_favorites::abort_download(void)
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
        
        event(_event_verbose, "Favorites: state changed to 'NONE'");
        
        m_current_task = NULL;
        m_field_680 = false;

        result = true;
    }

    return result;
}

bool c_panorama_favorites::start_download(void)
{
	bool result = false;

    if (m_upload_pending)
    {
        ASSERT(!m_download_pending);
    }
    else
    {
        abort_download();

        event(_event_verbose, "Favorites: state changed to 'start_download'");
        
        m_current_task = &c_panorama_favorites::start_download_worker;
        m_download_pending = true;

        result = true;
    }

    return result;
}

/* private code */

void c_panorama_favorites::finish_download(void)
{
    event(_event_verbose, "Favorites: downloaded %u bytes", m_download_results.dwBytesTotal);

    s_panorama_favorites_block* temp = m_current_favorites_block;
    m_pending_favorites_block = temp;

    ASSERT(m_download_pending==true);

    m_download_pending = false;
    m_field_680 = true;
    m_current_task = NULL;

    event(_event_verbose, "Favorites: state changed to 'NONE'");
    
    ++m_field_20;

    return;
}

void c_panorama_favorites::start_download_worker(void)
{
    uint32 path_length = NUMBEROF(m_server_path);
    uint32 result = XStorageBuildServerPath(0, XSTORAGE_FACILITY_PER_USER_TITLE, NULL, 0, L"favorites", m_server_path, &path_length);

    if (result==ERROR_SUCCESS)
    {
        csmemset(m_pending_favorites_block, 0, sizeof(*m_pending_favorites_block));
        result = XStorageDownloadToMemory(
            0,
            m_server_path,
            sizeof(m_pending_favorites_block->data),
            (uint8*)&m_pending_favorites_block->data,
            sizeof(m_download_results),
            &m_download_results,
            &m_overlapped
        );

        if (result==ERROR_IO_PENDING)
        {
            m_current_task = &c_panorama_favorites::pending_download_worker;

            event(_event_verbose, "Favorites: state changed to 'pending_download'");
        }
        else
        {
            ASSERT(result!=ERROR_SUCCESS);

            event(_event_error, "Favorites: failed to download favorites from Live: 0x%x", result);
        }
    }
    else
    {
        event(_event_error, "Favorites: failed to build server path on download: 0x%x", result);
    }

    if (m_current_task==&c_panorama_favorites::start_download_worker)
    {
        abort_download();
    }

    return;
}

void c_panorama_favorites::pending_download_worker(void)
{
    int32 result;

    ASSERT(m_download_pending);
    ASSERT(!m_upload_pending);
    ASSERT(m_pending_favorites_block != NULL);

    result = XGetOverlappedResult(&m_overlapped, 0, 0);

    if (result==ERROR_SUCCESS)
    {
        finish_download();
    }
    else
    {
        if (result!=ERROR_IO_INCOMPLETE)
        {
            uint32 extended_error = XGetOverlappedExtendedError(&m_overlapped);

            if (extended_error==XONLINE_E_STORAGE_FILE_NOT_FOUND)
            {
                m_pending_favorites_block->data.field_0 = 0;

                finish_download();
                abort_download();
            }
            else if (extended_error==0x8007007A)
            {
#if ASSERTS_ENABLED
                ASSERT(false);
#endif

                XStorageDelete(0, m_server_path, NULL);
                abort_download();
            }
            else
            {
                event(_event_error, "Favorites: async download failed result=0x%08X, extended_error=0x%08X", result, extended_error);
                
                abort_download();
            }
        }
    }

    return;
}
