#pragma once
#include "simulation_game_entities.h"

/* classes */

class c_simulation_game_statborg_entity_definition : public c_simulation_entity_definition
{
    virtual void entity_creation_encode(
        int32 creation_data_size,
        void const* creation_data,
        struct s_simulation_view_telemetry_data const* telemetry_data,
        class c_bitstream* packet,
        bool encode_for_network) override;
    virtual bool entity_creation_decode(
        int32 creation_data_size,
        void* creation_data,
        class c_bitstream* packet,
        bool decode_for_network) override;
};

/* prototypes */

void simulation_game_statborg_apply_patches(void);
