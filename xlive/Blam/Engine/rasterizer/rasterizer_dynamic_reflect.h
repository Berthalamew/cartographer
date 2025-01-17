#pragma once
#include "render/render_cameras.h"

/* prototypes */

bool __cdecl rasterizer_dynamic_reflect_initialize(void);

void __cdecl rasterizer_dynamic_reflect_set_camera_rotation(int32 face_index, int32 resolution, const real_point3d* location, render_camera* camera);
