#pragma once
#include "networking/replication/replication_entity_manager.h"

/* enums */

enum e_entity_creation_block_order
{
	_entity_creation_block_order_simulation_entity_creation= 0,
	_entity_creation_block_order_simulation_entity_state,
	_entity_creation_block_order_gamestate_index,
	_entity_creation_block_order_forward_memory_queue_element,
	k_entity_creation_block_order_count
};

enum e_entity_update_block_order
{
	_entity_update_block_order_simulation_entity_state = 0,
	_entity_update_block_order_forward_memory_queue_element,
	k_entity_update_block_order_count
};

/* classes */

class c_simulation_entity_database : public c_replication_entity_manager_client
{
public:
	virtual bool write_creation_to_packet(
		int32 entity_index,
		uint32 update_mask,
		void const* in_telemetry_data,
		class c_bitstream* packet,
		int32 must_leave_space_bits,
		uint32* out_update_mask);
	virtual e_network_read_result read_creation_from_packet(
		int32 entity_index,
		e_simulation_entity_type* out_entity_type,
		uint32* out_entity_initial_update_mask,
		int32 maximum_block_count,
		int32* block_count,
		struct s_replication_allocation_block* blocks,
		class c_bitstream* packet);
	virtual bool process_creation(int32 entity_index, e_simulation_entity_type entity_type, uint32 update_mask, int32 block_count, s_replication_allocation_block* blocks);
	virtual int32 calculate_creation_requirements(int32 entity_index, uint32 update_mask, void const* in_telemetry_data, real32* priority, int32* fixed_priority);
	virtual void write_creation_description_to_string(int32 entity_index, void* telemetry_data, int32 buffer_size, char* buffer);
	virtual bool write_update_to_packet(
		int32 entity_index,
		uint32 update_mask,
		void const* in_telemetry_data,
		class c_bitstream* packet,
		int32 must_leave_space_bits,
		uint32* out_update_mask);
	virtual e_network_read_result read_update_from_packet(int32 entity_index, uint32* out_update_mask, int32 maximum_block_count, int32* block_count, s_replication_allocation_block* blocks, class c_bitstream* packet);
	virtual void process_update(int32 entity_index, uint32 update_mask, int32 block_count, s_replication_allocation_block* blocks);
	virtual void calculate_update_requirements(int32 entity_index, uint32 a3, uint32 a4, void* a5, real32* priority, uint32* a7);
	virtual void calculate_deletion_requirements(int32 entity_index, int32 a3, real32* requirements);
	virtual void notify_mark_entity_for_deletion(int32 entity_index);
	virtual void notify_entity_collision(int32 old_entity_index, int32 entity_index, e_simulation_entity_type entity_type, const s_replication_allocation_block* blocks);
	virtual void notify_delete_entity(int32 entity_index);
	virtual bool notify_promote_to_authority(int32 entity_index);
	virtual void rotate_entity_seed(void);
	virtual uint32 generate_current_entity_update_mask(int32 entity_index);

	void initialize(class c_simulation_world* world, class c_replication_entity_manager* entity_manager, class c_simulation_type_collection* type_collection);
	void reset(void);
	void destroy(void);
	void process_pending_updates(void);

	s_simulation_entity const* entity_get(int32 entity_index) const;
	s_simulation_entity* entity_get(int32 entity_index);

	char const* get_entity_type_name(e_simulation_entity_type entity_type) const;

	bool entity_is_local(int32 entity_index) const;

	int32 entity_create(e_simulation_entity_type entity_type);
	void entity_capture_creation_data(int32 entity_index);
	void entity_delete(int32 entity_index);
	void entity_update(int32 entity_index, uint32 update_mask, bool force_update);

	s_simulation_entity* entity_try_and_get(int32 entity_index)
	{
		if (entity_index != NONE)
		{
			if (entity_get(entity_index)->entity_index == entity_index)
			{
				return entity_get(entity_index);
			}
		}

		return NULL;
	}

private:
	class c_simulation_type_collection* m_type_collection;
	s_simulation_entity m_entity_data[k_simulation_entity_database_maximum_entities];

	void entity_create_internal(int32 entity_index, e_simulation_entity_type entity_type, int32 creation_data_size, void* creation_data, int32 state_data_size, void* state_data);
	void entity_delete_gameworld(int32 entity_index, bool deletion_from_entity_collision);
	void entity_delete_internal(int32 entity_index);
	void entity_validate_creation_data(int32 entity_index) const;;
	void entity_validate_state_data(int32 entity_index) const;
	bool entity_allocate_creation_data(e_simulation_entity_type entity_type, int32* out_creation_data_size, void** out_creation_data) const;
	bool entity_allocate_state_data(e_simulation_entity_type entity_type, int32* out_state_data_size, void** out_state_data) const;
};
ASSERT_STRUCT_SIZE(c_simulation_entity_database, 36884);

/* prototypes */

void simulation_entity_database_apply_patches(void);
