#pragma once
#include "objects/object_definition.h"


/*********************************************************************
* name: scenery
* group_tag : scen
* header size : 196
* *********************************************************************/

#pragma pack(push,1)
struct s_scenery_group_definition
{
	object_definition objectTag;
	enum class e_pathfinding_policy : __int16
	{
		pathfinding_cutout = 0,
		pathfinding_static = 1,
		pathfinding_dynamic = 2,
		pathfinding_none = 3,
	};
	e_pathfinding_policy pathfinding_policy;//0xBC
	enum class e_flags : __int16
	{
		physically_simulates = FLAG(0),
	};
	e_flags flags;//0xBE
	enum class e_lightmapping_policy : __int16
	{
		pervertex = 0,
		perpixel_not_implemented = 1,
		dynamic = 2,
	};
	e_lightmapping_policy lightmapping_policy;//0xC0
	PAD(0x2);//0xC2
};
ASSERT_STRUCT_SIZE(s_scenery_group_definition, 0xC4);
#pragma pack(pop)

