#include "stdafx.h"
#include "simulation_game_statborg.h"

/* prototypes */

CLASS_HOOK_DECLARE_LABEL(c_simulation_game_statborg_entity_definition__entity_creation_encode, c_simulation_game_statborg_entity_definition::entity_creation_encode);
static void __declspec(naked) jmp_c_simulation_game_statborg_entity_definition__entity_creation_encode(void)
{
    CLASS_HOOK_JMP(c_simulation_game_statborg_entity_definition__entity_creation_encode, c_simulation_game_statborg_entity_definition::entity_creation_encode);
}

CLASS_HOOK_DECLARE_LABEL(c_simulation_game_statborg_entity_definition__entity_creation_decode, c_simulation_game_statborg_entity_definition::entity_creation_decode);
static void __declspec(naked) jmp_c_simulation_game_statborg_entity_definition__entity_creation_decode(void)
{
    CLASS_HOOK_JMP(c_simulation_game_statborg_entity_definition__entity_creation_decode, c_simulation_game_statborg_entity_definition::entity_creation_decode);
}

/* public code */

void simulation_game_statborg_apply_patches(
    void)
{
    WritePointer(Memory::GetAddress(0x3CA77C, 0x0), jmp_c_simulation_game_statborg_entity_definition__entity_creation_encode);
    WritePointer(Memory::GetAddress(0x3CA780, 0x0), jmp_c_simulation_game_statborg_entity_definition__entity_creation_decode);

    return;
}

void c_simulation_game_statborg_entity_definition::entity_creation_encode(
    int32 creation_data_size,
    void const* creation_data,
    struct s_simulation_view_telemetry_data const* telemetry_data,
    class c_bitstream* packet,
    bool encode_for_network)
{
    ASSERT(creation_data_size==0);

    return;
}

bool c_simulation_game_statborg_entity_definition::entity_creation_decode(
    int32 creation_data_size,
    void* creation_data,
    class c_bitstream* packet,
    bool decode_for_network)
{
    ASSERT(creation_data_size==0);

    return true;
}
