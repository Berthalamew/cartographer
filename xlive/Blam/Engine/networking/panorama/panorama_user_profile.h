#pragma once

class c_panorama_user_profile
{
public:
	bool start_download(void);
	bool cancel_download(void);

private:
	void(c_panorama_user_profile::* m_current_task)(void);
	XOVERLAPPED m_overlapped;
	void* m_pending_settings;
	int32 m_pending_settings_size;
	int32 m_field_28;
	bool m_download_started;
	bool m_download_pending;

	void start_download_worker(void);
};

/* prototypes */

c_panorama_user_profile* panorama_user_profile_get(void);
