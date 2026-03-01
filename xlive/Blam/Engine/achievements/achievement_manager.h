#pragma once

/* classes */

class c_achievement_manager
{
public:
	void start_upload(void);
	void submit_event(int32* event);
	void disable_all_achievements(void);
	void start_level_chosen(int8 level);

private:
	int8 m_field_0;
	int8 m_field_1;
	int8 m_level;
	int8 m_achievement;
	
	bool m_live;
	int8 m_pad[3];

	int8 m_gap[168];
	bool m_field_B0;
	real32 m_time_satisfied;
	uint8 m_player_minimum_satisfied;
};

/* prototypes */

c_achievement_manager* achievement_manager_get(void);
