#pragma once
#include "leaf_map.h"

enum
{
	MAXIMUM_SURFACE_REFERENCES_PER_STRUCTURE = 262144,
	MAXIMUM_NODES_PER_BSP2D = 131072,
	MAXIMUM_BSPS_PER_COLLISION_REGION = 64,
	MAXIMUM_WEATHER_PALETTE_ENTRIES_PER_STRUCTURE = 32,
};

// max count: MAXIMUM_SURFACE_REFERENCES_PER_STRUCTURE
struct structure_surface_reference
{
	uint16 strip_index;
	uint16 lightmap_triangle_index;
	uint32 bsp_node_index;
};
ASSERT_STRUCT_SIZE(structure_surface_reference, 8);

// max count: MAXIMUM_BSPS_PER_COLLISION_REGION
struct node_render_leaves
{
	tag_block<structure_leaf> collision_leaves;
	tag_block<structure_surface_reference> surface_references;
};
ASSERT_STRUCT_SIZE(node_render_leaves, 16);

// max count: MAXIMUM_SURFACES_PER_COLLISION_BSP 131072
struct bsp2d_ref
{
	int32 node_ref_or_sector_ref;
};
ASSERT_STRUCT_SIZE(bsp2d_ref, 4);

// max count: MAXIMUM_NODES_PER_BSP2D 131072
struct large_bsp2d_node
{
	real_plane2d plane;
	int32 left_child;
	int32 right_child;
};
ASSERT_STRUCT_SIZE(large_bsp2d_node, 20);

#pragma pack(push, 1)
struct s_structure_globals
{
	bool cluster_marker_initialized;
	int8 pad[2];
	int32 cluster_index;
	int32 cluster_instances[512];
	bool instanced_geometry_marker_initialized;
	int8 pad1[3];
	int32 instanced_geometry_index;
	int32 instanced_geometry_instances[1024];
};
#pragma pack(pop)
ASSERT_STRUCT_SIZE(s_structure_globals, 6159);

s_structure_globals* structure_globals_get(void);

// CLIENT ONLY
// Get index of sky block in the scenario that's currently visible
// Grabs the clear color and sets a bool if the sky has a clear color 
bool __cdecl structure_get_sky(int32 cluster_index, int32* visible_sky_index, real_rgb_color* clear_color, bool* has_clear_color);
