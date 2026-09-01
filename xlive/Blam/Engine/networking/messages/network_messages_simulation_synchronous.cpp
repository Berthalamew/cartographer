#include "stdafx.h"
#include "network_messages_simulation_synchronous.h"

#include "network_message_type_collection.h"

#include "game/game.h"
#include "main/main_game.h"
#include "memory/bitstream.h"
#include "networking/network_event.h"
#include "simulation/simulation.h"
#include "simulation/simulation_encoding.h"

/* prototypes */

CLASS_HOOK_DECLARE_LABEL(c_network_message_synchronous_update__encode, c_network_message_synchronous_update::encode);
static void __declspec(naked) jmp_c_network_message_synchronous_update__encode()
{
	CLASS_HOOK_JMP(c_network_message_synchronous_update__encode, c_network_message_synchronous_update::encode);
}

CLASS_HOOK_DECLARE_LABEL(c_network_message_synchronous_update__decode, c_network_message_synchronous_update::decode);
static void __declspec(naked) jmp_c_network_message_synchronous_update__decode()
{
	CLASS_HOOK_JMP(c_network_message_synchronous_update__decode, c_network_message_synchronous_update::decode);
}

/* public code */

void network_messages_simulation_synchronous_apply_patches(
	void)
{
	WriteValue<int32>(Memory::GetAddress(0x1ED3AB + 1), sizeof(struct simulation_update));
	WriteValue<int32>(Memory::GetAddress(0x1ED3B0 + 1), sizeof(struct simulation_update));
	
	WritePointer(Memory::GetAddress(0x1ED3A6+1), jmp_c_network_message_synchronous_update__encode);
	WritePointer(Memory::GetAddress(0x1ED3A1+1), jmp_c_network_message_synchronous_update__decode);

	return;
}

void c_network_message_synchronous_update::encode(
	c_bitstream* packet,
	int32 message_storage_size,
	void const* message_storage)
{
	s_network_message_synchronous_update const* message = (s_network_message_synchronous_update const*)message_storage;

	ASSERT(message_storage_size==sizeof(struct s_network_message_synchronous_update));

	simulation_update_encode(packet, &message->update);

	return;
}

bool c_network_message_synchronous_update::decode(
	c_bitstream* packet,
	int32 message_storage_size,
	void* message_storage)
{
	bool simulation_decoded;

	bool message_success = false;
	s_network_message_synchronous_update* message = (s_network_message_synchronous_update*)message_storage;

	if (game_in_progress() || main_game_reset_in_progress())
	{
		if (game_is_playback() && game_is_multiplayer() && game_get_active_structure_bsp_index()!=NONE)
		{
			event(_event_warning, "networking:messaging:sync-update: game is mp playback, and we don't have a bsp loaded, can't decode update");
		}
		else
		{
			ASSERT(message_storage_size==sizeof(struct s_network_message_synchronous_update));
			
			simulation_decoded = simulation_update_decode(packet, &message->update);
			message_success = !packet->error_occurred() && simulation_decoded;
			if (simulation_decoded && !message_success)
			{
				simulation_destroy_update(&message->update);
			}
		}
	}
	else
	{
		event(_event_warning, "networking:messaging:sync-update: game not in progress, can't decode update");
	}

	simulation_destroy_update(&message->update);

	return message_success;
}

void __cdecl network_message_types_register_simulation_synchronous(
	c_network_message_type_collection* message_collection)
{
	INVOKE(0x1ED397, 0x1CDD50, network_message_types_register_simulation_synchronous, message_collection);
	return;
}
