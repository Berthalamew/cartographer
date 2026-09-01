#include "stdafx.h"
#include "simulation_priority.h"

/* public code */

real32 __cdecl simulation_calculate_entity_creation_priority(
	struct s_simulation_entity const* entity,
	struct s_simulation_view_telemetry_data const* telemetry_data,
	real32* out_relevance)
{
    ASSERT(entity);
    ASSERT(telemetry_data);
    
    return INVOKE(0x1F5CFF, 0x0, simulation_calculate_entity_creation_priority, entity, telemetry_data, out_relevance);
}
