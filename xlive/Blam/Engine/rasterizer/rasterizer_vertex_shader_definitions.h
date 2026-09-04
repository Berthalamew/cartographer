#pragma once
#include "tag_files/tag_block.h"

/* enums */

enum e_global_vertex_shader : int32
{
	_global_vertex_shader_bloom = 0,
	_global_vertex_shader_convolution,
	_global_vertex_shader_debug,
	_global_vertex_shader_decal,
	_global_vertex_shader_effect,
	_global_vertex_shader_effect_billboarded,
	_global_vertex_shader_effect_parallel,
	_global_vertex_shader_effect_parallel_pushed,
	_global_vertex_shader_effect_perpendicular,
	_global_vertex_shader_filter_copy,
	_global_vertex_shader_lens_flare,
	_global_vertex_shader_lens_flare_horrible_legacy,
	_global_vertex_shader_screen,
	_global_vertex_shader_screen_old,
	_global_vertex_shader_screen2_old,
	_global_vertex_shader_stencil_debug_view,
	_global_vertex_shader_screen_xform_4,
	_global_vertex_shader_stencil_shadow_cap_proj,
	_global_vertex_shader_stencil_shadow_stitch_no_proj,
	_global_vertex_shader_stencil_shadow_stitch_proj,
	_global_vertex_shader_screen_hud,
	_global_vertex_shader_weather_plate,
	_global_vertex_shader_weather_particle,
	_global_vertex_shader_decal_hardcoded,
	_global_vertex_shader_screen_0_stage,
	_global_vertex_shader_screen_1_stage,
	_global_vertex_shader_screen_2_stage,
	_global_vertex_shader_screen_3_stage,
	_global_vertex_shader_screen_4_stage,
	_global_vertex_shader_screen_5_stage,
	_global_vertex_shader_screen_6_stage,
	_global_vertex_shader_screen_7_stage,
	k_global_vertex_shader_count
};

enum e_vertex_shader_platform : int16
{
	_vertex_shader_platform_pc,
	_vertex_shader_platform_xbox
};

/* structures */

class c_vertex_shader_classification
{
public:
	IDirect3DVertexShader9* m_handle;

	/****************************************
	* definition_name: compiled_shader_block
	* flags: 0
	* alignment_bit: 0
	****************************************/
	// DataSize(8192)
	tag_data compiled_shader;

	/****************************************
	* definition_name: code_block
	* flags: 6
	* alignment_bit: 0
	****************************************/
	// DataSize(65535)
	tag_data code;
};
ASSERT_STRUCT_SIZE(c_vertex_shader_classification, 20);

class c_vertex_shader_definition
{
	static const uint32 k_vertex_shader_classification_count = 12;

public:
	e_vertex_shader_platform m_platform;
	int16 pad;
	tag_block<c_vertex_shader_classification> m_geometry_classifications;
	int32 m_output_swizzles;
};
ASSERT_STRUCT_SIZE(c_vertex_shader_definition, 16);
