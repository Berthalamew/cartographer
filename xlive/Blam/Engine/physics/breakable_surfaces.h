#pragma once

/* constants */

enum
{
	k_maximum_distributed_networking_breakable_surface_groups= 8,
};

/* prototypes */

void breakable_surfaces_apply_patches(void);

int32* breakable_surface_gamestate_indices_get(void);

void __cdecl breakable_surfaces_initialize_for_new_structure_bsp(void);
void breajable_surfaces_reinitialize(void);

int32 breakable_surface_group_get_gamestate_index(
	int32 structure_bsp_index,
	int32 group_index);


void breakable_surface_group_set_gamestate_index(
	int32 structure_bsp_index,
	int32 group_index,
	int32 gamestate_index);
