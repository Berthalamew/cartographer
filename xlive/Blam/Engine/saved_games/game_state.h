#pragma once
#include "game/game_options.h"
#include "memory/rockall_heap_manager.h"

/* structures */

enum
{
	k_game_state_count = 2,

	k_game_state_cpu_size = 0x3BE000,

	// xbox specific when memory was unified?
	k_game_state_gpu_size = 0x40000,

	k_game_state_size = k_game_state_cpu_size + k_game_state_gpu_size,
};

/* structures */

struct game_state_header
{
	uint32 alloc_checksum;
	void* base_address;
	char scenario_name[256];
	char game_build[k_tag_string_length];
	uint32 map_checksum;
	int32 field_12C;
	s_game_options options;
	int32 active_bsp_index;
	int32 field_12BC;
	int32 field_12C0;
	int32 field_12C4;
	int32 field_12C8;
	int32 field_12CC;
	int32 field_12D0;
	int32 field_12D4;
	int32 field_12D8;
	int32 field_12DC;
	int32 field_12E0;
	int32 field_12E4;
	int32 field_12E8;
	int32 field_12EC;
	int32 field_12F0;
	int32 field_12F4;
};
ASSERT_STRUCT_SIZE(game_state_header, 4856);

struct s_game_state_globals
{
	bool initialized;
	bool locked;

	bool saved_game_valid;
	bool fallback_valid;
	int16 current_saved_game_index;
	game_state_header game_saves[k_game_state_count];
	uint32 game_time_at_last_save;
	int16 number_of_bad_reverts;

	void* base_address;
	uint32 cpu_allocation_size;
	uint32 gpu_allocation_size;
	uint32 allocation_size_checksum;

	uint32 revert_time;
	struct game_state_header* header;

	bool loaded_from_persistent_storage;
	class c_gamestate_allocation* gamestate_allocation;
};
ASSERT_STRUCT_SIZE(s_game_state_globals, 9760);

/* classes */

class c_gamestate_allocation : public c_allocation_base
{
public:
	virtual void* alloc(size_t bytes);
	virtual void* free_block(void* block);
};

/* prototypes */

void __cdecl game_state_initialize(void);

void* __cdecl game_state_malloc(const char* name, const char* description, uint32 size);

s_game_state_globals* game_state_globals_get();

void game_state_save_core(const char* name);

bool game_state_read_core(const char* name, void* buffer, uint32 buffer_size);
bool game_state_write_core(const char* name, void* buffer, uint32 buffer_size);

bool game_state_get_game_options_from_core(const char* core_name, s_game_options* options);