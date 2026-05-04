#pragma once

/* structures */

// todo : verify this struct
struct s_panorama_friends_block
{
	uint32 buffer_size;
	uint8* buffer;
	uint32 result;
};
ASSERT_STRUCT_SIZE(s_panorama_friends_block, 12);

/* classes */

class c_panorama_friends
{
public:
	bool has_active_task(void);
	void initialize_startup(void);
	void cancel_task(void);
	void start(void);

protected:
	void* m_current_task;
	XOVERLAPPED m_overlapped;
	HANDLE m_enumerator;
	int32 m_field_24;
	uint8 m_gap_28[24];
	struct s_panorama_friends_block* m_pending_friends_block;
	struct s_panorama_friends_block* field_44;
	bool m_field_48;
	bool m_op_pending;
	uint8 m_gap_4A[6];
};
ASSERT_STRUCT_SIZE(c_panorama_friends, 0x50);

/* prototypes */

c_panorama_friends* panorama_friends_get(void);