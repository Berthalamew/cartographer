#pragma once

#include "tag_files/string_id.h"

#define MAXIMUM_POSTURES_PER_UNIT 20

// max count: MAXIMUM_POSTURES_PER_UNIT 20
struct s_posture_definition
{
    string_id name;
    real_vector3d pill_offset;
};
ASSERT_STRUCT_SIZE(s_posture_definition, 16);

