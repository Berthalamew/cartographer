#pragma once
#include "Blam/Engine/objects/damage.h"
#include "Blam/Engine/math/color_math.h"

datum __cdecl effect_new_from_object(
    datum effect_tag_index,
    s_damage_owner* damage_owner,
    datum object_index,
    real32 a4,
    real32 a5,
    real_rgb_color* color,
    const void* effect_vector_field);