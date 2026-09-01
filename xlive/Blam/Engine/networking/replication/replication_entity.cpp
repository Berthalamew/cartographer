#include "stdafx.h"
#include "replication_entity.h"

#include "memory/bitstream.h"
#include "simulation/game_interface/simulation_game_entities.h"

/* public code */

void replication_entity_index_encode(
	c_bitstream* packet,
	int32 entity_index)
{
	int32 absolute_index = ENTITY_INDEX_TO_ABSOLUTE_INDEX(entity_index);
	uint8 seed = ENTITY_INDEX_TO_SEED(entity_index);

	ASSERT(packet);
	ASSERT(entity_index!=NONE);

	ASSERT(absolute_index>=0 && absolute_index<k_replication_entity_table_length);
	ASSERT(seed>=0 && seed<(1<<k_replication_entity_seed_bits));

	packet->write_integer("entity-absolute-index", absolute_index, k_replication_entity_absolute_index_bits);
	packet->write_integer("entity-seed", seed, k_replication_entity_seed_bits);

	return;
}

void replication_entity_index_decode(
	c_bitstream* packet,
	int32* entity_index)
{
	ASSERT(packet);
	ASSERT(entity_index);

	uint32 entity_abs_index = packet->read_integer("entity-absolute-index", k_replication_entity_absolute_index_bits);
	uint8 seed = (uint8)packet->read_integer("entity-seed", k_replication_entity_seed_bits);

	*entity_index = ENTITY_INDEX_NEW(entity_abs_index, seed);

	return;
}
