#pragma once

enum e_emblem_foreground
{
	_emblem_foreground_seventh_column = 0,
	_emblem_foreground_bullseye,
	_emblem_foreground_vortex,
	_emblem_foreground_halt,
	_emblem_foreground_spartan,
	_emblem_foreground_da_bomb,
	_emblem_foreground_trinity,
	_emblem_foreground_delta,
	_emblem_foreground_rampancy,
	_emblem_foreground_sergeant,
	_emblem_foreground_phenoix,
	_emblem_foreground_champion,
	_emblem_foreground_jolly_roger,
	_emblem_foreground_marathon,
	_emblem_foreground_cube,
	_emblem_foreground_radioactive,
	_emblem_foreground_smiley,
	_emblem_foreground_frowney,
	_emblem_foreground_spearhead,
	_emblem_foreground_sol,
	_emblem_foreground_waypoint,
	_emblem_foreground_ying_yang,
	_emblem_foreground_helmet,
	_emblem_foreground_triad,
	_emblem_foreground_grunt_symbol,
	_emblem_foreground_cleave,
	_emblem_foreground_thor,
	_emblem_foreground_skull_king,
	_emblem_foreground_triplicate,
	_emblem_foreground_subnova,
	_emblem_foreground_flaming_ninja,
	_emblem_foreground_doubleCresent,
	_emblem_foreground_spades,
	_emblem_foreground_clubs,
	_emblem_foreground_diamonds,
	_emblem_foreground_hearts,
	_emblem_foreground_wasp,
	_emblem_foreground_mark_of_shame,
	_emblem_foreground_snake,
	_emblem_foreground_hawk,
	_emblem_foreground_lips,
	_emblem_foreground_capsule,
	_emblem_foreground_cancel,
	_emblem_foreground_gas_mask,
	_emblem_foreground_grenade,
	_emblem_foreground_tsanta,
	_emblem_foreground_race,
	_emblem_foreground_valkyire,
	_emblem_foreground_drone,
	_emblem_foreground_grunt,
	_emblem_foreground_grunt_head,
	_emblem_foreground_brute_head,
	_emblem_foreground_runes,
	_emblem_foreground_trident,
	_emblem_foreground_number0,
	_emblem_foreground_number1,
	_emblem_foreground_number2,
	_emblem_foreground_number3,
	_emblem_foreground_number4,
	_emblem_foreground_number5,
	_emblem_foreground_number6,
	_emblem_foreground_number7,
	_emblem_foreground_number8,
	_emblem_foreground_number9,
	k_emblem_foreground_count
};

enum e_emblem_background
{
	_emblem_background_solid = 0,
	_emblem_background_vertical_split,
	_emblem_background_horizontal_split1,
	_emblem_background_horizontal_split2,
	_emblem_background_vertical_gradient,
	_emblem_background_horizontal_gradient,
	_emblem_background_triple_column,
	_emblem_background_triple_row,
	_emblem_background_quadrants1,
	_emblem_background_quadrants2,
	_emblem_background_diagonal_slice,
	_emblem_background_cleft,
	_emblem_background_x1,
	_emblem_background_x2,
	_emblem_background_dircle,
	_emblem_background_diamond,
	_emblem_background_cross,
	_emblem_background_square,
	_emblem_background_dual_half_circle,
	_emblem_background_triangle,
	_emblem_background_diagonal_quadrant,
	_emblem_background_three_quaters,
	_emblem_background_quarter,
	_emblem_background_four_rows1,
	_emblem_background_four_rows2,
	_emblem_background_split_circle,
	_emblem_background_one_third,
	_emblem_background_two_thirds,
	_emblem_background_upper_field,
	_emblem_background_top_and_bottom,
	_emblem_background_center_stripe,
	_emblem_background_left_and_right,
	k_emblem_background_count
};

enum e_emblem_flags
{
	_emblem_flag_hide_icon_secondary = 0,
	k_emblem_flag_count
};

struct s_emblem_info
{
	uint8 foreground_emblem;	// e_emblem_foreground
	uint8 background_emblem;	// e_emblem_background
	c_flags_no_init<e_emblem_flags, uint8, k_emblem_flag_count> emblem_flags;
};
ASSERT_STRUCT_SIZE(s_emblem_info, 3);
