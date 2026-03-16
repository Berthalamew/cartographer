#pragma once
#include "game/game_results.h"

/* structures */

struct s_network_message_distributed_game_results
{
	int32 establishment_identifier;
	int32 update_number;
	s_game_results_incremental_update incremental_update;
};
ASSERT_STRUCT_SIZE(s_network_message_distributed_game_results, 20164);
