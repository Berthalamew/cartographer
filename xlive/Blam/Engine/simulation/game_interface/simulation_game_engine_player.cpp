#include "stdafx.h"
#include "simulation_game_engine_player.h"

#include "game/game.h"
#include "game/game_engine_simulation.h"
#include "memory/bitstream.h"
#include "networking/network_event.h"
#include "simulation/simulation_gamestate_entities.h"

/* prototypes */

CLASS_HOOK_DECLARE_LABEL(c_simulation_game_engine_player_entity_definition__promote_game_entity_to_authority, c_simulation_game_engine_player_entity_definition::promote_game_entity_to_authority);
static void __declspec(naked) jmp_c_simulation_game_engine_player_entity_definition__promote_game_entity_to_authority(void)
{
	CLASS_HOOK_JMP(c_simulation_game_engine_player_entity_definition__promote_game_entity_to_authority, c_simulation_game_engine_player_entity_definition::promote_game_entity_to_authority);
}

CLASS_HOOK_DECLARE_LABEL(c_simulation_game_engine_player_entity_definition__delete_game_entity, c_simulation_game_engine_player_entity_definition::delete_game_entity);
static void __declspec(naked) jmp_c_simulation_game_engine_player_entity_definition__delete_game_entity(void)
{
	CLASS_HOOK_JMP(c_simulation_game_engine_player_entity_definition__delete_game_entity, c_simulation_game_engine_player_entity_definition::delete_game_entity);
}

CLASS_HOOK_DECLARE_LABEL(c_simulation_game_engine_player_entity_definition__update_game_entity, c_simulation_game_engine_player_entity_definition::update_game_entity);
static void __declspec(naked) jmp_c_simulation_game_engine_player_entity_definition__update_game_entity(void)
{
	CLASS_HOOK_JMP(c_simulation_game_engine_player_entity_definition__update_game_entity, c_simulation_game_engine_player_entity_definition::update_game_entity);
}

CLASS_HOOK_DECLARE_LABEL(c_simulation_game_engine_player_entity_definition__build_creation_data, c_simulation_game_engine_player_entity_definition::build_creation_data);
static void __declspec(naked) jmp_c_simulation_game_engine_player_entity_definition__build_creation_data(void)
{
	CLASS_HOOK_JMP(c_simulation_game_engine_player_entity_definition__build_creation_data, c_simulation_game_engine_player_entity_definition::build_creation_data);
}

CLASS_HOOK_DECLARE_LABEL(c_simulation_game_engine_player_entity_definition__entity_creation_encode, c_simulation_game_engine_player_entity_definition::entity_creation_encode);
static void __declspec(naked) jmp_c_simulation_game_engine_player_entity_definition__entity_creation_encode(void)
{
	CLASS_HOOK_JMP(c_simulation_game_engine_player_entity_definition__entity_creation_encode, c_simulation_game_engine_player_entity_definition::entity_creation_encode);
}

CLASS_HOOK_DECLARE_LABEL(c_simulation_game_engine_player_entity_definition__entity_creation_decode, c_simulation_game_engine_player_entity_definition::entity_creation_decode);
static void __declspec(naked) jmp_c_simulation_game_engine_player_entity_definition__entity_creation_decode(void)
{
	CLASS_HOOK_JMP(c_simulation_game_engine_player_entity_definition__entity_creation_decode, c_simulation_game_engine_player_entity_definition::entity_creation_decode);
}

CLASS_HOOK_DECLARE_LABEL(c_simulation_game_engine_player_entity_definition__create_game_entity, c_simulation_game_engine_player_entity_definition::create_game_entity);
static void __declspec(naked) jmp_c_simulation_game_engine_player_entity_definition__create_game_entity(void)
{
	CLASS_HOOK_JMP(c_simulation_game_engine_player_entity_definition__create_game_entity, c_simulation_game_engine_player_entity_definition::create_game_entity);
}

/* public code */

void simulation_game_engine_player_apply_patches(
	void)
{
	WritePointer(Memory::GetAddress(0x3CA8B4, 0x0), jmp_c_simulation_game_engine_player_entity_definition__update_game_entity);
	WritePointer(Memory::GetAddress(0x3CA8B8, 0x0), jmp_c_simulation_game_engine_player_entity_definition__delete_game_entity);
	WritePointer(Memory::GetAddress(0x3CA8BC, 0x0), jmp_c_simulation_game_engine_player_entity_definition__promote_game_entity_to_authority);
	WritePointer(Memory::GetAddress(0x3CA8A4, 0x0), jmp_c_simulation_game_engine_player_entity_definition__build_creation_data);
	WritePointer(Memory::GetAddress(0x3CA8B4, 0x0), jmp_c_simulation_game_engine_player_entity_definition__create_game_entity);

	WritePointer(Memory::GetAddress(0x3CA88C, 0x0), jmp_c_simulation_game_engine_player_entity_definition__entity_creation_encode);
	WritePointer(Memory::GetAddress(0x3CA890, 0x0), jmp_c_simulation_game_engine_player_entity_definition__entity_creation_decode);

	return;
}

bool c_simulation_game_engine_player_entity_definition::promote_game_entity_to_authority(
	int32 gamestate_index)
{
	int32 absolute_player_index;
	bool promoted = false;
	
	ASSERT(gamestate_index != NONE);
	
	absolute_player_index = game_engine_get_player_index_by_gamestate_index(gamestate_index);

	if (absolute_player_index==NONE)
	{
		event(_event_error, "networking:simulation:game_engine_players: failed to promote player with bad player index gamestate 0x%08X", gamestate_index);
	}
	// TODO: finish this
	else if (/*gameworld_attachment_valid(gamestate_index)*/ true)
	{
		promoted = true;
	}
	else
	{
		event(
			_event_warning,
			"networking:simulation:game_engine_players: failed to promote player 0x%08X due to attachment problems 0x%8X",
			absolute_player_index,
			gamestate_index
		);
	}

	return promoted;
}

void c_simulation_game_engine_player_entity_definition::build_creation_data(
	int32 gamestate_index,
	int32 creation_data_size, 
	void* creation_data)
{
	s_simulation_game_engine_player_creation_data* player_creation_data = (s_simulation_game_engine_player_creation_data*)creation_data;

	ASSERT(creation_data_size==sizeof(s_simulation_game_engine_player_creation_data));
	ASSERT(gamestate_index != NONE);

	player_creation_data->absolute_player_index = (int16)game_engine_get_player_index_by_gamestate_index(gamestate_index);

	if (player_creation_data->absolute_player_index==NONE)
	{
		event(_event_warning, "networking:simulation:entities:game_engine_players: failed to get player for gamestate 0x%08X", gamestate_index);
	}

	return;
}

void c_simulation_game_engine_player_entity_definition::entity_creation_encode(
	int32 creation_data_size,
	void const* creation_data,
	struct s_simulation_view_telemetry_data const* telemetry_data,
	class c_bitstream* packet,
	bool encode_for_network)
{
	s_simulation_game_engine_player_creation_data const* player_creation_data = (s_simulation_game_engine_player_creation_data const*)creation_data;

	ASSERT(creation_data_size==sizeof(struct s_simulation_game_engine_player_creation_data));
	ASSERT(creation_data);
	ASSERT(packet);

	packet->push_structure("player-create", NONE, 0);
	
	packet->write_integer("absolute-player-index", player_creation_data->absolute_player_index, k_player_index_bits+1);

	packet->pop_structure("player-create", NONE);

	return;
}

bool c_simulation_game_engine_player_entity_definition::entity_creation_decode(
	int32 creation_data_size,
	void* creation_data,
	class c_bitstream* packet,
	bool decode_for_network)
{
	bool success;
	s_simulation_game_engine_player_creation_data* player_creation_data = (s_simulation_game_engine_player_creation_data*)creation_data;

	ASSERT(creation_data_size==sizeof(struct s_simulation_game_engine_player_creation_data));
	ASSERT(creation_data);
	ASSERT(packet);

	packet->push_structure("player-create", NONE, 0);

	player_creation_data->absolute_player_index = (int16)packet->read_integer("absolute-player-index", k_player_index_bits+1);

	packet->pop_structure("player-create", NONE);

	success = VALID_INDEX(player_creation_data->absolute_player_index, k_maximum_players);

	return success;
}

bool c_simulation_game_engine_player_entity_definition::create_game_entity(
	int32 gamestate_index,
	int32 creation_data_size, 
	void const* creation_data,
	uint32 initial_update_mask,
	int32 initial_state_data_size,
	void const* initial_state_data)
{
	s_simulation_game_engine_player_creation_data const* player_creation_data = (s_simulation_game_engine_player_creation_data const*)initial_state_data;
	int16 absolute_player_index = player_creation_data->absolute_player_index;

	ASSERT(gamestate_index != NONE);
	ASSERT(simulation_gamestate_entity_get_object_index(gamestate_index) == NONE);
	ASSERT(initial_update_mask==0);

	game_engine_globals_set_player_gamestate_index(absolute_player_index, gamestate_index);

	return true;
}


bool c_simulation_game_engine_player_entity_definition::delete_game_entity(
	int32 gamestate_index)
{
	int32 absolute_player_index;
	
	bool deleted = false;

	ASSERT(gamestate_index != NONE);

	absolute_player_index = game_engine_get_player_index_by_gamestate_index(gamestate_index);

	if (absolute_player_index==NONE)
	{
		event(_event_warning, "networking:simulation:game-engine: player gamestate 0x%08X tried to delete but wasn't found", gamestate_index);
	}
	// TODO: finish this
	else if (/*gameworld_attachment_valid(gamestate_index)*/ true)
	{
		game_engine_globals_set_player_gamestate_index((int16)absolute_player_index, NONE);
		deleted = true;
	}
	else
	{
		event(
			_event_warning,
			"networking:simulation:game-engine: deleting player %d which is not attached properly to gamestate 0x%8X",
			absolute_player_index,
			gamestate_index
		);
	}

	return deleted;
}

bool c_simulation_game_engine_player_entity_definition::update_game_entity(
	int32 gamestate_index,
	uint32 update_mask,
	int32 update_state_data_size,
	void const* update_state_data)
{
	int32 absolute_player_index;

	bool updated_entity = false;

	ASSERT(update_mask!=0);
	ASSERT(update_state_data_size==state_data_size());
	ASSERT(update_state_data);
	ASSERT(gamestate_index != NONE);

	absolute_player_index = game_engine_get_player_index_by_gamestate_index(gamestate_index);

	if (absolute_player_index==NONE)
	{
		event(_event_error, "networking:simulation:game-engine: player not found for entity index [%d]", absolute_player_index);
	}
	// TODO: finish this
	else if (/*gameworld_attachment_valid(gamestate_index)*/ true)
	{
		if (game_engine_globals_apply_player_update((int16)absolute_player_index, update_mask, update_state_data_size, update_state_data))
		{
			updated_entity = true;
		}
		else
		{
			event(_event_warning, "networking:simulation:game-engine: unable to apply player state update to invalid player [#%d]", absolute_player_index);
		}
	}
	else
	{
		event(_event_warning, "networking:simulation:game-engine: player is not attached correctly for update [#%d] (gamestate 0x%8X)", absolute_player_index, gamestate_index);
	}


	return updated_entity;
}

/* private code */

int32 c_simulation_game_engine_player_entity_definition::game_engine_get_player_index_by_gamestate_index(
	int32 gamestate_index)
{
	int32 absolute_player_index = NONE;

	ASSERT(gamestate_index != NONE);

	for (int16 current_absolute_player_index = 0; current_absolute_player_index<k_maximum_multiplayer_players; ++current_absolute_player_index)
	{
		if (game_engine_globals_get_player_gamestate_index(current_absolute_player_index) == gamestate_index)
		{
			absolute_player_index = current_absolute_player_index;
			break;
		}
	}

	return absolute_player_index;
}

