#pragma once

/* classes */

class c_network_time_statistics
{
	struct s_statistics_interval
	{
		int32 events;
		int32 total_values;
	};

	uint64 m_total_events;
	uint64 m_total_values;
	uint32 m_current_interval_start_timestamp;
	s_statistics_interval m_current_interval;
	int32 m_period_duration_msec;
	int32 m_interval_duration_msec;
	real32 m_period_inverse_seconds;
	int32 m_next_interval_index;
	s_statistics_interval m_stored_intervals[20];
	s_statistics_interval m_stored_total;

public:
	void initialize(int32 period_duration_msec);
};
ASSERT_STRUCT_SIZE(c_network_time_statistics, 216);

class c_network_window_statistics
{
	struct s_statistics_window_entry
	{
		uint32 timestamp;
		int32 value;
	};

	int32 m_window_size;
	int32 m_window_next_entry;
	s_statistics_window_entry m_window_entries[32];
	int32 m_window_total_values;
	int32 m_window_aperture_msec;

public:
	real32 average_values_in_window(void) const;
};
ASSERT_STRUCT_SIZE(c_network_window_statistics, 272);
