#pragma once

/* enums */

/* object update flags */

enum
{
	_simulation_object_update_dead_bit = 0,
	_simulation_object_update_position_bit,
	_simulation_object_update_forward_and_up_bit,
	_simulation_object_update_scale_bit,
	_simulation_object_update_translational_velocity_bit,
	_simulation_object_update_angular_velocity_bit,
	_simulation_object_update_body_vitality_bit,
	_simulation_object_update_shield_vitality_bit,
	_simulation_object_update_region_state_bit,
	_simulation_object_update_constraints_bit,
	k_simulation_object_update_flag_count,
};

/* unit update flags */

enum
{
	_simulation_unit_update_control_bit = k_simulation_object_update_flag_count,
	_simulation_unit_update_parent_vehicle_bit,
	_simulation_unit_update_desired_aiming_vector_bit,
	_simulation_unit_update_desired_weapon_bit,

	_simulation_unit_update_first_weapon_type_bit,
	_simulation_unit_update_last_weapon_type_bit = _simulation_unit_update_first_weapon_type_bit+ (4-1),	// count: MAXIMUM_INITIAL_WEAPONS_PER_UNIT-1 

	_simulation_unit_update_first_weapon_state_bit,
	_simulation_unit_update_last_weapon_state_bit = _simulation_unit_update_first_weapon_state_bit+ (4-1),	// count: MAXIMUM_INITIAL_WEAPONS_PER_UNIT-1 

	_simulation_unit_update_grenade_counts_bit,
	_simulation_unit_update_active_camouflage_bit,
	k_simulation_unit_update_flag_count,
};

/* device update flags */

enum
{
	_simulation_device_update_position_bit = k_simulation_object_update_flag_count,
	_simulation_device_update_position_group_position_bit,
	k_simulation_device_update_flag_count,
};

/* item update flags */

/* projectile update flags */

/* weapon update flags */

/* breakable surface flags */

enum
{
	_simulation_breakable_surface_group_update_bit = 0,
	k_simulation_breakable_surface_group_update_flag_count,
};

/* vehicle update flags */

enum
{
	k_simulation_vehicle_update_flag_count = k_simulation_object_update_flag_count,
};
