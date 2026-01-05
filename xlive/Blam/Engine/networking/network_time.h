#pragma once

/* structures */

struct s_network_time_globals
{
	bool network_time_locked;
	int8 pad[3];
	uint32 network_time;
	class c_network_session_manager* session_manager;
};

/* prototypes */

bool network_session_time_get_id_and_time(int32 session_id, struct s_transport_secure_identifier* id, uint32* time);

uint32 __cdecl network_time_get_exact(void);

void network_session_time_register_session_manager(class c_network_session_manager* session_manager);
