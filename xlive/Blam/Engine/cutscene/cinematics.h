#pragma once
#include "Blam/Engine/tag_files/string_id.h"

struct s_cinematic_globals_flags
{
	bool show_letterbox;
	bool cinematic_in_progress;
	bool cinematic_skip_start;
	bool suppress_bsp_object_creation;
};
CHECK_STRUCT_SIZE(s_cinematic_globals_flags, 4);

struct s_cutscene_title
{
	int16 cutscene_title;
	short cutscene_title_time;
};
CHECK_STRUCT_SIZE(s_cutscene_title, 4);

struct s_cinematic_globals_cutscene_flags
{
	bool unk1;
	bool unk2;
	bool is_outro_cutscene;
	bool unk4;
};
CHECK_STRUCT_SIZE(s_cinematic_globals_cutscene_flags, 4);

struct s_cinematic_globals
{
	real32 letterbox_scale;
	s_cinematic_globals_flags flags;
	s_cutscene_title cutscene_title[4];
	string_id cinematic_title_string_id;
	real32 cinematic_title_time;
	s_cinematic_globals_cutscene_flags cutscene_flags;
};
CHECK_STRUCT_SIZE(s_cinematic_globals, 36);

s_cinematic_globals* get_cinematic_globals();

bool cinematic_is_running();

void cinematics_apply_patches();