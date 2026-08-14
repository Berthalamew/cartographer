#include "stdafx.h"
#include "player_mapping.h"

#include "players.h"
#include "player_constants.h"

/* public code */

int32 player_mapping_first_active_output_user(void)
{
    datum* mapping = player_user_mapping_get();
    int32 result = 0;

    for (size_t i = 0; mapping[i] == NONE; ++i)
    {
        if (i + 1 >= k_number_of_users)
        {
            result = NONE;
            break;
        }
    }

    return result;
}
