#pragma once
#include "replication_entity.h"

#include "game/game.h"
#include "networking/network_constants.h"
#include "simulation/game_interface/simulation_game_entities.h"

/* classes */

class c_replication_entity_manager
{
public:
	void initialize(void);
	void destroy(void);

	void reset(void);
	void attach_client(class c_simulation_entity_database* client);
	void detach_client(class c_simulation_entity_database* client);
	int32 create_local_entity(void);
	void delete_local_entity(int32 entity_index);
	void set_entity_dirty(int32 entity_index, uint32 update_mask);

	bool is_entity_allocated(
		int32 entity_index)
	{
		return try_and_get_entity(entity_index)!=NULL;
	}

	bool is_entity_local(
		int32 entity_index) const
	{
		s_replication_entity_data const* entity = get_entity(entity_index);

		return TEST_BIT(entity->flags, _replication_entity_local_flag);
	}

	bool is_entity_being_deleted(
		int32 entity_index) const
	{
		s_replication_entity_data const* entity = get_entity(entity_index);

		return TEST_BIT(entity->flags, _replication_entity_marked_for_deletion_flag);
	}

private: 
	friend class c_replication_entity_manager_view;

	class c_simulation_entity_database* m_client;
	class c_replication_entity_manager_view* m_views[k_maximum_players];
	uint32 m_view_mask;
	s_replication_entity_data m_entity_data[k_replication_entity_table_length];
	int32 m_entity_creation_start_position;

	int32 preallocate_entity(void);
	bool write_creation_to_packet(
		int32 entity_index,
		uint32 update_mask,
		void const* telemetry_data,
		class c_bitstream* packet,
		int32 must_leave_space_bits,
		uint32* out_update_mask);

	int32 create_local_entity_internal(int32 absolute_index);
	void delete_entity_internal(int32 entity_index);
	

	s_replication_entity_data* get_entity(
		int32 entity_index) const
	{
		int32 absolute_index = ENTITY_INDEX_TO_ABSOLUTE_INDEX(entity_index);
		s_replication_entity_data const* entity = &m_entity_data[absolute_index];

		ASSERT(entity_index != NONE);
		ASSERT(absolute_index>=0 && absolute_index<NUMBEROF(m_entity_data));
		ASSERT(TEST_BIT(entity->flags, _replication_entity_allocated_flag));
		ASSERT(entity->seed == ENTITY_INDEX_TO_SEED(entity_index));

		return (s_replication_entity_data*)entity;
	}

	s_replication_entity_data* try_and_get_entity(
		int32 entity_index) const
	{
		s_replication_entity_data const* entity = NULL;
		int32 absolute_index = ENTITY_INDEX_TO_ABSOLUTE_INDEX(entity_index);
		
		if (absolute_index < NUMBEROF(m_entity_data))
		{
			s_replication_entity_data const* test_entity = &m_entity_data[absolute_index];

			if (TEST_BIT(test_entity->flags, _replication_entity_allocated_flag) &&
				test_entity->seed == ENTITY_INDEX_TO_SEED(entity_index))
			{
				entity = test_entity;
			}
		}

		return (s_replication_entity_data*)entity;
	}

};

class c_replication_entity_manager_client
{
public:
	virtual bool write_creation_to_packet(int32 entity_index, uint32 update_mask, const void* telemetry_data, class c_bitstream* packet, int32 required_leave_space_bits, uint32* out_update_mask) = 0;
	virtual e_network_read_result read_creation_from_packet(int32 entity_index, e_simulation_entity_type* simulation_entity_type, uint32* out_update_mask, int32 a5, int32* block_count, struct s_replication_allocation_block* a7, c_bitstream* packet) = 0;
	virtual bool process_creation(int32 entity_index, e_simulation_entity_type type, uint32 update_mask, int32 block_count, struct s_replication_allocation_block* blocks) = 0;
	virtual int32 calculate_creation_requirements(int32 entity_index, uint32 update_mask, void const* in_telemetry_data, real32* priority, int32* fixed_priority) = 0;
	virtual void write_creation_description_to_string(int32 entity_index, void* telemetry_data, int32 buffer_size, char* buffer) = 0;
	virtual bool write_update_to_packet(
		int32 entity_index,
		uint32 update_mask,
		void const* in_telemetry_data,
		c_bitstream* packet,
		int32 must_leave_space_bits,
		uint32* out_update_mask) = 0;
	virtual e_network_read_result read_update_from_packet(int32 entity_index, uint32* out_update_mask, int32 maximum_block_count, int32* block_count, struct s_replication_allocation_block* blocks, class c_bitstream* packet) = 0;
	virtual void process_update(int32 entity_index, uint32 update_mask, int32 block_count, struct s_replication_allocation_block* blocks) = 0;
	virtual void calculate_update_requirements(int32 entity_index, uint32 a3, uint32 a4, void* a5, real32* priority, uint32* a7) = 0;
	virtual void calculate_deletion_requirements(int32 entity_index, int32 a3, real32* requirements) = 0;
	virtual void notify_mark_entity_for_deletion(int32 entity_index) = 0;
	virtual void notify_entity_collision(int32 old_entity_index, int32 entity_index, e_simulation_entity_type entity_type, const struct s_replication_allocation_block* blocks) = 0;
	virtual void notify_delete_entity(int32 entity_index) = 0;
	virtual bool notify_promote_to_authority(int32 entity_index) = 0;
	virtual void rotate_entity_seed(void) = 0;
	virtual uint32 generate_current_entity_update_mask(int32 entity_index) = 0;

protected:
	bool m_initialized;
	bool m_resetting;
	class c_simulation_world* m_world;
	class c_replication_entity_manager* m_entity_manager;
};
ASSERT_STRUCT_SIZE(c_replication_entity_manager_client, 16);

/* prototypes */

void replication_entity_manager_apply_patches(void);
