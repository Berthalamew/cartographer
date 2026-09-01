#include "stdafx.h"
#include "saved_game_files.h"

#include "cache/physical_memory_map.h"
#include "text/unicode.h"

/* constants  */

const wchar_t* k_saved_game_file_type_strings[k_number_of_saved_game_file_types]
{
	L"profile",
	L"slayer",
	L"koth",
	L"race",
	L"oddball",
	L"juggernaut",
	L"headhunter",
	L"ctf",
	L"assault",
	L"territories",
};

/* typedef */

typedef void(__cdecl* t_saved_games_load_save_file_information_from_disk)(c_static_array<s_saved_game_main_menu_globals_save_file_info, k_maximum_enumerated_saved_game_files_any_type_per_memory_unit>* save_files_storage);

/* prototypes */

static void saved_game_main_menu_globals_set(s_saved_game_main_menu_globals* new_globals);
static uint32 __cdecl saved_games_loading_allocate_storage(int32 a1, s_saved_game_file_loading_information* loading_information);
static void saved_games_load_save_file_information_from_disk(c_static_array<s_saved_game_main_menu_globals_save_file_info, k_maximum_enumerated_saved_game_files_any_type_per_memory_unit>* save_files_storage);
static void saved_game_main_menu_globals_initialize(void);
static void saved_game_files_memory_initialize(int32 unk);

/* globals */

t_saved_games_load_save_file_information_from_disk p_saved_games_load_save_file_information_from_disk;

/* public code */

void saved_game_files_apply_hooks(void)
{
	//WritePointer(Memory::GetAddress(0x39BD90), saved_game_files_memory_initialize);
	//DETOUR_ATTACH(p_saved_games_load_save_file_information_from_disk, Memory::GetAddress<t_saved_games_load_save_file_information_from_disk>(0x46596), saved_games_load_save_file_information_from_disk);
	return;
}

s_saved_game_main_menu_globals* saved_game_main_menu_globals_get()
{
	return *Memory::GetAddress<s_saved_game_main_menu_globals**>(0x482300);
}

s_saved_game_files_globals* saved_game_files_globals_get()
{
	return Memory::GetAddress<s_saved_game_files_globals*>(0x482424);
}

void __cdecl saved_game_files_update(void)
{
	INVOKE(0x46834, 0x0, saved_game_files_update);
	return;
}

bool saved_games_get_file_info(s_saved_game_main_menu_globals_save_file_info* out_info, uint32 enumerated_index)
{
	s_saved_game_main_menu_globals* saved_game_main_menu_globals = saved_game_main_menu_globals_get();
	s_saved_game_files_globals* saved_game_files_globals = saved_game_files_globals_get();

	if (saved_game_main_menu_globals)
	{
		// file is not a default save
		if (!ENUMERATED_INDEX_IS_DEFAULT_SAVE(enumerated_index))
		{
			auto abs_index = (enumerated_index >> 8) & 0x1FFF;
			auto last_index = saved_game_main_menu_globals->save_files.get_count() - 1;
			if ((abs_index <= last_index || abs_index == last_index))
			{
				csmemcpy(out_info, &saved_game_main_menu_globals->save_files[abs_index], saved_game_main_menu_globals->save_files.get_type_size());
				return true;
			}
		}
	}
	else if (saved_game_files_globals->cache_files_exist)
	{
		for (uint32 i = 0; i < saved_game_files_globals->cached_save_files.get_count(); ++i)
		{
			if (enumerated_index == saved_game_files_globals->cached_save_files[i].enumerated_index)
			{
				csmemcpy(out_info, &saved_game_files_globals->cached_save_files[i].file_info, sizeof(s_saved_game_main_menu_globals_save_file_info));
				return true;
			}
		}
	}

	return false;
}

const wchar_t* saved_games_get_file_type_as_string(e_saved_game_file_type file_type)
{
	return VALID_INDEX(file_type, k_number_of_saved_game_file_types) ? k_saved_game_file_type_strings[file_type] : L"unknown";
}

bool saved_games_append_file_type_to_path(const wchar_t* in_path, e_saved_game_file_type file_type, wchar_t* out_path)
{
	ustrncpy(out_path, in_path, 256);
	wchar_t* cat_path = ustrncat(out_path, saved_games_get_file_type_as_string(file_type), 256);
	cat_path[255] = '\0';
	ustrncpy(out_path, cat_path, 256);
	return true;
}

void saved_games_get_display_name(
	uint32 enumerated_index,
	wchar_t* display_name)
{
	ASSERT(display_name);

	display_name[0] = '\0';
	s_saved_game_main_menu_globals* saved_game_main_menu_globals = saved_game_main_menu_globals_get();

	if (saved_game_main_menu_globals && ENUMERATED_INDEX_IS_DEFAULT_SAVE(enumerated_index))
	{
		uint32 absolute_index = (enumerated_index >> 8) & 0x1FFF;
		uint32 last_index = saved_game_main_menu_globals->default_save_files.get_count() - 1;
		if (absolute_index <= last_index || absolute_index == last_index)
		{
			s_saved_game_main_menu_globals_default_save_file* default_save = &saved_game_main_menu_globals->default_save_files[absolute_index];
			if ((enumerated_index & 0xF) != 0)
			{
				if ((enumerated_index & 0xF) <= 9)
				{
					csmemcpy(display_name, &default_save->buffer[0xC], 128);
				}
			}
			else
			{
				csmemcpy(display_name, &default_save->buffer[0x8], 128);
			}
		}
	}
	else
	{
		s_saved_game_main_menu_globals_save_file_info file_info = {};
		if (saved_games_get_file_info(&file_info, enumerated_index))
		{
			ustrncpy(display_name, file_info.display_name, NUMBEROF(file_info.display_name));
		}
	}
	return;
}


/* private code */

static void saved_game_main_menu_globals_set(s_saved_game_main_menu_globals* new_globals)
{
	*Memory::GetAddress<s_saved_game_main_menu_globals**>(0x482300) = new_globals;
}

static uint32 __cdecl saved_games_loading_allocate_storage(int32 a1, s_saved_game_file_loading_information* loading_information)
{
	return INVOKE(0x9CC67, 0, saved_games_loading_allocate_storage, a1, loading_information);
}


static void saved_games_load_save_file_information_from_disk(c_static_array<s_saved_game_main_menu_globals_save_file_info, k_maximum_enumerated_saved_game_files_any_type_per_memory_unit>* save_files_storage)
{
	return p_saved_games_load_save_file_information_from_disk(save_files_storage);
}

static void saved_game_main_menu_globals_initialize(void)
{
	// TODO: properly implement this

	s_saved_game_files_globals* saved_game_files_globals = saved_game_files_globals_get();

	s_saved_game_main_menu_globals* saved_game_main_menu_globals = (s_saved_game_main_menu_globals*)c_physical_memory::allocate(sizeof(s_saved_game_main_menu_globals));

	ASSERT(saved_game_main_menu_globals);

	if (saved_game_main_menu_globals)
	{
		//saved_game_main_menu_globals->default_save_files.clear();
		//saved_game_main_menu_globals->save_files.clear();
	}
	else
	{
		saved_game_main_menu_globals = nullptr;
	}

	saved_game_main_menu_globals->saved_game_file_index_salt = saved_game_files_globals->unk_6;

	saved_game_files_globals->unk_6 = (saved_game_files_globals->unk_6 + 1) % 512;
	saved_game_files_globals->unk_3 = 3;

	saved_game_main_menu_globals_set(saved_game_main_menu_globals);
	return;
}

static void saved_game_files_memory_initialize(int32 unk)
{
	s_saved_game_files_globals* saved_game_files_globals = saved_game_files_globals_get();

	s_saved_game_main_menu_globals* saved_game_main_menu_globals = saved_game_main_menu_globals_get();
	ASSERT(saved_game_main_menu_globals);

	ASSERT(saved_game_files_globals->memory_initialized_for_game);
	
	if (unk == 1)
	{
		saved_game_main_menu_globals_initialize();
	}

	saved_game_files_globals->memory_initialized_for_game = true;
	return;
}
