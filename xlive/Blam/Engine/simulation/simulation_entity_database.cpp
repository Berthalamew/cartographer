#include "stdafx.h"
#include "simulation_entity_database.h"

#include "simulation.h"
#include "simulation_entity_definition.h"
#include "simulation_gamestate_entities.h"
#include "simulation_priority.h"
#include "simulation_queue_entities.h"
#include "simulation_view_telemetry.h"
#include "simulation_type_collection.h"
#include "simulation_world.h"

#include "game/game.h"
#include "memory/bitstream.h"
#include "networking/replication/replication_scheduler.h"
#include "networking/network_event.h"
#include "networking/network_memory.h"
#include "networking/network_utilities.h"

/* prototypes */

CLASS_HOOK_DECLARE_LABEL(c_simulation_entity_database__write_creation_to_packet, c_simulation_entity_database::write_creation_to_packet);
static __declspec(naked) void jmp_c_simulation_entity_database__write_creation_to_packet(void)
{
	CLASS_HOOK_JMP(c_simulation_entity_database__write_creation_to_packet, c_simulation_entity_database::write_creation_to_packet);
}

CLASS_HOOK_DECLARE_LABEL(c_simulation_entity_database__read_creation_from_packet, c_simulation_entity_database::read_creation_from_packet);
static __declspec(naked) void jmp_c_simulation_entity_database__read_creation_from_packet(void)
{
	CLASS_HOOK_JMP(c_simulation_entity_database__read_creation_from_packet, c_simulation_entity_database::read_creation_from_packet);
}

CLASS_HOOK_DECLARE_LABEL(c_simulation_entity_database__process_creation, c_simulation_entity_database::process_creation);
static __declspec(naked) void jmp_c_simulation_entity_database__process_creation(void)
{
	CLASS_HOOK_JMP(c_simulation_entity_database__process_creation, c_simulation_entity_database::process_creation);
}

CLASS_HOOK_DECLARE_LABEL(c_simulation_entity_database__write_update_to_packet, c_simulation_entity_database::write_update_to_packet);
static __declspec(naked) void jmp_c_simulation_entity_database__write_update_to_packet(void)
{
	CLASS_HOOK_JMP(c_simulation_entity_database__write_update_to_packet, c_simulation_entity_database::write_update_to_packet);
}

CLASS_HOOK_DECLARE_LABEL(c_simulation_entity_database__read_update_from_packet, c_simulation_entity_database::read_update_from_packet);
static __declspec(naked) void jmp_c_simulation_entity_database__read_update_from_packet(void)
{
	CLASS_HOOK_JMP(c_simulation_entity_database__read_update_from_packet, c_simulation_entity_database::read_update_from_packet);
}

CLASS_HOOK_DECLARE_LABEL(c_simulation_entity_database__process_update, c_simulation_entity_database::process_update);
static __declspec(naked) void jmp_c_simulation_entity_database__process_update(void)
{
	CLASS_HOOK_JMP(c_simulation_entity_database__process_update, c_simulation_entity_database::process_update);
}

CLASS_HOOK_DECLARE_LABEL(c_simulation_entity_database__notify_mark_entity_for_deletion, c_simulation_entity_database::notify_mark_entity_for_deletion);
static __declspec(naked) void jmp_c_simulation_entity_database__notify_mark_entity_for_deletion(void)
{
	CLASS_HOOK_JMP(c_simulation_entity_database__notify_mark_entity_for_deletion, c_simulation_entity_database::notify_mark_entity_for_deletion);
}

CLASS_HOOK_DECLARE_LABEL(c_simulation_entity_database__notify_promote_to_authority, c_simulation_entity_database::notify_promote_to_authority);
static __declspec(naked) void jmp_c_simulation_entity_database__notify_promote_to_authority(void)
{
	CLASS_HOOK_JMP(c_simulation_entity_database__notify_promote_to_authority, c_simulation_entity_database::notify_promote_to_authority);
}

/* public code */

void simulation_entity_database_apply_patches(void)
{
	LLVM_JMP_ERROR;

	WritePointer(Memory::GetAddress(0x3C6224, 0x381D0C), jmp_c_simulation_entity_database__write_creation_to_packet);
	WritePointer(Memory::GetAddress(0x3C6228, 0x381D10), jmp_c_simulation_entity_database__read_creation_from_packet);
	WritePointer(Memory::GetAddress(0x3C622C, 0x381D14), jmp_c_simulation_entity_database__process_creation);
	// allow the creation of turrets by increasing the block count, block count was hardcoded
	WriteValue<int8>(Memory::GetAddress(0x1D7081, 0x1DA3A2) + 1, (int8)k_entity_creation_block_order_count);
	WriteValue<int8>(Memory::GetAddress(0x1D7091, 0x1DA3B2) + 2, (int8)sizeof(s_replication_allocation_block) * k_entity_creation_block_order_count);

	WritePointer(Memory::GetAddress(0x3C6238, 0x381D20), jmp_c_simulation_entity_database__write_update_to_packet);
	WritePointer(Memory::GetAddress(0x3C623C, 0x381D24), jmp_c_simulation_entity_database__read_update_from_packet);

	WritePointer(Memory::GetAddress(0x3C6240, 0x381D28), jmp_c_simulation_entity_database__process_update);

	WritePointer(Memory::GetAddress(0x3C624C, 0x381D34), jmp_c_simulation_entity_database__notify_mark_entity_for_deletion);

	WritePointer(Memory::GetAddress(0x3C6258, 0x381D40), jmp_c_simulation_entity_database__notify_promote_to_authority);
	return;
}

void c_simulation_entity_database::initialize(
	c_simulation_world* world,
	c_replication_entity_manager* entity_manager, 
	c_simulation_type_collection* type_collection)
{
	ASSERT(!m_initialized);
	ASSERT(world);
	ASSERT(world->exists());
	ASSERT(entity_manager);
	ASSERT(type_collection);

	m_resetting = false;
	m_world = world;
	m_entity_manager = entity_manager;
	m_type_collection = type_collection;
	m_entity_manager->attach_client(this);
	reset();
	m_initialized = true;

	return;
}

void c_simulation_entity_database::reset(
	void)
{
	m_resetting = true;

	if (!m_initialized)
	{
		csmemset(m_entity_data, 0, sizeof(m_entity_data));
	}

	for (uint32 i = 0; i < k_simulation_entity_database_maximum_entities; i++)
	{
		s_simulation_entity* entity = &m_entity_data[i];
		if (m_initialized)
		{
			if (entity->entity_index != NONE)
			{
				entity_delete_internal(entity->entity_index);
				ASSERT(entity->entity_index == NONE);
			}

			ASSERT(entity->entity_type == NONE);
		}
		else
		{
			entity->entity_index = NONE;
			entity->entity_type = k_simulation_entity_type_none;
		}

		ASSERT(entity->creation_data_size == 0);
		ASSERT(entity->creation_data == NULL);
		ASSERT(entity->state_data_size == 0);
		ASSERT(entity->state_data == NULL);
	}

	m_resetting = false;

	return;
}

void c_simulation_entity_database::destroy(
	void)
{
	ASSERT(m_initialized);
	ASSERT(m_entity_manager);

	reset();
	m_entity_manager->detach_client(this);
	m_initialized = false;
	m_world = NULL;
	m_entity_manager = NULL;
	m_type_collection = NULL;

	return;
}

void c_simulation_entity_database::process_pending_updates(
	void)
{
	int32 entity_absolute_index;

	ASSERT(m_initialized);
	ASSERT(m_world);
	ASSERT(m_world->is_distributed());
	ASSERT(m_world->is_authority());

	for (entity_absolute_index = 0; entity_absolute_index<NUMBEROF(m_entity_data); ++entity_absolute_index)
	{
		s_simulation_entity* entity = &m_entity_data[entity_absolute_index];

		if (entity->entity_index!=NONE && entity->force_update_mask)
		{
			if (entity->exists_in_gameworld)
			{
				uint32 actual_update_mask;
				c_simulation_entity_definition* entity_definition = m_type_collection->get_entity_definition(entity->entity_type);
				
				ASSERT(m_entity_manager);
				ASSERT(m_entity_manager->is_entity_local(entity->entity_index));
				ASSERT(!m_entity_manager->is_entity_being_deleted(entity->entity_index));


				ASSERT(entity_definition);
				ASSERT(entity->state_data_size==entity_definition->state_data_size());

				actual_update_mask = entity->pending_update_mask;
				if (entity_definition->build_updated_state_data(entity, &actual_update_mask, entity->state_data_size, entity->state_data))
				{
					actual_update_mask |= entity->force_update_mask;

					ASSERT((actual_update_mask & ~MASK(entity_definition->update_flag_count()))==0);
					ASSERT((actual_update_mask & entity->pending_update_mask) == actual_update_mask);

					entity_validate_state_data(entity->entity_index);

					if (actual_update_mask)
					{
						m_entity_manager->set_entity_dirty(entity->entity_index, actual_update_mask);
					}
				}
				else
				{
					event(
						_event_error,
						"simulation:entity: process_pending_updates: cannot capture state data for entity [0x%08X] type %d",
						entity->entity_index,
						entity->entity_type
					);
				}
			}
		}
	}

	return;
}

s_simulation_entity const* c_simulation_entity_database::entity_get(
	int32 entity_index) const
{
	int32 absolute_index = ENTITY_INDEX_TO_ABSOLUTE_INDEX(entity_index);

	ASSERT(m_entity_manager);
	ASSERT(m_entity_manager->is_entity_allocated(entity_index));
	ASSERT(m_entity_data[absolute_index].entity_index == entity_index);

	return &m_entity_data[absolute_index];
}

s_simulation_entity* c_simulation_entity_database::entity_get(
	int32 entity_index)
{
	int32 absolute_index = ENTITY_INDEX_TO_ABSOLUTE_INDEX(entity_index);

	ASSERT(m_entity_manager);
	ASSERT(m_entity_manager->is_entity_allocated(entity_index));
	ASSERT(m_entity_data[absolute_index].entity_index == entity_index);

	return &m_entity_data[absolute_index];
}

char const* c_simulation_entity_database::get_entity_type_name(
	e_simulation_entity_type entity_type) const
{
	ASSERT(m_initialized);
	ASSERT(m_type_collection);

	return m_type_collection->get_entity_type_name(entity_type);
}

bool c_simulation_entity_database::process_creation(
	int32 entity_index,
	e_simulation_entity_type entity_type,
	uint32 update_mask,
	int32 block_count,
	s_replication_allocation_block* blocks)
{
	bool result = false;

	ASSERT(entity_index!=NONE);

	const uint32 absolute_index = ENTITY_INDEX_TO_ABSOLUTE_INDEX(entity_index);
	
	ASSERT(absolute_index>=0 && absolute_index<k_simulation_entity_database_maximum_entities);

	ASSERT(m_entity_manager);
	ASSERT(m_entity_manager->is_entity_allocated(entity_index));
	ASSERT(entity_type!=NONE);
	ASSERT(block_count==4);
	
	ASSERT(blocks[0].block_type==_network_memory_block_simulation_entity_creation);
	ASSERT(blocks[1].block_type==_network_memory_block_simulation_entity_state);
	ASSERT(blocks[2].block_type==_network_memory_block_forward_gamestate_element);
	ASSERT(blocks[3].block_type==_network_memory_block_forward_simulation_queue_element);

	const int32 creation_data_size = blocks[_entity_creation_block_order_simulation_entity_creation].block_size;
	void* creation_data = blocks[_entity_creation_block_order_simulation_entity_creation].block_data;
	
	const int32 state_data_size = blocks[_entity_creation_block_order_simulation_entity_state].block_size;
	void* state_data = blocks[_entity_creation_block_order_simulation_entity_state].block_data;
	
	const int32 gamestate_index = (int32)blocks[_entity_creation_block_order_gamestate_index].block_data;
	
	s_simulation_queue_element* simulation_queue_element = *(s_simulation_queue_element**)blocks[_entity_creation_block_order_forward_memory_queue_element].block_data;

	// free these blocks
	network_heap_free_block((uint8*)blocks[_entity_creation_block_order_forward_memory_queue_element].block_data);
	network_heap_free_block((uint8*)blocks[_entity_creation_block_order_gamestate_index].block_data);

	csmemset(blocks, 0, sizeof(s_replication_allocation_block) * block_count);

	const c_simulation_entity_definition* entity_definition = m_type_collection->get_entity_definition(entity_type);
	
	ASSERT(entity_definition != NULL);
	ASSERT(creation_data_size == entity_definition->creation_data_size());
	ASSERT(state_data_size == entity_definition->state_data_size());
	ASSERT((creation_data == NULL) == (creation_data_size == 0));
	ASSERT((state_data == NULL) == (state_data_size == 0));
	ASSERT(gamestate_index != NONE);
	ASSERT(simulation_queue_element != NULL);

	entity_create_internal(entity_index, entity_type, creation_data_size, creation_data, state_data_size, state_data);
	s_simulation_entity* entity= entity_get(entity_index);
	
	ASSERT(entity!=NULL);

	entity_validate_creation_data(entity_index);
	entity_validate_state_data(entity_index);

	entity->gamestate_index = gamestate_index;
	simulation_gamestate_entity_set_simulation_entity_index(gamestate_index, entity->entity_index);
	simulation_queue_entity_creation_insert(simulation_queue_element);
	entity->exists_in_gameworld = true;

	return result;
}

/* TODO finish
int32 c_simulation_entity_database::calculate_creation_requirements(
	int32 entity_index,
	uint32 update_mask,
	void const* in_telemetry_data,
	real32* priority,
	int32* minimum_required_bits)
{
	int32 minimum_required_bits;
	uint32 potential_update_mask;

	s_simulation_view_telemetry_data const* telemetry_data = (s_simulation_view_telemetry_data const*)in_telemetry_data;
	s_simulation_entity const* entity = entity_get(entity_index);
	c_simulation_entity_definition* entity_definition = m_type_collection->get_entity_definition(entity->entity_type);

	ASSERT(entity_definition);
	ASSERT(entity->creation_data_size==entity_definition->creation_data_size());
	ASSERT(entity->pending_update_mask==0);
	ASSERT(entity->force_update_mask==0);

	if (
		(!telemetry_data->joining || entity_definition->entity_replication_required_for_view_activation(entity)) &&
		entity_definition->entity_can_be_created(entity, telemetry_data)
	)
	{
		*fixed_priority = simulation_calculate_entity_creation_priority(entity, telemetry_data, NULL);
		update_minimum_required_bits = entity_definition->creation_minimum_required_bits(entity, telemetry_data, minimum_required_bits);

		s_entity_update_data update_data;
		int32 update_minimum_required_bits;

		update_data.update_mask = update_mask& ;
		update_data.update_timestamp = 0;
		update_data.telemetry = telemetry_data;
		entity_definition->write_update_description_to_string(entity, &update_data, , fixed_priority)
	}
	else
	{

	}

	return ;
}
*/

bool c_simulation_entity_database::write_creation_to_packet(
	int32 entity_index,
	uint32 update_mask,
	void const* in_telemetry_data,
	c_bitstream* packet,
	int32 must_leave_space_bits,
	uint32* out_update_mask)
{
	s_simulation_view_telemetry_data const* telemetry_data= (struct s_simulation_view_telemetry_data const*)in_telemetry_data;
	uint32 potential_update_mask;

	s_simulation_entity const* entity = entity_get(entity_index);
	c_simulation_entity_definition* entity_definition = m_type_collection->get_entity_definition(entity->entity_type);
	bool write_success = true;

	ASSERT(entity_definition);
	ASSERT(entity->creation_data_size==entity_definition->creation_data_size());
	ASSERT(entity->state_data_size==entity_definition->state_data_size());
	
	ASSERT(out_update_mask);

	// TODO: figure out
	//bandwidth_profiler_record_push(unk, packet);

	entity_definition->entity_creation_encode(entity->creation_data_size, entity->creation_data, telemetry_data, packet, true);
	potential_update_mask = entity_definition->initial_update_mask();

	if (potential_update_mask)
	{
		uint32 desired_update_mask = update_mask&potential_update_mask;
		
		packet->write_bool("initial-update-exists", desired_update_mask!=0);
		
		if (desired_update_mask!=0)
		{
			bool wrote_update;

			entity_validate_state_data(entity_index);

			wrote_update = entity_definition->entity_update_encode(
				true,
				desired_update_mask,
				out_update_mask,
				entity->state_data_size,
				entity->state_data,
				telemetry_data,
				packet,
				must_leave_space_bits,
				true);

			ASSERT((*out_update_mask&~desired_update_mask)==0);

			if (!wrote_update || *out_update_mask != desired_update_mask)
			{
				event(
					_event_verbose,
					"networking:simulation:entity:write_creation_to_packet: entity %d/%s/0x%08X unable to write entire initial update (write-%s, actual 0x%08X, desired 0x%08X)",
					entity->entity_type,
					entity_definition->entity_type_name(),
					entity_index,
					wrote_update ? "success" : "failed",
					*out_update_mask,
					desired_update_mask
				);
			}
		}
	}

	// TODO: figure out
	//bandwidth_profiler_record_pop(unk, packet);
	
	return write_success;
}

e_network_read_result c_simulation_entity_database::read_creation_from_packet(
	int32 entity_index,
	e_simulation_entity_type* out_entity_type, 
	uint32* out_entity_initial_update_mask, 
	int32 maximum_block_count,
	int32* block_count, 
	s_replication_allocation_block* blocks,
	c_bitstream* packet)
{
	const uint16 absolute_index = ENTITY_INDEX_TO_ABSOLUTE_INDEX(entity_index);

	ASSERT(VALID_INDEX(absolute_index, k_simulation_entity_database_maximum_entities));
	ASSERT(packet);

	e_network_read_result result = _network_read_result_corrupt;
	e_simulation_entity_type entity_type = (e_simulation_entity_type)packet->read_integer("entity-type", 5);
	c_simulation_entity_definition* entity_definition = m_type_collection->get_entity_definition(entity_type);
	
	if (entity_definition)
	{
		uint32 creation_data_size = entity_definition->creation_data_size();
		uint32 state_data_size = entity_definition->state_data_size();

		// Allocate creation data
		uint8* creation_data = NULL;
		if (creation_data_size > 0)
		{
			creation_data = network_heap_allocate_block(creation_data_size);
			if (!creation_data)
			{
#ifdef EVENTS_ENABLED
				char description[1024];
				event(
					_event_error,
					"networking:simulation:entity: OUT OF MEMORY allocating %s creation data [%d] bytes [%s]",
					creation_data_size,
					network_heap_describe(description, sizeof(description))
				);
#endif
				result = _network_read_result_discard;
			}
		}

		// Allocate state data
		uint8* state_data = network_heap_allocate_block(state_data_size);
		if (!state_data)
		{
#ifdef EVENTS_ENABLED
			char description[1024];
			event(
				_event_error, 
				"networking:simulation:entity: OUT OF MEMORY allocating %s state data [%d] bytes, heap [%s]",
				state_data_size,
				network_heap_describe(description, sizeof(description))
			);
#endif
			result = _network_read_result_discard;
		}

		// Allocate gamestate data
		int32* gamestate_index = (int32*)network_heap_allocate_block(sizeof(int32));
		if (!gamestate_index)
		{
#ifdef EVENTS_ENABLED
			char description[1024];
			event(
				_event_error,
				"networking:simulation:entity: OUT OF MEMORY allocating %s gamestate data [%d] bytes [%s]",
				sizeof(int32),
				network_heap_describe(description, sizeof(description))
			);
#endif
			result = _network_read_result_discard;
		}

		// Allocate queue data
		s_simulation_queue_element** simulation_queue_element = (s_simulation_queue_element**)network_heap_allocate_block(sizeof(s_simulation_queue_element*));
		if (!simulation_queue_element)
		{
#ifdef EVENTS_ENABLED
			char description[1024];
			event(
				_event_error,
				"networking:simulation:entity: OUT OF MEMORY allocating %s simulation queue data [%d] bytes [%s]",
				sizeof(s_simulation_queue_element*),
				network_heap_describe(description, sizeof(description))
			);
#endif
			result = _network_read_result_discard;
		}

		// check if creation size is > 0 and if network heap block have been successfully allocated
		if ((!creation_data_size || creation_data != NULL) &&
			(state_data && simulation_queue_element &&gamestate_index))
		{
			if (creation_data_size > 0)
			{
				csmemset(creation_data, 0, creation_data_size);
			}

			*simulation_queue_element = NULL;

			if (entity_definition->entity_creation_decode(creation_data_size, creation_data, packet, true))
			{
				if (entity_definition->build_baseline_state_data(
					creation_data_size,
					creation_data,
					state_data_size,
					state_data))
				{
					uint32 entity_initial_update_mask = 0;
					uint32 entity_allowed_initial_update_mask = entity_definition->initial_update_mask();

					bool read_success = true;
					if (entity_allowed_initial_update_mask != 0 && packet->read_bool("initial-update-exists"))
					{
						if (entity_definition->entity_update_decode(true, &entity_initial_update_mask, state_data_size, state_data, packet, true))
						{
							// check if the state contains updates allowed only on creation
							if (TEST_FLAG(entity_initial_update_mask, ~entity_allowed_initial_update_mask))
							{
								event(
									_event_error,
									"networking:simulation:entity:read_update_from_packet: initial creation update read [0x%08X] (expecting mask of [0x%08X]) for entity [0x%08X] type %d",
									entity_initial_update_mask,
									entity_allowed_initial_update_mask,
									entity_type,
									entity_definition->entity_type());
								read_success = false;
							}
						}
						else
						{
							event(
								_event_error,
								"networking:simulation:entity:read_update_from_packet: failed to decode initial creation update for entity %d/%s/0x%08X",
								entity_type,
								entity_definition->entity_type_name(),
								entity_index);
							read_success = false;
						}
					}

					if (read_success)
					{
						/* Original code
						*out_entity_type = entity_type;
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

						if (!packet->read_only_for_consistency() &&
							!simulation_queue_entity_creation_allocate(
								&sim_queue_entity_data,
								entity_initial_update_mask,
								simulation_queue_element,
								gamestate_index))
						{
							read_success = false;
						}
					}

					if (read_success)
					{
						ASSERT(out_entity_type);
						ASSERT(out_entity_initial_update_mask);

						*out_entity_type = entity_type;
						*out_entity_initial_update_mask = entity_initial_update_mask;

						ASSERT(block_count);
						ASSERT(blocks);
						ASSERT(*block_count + 4 <= maximum_block_count);

						// copy the block, allow the process function to use this
						blocks[*block_count + _entity_creation_block_order_simulation_entity_creation].block_type = _network_memory_block_simulation_entity_creation;
						blocks[*block_count + _entity_creation_block_order_simulation_entity_creation].block_size = (int16)creation_data_size;
						blocks[*block_count + _entity_creation_block_order_simulation_entity_creation].block_data = creation_data;

						blocks[*block_count + _entity_creation_block_order_simulation_entity_state].block_type = _network_memory_block_simulation_entity_state;
						blocks[*block_count + _entity_creation_block_order_simulation_entity_state].block_size = (int16)state_data_size;
						blocks[*block_count + _entity_creation_block_order_simulation_entity_state].block_data = state_data;

						blocks[*block_count + _entity_creation_block_order_gamestate_index].block_type = _network_memory_block_forward_gamestate_element;
						blocks[*block_count + _entity_creation_block_order_gamestate_index].block_size = sizeof(*gamestate_index);
						blocks[*block_count + _entity_creation_block_order_gamestate_index].block_data = gamestate_index;

						blocks[*block_count + _entity_creation_block_order_forward_memory_queue_element].block_type = _network_memory_block_forward_simulation_queue_element;
						blocks[*block_count + _entity_creation_block_order_forward_memory_queue_element].block_size = sizeof(s_simulation_queue_element*);
						blocks[*block_count + _entity_creation_block_order_forward_memory_queue_element].block_data = simulation_queue_element;

						*block_count += k_entity_creation_block_order_count;

						result = _network_read_result_ok;
					}
					else
					{
						event(
							_event_error,
							"networking:simulation:entity:read_creation_from_packet: unable to read initial update for entity %d/%s/[0x%08X]",
							entity_type,
							entity_definition->entity_type_name(),
							entity_index);
						
						result = _network_read_result_corrupt;

						if (!packet->read_only_for_consistency())
						{
							DISPLAY_ASSERT("entity initial update decode failed consistency checking a packet");
						}
					}
				}
			}
			else
			{
				event(
					_event_error,
					"networking:simulation:entity:read_creation_from_packet: corrupt creation data reading entity %d/%s/[0x%08X]",
					entity_type,
					entity_definition->entity_type_name(),
					entity_index
				);

				result = _network_read_result_corrupt;

				if (!packet->read_only_for_consistency())
				{
					DISPLAY_ASSERT("entity creation data decode failed consistency checking a packet");
				}
			}
		}

		if (result == _network_read_result_corrupt || result == _network_read_result_discard)
		{
			if (creation_data != NULL)
			{
				network_heap_free_block(creation_data);
			}

			if (state_data != NULL)
			{
				network_heap_free_block(state_data);
			}

			if (gamestate_index != NULL)
			{
				network_heap_free_block(gamestate_index);
			}

			if (simulation_queue_element != NULL)
			{
				network_heap_free_block(simulation_queue_element);
			}
		}
	}
	else
	{
		event(_event_error, "simulation:entity: read_creation_from_packet: unknown entity type %d", entity_type);
	}

	return result;
}

void c_simulation_entity_database::process_update(
	int32 entity_index,
	uint32 update_mask,
	int32 block_count,
	s_replication_allocation_block* blocks)
{
	s_simulation_entity* entity = entity_get(entity_index);
	c_simulation_entity_definition* entity_definition = m_type_collection->get_entity_definition(entity->entity_type);
	
	ASSERT(entity_definition);
	ASSERT(update_mask!=0);
	ASSERT(block_count==2);
	ASSERT(blocks[0].block_type==_network_memory_block_simulation_entity_state);
	ASSERT(blocks[0].block_size==entity->state_data_size);
	ASSERT(blocks[0].block_data!=NULL);
	ASSERT(blocks[1].block_type==_network_memory_block_forward_simulation_queue_element);
	ASSERT(blocks[1].block_size==sizeof(s_simulation_queue_element *));
	ASSERT(blocks[1].block_data!=NULL);

	s_simulation_queue_element* simulation_queue_element = *(s_simulation_queue_element**)blocks[_entity_update_block_order_forward_memory_queue_element].block_data;

	csmemcpy(entity->state_data, blocks[_entity_update_block_order_simulation_entity_state].block_data, entity->state_data_size);
	simulation_queue_entity_update_insert(simulation_queue_element);
	network_heap_free_block(blocks[_entity_update_block_order_forward_memory_queue_element].block_data);
	
	csmemset(&blocks[_entity_update_block_order_forward_memory_queue_element], 0, sizeof(blocks[_entity_update_block_order_forward_memory_queue_element]));
	
	return;
}

bool c_simulation_entity_database::write_update_to_packet(
	int32 entity_index,
	uint32 update_mask,
	void const* in_telemetry_data,
	c_bitstream* packet,
	int32 must_leave_space_bits,
	uint32* out_update_mask)
{
	bool wrote_update;

	s_simulation_entity const* entity = entity_get(entity_index);
	c_simulation_entity_definition* entity_definition = m_type_collection->get_entity_definition(entity->entity_type);
	s_simulation_view_telemetry_data const* telemetry_data = (s_simulation_view_telemetry_data const*)in_telemetry_data;

	ASSERT(update_mask!=0);
	ASSERT(packet);
	ASSERT(out_update_mask);
	ASSERT(entity_definition!=NULL);
	ASSERT(entity->state_data_size==entity_definition->state_data_size());

	entity_validate_state_data(entity_index);

	wrote_update = entity_definition->entity_update_encode(
		false,
		update_mask,
		out_update_mask,
		entity->state_data_size,
		entity->state_data,
		telemetry_data,
		packet,
		must_leave_space_bits,
		true
	);

	return wrote_update;
}

e_network_read_result c_simulation_entity_database::read_update_from_packet(
	int32 entity_index, 
	uint32* out_update_mask, 
	int32 maximum_block_count, 
	int32* block_count, 
	s_replication_allocation_block* blocks,
	c_bitstream* packet
)
{
	e_network_read_result read_result = _network_read_result_corrupt;
	s_simulation_entity* entity = entity_try_and_get(entity_index);

	if (entity)
	{
		c_simulation_entity_definition* entity_definition = m_type_collection->get_entity_definition(entity->entity_type);
		bool state_data_valid = false;

		ASSERT(entity_definition!=NULL);
		ASSERT(entity->state_data_size==entity_definition->state_data_size());

		// Allocate state data
		void* state_data = network_heap_allocate_block(entity->state_data_size);
		if (state_data)
		{
			if (packet->read_only_for_consistency())
			{
				state_data_valid = entity_definition->build_baseline_state_data(
					entity->creation_data_size,
					entity->creation_data,
					entity->state_data_size,
					state_data
				);

				if (!state_data_valid)
				{
					event(
						_event_error,
						"networking:simulation:entity:read_update_from_packet: unable to build baseline data for entity [0x%08X] type %d",
						entity_index,
						entity->entity_type
					);
					read_result = _network_read_result_corrupt;
				}
			}
			else
			{
				csmemcpy(state_data, entity->state_data, entity->state_data_size);
				state_data_valid = true;
			}
		}
		else
		{
#ifdef EVENTS_ENABLED
			char heapbuf[1024];
			event(
				_event_error,
				"networking:simulation:entity: OUT OF MEMORY allocating %s pending state data [%d] bytes, heap [%s]",
				entity_definition->entity_type_name(),
				entity->state_data_size,
				network_heap_describe(heapbuf, sizeof(heapbuf))
			);
#endif
			read_result = _network_read_result_discard;
		}

		s_simulation_queue_element** simulation_queue_element_data = (s_simulation_queue_element **)network_heap_allocate_block(sizeof(s_simulation_queue_element*));

		if (!simulation_queue_element_data)
		{
#ifdef EVENTS_ENABLED
			char heapbuf[1024];
			event(
				_event_error,
				"networking:simulation:entity: OUT OF MEMORY allocating %s simulation queue data for update [%d] bytes [%s]",
				entity_definition->entity_type_name(),
				sizeof(*simulation_queue_element_data),
				network_heap_describe(heapbuf, sizeof(heapbuf))
			);
#endif
			read_result = _network_read_result_discard;
		}


		if (state_data_valid)
		{
			uint32 update_mask = 0;
			
			// TODO: figure out this
			//bandwidth_profiler_record_push(unknown, packet);

			if (entity_definition->entity_update_decode(
					false,
					&update_mask,
					entity->state_data_size,
					state_data,
					packet,
					true)
				)
			{
				s_simulation_queue_entity_data entity_data;
				entity_data.entity_index = entity_index;
				entity_data.entity_type = entity->entity_type;
				entity_data.creation_data_size = entity->creation_data_size;
				entity_data.creation_data = (uint8*)entity->creation_data;
				entity_data.state_data_size = entity->state_data_size;
				entity_data.state_data = state_data;

				if (!packet->read_only_for_consistency() &&
					!simulation_queue_entity_update_allocate(&entity_data, NONE, update_mask, simulation_queue_element_data))
				{
					state_data_valid = false;
				}
			}

			if (state_data_valid)
			{
				ASSERT(out_update_mask);
				ASSERT(block_count);
				ASSERT(blocks);

				*out_update_mask = update_mask;

				ASSERT(*block_count+2<=maximum_block_count);


				blocks[*block_count+_entity_update_block_order_simulation_entity_state].block_type = _network_memory_block_simulation_entity_state;
				blocks[*block_count+_entity_update_block_order_simulation_entity_state].block_size = (int16)entity->state_data_size;
				blocks[*block_count+_entity_update_block_order_simulation_entity_state].block_data = state_data;
				state_data = NULL;

				blocks[*block_count+_entity_update_block_order_forward_memory_queue_element].block_type = _network_memory_block_forward_simulation_queue_element;
				blocks[*block_count+_entity_update_block_order_forward_memory_queue_element].block_size = sizeof(s_simulation_queue_element*);
				blocks[*block_count+_entity_update_block_order_forward_memory_queue_element].block_data = simulation_queue_element_data;
				simulation_queue_element_data = NULL;

				*block_count += k_entity_update_block_order_count;

				read_result = _network_read_result_ok;
			}
			else
			{
				event(
					_event_error,
					"networking:simulation:entity:read_update_from_packet: failed to decode update for entity [0x%08X] type %d",
					entity_index,
					entity->entity_type
				);
			}

			// TODO: figure out this
			//bandwidth_profiler_record_pop(unknown, packet);
		}

		if (state_data)
		{
			network_heap_free_block(state_data);
		}

		if (simulation_queue_element_data)
		{
			network_heap_free_block(simulation_queue_element_data);
		}
	}
	else
	{
		event(_event_warning, "networking:simulation:entity:read_update_from_packet: invalid entity 0x%08X", entity_index);
	}

	return read_result;
}

bool c_simulation_entity_database::notify_promote_to_authority(int32 entity_index)
{
	s_simulation_entity* entity = this->entity_get(entity_index);
	simulation_queue_entity_promotion_insert(entity);
	return true;
}

bool c_simulation_entity_database::entity_is_local(
	int32 entity_index) const
{
	ASSERT(m_entity_manager);
	ASSERT(m_world->is_distributed());

	return m_entity_manager->is_entity_local(entity_index);
}

int32 c_simulation_entity_database::entity_create(
	e_simulation_entity_type entity_type)
{
	int32 creation_data_size;
	int32 state_data_size;

	int32 entity_index = NONE;
	void* creation_data = NULL;
	void* state_data = NULL;

	ASSERT(m_world);
	ASSERT(m_entity_manager);
	ASSERT(m_world->is_distributed());
	ASSERT(m_world->is_authority());

	if (entity_allocate_creation_data(entity_type, &creation_data_size, &creation_data) &&
		entity_allocate_state_data(entity_type, &state_data_size, &state_data))
	{
		entity_index = m_entity_manager->create_local_entity();

		if (entity_index != NONE)
		{
			entity_create_internal(
				entity_index,
				entity_type,
				creation_data_size,
				creation_data,
				state_data_size,
				state_data);

			event(
				_event_status,
				"simulation:entity: created entity 0x%08X of type %d",
				entity_index,
				entity_type
			);
		}
		else
		{
			event(
				_event_error,
				"simulation:entity: unable to allocate replication instance for new simulation entity (type %d)",
				entity_type
			);
		}
	}
	else
	{
		event(
			_event_error,
			"simulation:entity: unable to allocate memory for new simulation entity (type %d)",
			entity_type
		);
	}

	if (creation_data)
	{
		network_heap_free_block(creation_data);
	}

	if (state_data)
	{
		network_heap_free_block(state_data);
	}

	return entity_index;
}

void c_simulation_entity_database::entity_capture_creation_data(
	int32 entity_index)
{
	bool baseline_valid;

	s_simulation_entity* entity = entity_get(entity_index);

	ASSERT(m_world);
	ASSERT(m_world->is_distributed());
	ASSERT(m_world->is_authority());
	ASSERT(m_type_collection);
	
	c_simulation_entity_definition* entity_definition = m_type_collection->get_entity_definition(entity->entity_type);

	ASSERT(entity_definition);
	ASSERT(entity->creation_data_size==entity_definition->creation_data_size());

	entity_definition->build_creation_data(entity->gamestate_index, entity->creation_data_size, entity->creation_data);

	ASSERT(entity->state_data_size==entity_definition->state_data_size());

	baseline_valid = entity_definition->build_baseline_state_data(entity->creation_data_size, entity->creation_data, entity->state_data_size, entity->state_data);
	
	entity_validate_creation_data(entity_index);

	entity->exists_in_gameworld = true;

	entity->pending_update_mask = MASK(entity_definition->update_flag_count());
	entity->force_update_mask = 0;

	return;
}

void c_simulation_entity_database::entity_delete(
	int32 entity_index)
{
	ASSERT(m_world);
	ASSERT(m_world->is_distributed());
	ASSERT(m_world->is_authority());
	ASSERT(m_entity_manager);
	ASSERT(m_entity_manager->is_entity_local(entity_index));

	return m_entity_manager->delete_local_entity(entity_index);
}

void c_simulation_entity_database::entity_update(
	int32 entity_index,
	uint32 update_mask,
	bool force_update)
{
	s_simulation_entity* entity = entity_get(entity_index);

	ASSERT(update_mask != 0);
	ASSERT(entity->exists_in_gameworld);

	ASSERT(m_world);
	ASSERT(m_world->is_distributed());
	ASSERT(m_world->is_authority());
	ASSERT(m_entity_manager);
	ASSERT(m_entity_manager->is_entity_local(entity_index));
	ASSERT(!m_entity_manager->is_entity_being_deleted(entity_index));

	//c_simulation_entity_definition* entity_definition = m_type_collection->get_entity_definition(entity->entity_type);
	
	// TODO: finish this
	if (/*entity_definition->gameworld_attachment_valid(entity->gamestate_index)*/ true)
	{
		if (simulation_gamestate_entity_get_simulation_entity_index(entity->gamestate_index) != entity->entity_index)
		{
			event(
				_event_error,
				"networking:simulation:entity_database: entity type %d index 0x%8X (!= 0x%08X) not attached properly to gamestate 0x%8X (update)",
				entity->entity_type,
				entity_index,
				entity->gamestate_index
			);
		}
	}
	else
	{
		event(
			_event_error,
			"networking:simulation:entity_database: entity type %d index 0x%8X not attached properly to gamestate 0x%8X (update)",
			entity->entity_type,
			entity_index,
			entity->gamestate_index
		);
	}

	entity->pending_update_mask |= update_mask;

	if (force_update)
	{
		entity->force_update_mask |= update_mask;
	}

	return;
}

void c_simulation_entity_database::notify_mark_entity_for_deletion(
	int32 entity_index)
{
	entity_delete_gameworld(entity_index, false);

	return;
}

/* private code */

void c_simulation_entity_database::entity_create_internal(
	int32 entity_index,
	e_simulation_entity_type entity_type,
	int32 creation_data_size,
	void* creation_data,
	int32 state_data_size,
	void* state_data)
{
	c_simulation_entity_definition* entity_definition = m_type_collection->get_entity_definition(entity_type);

	ASSERT(m_entity_manager);
	ASSERT(m_entity_manager->is_entity_allocated(entity_index));
	ASSERT(entity_definition);
	ASSERT(creation_data_size==entity_definition->creation_data_size());
	ASSERT(creation_data_size==0 || creation_data!=NULL);
	ASSERT(state_data_size==entity_definition->state_data_size());
	ASSERT(state_data!=NULL);

	int32 absolute_index = ENTITY_INDEX_TO_ABSOLUTE_INDEX(entity_index);

	ASSERT(absolute_index>=0 && absolute_index<NUMBEROF(m_entity_data));

	s_simulation_entity* entity = &m_entity_data[absolute_index];

	ASSERT(entity->entity_index==NONE);
	ASSERT(entity->entity_type==NONE);

	entity->entity_index = entity_index;
	entity->entity_type = entity_type;
	entity->exists_in_gameworld = false;
	entity->gamestate_index = NONE;
	entity->pending_update_mask = 0;
	entity->force_update_mask = 0;
	entity->event_reference_count = 0;
	entity->creation_data_size = creation_data_size;
	entity->creation_data = creation_data;
	entity->state_data_size = state_data_size;
	entity->state_data = state_data;

	return;
}

void c_simulation_entity_database::entity_delete_gameworld(
	int32 entity_index,
	bool deletion_from_entity_collision)
{
	s_simulation_entity* entity = entity_get(entity_index);
	if (entity->gamestate_index != NONE)
	{
		simulation_gamestate_entity_set_simulation_entity_index(entity->gamestate_index, NONE);
		simulation_queue_entity_deletion_insert(entity, deletion_from_entity_collision);
	}

	entity->gamestate_index = NONE;
	entity->exists_in_gameworld = false;
	entity->pending_update_mask = 0;
	entity->force_update_mask = 0;

	return;
}

void c_simulation_entity_database::entity_delete_internal(
	int32 entity_index)
{
	s_simulation_entity* entity = entity_get(entity_index);
	
	// Discard creation data
	if (entity->creation_data)
	{
		network_heap_free_block((uint8*)entity->creation_data);
		entity->creation_data = NULL;
		entity->creation_data_size = 0;
	}

	// Discard state data
	if (entity->state_data)
	{
		network_heap_free_block((uint8*)entity->state_data);
		entity->state_data = NULL;
		entity->state_data_size = 0;
	}

	// Clear entity data
	entity->entity_type = k_simulation_entity_type_none;
	entity->entity_index = NONE;

	return;
}

void c_simulation_entity_database::entity_validate_creation_data(
	int32 entity_index) const
{
	if (game_in_progress() && g_simulation_entity_validate)
	{
		s_simulation_entity const* entity = entity_get(entity_index);
		c_simulation_entity_definition* entity_definition = m_type_collection->get_entity_definition(entity->entity_type);
	
		uint8 encoded_buffer[128];
		c_bitstream encoded_stream(encoded_buffer, sizeof(encoded_buffer));
		
		ASSERT(entity_definition);

		encoded_stream.begin_writing(k_bitstream_default_alignment);

		entity_definition->entity_creation_encode(entity->creation_data_size, entity->creation_data, NULL, &encoded_stream, true);

		encoded_stream.finish_writing(NULL);

		if (encoded_stream.begin_consistency_check())
		{
			uint8 decoded_creation_data[128];
			int32 decoded_creation_data_size = entity_definition->creation_data_size();

			ASSERT(decoded_creation_data_size<=sizeof(decoded_creation_data));

			csmemset(decoded_creation_data, 0, decoded_creation_data_size);

			bool decode_success = entity_definition->entity_creation_decode(decoded_creation_data_size, decoded_creation_data, &encoded_stream, true);

			encoded_stream.finish_consistency_check();

			vassert(decode_success, "decode failed on outgoing entity creation", NULL);

			// Compare decoded and input creation data
			if (true)
			{
				uint8 copied_creation_data[128];
				uint8 copied_decoded_creation_data[128];

				ASSERT(entity->creation_data_size<=sizeof(copied_creation_data));
				
				csmemcpy(copied_creation_data, entity->creation_data, entity->creation_data_size);
				csmemcpy(copied_decoded_creation_data, decoded_creation_data, entity->creation_data_size);

				vassert(
					entity_definition->entity_creation_lossy_compare(copied_creation_data, copied_decoded_creation_data, decoded_creation_data_size),
					"decode compare mismatch on outgoing entity creation", NULL);
				vassert(!csmemcmp(copied_creation_data, copied_decoded_creation_data, entity->creation_data_size), "decode memcmp mismatch on outgoing creation state", NULL);
			}
		}
	}

	return;
}

void c_simulation_entity_database::entity_validate_state_data(
	int32 entity_index) const
{
	if (game_in_progress() && g_simulation_entity_validate)
	{
		uint8 encoded_buffer[1024];

		uint32 test_update_mask;
		uint32 wrote_update_mask;

		s_simulation_entity const* entity = entity_get(entity_index);
		c_simulation_entity_definition* entity_definition = m_type_collection->get_entity_definition(entity->entity_type);
		
		c_bitstream encoded_stream(encoded_buffer, sizeof(encoded_buffer));

		ASSERT(entity_definition);

		test_update_mask = MASK(entity_definition->update_flag_count());
		wrote_update_mask = 0;

		encoded_stream.begin_writing(k_bitstream_default_alignment);

		if (!entity_definition->entity_update_encode(false, test_update_mask, &wrote_update_mask, entity->state_data_size, entity->state_data, NULL, &encoded_stream, 0, true))
		{
			vassert(false, "entity update encode overflowed buffer", NULL);
		}

		encoded_stream.finish_writing(NULL);

		if (encoded_stream.begin_consistency_check())
		{
			uint8 decoded_state_data[1024];
			bool decode_success;
			
			int32 decoded_state_data_size = entity_definition->state_data_size();
			uint32 decoded_update_mask = 0;

			ASSERT(decoded_state_data_size > 0 && decoded_state_data_size<=sizeof(decoded_state_data));

			csmemset(decoded_state_data, 0, decoded_state_data_size);
			
			bandwidth_profiler_record_push(5, &encoded_stream);

			decode_success = entity_definition->entity_update_decode(false, &decoded_update_mask, decoded_state_data_size, decoded_state_data, &encoded_stream, true);

			bandwidth_profiler_record_pop(5, &encoded_stream);

			encoded_stream.finish_consistency_check();

			vassert(decode_success, "decode failed on outgoing entity state", NULL);

			// Compare decoded and input state data
			if (true)
			{
				uint8 copied_decoded_state_data[1024];
				uint8 copied_state_data[1024];

				ASSERT(entity->state_data_size > 0 && entity->state_data_size<=sizeof(copied_state_data));

				csmemcpy(copied_state_data, entity->state_data, entity->state_data_size);

				ASSERT(entity->state_data_size<=sizeof(copied_decoded_state_data) && entity->state_data_size<=sizeof(decoded_state_data));

				csmemcpy(copied_decoded_state_data, decoded_state_data, entity->state_data_size);

				vassert(entity_definition->entity_state_lossy_compare(copied_state_data, copied_decoded_state_data, decoded_state_data_size), "decode compare mismatch on outgoing entity state", NULL);
				
				// Don't check this since it'll just break in state data that contains quantized floats
				if (false)
				{
					vassert(!csmemcmp(copied_state_data, decoded_state_data, entity->state_data_size), "decode memcmp mismatch on outgoing entity state", NULL);
				}
			}
		}
	}

	return;
}

bool c_simulation_entity_database::entity_allocate_creation_data(
	e_simulation_entity_type entity_type,
	int32* out_creation_data_size,
	void** out_creation_data) const
{
	c_simulation_entity_definition* entity_definition = m_type_collection->get_entity_definition(entity_type);
	void* creation_data = NULL;
	bool success = true;

	ASSERT(entity_definition);
	ASSERT(out_creation_data_size);
	ASSERT(out_creation_data);

	int32 creation_data_size = entity_definition->creation_data_size();

	ASSERT(creation_data_size>=0 && creation_data_size<=k_simulation_entity_maximum_creation_data_size);

	if (creation_data_size>0)
	{
		creation_data = network_heap_allocate_block(creation_data_size);
		
		if (creation_data)
		{
			csmemset(creation_data, 0, creation_data_size);
		}
		else
		{
#ifdef EVENTS_ENABLED
			char heapbuf[1024];
			event(
				_event_error,
				"simulation:entity: OUT OF MEMORY allocating creation data for new simulation entity (type %d, [%d] bytes) [%s]",
				entity_type,
				creation_data_size,
				network_heap_describe(heapbuf, sizeof(heapbuf))
			);
#endif
			success = false;
		}
	}

	*out_creation_data_size = creation_data_size;
	*out_creation_data = creation_data;

	return success;
}

bool c_simulation_entity_database::entity_allocate_state_data(
	e_simulation_entity_type entity_type,
	int32* out_state_data_size,
	void** out_state_data) const
{
	c_simulation_entity_definition* entity_definition = m_type_collection->get_entity_definition(entity_type);
	void* state_data = NULL;
	bool success = true;

	ASSERT(entity_definition);
	ASSERT(out_state_data_size);
	ASSERT(out_state_data);

	int32 state_data_size = entity_definition->state_data_size();

	ASSERT(state_data_size>=0 && state_data_size<=k_simulation_entity_maximum_state_data_size);

	state_data = (void*)network_heap_allocate_block(state_data_size);

	if (state_data)
	{
		csmemset(state_data, 0, state_data_size);
	}
	else
	{
#ifdef EVENTS_ENABLED
		char heapbuf[1024];
		event(
			_event_error,
			"simulation:entity: OUT OF MEMORY allocating state data for new simulation entity (type %d, [%d] bytes) [%s]",
			entity_type,
			state_data_size,
			network_heap_describe(heapbuf, sizeof(heapbuf))
		);
#endif
		success = false;
	}

	*out_state_data_size = state_data_size;
	*out_state_data = state_data;

	return success;
}
