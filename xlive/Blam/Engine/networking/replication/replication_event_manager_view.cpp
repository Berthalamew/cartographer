#include "stdafx.h"
#include "replication_event_manager_view.h"

#include "replication_entity.h"
#include "replication_event_manager.h"

#include "memory/bitstream.h"
#include "networking/network_event.h"

/* structures */

struct s_replication_event_request_data
{
    int32 entity_reference_indices[2];
};

/* prototypes */

CLASS_HOOK_DECLARE_LABEL(c_replication_event_manager_view__read_from_packet, c_replication_event_manager_view::read_from_packet);
static __declspec(naked) void jmp_c_replication_event_manager_view__read_from_packet(void)
{
    CLASS_HOOK_JMP(c_replication_event_manager_view__read_from_packet, c_replication_event_manager_view::read_from_packet);
}

/* public code */

void replication_event_manager_view_apply_patches(
    void)
{
    WritePointer(Memory::GetAddress(0x3C61E4), jmp_c_replication_event_manager_view__read_from_packet);

    return;
}

void c_replication_event_manager_view::write_to_packet(
	void* request_identifier,
	int32 request_type,
	void const* telemetry_data,
	int32 packet_sequence_number,
	class c_bitstream* packet,
	int32 must_leave_space_bits)
{
    /*
    c_packet_record* packet_record;

    {
        char heapbuf[1024];
        {
            c_event local_event;
            static long volatile x_event_category_index; // 0x18275E000
            long local_event_category_index;
            {
                long generated_event_category_index;
            }
        }
    }
    {
        c_replication_outgoing_event* event;
        c_event_record* outgoing_event_record;
        {
            long entity_reference;
            {
                long entity_index;
            }
        }
        {
            char heapbuf[1024];
            {
                c_event local_event;
                static long volatile x_event_category_index; // 0x18275E020
                long local_event_category_index;
                {
                    long generated_event_category_index;
                }
            }
        }
    }
    */
	return;
}

e_network_read_result c_replication_event_manager_view::read_from_packet(
    int32 packet_sequence_number,
    c_bitstream* packet,
    int32 maximum_number_of_requests,
    s_replication_incoming_request* requests,
    int32* out_number_of_requests)
{
    e_network_read_result result = _network_read_result_ok;
    int32 number_of_requests = 0;

    ASSERT(m_initialized);

    if (m_fatal_error)
    {
        result = _network_read_result_discard;
    }

    while (result == _network_read_result_ok && packet->read_bool("event-exists"))
    {
        int32 event_type;
        
        packet->push_structure("entity-control", NONE, 0);
        
        if (number_of_requests >= maximum_number_of_requests)
        {
            event(_event_error, "networking:replication:event:read_from_packet: ran out of requests");
            result = _network_read_result_corrupt;
            break;
        }
        
        event_type = packet->read_integer("event-type", 6);

        // TODO: Not sure what the 32 here is
        if (VALID_INDEX(event_type, 32))
        {
            s_replication_event_request_data request_data;
            s_replication_allocation_block event_blocks[1];
            
            int32 event_block_count = 0;

            for (int32 reference_num=0; reference_num<NUMBEROF(request_data.entity_reference_indices); ++reference_num)
            {
                if (packet->read_bool("entity-reference-exists"))
                {
                    replication_entity_index_decode(packet, &request_data.entity_reference_indices[reference_num]);
                }
                else
                {
                    request_data.entity_reference_indices[reference_num] = NONE;
                }
            }
            
            result = m_event_manager->read_incoming_event(event_type, request_data.entity_reference_indices, NUMBEROF(event_blocks), &event_block_count, event_blocks, packet);

            if (result == _network_read_result_ok)
            {

                 s_replication_incoming_request* request = &requests[number_of_requests++];

                 ASSERT(number_of_requests<=maximum_number_of_requests);

                 request->request_identifier = NONE;
                 request->request_type = event_type;
                 request->block_count = event_block_count;

                 if (event_block_count>0)
                 {
                     ASSERT(event_block_count<=NUMBEROF(request->blocks));

                     csmemcpy(request->blocks, event_blocks, sizeof(request->blocks)*event_block_count);
                 }

                 csmemcpy(request->storage, &request_data, sizeof(request_data));
            }
        }
        else
        {
            event(_event_error, "networking:replication:event:read_from_packet: unknown event type %d", event_type);
        }
    }

    *out_number_of_requests = number_of_requests;
    return result;
}
