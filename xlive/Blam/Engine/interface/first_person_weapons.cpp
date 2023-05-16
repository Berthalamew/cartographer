#include "stdafx.h"
#include "first_person_weapons.h"

#include "Blam/Engine/game/cheats.h"
#include "Util/Hooks/Hook.h"

bool b_showFirstPerson = true;

typedef bool(__cdecl* render_first_person_check_t)(e_skull_type skull_type);

bool __cdecl render_first_person_check(e_skull_type skull_type)
{
	return ice_cream_flavor_available(skull_type) || !b_showFirstPerson;
}

void toggle_first_person(bool state)
{
	b_showFirstPerson = state;
}

void first_person_weapons_apply_patches()
{
	if (Memory::IsDedicatedServer()) { return; }

	PatchCall(Memory::GetAddress<render_first_person_check_t>(0x228579), render_first_person_check);
}