#pragma once
#include "game_interface/simulation_game_events.h"
#include "networking/network_constants.h"

class c_replication_event_manager_client
{
public:
    virtual e_network_read_result read_incoming_event(
        int32 event_type,
        int32 const* entity_reference_indices,
        int32 maximum_block_count,
        int32* block_count,
        struct s_replication_allocation_block* blocks,
        class c_bitstream* packet) = 0;
    virtual void process_incoming_event(e_simulation_event_type simulation_event_type, int32* entity_reference_indices, int32 block_count, struct s_replication_allocation_block* block) = 0;
    virtual void write_outgoing_event(int32 a2, int32 a3, uint32* a4, int8* a5) = 0;
    virtual void notify_outgoing_event_retired(int32 a2) = 0;
    virtual int8 calculate_requirements(int32 a2, uint32** a3, real32* a4, uint32* a5) = 0;
    virtual int32 write_description_to_string(int32 a2, int32 a3, int32 a4, int32 a5) = 0;
};

class c_simulation_event_handler : c_replication_event_manager_client
{
public:
    void initialize(
        class c_simulation_world* world,
        class c_replication_event_manager* event_manager,
        class c_simulation_type_collection* type_collection,
        class c_simulation_entity_database* entity_database);

    void destroy(void);

    e_network_read_result read_incoming_event(
        int32 event_type,
        int32 const* entity_reference_indices,
        int32 maximum_block_count,
        int32* block_count,
        struct s_replication_allocation_block* blocks,
        class c_bitstream* packet) override;
    void process_incoming_event(e_simulation_event_type simulation_event_type, int32* entity_reference_indices, int32 block_count, struct s_replication_allocation_block* payload_block) override;
    void write_outgoing_event(int32 a2, int32 a3, uint32* a4, int8* a5) override;
    void notify_outgoing_event_retired(int32 a2) override;
    int8 calculate_requirements(int32 a2, uint32** a3, real32* a4, uint32* a5) override;
    int32 write_description_to_string(int32 a2, int32 a3, int32 a4, int32 a5) override;

	void reset()
    {
    }

private:
    bool m_initialized;
    class c_simulation_world* m_world;
    class c_replication_event_manager* m_event_manager;
    class c_simulation_type_collection* m_type_collection;
    class c_simulation_entity_database* m_entity_database;
};

/* prototypes */

void simulation_event_handler_apply_patches();