#pragma once


enum e_sound_definition_compression : uint8
{
	_sound_definition_compression_none_big_endian = 0,
	_sound_definition_compression_xbox_adpcm = 1,
	_sound_definition_compression_ima_adpcm = 2,
	_sound_definition_compression_none_little_endian = 3,
	_sound_definition_compression_wma = 4
};

enum e_sound_definition_encoding : uint8
{
	_sound_definition_encoding_mono = 0,
	_sound_definition_encoding_stereo = 1,
	_sound_definition_encoding_codec = 2
};

enum e_sound_definition_sample_rate : uint8
{
	_sound_definition_sample_rate_22khz = 0,
	_sound_definition_sample_rate_44khz = 1,
	_sound_definition_sample_rate_32khz = 2
};

enum e_sound_definition_flags : uint16
{
	_sound_definition_flag_fit_to_adpcm_block_size = 0,
	_sound_definition_flag_split_long_sound_into_permutations = 1,
	_sound_definition_flag_always_spatialize_always_play_as_3d_sound_even_in_first_person = 2,
	_sound_definition_flag_never_obstruct_disable_occlusion_obstruction = 3,
	_sound_definition_flag_internal_dont_touch = 4,
	_sound_definition_flag_use_huge_sound_transmission = 5,
	_sound_definition_flag_link_count_to_owner_unit = 6,
	_sound_definition_flag_pitch_range_is_language = 7,
	_sound_definition_flag_dont_use_sound_class_speaker_flag = 8,
	_sound_definition_flag_dont_use_lipsync_data = 9,
	k_sound_definition_flag_count
};
struct sound_definition_v1
{
	c_flags_no_init<e_sound_definition_flags, uint16, k_sound_definition_flag_count> flags;

	int8/*e_sound_class_type*/ sound_class;
	e_sound_definition_sample_rate sample_rate;
	e_sound_definition_encoding encoding;
	e_sound_definition_compression compression;

	int16 gestalt_playback_parameter_index;
	int16 gestalt_pitch_range_index;
	int8 gestalt_unknown_index;
	int8 gestalt_scale_index;
	int8 gestalt_promotion_index;
	int8 gestalt_custom_playback_index;
	int16 gestalt_extra_info_index;
	uint32 maximum_play_time;
};
ASSERT_STRUCT_SIZE(sound_definition_v1, 20);

// max count: 1
struct s_sound_globals_definition
{
	tag_reference sound_classes;                    // sncl
	tag_reference sound_effects;                    // sfx+
	tag_reference sound_mix;                        // snmx
	tag_reference sound_combat_dialogue_constants;  // spk!
	datum sound_gesalt;                             // ugh!
};
ASSERT_STRUCT_SIZE(s_sound_globals_definition, 36);
