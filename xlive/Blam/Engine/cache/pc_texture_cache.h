#pragma once

enum
{
	k_max_pc_texture_cache_count = 10000
};

/* structures */

// todo: unfinished very large struct
struct s_pc_texture_cache_globals
{
	struct data_array* predicted_textures;
};

/* prototypes */

void __cdecl texture_cache_new(void);

struct IDirect3DTexture9* __cdecl texture_cache_bitmap_get_hardware_format_add_to_texture_cache(struct bitmap_data* data, real32 a2, int32 a3);

struct IDirect3DTexture9* __cdecl pc_texture_cache_preload_bitmap(struct bitmap_data* bitmap_data, uint8 flags, real32 unk, bool* unused);

void __cdecl texture_cache_block_for_one_frame(int32 a1);

bool __cdecl texture_cache_open_pregame(void);
