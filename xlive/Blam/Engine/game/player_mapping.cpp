#include "stdafx.h"
#include "player_mapping.h"

#include "players.h"

/* public code */

int32 player_mapping_first_active_output_user(void)
{
    int32* mapping = player_user_mapping_get();
    int32 result = 0;

    for (size_t i = 0; mapping[i] == NONE; ++i)
    {
        if (i + 1 >= NUMBEROF(mapping))
        {
            result = NONE;
            break;
        }
    }

    return result;
}
