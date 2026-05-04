#pragma once
#include <XLive/XStorage/XStorage.h>

/* structures */

struct s_panorama_buffer
{
	uint8 gap[136];
	uint16 field_88;
	uint8 gap1[3334];
};

/* classes */

class c_panorama_user_history
{
public:
	void handle_signin(XUSER_SIGNIN_STATE state);
	bool start_download(void);
	bool abort_download(void);
	bool abort_upload(void);

private:
	void(c_panorama_user_history::*m_worker_proc)(void);
	XOVERLAPPED m_overlapped;
	s_panorama_buffer m_download_buffer_0;
	s_panorama_buffer m_download_buffer_1;
	bool m_field_1B40;
	bool m_field_1B41;
	bool m_download_pending;
	bool m_upload_pending;
	wchar_t m_server_path[MAX_PATH];
	XSTORAGE_DOWNLOAD_TO_MEMORY_RESULTS m_downloaded_results;

	void start_download_worker(void);
	void pending_download_worker(void);
	void finish_download(void);
};

/* prototypes */

c_panorama_user_history* panorama_user_history_get(void);
