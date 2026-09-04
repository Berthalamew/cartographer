#pragma once
#include "tag_groups.h"

// ### TODO Rename to c_tag_block and refactor

template<typename T = void>
struct tag_block
{
	int32 count;
	int32 data;

	int32 data_size() const
	{
		return count * type_size();
	}
	int32 type_size() const
	{
		return sizeof(T);
	}

	inline T* operator[](int32 index) const
	{
		return TAG_BLOCK_GET_ELEMENT((s_tag_block*)this, index, T);
	}
};
ASSERT_STRUCT_SIZE(tag_block<>, 8);
