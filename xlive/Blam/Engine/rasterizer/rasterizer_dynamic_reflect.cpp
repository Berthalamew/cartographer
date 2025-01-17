#include "stdafx.h"
#include "rasterizer_dynamic_reflect.h"

/* public code */

bool __cdecl rasterizer_dynamic_reflect_initialize(void)
{
	return INVOKE(0x2788A0, 0x0, rasterizer_dynamic_reflect_initialize);
}

void __cdecl rasterizer_dynamic_reflect_set_camera_rotation(int32 face_index, int32 resolution, const real_point3d* location, render_camera* camera)
{
	INVOKE(0x2788CB, 0x0, rasterizer_dynamic_reflect_set_camera_rotation, face_index, resolution, location, camera);
	return;
}
