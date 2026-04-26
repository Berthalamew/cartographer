#include "stdafx.h"
#include "rasterizer_profile.h"

#ifdef RASTERIZER_PROFILE_ENABLED

/* constants */

enum
{
	k_maxmium_profile_warning_count = 3,
};

/* structures */

// TODO: finish reversing
struct s_rasterizer_profile_globals
{
	int32 field_0;
	int32 rasterizer_artist_profile;
	int32 field_8;
	int32 frame_index;
	int32 field_10;
	int32 field_14;
	int32 flags;
	int32 index;
	int32 field_20;
	int32 field_24;
	int8 gap1[463000];
	bool wait_in_progress;
	int8 pad[3];
	int32 field_710C4;
	int64 field_710C8;
	int8 gap2[88];
};

/* prototypes */

static void rasterizer_profile_warning(bool condition, int32 index, char const* message);

/* globals */

int32 g_rasterizer_artist_profile;
int32 g_rasterizer_profile_warning_count;

static s_rasterizer_profile_globals rasterizer_profile_globals;

/* public code */

bool rasterizer_profile_initialize(void)
{
	// TODO: implement

	return true;
}

void rasterizer_profile_frame_begin(void)
{
	// TODO: implement

	return;
}

/* private code */

static void rasterizer_profile_warning(
	bool condition,
	int32 index,
	char const* message)
{
	ASSERT(message);

	if (!condition)
	{
		if (g_rasterizer_profile_warning_count<k_maxmium_profile_warning_count)
		{
			char string[256];

			if (index!=NONE)
			{
				csprintf(string, NUMBEROF(string), "%s (index=%d, frame=%d)", message, index&SHORT_MAX, (index & 2147450880) >> 15);
			}
			else
			{
				csstrncpy(string, message, NUMBEROF(string));
			}
				
			error(_error_delayed, "### RASTERIZER PROFILE: %s", string);
			
			++g_rasterizer_profile_warning_count;
		}
	}

	return;
}

#endif
