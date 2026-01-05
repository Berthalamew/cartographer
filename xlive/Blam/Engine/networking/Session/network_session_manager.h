#pragma once
#include "networking/network_constants.h"

class c_network_session_manager
{
	class c_network_session* m_sessions[k_network_maximum_sessions];

public:
	bool initialize_session_manager(void);
	class c_network_session* get_session(uint32 session_index) const;
	class c_network_session* get_session(const struct s_transport_secure_identifier* target_id) const;
};