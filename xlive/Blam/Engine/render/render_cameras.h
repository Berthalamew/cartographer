#pragma once
#include "Blam/Engine/camera/camera.h"
#include "Blam/Engine/math/matrix_math.h"

void __cdecl render_camera_build_projection_hook(s_camera* camera, float* frustum_bounds, real_matrix4x3* out_projection);
void render_cameras_apply_patches();