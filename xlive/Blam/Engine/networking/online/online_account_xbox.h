#pragma once

/* macros */

#define ONLINE_USER_VALID(user) ((user)!=0)

/* public code */

bool online_xuid_is_guest_account(struct s_player_identifier* player_identifier);

uint8 online_xuid_get_guest_account_number(struct s_player_identifier* player_identifier);

bool __cdecl online_connected_to_xbox_live();

void online_account_transition_to_offline(int32 user_index);
