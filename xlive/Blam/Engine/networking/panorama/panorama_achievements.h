#pragma once
#include "achievements/achievement_list.h"
	
/* constants */

enum
{
	k_replicated_achievement_count = 2,
};

/* structures */

struct s_panorama_achievement
{
	int32 field_0;
	int8 gap[28];
	int32 flags;
};

/* classes */

class c_panorama_achievements
{
public:
	void cancel_task(void);
	void dispose(void);
	void enumerate(void);
	void set_replicated_achievements(void) const;

private:
	s_panorama_achievement m_achievements[k_achievement_type_count];
	XOVERLAPPED m_overlapped;
	HANDLE m_enumerator;
	bool m_op_pending;
};
ASSERT_STRUCT_SIZE(c_panorama_achievements, 0x5E8);

/* prototypes */

c_panorama_achievements* panorama_achievements_get(void);
