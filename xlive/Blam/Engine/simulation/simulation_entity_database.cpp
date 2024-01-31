#include "stdafx.h"
#include "simulation_entity_database.h"

#include "simulation_queue_entities.h"

#include "Util/Hooks/Hook.h"

c_simulation_entity_database* simulation_get_entity_database()
{
    return (c_simulation_entity_database*)((uint8*)simulation_get_world()->m_distributed_world + 8352);
}

bool c_simulation_entity_database::process_creation(int32 entity_index, e_simulation_entity_type type, uint32 update_mask, int32 block_count, s_replication_allocation_block* blocks)
{
    bool result = false;
    c_simulation_entity_definition* entity_definition = m_type_collection->get_entity_definition(type);
    s_simulation_game_entity* game_entity = entity_get(entity_index);
    game_entity->entity_index = entity_index;
    game_entity->entity_type = type;
    game_entity->entity_update_flag = 0;
    game_entity->field_10 = 0;
    game_entity->event_reference_count = 0;
    game_entity->exists_in_gameworld = false;
    game_entity->object_index = DATUM_INDEX_NONE;

    // we could also validate here the type of the blocks
    game_entity->creation_data = blocks[_entity_creation_block_order_simulation_entity_creation].block_data;
    game_entity->creation_data_size = blocks[_entity_creation_block_order_simulation_entity_creation].block_size;
    game_entity->state_data = blocks[_entity_creation_block_order_simulation_entity_state].block_data;
    game_entity->state_data_size = blocks[_entity_creation_block_order_simulation_entity_state].block_size;

	s_simulation_queue_element* queue_element = *(s_simulation_queue_element**)blocks[_entity_creation_block_order_forward_memory_queue_element].block_data;

	// TODO when we implement this
	// datum gamestate_index = *(datum*)blocks[_entity_creation_block_order_gamestate_index].block_data;

    // free these blocks
	network_heap_free_block((uint8*)blocks[_entity_creation_block_order_forward_memory_queue_element].block_data);
	// network_heap_free_block((uint8*)blocks[_entity_creation_block_order_gamestate_index].block_data);

    csmemset(blocks, 0, sizeof(s_replication_allocation_block) * block_count);

	if (queue_element)
	{
		// insert the queue element to the buffer previously created in read_creation_from_packet()
		// and passed by a replication allocation block
        simulation_queue_entity_creation_insert(queue_element);
        result = true;
    }
    else
    {
        result = entity_definition->create_game_entity(
            game_entity,
            game_entity->creation_data_size,
            game_entity->creation_data,
            update_mask,
            game_entity->state_data_size,
            game_entity->state_data);
        game_entity->exists_in_gameworld = result;
    }

    return result;
}

__declspec(naked) void jmp_c_simulation_entity_database__process_creation() { __asm { jmp c_simulation_entity_database::process_creation } }


uint32 c_simulation_entity_database::read_creation_from_packet(int32 entity_index, e_simulation_entity_type* simulation_entity_type, uint32* out_entity_initial_update_mask, int32 block_max_count, int32* block_count, s_replication_allocation_block* blocks, c_bitstream* packet)
{
    uint32 result = 3;
    e_simulation_entity_type entity_type = (e_simulation_entity_type)packet->read_integer("entity-type", 5);
    c_simulation_entity_definition* entity_def = m_type_collection->get_entity_definition(entity_type);
    
    if (entity_def)
    {
        uint32 creation_data_size = entity_def->creation_data_size();
        uint32 state_data_size = entity_def->state_data_size();

        // Allocate creation data
        uint8* creation_data = NULL;
        if (creation_data_size > 0)
        {
            creation_data = network_heap_allocate_block(creation_data_size);
        }

        // Allocate state data
        uint8* state_data = network_heap_allocate_block(state_data_size);

        // Allocate gamestate data
        //int32* gamestate_index = (int32*)network_heap_allocate_block(sizeof(int32));

        // Allocate queue data
        uint8* queue_element = (uint8*)network_heap_allocate_block(sizeof(s_simulation_queue_element*));
        result = (!creation_data || !queue_element || !state_data /*|| !gamestate_index*/ ? 2 : result);

        // check if creation size is > 0 and if network heap block have been successfully allocated
        if ((!creation_data_size || creation_data != NULL) && (state_data && queue_element /* && gamestate_index*/))
        {
            if (creation_data_size > 0)
                csmemset(creation_data, 0, creation_data_size);

            *queue_element = NULL;

            if (entity_def->entity_creation_decode(creation_data_size, creation_data, packet))
            {
                if (entity_def->build_baseline_state_data(
                    creation_data_size,
                    creation_data,
                    state_data_size,
                    (s_simulation_baseline_state_data*)state_data))
                {
                    uint32 entity_initial_update_mask = 0;
                    uint32 entity_allowed_initial_update_mask = entity_def->initial_update_mask();

                    bool decode_success = true;
                    if (entity_allowed_initial_update_mask != 0
                        && packet->read_bool("initial-update-exists")
                        )
                    {
                        if (entity_def->entity_update_decode(true, &entity_initial_update_mask, state_data_size, state_data, packet))
                        {
                            // check if the state contains updates allowed only on creation
                            if (TEST_FLAG(~entity_allowed_initial_update_mask, entity_initial_update_mask))
                            {
                                decode_success = false;
                            }
                        }
                        else
                        {
                            decode_success = false;
                        }
                    }

                    if (decode_success)
                    {
                        /* Original code
                        *simulation_entity_type = entity_type;
                        *out_entity_initial_update_mask = entity_initial_update_mask;
                        blocks[*block_count].block_type = _network_memory_block_simulation_entity_creation;
                        blocks[*block_count].block_size = creation_data_size;
                        blocks[*block_count].block_data = creation_data;
                        blocks[*block_count + 1].block_type = _network_memory_block_simulation_entity_state;
                        blocks[*block_count + 1].block_size = state_data_size;
                        blocks[*block_count + 1].block_data = state_data;
                        *block_count += 2;
                        */

                        s_simulation_queue_entity_data sim_queue_entity_data;
                        sim_queue_entity_data.entity_index = entity_index;
                        sim_queue_entity_data.entity_type = entity_type;
                        sim_queue_entity_data.creation_data_size = creation_data_size;
                        sim_queue_entity_data.creation_data = creation_data;
                        sim_queue_entity_data.state_data_size = state_data_size;
                        sim_queue_entity_data.state_data = state_data;

                        if (!packet->read_only_for_consistency()
                            && !simulation_queue_entity_creation_allocate(&sim_queue_entity_data, entity_initial_update_mask, (s_simulation_queue_element**)queue_element, NULL))
                        {
                            decode_success = false;
                        }
                    }

                    if (decode_success)
                    {
                        *simulation_entity_type = entity_type;
                        *out_entity_initial_update_mask = entity_initial_update_mask;

                        //blocks[_entity_creation_block_order_gamestate_index].block_type = _network_memory_block_forward_gamestate_element;
                        //blocks[_entity_creation_block_order_gamestate_index].block_size = sizeof(gamestate_index);
                        //blocks[_entity_creation_block_order_gamestate_index].block_data = gamestate_index;

                        // copy the block, allow the process function to use this
                        blocks[*block_count + _entity_creation_block_order_simulation_entity_creation].block_type = _network_memory_block_simulation_entity_creation;
                        blocks[*block_count + _entity_creation_block_order_simulation_entity_creation].block_size = creation_data_size;
                        blocks[*block_count + _entity_creation_block_order_simulation_entity_creation].block_data = creation_data;

                        blocks[*block_count + _entity_creation_block_order_simulation_entity_state].block_type = _network_memory_block_simulation_entity_state;
                        blocks[*block_count + _entity_creation_block_order_simulation_entity_state].block_size = state_data_size;
                        blocks[*block_count + _entity_creation_block_order_simulation_entity_state].block_data = state_data;

                        blocks[*block_count + _entity_creation_block_order_forward_memory_queue_element].block_type = _network_memory_block_forward_simulation_queue_element;
                        blocks[*block_count + _entity_creation_block_order_forward_memory_queue_element].block_size = sizeof(s_simulation_queue_element*);
                        blocks[*block_count + _entity_creation_block_order_forward_memory_queue_element].block_data = queue_element;

                        *block_count += k_entity_creation_block_order_count;

                        result = 0;
                    }
                }
            }
        }

        if (result == 3
            || result == 2)
        {
            if (creation_data != NULL)
            {
                network_heap_free_block(creation_data);
            }
            if (state_data != NULL)
            {
                network_heap_free_block(state_data);
            }

            if (queue_element != NULL)
            {
                if (*queue_element != NULL)
                {
                    simulation_get_world()->simulation_queue_free(*(s_simulation_queue_element**)queue_element);
                }

                network_heap_free_block(queue_element);
            }

            /*if (gamestate_index != NULL)
            {
                network_heap_free_block((uint8*)gamestate_index);
            }*/
        }
    }

    return result;
}

__declspec(naked) void jmp_c_simulation_entity_database__read_creation_from_packet() { __asm { jmp c_simulation_entity_database::read_creation_from_packet } }

bool c_simulation_entity_database::process_update(int32 entity_index, uint32 update_mask, int32 block_count, s_replication_allocation_block* blocks)
{
    bool result = false;
    s_simulation_game_entity* game_entity = entity_try_and_get(entity_index);
    s_simulation_queue_element* queue_element = *(s_simulation_queue_element**)blocks[_entity_update_block_order_forward_memory_queue_element].block_data;
    uint8* state_data = (uint8*)blocks[_entity_update_block_order_simulation_entity_state].block_data;

    if (game_entity)
    {
        c_simulation_entity_definition* entity_def = m_type_collection->get_entity_definition(game_entity->entity_type);
        csmemcpy(game_entity->state_data, state_data, game_entity->state_data_size);
        simulation_queue_entity_update_insert(queue_element);
    }
    network_heap_free_block((uint8*)blocks[_entity_update_block_order_forward_memory_queue_element].block_data);
    csmemset(&blocks[_entity_update_block_order_forward_memory_queue_element], 0, sizeof(blocks[_entity_update_block_order_forward_memory_queue_element]));
    return true;
}

__declspec(naked) void jmp_c_simulation_entity_database__process_update() { __asm { jmp c_simulation_entity_database::process_update } }

int32 c_simulation_entity_database::read_update_from_packet(
    int32 entity_index, 
    uint32* out_update_mask, 
    int32 maximum_block_count, 
    int32* block_count, 
    s_replication_allocation_block* blocks,
    c_bitstream* packet
)
{
	uint32 result = 3;
	s_simulation_game_entity* game_entity = entity_try_and_get(entity_index);
	if (game_entity && game_entity != (void*)-20)
	{
		c_simulation_entity_definition* entity_def = m_type_collection->get_entity_definition(game_entity->entity_type);

		// Allocate state data
		uint8* state_data = network_heap_allocate_block(game_entity->state_data_size);
        uint8* queue_element = (uint8*)network_heap_allocate_block(sizeof(s_simulation_queue_element*));

        result = (!state_data || !queue_element ? 2 : result);

        if (state_data && queue_element)
        {
            uint32 update_mask = 0;
            bool decode_success = false;

			if (packet->read_only_for_consistency())
			{
				decode_success =
					entity_def->build_baseline_state_data(
                        game_entity->creation_data_size, 
                        game_entity->creation_data, 
                        game_entity->state_data_size, 
                        (s_simulation_baseline_state_data*)state_data
                    );
			}
			else
			{
				csmemcpy(state_data, game_entity->state_data, game_entity->state_data_size);
				decode_success = true;
			}

			if (decode_success)
			{
                decode_success = entity_def->entity_update_decode(
                    false,
                    &update_mask,
                    game_entity->state_data_size,
                    state_data,
                    packet
                );
			}

            if (decode_success)
            {
                s_simulation_queue_entity_data sim_queue_entity_data;
                sim_queue_entity_data.entity_index = entity_index;
                sim_queue_entity_data.entity_type = game_entity->entity_type;
                sim_queue_entity_data.creation_data_size = game_entity->creation_data_size;
                sim_queue_entity_data.creation_data = (uint8*)game_entity->creation_data;
                sim_queue_entity_data.state_data_size = game_entity->state_data_size;
                sim_queue_entity_data.state_data = state_data;

                if (!packet->read_only_for_consistency()
                    && !simulation_queue_entity_update_allocate(&sim_queue_entity_data, DATUM_INDEX_NONE, update_mask, (s_simulation_queue_element**)queue_element))
                {
                    decode_success = false;
                }
            }

            if (decode_success)
            {
                *out_update_mask = update_mask;

                blocks[*block_count + _entity_update_block_order_simulation_entity_state].block_type = _network_memory_block_simulation_entity_state;
                blocks[*block_count + _entity_update_block_order_simulation_entity_state].block_size = game_entity->state_data_size;
                blocks[*block_count + _entity_update_block_order_simulation_entity_state].block_data = state_data;

                blocks[*block_count + _entity_update_block_order_forward_memory_queue_element].block_type = _network_memory_block_forward_simulation_queue_element;
                blocks[*block_count + _entity_update_block_order_forward_memory_queue_element].block_size = sizeof(s_simulation_queue_element*);
                blocks[*block_count + _entity_update_block_order_forward_memory_queue_element].block_data = queue_element;

                *block_count += k_entity_update_block_order_count;

                result = 0;
            }
        }

		if (result == 3
			|| result == 2)
		{
            if (state_data)
			    network_heap_free_block(state_data);

            if (queue_element)
            {
                if (*queue_element != NULL)
                {
                    simulation_get_world()->simulation_queue_free(*(s_simulation_queue_element**)queue_element);
                }

                network_heap_free_block(queue_element);
            }
		}
	}

	return result;
}

__declspec(naked) void jmp_c_simulation_entity_database__read_update_from_packet() { __asm { jmp c_simulation_entity_database::read_update_from_packet } }

void c_simulation_entity_database::notify_mark_entity_for_deletion(int32 entity_index)
{
    this->entity_delete_gameworld(entity_index);
}

__declspec(naked) void jmp_c_simulation_entity_database__notify_mark_entity_for_deletion() { __asm { jmp c_simulation_entity_database::notify_mark_entity_for_deletion } }

void c_simulation_entity_database::entity_delete_gameworld(int32 entity_index)
{
    s_simulation_game_entity* game_entity = entity_get(entity_index);
    if (game_entity->exists_in_gameworld)
    {
        c_simulation_entity_definition* entity_definition = m_type_collection->get_entity_definition(game_entity->entity_type);
        simulation_queue_entity_deletion_insert(game_entity);
        game_entity->exists_in_gameworld = false;
        game_entity->entity_update_flag = 0;
        game_entity->field_10 = 0;
    }
    return;
}

void simulation_entity_database_apply_patches(void)
{
	WritePointer(Memory::GetAddress(0x3C6228, 0x381D10), jmp_c_simulation_entity_database__read_creation_from_packet);
	WritePointer(Memory::GetAddress(0x3C622C, 0x381D14), jmp_c_simulation_entity_database__process_creation);

    WritePointer(Memory::GetAddress(0x3C623C, 0x0), jmp_c_simulation_entity_database__read_update_from_packet);
    WritePointer(Memory::GetAddress(0x3C6240, 0x0), jmp_c_simulation_entity_database__process_update);

    WritePointer(Memory::GetAddress(0x3C624C, 0x381D34), jmp_c_simulation_entity_database__notify_mark_entity_for_deletion);
	return;
}