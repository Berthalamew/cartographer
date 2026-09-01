#include "stdafx.h"
#include "breakable_surfaces.h"

#include "game/game.h"
#include "networking/network_event.h"
#include "simulation/game_interface/simulation_game_action.h"

/* prototypes */


static void __cdecl breakable_surfaces_initialize_for_new_structure_bsp_internal(void);

/* public code */

void breakable_surfaces_apply_patches(
	void)
{
	WritePointer((uintptr_t)&get_game_systems()[35].initialize_for_new_structure_bsp_proc, breakable_surfaces_initialize_for_new_structure_bsp);

	return;
}

int32* breakable_surface_gamestate_indices_get(
	void)
{
	return Memory::GetAddress<int32*>(0x4D0A78, 0x0);
}


void __cdecl breakable_surfaces_initialize_for_new_structure_bsp(
	void)
{
	if (game_is_distributed())
	{
		breajable_surfaces_reinitialize();
	}

	return;
}

void breajable_surfaces_reinitialize(
	void)
{
	ASSERT(game_is_distributed());

	simulation_action_breakable_surfaces_delete();
	breakable_surfaces_initialize_for_new_structure_bsp_internal();

	return;
}

int32 breakable_surface_group_get_gamestate_index(
	int32 structure_bsp_index,
	int32 group_index)
{
	int32 gamestate_index= NONE;

	if (VALID_INDEX(group_index, k_maximum_distributed_networking_breakable_surface_groups))
	{
		gamestate_index = breakable_surface_gamestate_indices_get()[group_index];
	}
	else
	{
		event(_event_warning, "networking:simulation:event: attempting to get an gamestate index with a bad group index 0x%08X", group_index);
	}

	return gamestate_index;

}

void breakable_surface_group_set_gamestate_index(
	int32 structure_bsp_index,
	int32 group_index,
	int32 gamestate_index)
{
	if (VALID_INDEX(group_index, k_maximum_distributed_networking_breakable_surface_groups))
	{
		breakable_surface_gamestate_indices_get()[group_index] = gamestate_index;
	}
	else
	{
		event(_event_warning, "networking:simulation:event: attempting to get an gamestate index with a bad group index 0x%08X", group_index);
	}

	return;
}

/* private code */

static void __cdecl breakable_surfaces_initialize_for_new_structure_bsp_internal(
	void)
{
	INVOKE(0xB1DCF, 0x0, breakable_surfaces_initialize_for_new_structure_bsp_internal);

	return;
}
