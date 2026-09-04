#pragma once
#include "controllers.h"
#include "input_constants.h"

/* macros */

#define THUMBSTICK_PERCENTAGE_TO_POINT(_percentage) ((real32)INT16_MAX * ((_percentage) / 100.f))
#define THUMBSTICK_POINT_TO_PERCENTAGE(_point) (((real32)_point / (real32)INT16_MAX) * 100.f)

/* constants */

enum
{
	k_key_hold_threshold_msec = 233,
	k_last_used_device_was_gamepad = 1,
	NUMBER_OF_GAMEPAD_STICKS = 2,
	MAX_BUTTONS_PER_CONTROL = 8,
};

// Set this to a default of 8
extern const real32 k_default_right_thumbstick_deadzone_radial_percentage;

extern const real_point2d k_default_right_thumbstick_deadzone_axial_percentage;

/* enums */

enum e_input_device_types
{
	_input_device_type_general = NONE,
	_input_device_type_mouse = 0,
	_input_device_type_keyboard,
	_input_device_type_gamepad
};

enum e_joystick_preset_types : int8
{
	_joystick_preset_default = 0,
	_joystick_preset_south_paw,
	_joystick_preset_legacy,
	_joystick_preset_legacy_south_paw,
	_joystick_preset_unused4
};

enum e_button_preset_types : int8
{
	_button_preset_default = 0,
	_button_preset_south_paw,
	_button_preset_boxer,
	_button_preset_green_thumb,
	//_button_preset_unused4,
	_button_preset_jumpy,
	_button_preset_unused5,
};

enum e_custom_keyboard_preset_types : int8
{
	_custom_keyboard_preset_right_hold = 0,	//default
	_custom_keyboard_preset_right_split,	//skirmish
	_custom_keyboard_preset_left_hold,		//southpaw
	_custom_keyboard_preset_left_split,		//southpaw_boxer
	_custom_keyboard_preset_custom
};

enum e_abstract_gamepad_stick_types
{
	_abstract_gamepad_stick_unknown = NONE,
	_abstract_gamepad_stick_left = 0,
	_abstract_gamepad_stick_right,
	k_abstract_gamepad_stick_count
};

enum e_button_action
{
	_button_jump = 0x0,
	_button_trick = 0x1,
	_button_brake = 0x2,
	_button_switch_grenade = 0x3,
	_button_switch_weapon = 0x4,
	_button_melee_attack = 0x5,
	_button_flashlight = 0x6,
	_button_throw_grenade = 0x7,
	_button_speed_boost = 0x8,
	_button_e_brake = 0x9,
	_button_fire = 0xA,
	_button_start = 0xB,
	_button_back = 0xC,
	_button_crouch = 0xD,
	_button_scope_zoom = 0xE,
	_button_scope_zoom_in = 0xF,
	_button_scope_zoom_out = 0x10,
	_button_lean_left = 0x11,
	_button_lean_right = 0x12,
	_button_accept = 0x13,
	_button_cancel = 0x14,
	_button_banshee_bomb = 0x15,
	_button_dual_wield_primary_fire = 0x16,
	_button_dual_wield_secondary_fire = 0x17,
	_button_vehicle_primary_fire = 0x18,
	_button_vehicle_secondary_fire = 0x19,
	_button_text_chat_toggle = 0x1A,
	_button_text_chat_team = 0x1B,
	_button_ui_scroll_up = 0x1C,
	_button_ui_scroll_down = 0x1D,
	_button_team_voice = 0x1E,
	_button_flip_vehicle = 0x1F,
	_button_touch_device = 0x20,
	_button_enter_vehicle = 0x21,
	_button_board_vehicle = 0x22,
	_button_evict_from_vehicle = 0x23,
	_button_trade_weapon = 0x24,
	_button_pick_up_primary_weapon = 0x25,
	_button_pick_up_primary_multiplayer_weapon = 0x26,
	_button_pick_up_secondary_weapon = 0x27,
	_button_pick_up_secondary_multiplayer_weapon = 0x28,
	_button_put_away_secondary_weapon = 0x29,
	_button_put_away_or_drop_secondary_weapon = 0x2A,
	_button_reload = 0x2B,
	_button_exit_vehicle = 0x2C,
	_button_move_forward = 0x2D,
	_button_move_backward = 0x2E,
	_button_strafe_left = 0x2F,
	_button_strafe_right = 0x30,
	_button_mouse_yaw_left = 0x31,
	_button_mouse_yaw_right = 0x32,
	_button_mouse_pitch_forward = 0x33,
	_button_mouse_pitch_backward = 0x34,
	_extended_button_gamepad_yaw_left = 0x35,
	_extended_button_gamepad_yaw_right = 0x36,
	_extended_button_gamepad_pitch_forward = 0x37,
	_extended_button_gamepad_pitch_backward = 0x38,

	NUMBER_OF_EXTENDED_CONTROL_BUTTONS = 0x39,
	NUMBER_OF_ABSTRACT_BUTTONS = 0x2D,
};


enum e_gamepad_buttons
{
	_gamepad_analog_button_left_trigger = 0x0,
	_gamepad_analog_button_right_trigger = 0x1,
	_gamepad_binary_button_dpad_up = 0x2,
	_gamepad_binary_button_dpad_down = 0x3,
	_gamepad_binary_button_dpad_left = 0x4,
	_gamepad_binary_button_dpad_right = 0x5,
	_gamepad_binary_button_start = 0x6,
	_gamepad_binary_button_back = 0x7,
	_gamepad_binary_button_left_thumb = 0x8,
	_gamepad_binary_button_right_thumb = 0x9,
	_gamepad_binary_button_left_shoulder = 0xA,
	_gamepad_binary_button_right_shoulder = 0xB,
	_gamepad_binary_button_a = 0xC,
	_gamepad_binary_button_b = 0xD,
	_gamepad_binary_button_x = 0xE,
	_gamepad_binary_button_y = 0xF,
	NUMBER_OF_GAMEPAD_BUTTONS,

	_gamepad_analog_left_stick_up = 0x10,
	_gamepad_analog_left_stick_down = 0x11,
	_gamepad_analog_left_stick_left = 0x12,
	_gamepad_analog_left_stick_right = 0x13,
	_gamepad_analog_right_stick_up = 0x14,
	_gamepad_analog_right_stick_down = 0x15,
	_gamepad_analog_right_stick_left = 0x16,
	_gamepad_analog_right_stick_right = 0x17,

	NUMBER_OF_GAMEPAD_BUTTON_STRINGS = 0x18,
};

enum e_mouse_buttons
{
	_mouse_button_left = 0x0,
	_mouse_button_middle = 0x1,
	_mouse_button_right = 0x2,
	_mouse_button_4 = 0x3,
	_mouse_button_5 = 0x4,
	_mouse_button_6 = 0x5,
	_mouse_button_7 = 0x6,
	_mouse_button_8 = 0x7,
	NUMBER_OF_MOUSE_BUTTONS,


	_mouse_delta_x_left = 0x8,
	_mouse_delta_x_right = 0x9,
	_mouse_delta_y_up = 0xA,
	_mouse_delta_y_down = 0xB,
	_mouse_delta_w_forward = 0xC,
	_mouse_delta_w_back = 0xD,

	NUMBER_OF_MOUSE_BUTTON_STRINGS = 0xE,
};


/* structures */

struct key_stroke
{
	uint8 modifier_flags;
	int8 ascii_code;
	wchar_t utf16_code;
	e_input_key_code key_code;
	bool repeating;
};
ASSERT_STRUCT_SIZE(key_stroke, 0x8);

struct s_input_button
{
	e_input_device_types m_device_type;
	uint32 m_device_key;
	uint32 m_device_key_held_time_msec;
};
ASSERT_STRUCT_SIZE(s_input_button, 0xC);

class c_input_control
{
public:
	uint32 button_count;
	s_input_button buttons[MAX_BUTTONS_PER_CONTROL];

	bool add_button(e_input_device_types device_type, uint32 keycode, uint32 held_time, bool return_value_handled);
	bool remove_button(uint32 button_index);

};
ASSERT_STRUCT_SIZE(c_input_control, 0x64);


struct s_gamepad_input_preferences
{
	real32 mouse_yaw_rate;
	real32 mouse_pitch_rate;
	real32 gamepad_yaw_rate;
	real32 gamepad_pitch_rate;
	bool gamepad_invert_look;
	bool mouse_invert_look;
	bool invert_aircraft_control;
	uint8 gap_13;
	c_input_control game_controls_to_hardware[NUMBER_OF_EXTENDED_CONTROL_BUTTONS];
	uint32 field_1658;
	real32 binary_yaw_rate;
	real32 binary_pitch_rate;
	real32 stick_threshold;
	real32 mouse_delta_threshold;
	real32 mouse_wheel_threshold;
	bool invert_dual_wield;
	uint8 gap_1671[3];
	real32 mouse_acceleration;
	point2d gamepad_axial_deadzone_left;
	point2d gamepad_axial_deadzone_right;
};
ASSERT_STRUCT_SIZE(s_gamepad_input_preferences, 0x1680);

class s_abstract_button
{
	real32 m_down_amount;
	uint8 m_down_frames;
	char gap;
	uint16 m_down_msec;
};
ASSERT_STRUCT_SIZE(s_abstract_button, 8);

struct s_game_abstracted_input_state
{
	s_abstract_button buttons[NUMBER_OF_EXTENDED_CONTROL_BUTTONS];
};
ASSERT_STRUCT_SIZE(s_game_abstracted_input_state, 0x1C8);

// TODO : verify this struct and NUMBER_OF_ABSTRACT_BUTTONS
struct s_game_input_state
{
	uint8 m_down_frames[NUMBER_OF_ABSTRACT_BUTTONS];
	uint8 pad2E;
	uint16 m_down_msec[NUMBER_OF_ABSTRACT_BUTTONS];
	real32 primary_fire;
	real32 secondary_fire;
	real32 forward_movement;
	real32 strafe;
	real_euler_angles2d mouse;
	real_euler_angles2d gamepad;
	real32 mouse_pitch2;
	real32 gamepad_pitch2;
	uint32 field_B0;
	uint32 field_B4;
};
ASSERT_STRUCT_SIZE(s_game_input_state, 0xB8);

struct s_input_abstraction_globals
{
	s_gamepad_input_preferences preferences[k_number_of_controllers];
	s_game_abstracted_input_state abstracted_inputs;
	s_game_input_state input_states[k_number_of_controllers];
	uint32 controller_detection_timer;
	bool input_has_gamepad[k_number_of_controllers];
	bool input_device_changed;
	uint8 gap_5EB1[7];
};
ASSERT_STRUCT_SIZE(s_input_abstraction_globals, 0x5EB8);

/* globals */

extern s_input_abstraction_globals* g_input_abstraction_globals;

/* prototypes */

void input_abstraction_patches_apply(void);
void __cdecl input_abstraction_initialize(void);
void __cdecl input_abstraction_dispose(void);
void __cdecl input_abstraction_handle_device_change(uint32 flags);
void __cdecl input_abstraction_get_controller_preferences(e_controller_index controller_index, s_gamepad_input_preferences* preferences);
void __cdecl input_abstraction_get_input_state(e_controller_index controller_index, s_game_input_state* state);
void __cdecl input_abstraction_get_player_look_angular_velocity(e_controller_index controller_index, real_euler_angles2d* angular_velocity);
void __cdecl input_abstraction_get_player_look_angular_velocity_for_mouse(e_controller_index controller_index, real_euler_angles2d* angular_velocity);
void __cdecl input_abstraction_get_default_preferences(s_gamepad_input_preferences* out_preference, e_joystick_preset_types thumbstick_layout, e_button_preset_types button_preset_type, e_custom_keyboard_preset_types kb_layout, bool caller_handles_failures);
void __cdecl input_abstraction_set_controller_settings_from_preferences(s_gamepad_input_preferences* preferences, struct s_saved_game_profile_input_preferences* controller_settings);
void __cdecl input_abstraction_set_preferences_from_controller_settings(s_gamepad_input_preferences* preferences, struct s_saved_game_profile_input_preferences* controller_settings);
void input_abstraction_set_mouse_look_sensitivity(e_controller_index controller, real32 value);
void input_abstraction_set_controller_look_sensitivity(e_controller_index controller, real32 value);
void input_abstraction_set_controller_right_thumb_deadzone(e_controller_index controller);
bool __cdecl input_abstraction_controller_button_test(e_controller_index controller_index, e_button_action button_index);
e_button_action __cdecl input_abstraction_get_primary_fire_button(datum unit);
e_button_action __cdecl input_abstraction_get_secondary_fire_button(datum unit);
void __cdecl input_abstraction_update(void);
void __cdecl input_abstraction_update_input_state(e_controller_index controller_index, s_gamepad_input_preferences* preference, struct s_gamepad_input_button_state* gamepad_state, real_euler_angles2d* left_stick_analog, real_euler_angles2d* right_stick_analog, s_game_input_state* input_state);
uint32 s_input_abstraction_globals_sub_45E501(e_button_action button, void* a3);
int32 __cdecl input_abstraction_get_last_used_device(e_controller_index controller);
