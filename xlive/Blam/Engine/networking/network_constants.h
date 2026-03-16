#pragma once
#include "game/game.h"

/* constants */

enum
{
	k_network_maximum_sessions = 2,

	k_network_maximum_machines_per_session = k_maximum_players + 1,

	k_entity_reference_indices_count_max = 2,

	k_network_maximum_views_per_simulation = 16,

	k_network_maximum_players_per_session = k_maximum_players,

	k_network_maximum_actors_per_simulation = 16,
	k_network_actor_index_bit_count = 4,	// 4 bits for 16 (k_network_maximum_actors_per_simulation)

};

/* enums */

enum e_network_read_result
{
	_network_read_result_ok = 0,
	_network_read_result_premature_stop,
	_network_read_result_discard,
	_network_read_result_corrupt,
	k_network_read_result_count,
};
