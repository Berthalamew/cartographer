#pragma once

/* prototypes */

real32 __cdecl simulation_calculate_entity_creation_priority(struct s_simulation_entity const* entity, struct s_simulation_view_telemetry_data const* telemetry_data, real32* out_relevance);

