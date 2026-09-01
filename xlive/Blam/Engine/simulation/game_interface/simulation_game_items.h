#pragma once
#include "simulation_game_objects.h"

/* structures */

struct s_simulation_item_creation_data
{
	s_simulation_object_creation_data object;
};
ASSERT_STRUCT_SIZE(s_simulation_item_creation_data, 16);

/* classes */

class c_simulation_item_entity_definition : public c_simulation_object_entity_definition
{
public:
	virtual void build_object_creation_data(int32 item_index, int32 creation_data_size, void* creation_data) override;

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

void simulation_game_items_apply_patches(void);
