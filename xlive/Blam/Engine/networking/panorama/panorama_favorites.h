#pragma once

#include <XLive/XStorage/XStorage.h>

/* classes */

struct s_panorama_favorites_data
{
	int32 field_0;
	uint8 buffer[800];
};

struct s_panorama_favorites_block
{
	int32 field_0;
	s_panorama_favorites_data data;
};

class c_panorama_favorites
{
public:
	bool abort_download(void);
	bool start_download(void);

private:
	void(c_panorama_favorites::*m_current_task)(void);
	XOVERLAPPED m_overlapped;
	int32 m_field_20;
	int8 m_gap0[1620];
	s_panorama_favorites_block* m_pending_favorites_block;
	s_panorama_favorites_block* m_current_favorites_block;
	bool m_field_680;
	bool m_download_pending;
	bool m_upload_pending;
	wchar_t m_server_path[MAX_PATH];
	XSTORAGE_DOWNLOAD_TO_MEMORY_RESULTS m_download_results;

	void finish_download(void);
	void start_download_worker(void);
	void pending_download_worker(void);
};
ASSERT_STRUCT_SIZE(c_panorama_favorites, 0x8A0);

/* prototypes */

c_panorama_favorites* panorama_favorites_get(void);
