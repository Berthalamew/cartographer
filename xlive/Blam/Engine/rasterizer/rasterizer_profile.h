#pragma once

#ifdef RASTERIZER_PROFILE_ENABLED

/* prototypes */

bool rasterizer_profile_initialize(void);

void rasterizer_profile_frame_begin(void);

/* globals */

extern int32 g_rasterizer_artist_profile;
extern int32 g_rasterizer_profile_warning_count;

#endif
