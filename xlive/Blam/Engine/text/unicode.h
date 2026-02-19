#pragma once

/* macros */

#define TO_PRIVATE_CHAR(id) ((k_first_unicode_private_use_character + (id)))

/* enums */

enum e_utf32
{
    _unicode_character_not_found = 0x25A1,

	/* private characters */

	k_first_unicode_private_use_character = 0xE000,

	_unicode_private_font_icon_a_button = 0xE100,
	_unicode_private_font_icon_b_button,
	_unicode_private_font_icon_x_button,
	_unicode_private_font_icon_y_button,
	_unicode_private_font_icon_black_button,
	_unicode_private_font_icon_white_button,
	_unicode_private_font_icon_left_trigger,
	_unicode_private_font_icon_right_trigger,
	_unicode_private_font_icon_dpad_up,
	_unicode_private_font_icon_dpad_down,
	_unicode_private_font_icon_dpad_left,
	_unicode_private_font_icon_dpad_right,
	_unicode_private_font_icon_start_button,
	_unicode_private_font_icon_back_button,
	_unicode_private_font_icon_left_thumb,
	_unicode_private_font_icon_right_thumb,
	_unicode_private_font_icon_left_stick,
	_unicode_private_font_icon_right_stick,

	/* weapons */
	
	_unicode_private_font_icon_first_weapon,
	_unicode_private_font_icon_battle_rifle = _unicode_private_font_icon_first_weapon,
	_unicode_private_font_icon_bomb,
	_unicode_private_font_icon_brute_shot,
	_unicode_private_font_icon_carbine,
	_unicode_private_font_icon_flag,
	_unicode_private_font_icon_flak_cannon,
	_unicode_private_font_icon_flamethrower,
	_unicode_private_font_icon_fuel_rod,
	_unicode_private_font_icon_magnum,
	_unicode_private_font_icon_needler,
	_unicode_private_font_icon_oddball,
	_unicode_private_font_icon_plasma_pistol,
	_unicode_private_font_icon_plasma_rifle,
	_unicode_private_font_icon_plasma_sword,
	_unicode_private_font_icon_rocket_launcher,
	_unicode_private_font_icon_shotgun,
	_unicode_private_font_icon_smg,
	_unicode_private_font_icon_sniper,
	_unicode_private_font_icon_beam_rifle,
	_unicode_private_font_icon_disintegrator,
	_unicode_private_font_icon_sentinel_beam_weapon,
	_unicode_private_font_icon_sentinel_needle_weapon,
	_unicode_private_font_icon_sentinel_grenade_weapon,
	_unicode_private_font_icon_sentinel_charge_weapon,
	_unicode_private_font_icon_brute_plasma_rifle,
	_unicode_private_font_icon_silenced_smg,
	_unicode_private_font_icon_last_weapon = _unicode_private_font_icon_silenced_smg,

	_unicode_private_font_icon_animating_left_thumbstick0,
	_unicode_private_font_icon_animating_left_thumbstick1,
	_unicode_private_font_icon_animating_left_thumbstick2,
	_unicode_private_font_icon_animating_right_thumbstick0,
	_unicode_private_font_icon_animating_right_thumbstick1,
	_unicode_private_font_icon_animating_right_thumbstick2,

	_unicode_private_use_keyboard_button_background0,
	_unicode_private_use_keyboard_button_background1,
	_unicode_private_use_keyboard_button_background2,
	_unicode_private_use_keyboard_button_background3,
	_unicode_private_use_footer_button_bg0_normal,
	_unicode_private_use_footer_button_bg1_normal,
	_unicode_private_use_footer_button_bg2_normal,
	_unicode_private_use_footer_button_bg0_hovered,
	_unicode_private_use_footer_button_bg1_hovered,
	_unicode_private_use_footer_button_bg2_hovered,
	_unicode_private_use_gold_only,

	/* sequences */

	k_first_unicode_private_sequence_character = 0xE300,
	_unicode_private_use_set_font = k_first_unicode_private_sequence_character,
	_unicode_private_use_set_color,
	k_last_unicode_private_sequence_character = 0xE3FF,

	/* private use characters */

	k_first_non_icon_private_use_character = 0xE400,
	_unicode_private_use_style_plain = k_first_non_icon_private_use_character,
	_unicode_private_use_style_italic,
	_unicode_private_use_style_bold,
	_unicode_private_use_style_condense,
	_unicode_private_use_style_underline,
	_unicode_private_use_justification_left,
	_unicode_private_use_justification_right,
	_unicode_private_use_justification_center,
	_unicode_private_use_mapname,
	_unicode_private_use_gametype,
	_unicode_private_use_variant,
	_unicode_private_use_motion_sensor_enabled,
	_unicode_private_use_teams_enabled,
	_unicode_private_use_vehicle_set,
	_unicode_private_use_weapon_set,
	_unicode_private_use_player0_profile_name,
	_unicode_private_use_player1_profile_name,
	_unicode_private_use_player2_profile_name,
	_unicode_private_use_player3_profile_name,
	_unicode_private_use_countdown,
	_unicode_private_use_name_of_last_person_to_delay_countdown,
	_unicode_private_use_build_number,
	_unicode_private_use_leader,
	_unicode_private_use_live_service_qos_probes_sent,
	_unicode_private_use_live_service_qos_probes_received,
	_unicode_private_use_live_service_qos_ping_minimum,
	_unicode_private_use_live_service_qos_ping_median,
	_unicode_private_use_live_service_qos_bandwidth_downstream,
	_unicode_private_use_live_service_qos_bandwidth_upstream,
	_unicode_private_use_live_max_machines,
	_unicode_private_use_live_nat_type,
	_unicode_private_use_active_multiplayer_protocol,
	_unicode_private_use_live_ui_driver_gamertag,
	_unicode_private_use_live_ui_driver_clan_name,
	_unicode_private_use_hud_quantity,
	_unicode_private_use_game_engine_round_time_left,
	_unicode_private_use_target_player_gamertag,
	_unicode_private_use_network_game_player_count,
	_unicode_private_use_coop_level_name,
	_unicode_private_use_coop_level_description,
	_unicode_private_use_coop_difficulty,
	_unicode_private_use_player0_gamertag,
	_unicode_private_use_player1_gamertag,
	_unicode_private_use_player2_gamertag,
	_unicode_private_use_player3_gamertag,
	_unicode_private_use_hud_scoreboard_player_score,
	_unicode_private_use_hud_scoreboard_other_player_score,
	_unicode_private_use_hud_scoreboard_variant_name,
	_unicode_private_use_hud_scoreboard_time_left,
	_unicode_private_use_edit_player_profile_name,
	_unicode_private_use_live_estimated_bandwidth,
	_unicode_private_use_player_profile_blocks,
	_unicode_private_use_game_variant_blocks,
	_unicode_private_use_playlist_blocks,
	_unicode_private_use_saved_film_blocks,
	_unicode_private_use_target_player_clan_name,
	_unicode_private_use_target_player_game_title_name,
	_unicode_private_use_playlist,
	_unicode_private_use_hopper_name,
	_unicode_private_use_hopper_description,
	_unicode_private_use_target_player_clan_level,
	_unicode_private_use_live_ui_driver_clan_level,
	_unicode_private_use_hour_12,
	_unicode_private_use_hour_24,
	_unicode_private_use_minute,
	_unicode_private_use_day,
	_unicode_private_use_month,
	_unicode_private_use_year,
	_unicode_private_use_saved_game_size,
	_unicode_private_use_total_minimal_storage_blocks,
	_unicode_private_use_button_jump,
	_unicode_private_use_button_switch_grenade,
	_unicode_private_use_button_action_reload,
	_unicode_private_use_button_switch_weapon,
	_unicode_private_use_button_melee_attack,
	_unicode_private_use_button_flashlight,
	_unicode_private_use_button_throw_grenade,
	_unicode_private_use_button_fire,
	_unicode_private_use_button_start,
	_unicode_private_use_button_back,

	_unicode_private_use_button_crouch,
	_unicode_private_use_button_scope_zoom,
	_unicode_private_use_button_scope_zoom_in,
	_unicode_private_use_button_scope_zoom_out,
	_unicode_private_use_button_lean_left,
	_unicode_private_use_button_lean_right,
	_unicode_private_use_button_accept,
	_unicode_private_use_button_cancel,
	_unicode_private_use_button_trick,
	_unicode_private_use_button_brake,
	_unicode_private_use_button_pickup_weapon,
	_unicode_private_use_button_take_weapon,
	_unicode_private_use_button_speed_boost,
	_unicode_private_use_button_e_brake,
	_unicode_private_use_button_banshee_bomb,
	_unicode_private_use_button_stealth,

	_unicode_private_use_button_chat = TO_PRIVATE_CHAR(0x460),
	_unicode_private_use_button_touch_device = TO_PRIVATE_CHAR(0x461),


	_unicode_private_use_button_flip_vehicle = TO_PRIVATE_CHAR(0x466),
	_unicode_private_use_button_enter_vehicle = TO_PRIVATE_CHAR(0x467),
	_unicode_private_use_button_board_vehicle = TO_PRIVATE_CHAR(0x468),
	_unicode_private_use_button_evict_from_vehicle = TO_PRIVATE_CHAR(0x469),
	_unicode_private_use_button_pickup_dual_wield = TO_PRIVATE_CHAR(0x46A),
	_unicode_private_use_button_move_forward = TO_PRIVATE_CHAR(0x46B),
	_unicode_private_use_button_move_backward = TO_PRIVATE_CHAR(0x46C),
	_unicode_private_use_button_strafe_left = TO_PRIVATE_CHAR(0x46D),
	_unicode_private_use_button_strafe_right = TO_PRIVATE_CHAR(0x46E),
	_unicode_private_use_button_yaw_left = TO_PRIVATE_CHAR(0x46F),

	_unicode_private_use_button_yaw_right = TO_PRIVATE_CHAR(0x470),
	_unicode_private_use_button_pitch_forward = TO_PRIVATE_CHAR(0x471),
	_unicode_private_use_button_pitch_backward = TO_PRIVATE_CHAR(0x472),
	_unicode_private_use_stick_move = TO_PRIVATE_CHAR(0x473),
	_unicode_private_use_stick_look = TO_PRIVATE_CHAR(0x474),
	_unicode_private_use_hud_betraying_player = TO_PRIVATE_CHAR(0x475),
	_unicode_private_use_animating_left_thumbstick = TO_PRIVATE_CHAR(0x476),
	_unicode_private_use_animating_right_thumbstick = TO_PRIVATE_CHAR(0x477),
	_unicode_private_use_matchmaking_countdown = TO_PRIVATE_CHAR(0x478),
	_unicode_private_use_new_content = TO_PRIVATE_CHAR(0x479),
	_unicode_private_use_map_load_percent = TO_PRIVATE_CHAR(0x47A),
	_unicode_private_use_button_dual_fire = TO_PRIVATE_CHAR(0x47B),
	_unicode_private_use_button_verb1 = TO_PRIVATE_CHAR(0x47C),
	_unicode_private_use_button_verb2 = TO_PRIVATE_CHAR(0x47D),
	_unicode_private_use_button_verb3 = TO_PRIVATE_CHAR(0x47E),
	_unicode_private_use_button_verb4 = TO_PRIVATE_CHAR(0x47F),


	_unicode_private_use_training_looklong_grouping = TO_PRIVATE_CHAR(0x480),
	_unicode_private_use_training_looklongH_grouping = TO_PRIVATE_CHAR(0x481),
	_unicode_private_use_training_move_grouping = TO_PRIVATE_CHAR(0x482),
	_unicode_private_use_training_moveURL_grouping = TO_PRIVATE_CHAR(0x483),
	_unicode_private_use_grouping_EOS = TO_PRIVATE_CHAR(0x486),
	_unicode_private_use_looklong_grouping_nested = TO_PRIVATE_CHAR(0x487),
	_unicode_private_use_move_grouping_nested = TO_PRIVATE_CHAR(0x488),
	_unicode_private_use_map_download_percent = TO_PRIVATE_CHAR(0x489),
	_unicode_private_use_proposer = TO_PRIVATE_CHAR(0x48A),
	_unicode_private_use_yes_votes = TO_PRIVATE_CHAR(0x48B),
	_unicode_private_use_no_votes = TO_PRIVATE_CHAR(0x48C),
	_unicode_private_use_vote_booted = TO_PRIVATE_CHAR(0x48D),
	_unicode_private_use_vote_timer = TO_PRIVATE_CHAR(0x48E),
	_unicode_private_use_vote_variant = TO_PRIVATE_CHAR(0x48F),

	_unicode_private_use_vote_mapname = TO_PRIVATE_CHAR(0x490),
	_unicode_private_use_pending_votes = TO_PRIVATE_CHAR(0x491),
	_unicode_private_use_resolution_change_seconds_remaining = TO_PRIVATE_CHAR(0x492),
	_unicode_private_use_button_roster = TO_PRIVATE_CHAR(0x493),

	k_last_unicode_private_use_character = 0xF8FF
};

/* structures */

struct s_escape_table
{
    wchar_t wchar_character;
    e_utf32 unicode_character;
};

/* prototypes */

int32 ustrcmp(const wchar_t* string1, const wchar_t* string2);

size_t ustrlen(const wchar_t* string);

size_t ustrnlen(const wchar_t* string, size_t max_count);

int32 ustrcoll(const wchar_t* string1, const wchar_t* string2);

size_t ustrcspn(const wchar_t* string, const wchar_t* character_set);

wchar_t* ustrncat(wchar_t* dest, const wchar_t* src, size_t count);

int32 ustrncmp(const wchar_t* string1, const wchar_t* string2, size_t max_count);

wchar_t* ustrncpy(wchar_t* dst, const wchar_t* src, size_t count);

const wchar_t* ustrpbrk(const wchar_t* string, const wchar_t* character_set);

const wchar_t* ustrrchr(const wchar_t* string, wchar_t c);

size_t ustrspn(const wchar_t* string, const wchar_t* character_set);

const wchar_t* ustrstr(const wchar_t* string, const wchar_t* character_set);

size_t ustrxfrm(wchar_t* dest, wchar_t* src, size_t count);

wchar_t* ustrnlwr(wchar_t* string, int32 count);

wchar_t* ustrnupr(wchar_t* string, int32 count);

int32 ustricmp(const wchar_t* string1, const wchar_t* string2);

int32 ustrnicmp(const wchar_t* string1, const wchar_t* string2, size_t count);

int32 uisalpha(wchar_t c);

int32 uisupper(wchar_t c);

int32 uislower(wchar_t c);

int32 uiswdigit(wchar_t c);

int32 uisxdigit(wchar_t c);

int32 uisspace(wchar_t c);

int32 uispunct(wchar_t c);

int32 uisalnum(wchar_t c);

int32 uisprint(wchar_t c);

int32 uisgraph(wchar_t c);

int32 uiscntrl(wchar_t c);

wchar_t utoupper(wchar_t c);

wchar_t utolower(wchar_t c);

wchar_t ufgetc(struct _iobuf* stream);

wchar_t ufputc(wchar_t c, struct _iobuf* stream);

wchar_t uungetc(wchar_t c, struct _iobuf* stream);

wchar_t* ufgets(wchar_t* string, int32 size, struct _iobuf* stream);

int32 ufputs(const wchar_t* string, struct _iobuf* stream);

wchar_t* ugets(wchar_t* string, size_t count);

int32 uputs(const wchar_t* string);

int32 ufprintf(struct _iobuf* stream, const wchar_t* format, ...);

int32 uprintf(const wchar_t* format, ...);

int32 usnprintf(wchar_t* string, size_t size, const wchar_t* format, ...);

int32 uvfprintf(struct _iobuf* stream, const wchar_t* format, char* ap);

int32 uvprintf(const wchar_t* format, char* ap);

int32 uvsnprintf(wchar_t* string, size_t size, const wchar_t* format, char* ap);

struct _iobuf* ufdopen(int32 fd, const wchar_t* path);

struct _iobuf* ufopen(const wchar_t* path, const wchar_t* mode);

int32 ufclose(struct _iobuf* stream);

struct _iobuf* ufreopen(const wchar_t* path, const wchar_t* mode, struct _iobuf* stream);

void uperror(const wchar_t* string);

struct _iobuf* upopen(const wchar_t* command, const wchar_t* mode);

int32 uremove(const wchar_t* path);

bool utmpnam(wchar_t* Buffer, size_t count);

int32 ustrtol(const wchar_t* nptr, wchar_t** endptr, int32 base);

int32 ustrtoul(const wchar_t* nptr, wchar_t** endptr, int32 base);

real64 ustrtod(const wchar_t* nptr, wchar_t** endptr);

int32 utol(const wchar_t* string);

void wchar_string_to_ascii_string(const wchar_t* src, char* dst, int32 count);

void __cdecl ascii_string_to_wchar_string(const char* src, wchar_t* dst, int32 count);

// Converts a utf8 string to a wide char string with the specified length
void __cdecl utf8_string_to_wchar_string(const utf8* src, wchar_t* dst, int32 count);

// Converts a wide char string to a utf8 string with the specified length
void __cdecl wchar_string_to_utf8_string(const wchar_t* src, utf8* dst, int32 count);

// Returns length (in characters) of utf8 string
size_t utf8_string_length(const utf8* src, size_t size);

bool ugetenv(wchar_t* buffer, size_t count, const wchar_t* var_name);

utf32 ascii_string_to_utf32_characters(const char* in_src, const char** out_next);

utf32 __cdecl wchar_string_to_utf32_characters(const wchar_t* in_src, const wchar_t** out_next);

void __cdecl ascii_string_to_utf32_string(const char* string, utf32* utf32_string, int32 count);

void __cdecl wchar_string_to_utf32_string(const wchar_t* string, utf32* utf32_string, int32 count);


/* public code */

template <typename t_storage_type>
void string_to_utf32_string(
    const t_storage_type* in_src,
    utf32* in_dst,
    utf32(*character_type_string_to_utf32_characters)(const t_storage_type*, const t_storage_type**),
    int32 in_buffer_count
)
{
	while (in_buffer_count > 0)
	{
		const bool last_character = in_buffer_count == 1;
		in_dst[0].character = last_character ? 0 : character_type_string_to_utf32_characters(in_src, &in_src).character;
        if (!in_dst[0].character)
        {
            break;
        }

        ++in_dst;
        --in_buffer_count;
    }
	return;
}
