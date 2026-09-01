#include "stdafx.h"
#include "simulation_view_telemetry.h"

#include "simulation_view.h"

/* public code */

bool c_simulation_view_telemetry_provider::entity_is_active(
	int32 entity_index) const
{
	c_simulation_distributed_view const* distributed_view = m_view->get_distributed_view();

	return distributed_view->m_entity_view.entity_is_active(entity_index);
}
