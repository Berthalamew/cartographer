#pragma once

/* prototypes */

void rasterizer_occlusion_initialize(void);

void submit_occlusion_tests(bool lens_flare_occlusion_test);

void __cdecl rasterizer_occlusion_submit(void);

void __cdecl rasterizer_occlusion_begin_for_new_frame(void);
