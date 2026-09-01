#include "stdafx.h"
#include "replication_scheduler.h"

#include "memory/bitstream.h"
#include "networking/network_memory.h"
#include "simulation/simulation.h"
#include "simulation/simulation_gamestate_entities.h"
#include "simulation/simulation_queue.h"
#include "simulation/simulation_world.h"

/* prototypes */

CLASS_HOOK_DECLARE_LABEL(c_replication_scheduler__read_from_packet, c_replication_scheduler::read_from_packet);
static __declspec(naked) void jmp_c_replication_scheduler__read_from_packet(void)
{
    CLASS_HOOK_JMP(c_replication_scheduler__read_from_packet, c_replication_scheduler::read_from_packet);
}

/* public code */

void replication_scheduler_apply_patches(
    void)
{
    WritePointer(Memory::GetAddress(0x3C61C0), jmp_c_replication_scheduler__read_from_packet);

    return;
}

e_network_read_result c_replication_scheduler::read_from_packet(
    int32* packet_sequence_number,
    c_bitstream* packet)
{
    int32 client_first_request[4];
    s_replication_incoming_request requests[256];

    int32 number_of_requests = 0;
    e_network_read_result read_result = _network_read_result_ok;

    ASSERT(m_initialized);

    packet->push_structure("replication-scheduler", NONE, 0);

    for (int32 client_index = 0; client_index<NUMBEROF(m_clients); ++client_index)
    {
        c_replication_scheduler_client* client = m_clients[client_index];
        client_first_request[client_index] = number_of_requests;

        if (client && read_result==_network_read_result_ok)
        {
            int32 client_number_of_requests= 0;

            packet->push_structure("client-payload", NONE, 0);

            read_result= client->read_from_packet(*packet_sequence_number, packet, NUMBEROF(requests)-number_of_requests, &requests[number_of_requests], &client_number_of_requests);

            if (read_result==_network_read_result_ok)
            {
                packet->pop_structure("client-payload", NONE);
            }

            ASSERT(number_of_requests<=NUMBEROF(requests));
        }
    }
        
    client_first_request[NUMBEROF(client_first_request)-1] = number_of_requests;

    if (read_result==_network_read_result_ok)
    {
        packet->pop_structure("replication-scheduler", NONE);

        if (packet->read_only_for_consistency())
        {
            for (int32 client_index = 0; client_index<NUMBEROF(m_clients); ++client_index)
            {
                c_replication_scheduler_client* client = m_clients[client_index];

                int32 request_index = client_first_request[client_index];
                int32 sentinel_request_index = client_first_request[client_index+1];

                while (request_index < sentinel_request_index)
                {
                    ASSERT(client!=NULL);

                    client->process_incoming_request(&requests[request_index++]);
                }
            }

        }
    }
        
    for (int32 request_index = 0; request_index<number_of_requests; ++request_index)
    {
        s_replication_incoming_request* request = &requests[request_index];
            
        for (int32 block_index= 0; block_index<request->block_count; ++block_index)
        {
            if (!packet->read_only_for_consistency())
            {
                network_heap_free_block(request->blocks[block_index].block_data);
            }
            else
            {
                if (request->blocks[block_index].block_type==_network_memory_block_forward_simulation_queue_element)
                {
                    s_simulation_queue_element* simulation_queue_element = *(s_simulation_queue_element**)request->blocks[block_index].block_data;
                    c_simulation_world* world = simulation_get_world();

                    world->simulation_queue_free(simulation_queue_element);
                }
                else if (request->blocks[block_index].block_type==_network_memory_block_forward_gamestate_element)
                {
                    int32 gamestate_index = *(int32*)request->blocks[block_index].block_data;

                    simulation_gamestate_entity_delete(gamestate_index);
                }

                network_heap_free_block(request->blocks[block_index].block_data);
            }
                
            request->blocks[block_index].block_data= NULL;
        }
    }

    return read_result;
}
