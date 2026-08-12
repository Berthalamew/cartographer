#pragma once

/* structures */

struct s_scenario_light_fixture_datum
{
	real_rgb_color color;
	real32 intensity;
	real32 falloff_angle_degrees;
	real32 cutoff_angle_degrees;
};

// max count: MAXIMUM_LIGHT_FIXTURE_DATUMS_PER_SCENARIO 500
struct s_scenario_light_fixture
{
	// filt
	// Block index: scenario_light_fixture_palette
	int16 type;
	// filt
	// Block index: scenario_object_name
	int16 name;

	s_scenario_object_datum object_data;
	s_scenario_device_datum device_data;
	s_scenario_light_fixture_datum lightfixture_data;
};
ASSERT_STRUCT_SIZE(s_scenario_light_fixture, 84);
