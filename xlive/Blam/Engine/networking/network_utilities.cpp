#include "stdafx.h"
#include "network_utilities.h"

#include "shell/shell.h"

/* constants */


/* typedefs */
typedef void (__cdecl* t_network_utilities_get_game_version)(int32*, int32*, int32*);
typedef bool(__stdcall* t_locator_parse_search_result)(void*, int a2, s_network_locator_result* session_out);

/* prototypes */

void network_utilities_apply_patches();

void __cdecl network_utilities_get_game_version(int32* executable_type, int32* executable_version, int32* compatible_version);

static bool __cdecl check_executable_type_hook(int32 executable_type);
static bool __cdecl check_joining_session_version_hook(int32 executable_type, int32 executable_version, int32 compatible_version);
static bool __stdcall network_locator_parse_search_result(void* thisx, int a2, s_network_locator_result* session_out);

/* globals */
t_network_utilities_get_game_version p_network_utlilities_get_game_version;
t_locator_parse_search_result p_xlocator_parse_search_result;

/* public code */

void network_utilities_apply_patches()
{
	DETOUR_ATTACH(p_network_utlilities_get_game_version, Memory::GetAddress<t_network_utilities_get_game_version>(0x1B4BF5, 0x1B0043), network_utilities_get_game_version);

	if (!shell_is_dedicated_server())
	{
		DetourFunc(Memory::GetAddress<BYTE*>(0x1B4C14), (BYTE*)check_joining_session_version_hook, 5);
		DetourFunc(Memory::GetAddress<BYTE*>(0x1B4C32), (BYTE*)check_executable_type_hook, 8);
		p_xlocator_parse_search_result = (t_locator_parse_search_result)DetourClassFunc(Memory::GetAddress<BYTE*>(0x1DA8ED), (BYTE*)network_locator_parse_search_result, 8);
	}
}

void __cdecl network_utilities_get_game_version(int32* executable_type, int32* executable_version, int32* compatible_version)
{
	ASSERT(executable_type != NULL);
	ASSERT(executable_version != NULL);
	ASSERT(compatible_version != NULL);

	*executable_type = EXECUTABLE_TYPE;
	*executable_version = EXECUTABLE_VERSION;
	*compatible_version = COMPATIBLE_VERSION;
	return;
}

/* private code */

bool __cdecl check_executable_type_hook(int32 executable_type)
{
	// will not display servers that don't match this in server list
	int32 current_executable_type, current_executable_version, current_compatible_version;
	network_utilities_get_game_version(&current_executable_type, &current_executable_version, &current_compatible_version);
	return executable_type == current_executable_type;
}

bool __cdecl check_joining_session_version_hook(int32 executable_type, int32 executable_version, int32 compatible_version)
{
	int32 current_executable_type, current_executable_version, current_compatible_version;
	network_utilities_get_game_version(&current_executable_type, &current_executable_version, &current_compatible_version);
	return executable_type == current_executable_type && executable_version >= current_executable_version && compatible_version <= current_compatible_version;
}

// ### TODO move out
bool __stdcall network_locator_parse_search_result(void* thisx, int a2, s_network_locator_result* session_out)
{
	bool valid = p_xlocator_parse_search_result(thisx, a2, session_out)
		&& check_joining_session_version_hook(session_out->executable_type, session_out->executable_version, session_out->compatible_version);
	return valid;
}