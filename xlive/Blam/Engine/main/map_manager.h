#pragma once
#include "math/crypto_windows.h"

/* constants */

enum
{
	k_custom_map_name_length = 32,
};

/* structures */

struct s_secure_map_id
{
	wchar_t map_name[k_custom_map_name_length];
	uint8 hash[k_sha256_hash_size_bytes];
};

