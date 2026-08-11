#pragma once
#include "actor_firing_position.h"
#include "ai_destination.h"
#include "ai_direction.h"
#include "ai_path.h"
#include "cl_engine.h"
#include "pathfinding_utilities.h"

/* constants */

enum
{
	k_maximum_prop_ref_indices = 32
};

/* structure */

struct actor_meta_data
{
	int16 type;
	bool general_update;
	bool swarm;
	bool frozen;
	bool active;
	bool squadless;
	bool force_active;
	bool disposable;
	bool erase_next_tick;
	int32 last_active_time;
	int32 last_in_pvs_time;
	datum unit_index;
	datum swarm_index;
	datum next_actor_index;
	int16 team_index;
	int16 highest_prop_status;
	datum prevehicle_squad_index;
	int16 squad_adoption_timeout;
	int16 squad_adoption_attempts;
	datum squad_index;
	datum migration_squad_index;
	int32 starting_location_name;
	bool squad_limbo;
	int16 bsp_index;
	int8 pathfinding_timeslice;
	int16 pathfinding_service_timer;
	int8 perception_timeslice;
	int16 perception_service_timer;
	bool allowed_to_refresh_status;
	int32 spawn_time;
	int16 highest_prop_status_timer;
	int16 highest_prop_state_timer;
	int32 character_definition_index;
	datum first_prop_ref_index;
	int8 prop_ref_indices[k_maximum_prop_ref_indices];
	datum clump_index;
	datum clump_next_actor_index;
};
ASSERT_STRUCT_SIZE(actor_meta_data, 128);

struct actor_state_data
{
	int16 mode;
	int16 combat_status;
	int32 combat_status_timer;
	bool state_had_visible_enemy;
	int8 behavior_state_data[259];
	int16 leaf_layer;
	int8 invitation[48];
	int8 behavior_stimulus[36];
	int16 stimulus_flags;
	int8 behavior_check_timer[56];
	bool state_searching;
	bool group_searching;
	bool state_disallow_retreating;
	bool suppress_combat;
	bool inhibit_vehicle_use;
	bool state_berserk;
	uint8 weapon_slot_index;
	uint8 behavior_index;
	bool blind;
	bool flying;
	bool carrying_turret;
	int8 sentinel_weapon_state;
};
ASSERT_STRUCT_SIZE(actor_state_data, 424);

struct actor_position_data
{
	real_point3d head_position;
	real_point3d body_position;
	real_vector3d body_facing;
	s_location body_location;
	real_vector3d velocity;
};
ASSERT_STRUCT_SIZE(actor_position_data, 56);

struct actor_input_data
{
	actor_position_data input_position;
	bool in_midair;
	bool underwater;
	bool driver;
	bool passenger;
	bool gunner;
	bool boarding;
	int16 input_pad_3E;
	datum root_vehicle_index;
	int16 vehicle_driver_type;
	datum gunning_unit_index;
	bool pathfinding_location_valid;
	real_point3d pathfinding_point;
	int16 pathfinding_point_m_reference_frame;
	int32 sector_reference;
	real_vector3d facing_vector;
	real_vector3d aiming_vector;
	real_vector3d looking_vector;
	real_vector3d looking_left_vector;
	real_vector3d looking_up_vector;
	real32 scariness;
	real32 body_vitality;
	real32 shield_vitality;
	real32 recent_body_damage;
	real32 recent_shield_damage;
	bool player_weighting_valid;
	real32 player_weighting;
};
ASSERT_STRUCT_SIZE(actor_input_data, 188);

#pragma pack(push, 1)
struct actor_vehicle_data
{
	datum recent_vehicle_index;
	int16 recent_seat_index;
	int16 recent_vehicle_exit_ticks;
	bool turtled;
	int8 vehicle_pad_09;
	int16 recent_vehicle_disallow_ticks;
	int16 disallow_vehicle_exit_ticks;
	int16 vehicle_pad_0E;
	real32 attachment_score;
	datum attached_vehicle_index;
	datum attached_seat_unit_index;
	int16 attachment_status;
};
ASSERT_STRUCT_SIZE(actor_vehicle_data, 0x1E);

struct actor_memory_data
{
	int16 ticks_since_enemy_engaged;
	int32 last_damage_vocalization_time;
};
ASSERT_STRUCT_SIZE(actor_memory_data, 6);
#pragma pack(pop)

struct actor_player_data
{
	bool player_looking_at_me;
	int16 player_looking_ticks;
	int16 player_looking_index;
	uint32 postcombat_flags;
	real32 boss_accumulated_body_damage;
	int16 player_index_interested;
	int16 player_interested_ticks;
	int16 player_uninterested_ticks;
};
ASSERT_STRUCT_SIZE(actor_player_data, 24);

struct actor_situation
{
	uint8 known_enemies;
	uint16 nearby_enemy_counts;
	int16 highest_prop_class;
	datum close_enemy;
};
ASSERT_STRUCT_SIZE(actor_situation, 12);

struct actor_target_data
{
	uint16 target_flags;
	int16 preferred_target_team_index;
	datum preferred_ai_index;
	datum target_prop_index;
	int32 target_marker;
	bool target_marker_valid;
	datum retreat_target_prop_index;
	datum target_occluding_object_index;
	bool target_group_include_player;
	int32 since_any_target_visible_timer;
	bool target_salience_boost;
};
ASSERT_STRUCT_SIZE(actor_target_data, 40);

struct actor_danger_zone
{
	int16 danger_zone_danger_type;
	int16 hostility;
	bool allow_dive_evasion;
	bool attached_to_us;
	datum object_index;
	datum owner_unit_index;
	datum pref_index;
	real32 danger_radius;
	real_point3d position;
	real_vector3d velocity;
	real_point3d predict_danger_position;
	real32 current_distance_from_actor;
	real32 current_distance_from_center;
	real32 bounding_sphere_radius;
	real_point3d bounding_sphere_center;
	int16 danger_zone_start_ticks;
};
ASSERT_STRUCT_SIZE(actor_danger_zone, 88);

struct actor_stimulus_data
{
	int16 surprise_level;
	datum surprise_prop_index;
	bool stunned;
	real32 stun_level;
	real_vector3d stunned_vector;
	datum cover_friend_actor_index;
	int16 cover_friend_behavior_index;
	int16 cover_friend_dialogue_type;
};
ASSERT_STRUCT_SIZE(actor_stimulus_data, 36);

struct actor_emotion_data
{
	real32 instantaneous_danger;
	real32 perceived_danger;
	bool prop_closing;
	bool charging_at_prop;
	bool prop_visible;
	bool visible_to_target;
	int16 approach_cooldown_ticks;
	int16 charge_cooldown_ticks;
	bool ignorant_of_broken_surfaces;
	int32 last_vehicle_charge_time;
	int16 field_18;
};
ASSERT_STRUCT_SIZE(actor_emotion_data, 0x1C);

struct actor_discarded_firing_position
{
	bool temporary;
	firing_position_ref index;
};
ASSERT_STRUCT_SIZE(actor_discarded_firing_position, 0x6);

struct actor_firing_position_data
{
	bool current_position_found_outside_range;
	bool proxy_valid;
	bool search_areas_available;
	datum dynamic_firing_set_index;
	datum dynamic_firing_set_support_object_index;
	int16 current_discarded_firing_positions_entry;
	actor_discarded_firing_position discarded_firing_positions[4];
	datum current_position_index;
};
ASSERT_STRUCT_SIZE(actor_firing_position_data, 0x2C);

struct actor_look_orders
{
	int16 aim_type;
	real_vector3d aim_vector;
	real32 aim_direction_z;
	int16 aim_source_type;
	real_vector3d look_vector;
	real32 look_direction_z;
	int16 look_direction_ref;
};
ASSERT_STRUCT_SIZE(actor_look_orders, 44);

struct actor_move_orders
{
	bool stationary_crouch;
	bool moving_crouch;
	bool disallow_combat_crouch;
	bool walk;
	bool move_type_override;
	bool force_weapon_down;
	int32 override_movement_type;
	int16 jump_target_pref_index;
	int8 override_movement_direction;
	real_vector3d override_direction_vector;
	bool jump_boosted;
	bool jump_leap;
	bool jump_targeted;
	real_vector2d jump_alignment_vector;
	real32 jump_target_horizontal_vel;
	real32 jump_target_vertical_vel;
	bool ignore_obstacles;
	real32 scripted_turn_urgency;
	bool scripted_disallow_movement;
	bool zig_zag;
	bool smart_boost;
	bool allow_path_jump;
	bool allow_poses;
	bool allow_moving_poses;
};
ASSERT_STRUCT_SIZE(actor_move_orders, 64);

struct actor_combat_orders
{
	bool shoot_at_target;
	bool bombard_target;
	bool abort_burst;
	bool override_firing_restrictions;
	bool target_manual_point;
	real_point3d target_position;
	int32 target_sector_ref;
	bool disregard_visibility;
	bool lower_weapon;
	bool raise_weapon;
};
ASSERT_STRUCT_SIZE(actor_combat_orders, 28);

struct actor_swarm_orders
{
	int8 action;
	int8 target_type;
	datum target_index;
};
ASSERT_STRUCT_SIZE(actor_swarm_orders, 8);

struct actor_orders
{
	actor_look_orders look;
	actor_move_orders move;
	actor_combat_orders combat;
	actor_swarm_orders swarm;
};
ASSERT_STRUCT_SIZE(actor_orders, 144);

struct actor_path_control_data
{
	c_destination_orders destination_orders;
	int16 status;
	int8 refreshed_this_tick;
	int16 failure_count;
	c_path_result path_result;
};
ASSERT_STRUCT_SIZE(actor_path_control_data, 292);

#pragma pack(push, 1)
struct actor_control_data
{
	actor_path_control_data path;
	bool moving;
	bool nondirectional_movement;
	bool at_goal;
	bool advance_path_output;
	bool crouch;
	char control_pad_129;
	int16 movement_throttle_scale;
	bool stuck;
	int8 control_pad_12D;
	int16 stuck_ticks;
	real_point3d stuck_point;
	int16 isolation_count;
	int16 control_pad_13E;
	real_vector3d moving_towards_vector;
	real_vector3d moving_forced_aim_direction;
	bool movement_direction_change;
	bool persistent_movement_valid;
	int16 control_pad_15A;
	int8 persistent_movement_plane[16];
	real32 persistent_movement_distance;
	int16 persistent_movement_ticks;
	int16 freeze_ticks;
	int16 suppress_shooting_ticks;
	bool overcomes_oversteer;
	int8 control_pad_177;
	real_vector2d jump_alignment_vector;
	real32 jump_target_horizontal_vel;
	real32 jump_target_vertical_vel;
	int32 approach_time;
	int8 approach_point[16];
	real_vector3d approach_facing;
	int16 approach_flags;
	int16 flying_turn_movement_mode;
	real32 steering_yaw_rate;
	real32 steering_yaw;
	real32 steering_pitch_rate;
	real32 steering_pitch;
	real32 steering_accumulated_yaw;
	real_vector3d steering_movement_vector;
	real_vector3d steering_flying_data;
	int16 glance_priority;
	int16 glance_timer;
	c_ai_direction glance_direction;
	int32 idle_time_major;
	int32 idle_time_minor;
	c_ai_direction idle_major_direction;
	c_ai_direction idle_minor_direction;
	bool pose_active;
	int8 pose_pad_215[3];
	int32 pose_timer;
	int32 dialogue_gesture;
	bool dialogue_gesture_pending;
	int8 control_pad_221;
	int16 emotion_ticks;
	int8 firing_disabled;
	bool movement_pending;
	bool movement_changed;
	int8 control_pad_227;
	real_vector3d desired_facing_vector;
	real_vector3d desired_aiming_vector;
	real_vector3d desired_looking_vector;
	bool looking_valid;
	int8 control_pad_24D;
	int16 zig_zag_ticks;
	uint16 weapon_flags;
	int16 fire_state;
	int16 fire_state_timer;
	int16 burst_disable_timer;
	int16 trigger_delay_timer;
	int16 blocked_timer;
	int16 special_fire_delay;
	int16 special_fire_deny_attempts;
	bool accuracy_reset_pending;
	int8 control_pad_261[3];
	real32 accuracy;
	bool suppress_projectile_impact;
	int8 control_pad_269[3];
	int32 last_burst_start_time;
	real32 burst_distance;
	int16 blocked_projectiles_count;
	int16 current_fire_target_type;
	datum fire_target_prop_index;
	real_point3d current_fire_target_manual_point;
	int32 current_fire_target_timer;
	uint8 fire_target_type;
	uint8 fire_target_perception;
	uint8 fire_target_aim_state;
	bool fire_target_suppress_tracking;
	uint8 fire_target_los_flags;
	int8 fire_target_pad_291;
	int16 current_fire_target_line_of_sight;
	bool fire_target_acquired;
	int8 fire_target_pad_295[3];
	c_ai_point3d current_fire_target_position;
	real32 current_fire_target_range;
	real_vector3d current_fire_target_aim_vector;
	real32 current_fire_target_distance;
	real_point3d burst_initial_position;
	int32 burst_pad_2C8;
	real_point3d burst_origin;
	real_point3d burst_relative_position;
	real_point3d burst_adjustment;
	real_point3d burst_target;
	real32 burst_aim_by_vector_velocity;
	real32 burst_aim_speed;
	real_vector3d burst_aim_vector;
	real32 burst_error;
	real32 burst_damage_modifier;
	int16 grenade_request_ticks;
	uint16 grenade_flags;
	int32 grenade_last_throw_time;
	bool grenade_retarget;
	int8 control_pad_321[3];
	c_ai_point3d grenade_current_target;
	datum grenade_current_prop_index;
	datum grenade_current_ignore_object_index;
	real_vector3d grenade_current_aim_vector;
	real32 grenade_current_aim_speed;
	int32 last_jump_time;
	int16 control_pad_350;
};
ASSERT_STRUCT_SIZE(actor_control_data, 850);
#pragma pack(pop)

struct actor_output_data
{
	int32 movement_type;
	int32 animation_impulse;
	real_vector2d animation_alignment_vector;
	int16 aiming_speed;
	int8 gap_20[6];
	uint32 control_flags;
	uint32 control_flags_high;
	real_vector3d throttle;
	real32 analog_primary_trigger;
	real32 analog_secondary_trigger;
	real_vector3d facing_vector;
	real_vector3d aiming_vector;
	real_vector3d looking_vector;
	real_point3d gaze_position;
	int32 padding;
};
ASSERT_STRUCT_SIZE(actor_output_data, 104)

struct actor_script_data
{
	datum first_command_script_index;
	datum active_command_script_index;
	int script_pad_08;
	cs_control control;
};
ASSERT_STRUCT_SIZE(actor_script_data, 48);

struct actor_datum
{
	int32 identifier;
	actor_meta_data meta;
	actor_state_data state;
	actor_input_data input;
	actor_vehicle_data vehicle;
	actor_memory_data memory;
	actor_player_data player;
	actor_situation situation;
	actor_target_data target;
	actor_danger_zone danger_zone;
	actor_stimulus_data stimulus;
	actor_emotion_data emotions;
	actor_firing_position_data firing_position;
	actor_orders orders;
	actor_control_data control;
	actor_output_data output;
	actor_script_data commands;
};
ASSERT_STRUCT_SIZE(actor_datum, 2200);

/* prototypes */

data_array* get_actor_table(void);

void __cdecl actors_initialize_for_new_map(void);
