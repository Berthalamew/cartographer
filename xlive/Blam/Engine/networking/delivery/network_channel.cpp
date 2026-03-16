#include "stdafx.h"
#include "network_channel.h"

/* public code */

bool c_network_channel::get_network_address(
	transport_address* address_out) const
{
	bool result = false;
	if (channel_state >= _network_channel_state_2)
	{
		csmemcpy(address_out, &address, sizeof(transport_address));
		result = true;
	}

	return result;
}

int32 c_network_channel::get_message_space_available(
	void) const
{
	return INVOKE_TYPE(0x1BB531, 0x0, int32(__thiscall*)(const c_network_channel*), this);
}

void c_network_channel_simulation_interface::set_established(
	bool established)
{
	ASSERT(m_initialized);
	m_established = established;

	return;
}

bool c_network_channel_simulation_interface::established(
	void) const
{
	ASSERT(m_initialized);

	return m_established;
}
