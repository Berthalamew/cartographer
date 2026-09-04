#pragma once

/* enums */

enum e_render_layer
{
	_render_layer_none = 0,
	_render_layer_texture_accumulate,
	_render_layer_overlay,
	_render_layer_lightmap_indirect,
	_render_layer_lightmap_specular,
	_render_layer_spherical_harmonics_prt,
	_render_layer_water_alpha_masks,
	_render_layer_selfillumination,
	_render_layer_enviroment_map,
	_render_layer_stencil_shadows,
	_render_layer_shadow_buffer_generate,
	_render_layer_shadow_buffer_apply,
	_render_layer_lightaccum_diffuse,
	_render_layer_lightaccum_specular,
	_render_layer_lightaccum_albedo,
	_render_layer_transparent,
	_render_layer_fog,
	_render_layer_selfibloomination,
	_render_layer_active_camo,
	_render_layer_active_camo_stencil_modulate,
	_render_layer_decal,
	_render_layer_hologram,
	_render_layer_error_report,
	_render_layer_debug_view,
	_render_layer_hud,

	k_number_of_render_layers,
	_render_layer_mask_all = NONE
};

/* structures */

struct s_render_layer_globals
{
	bool process_bloom_layer;
	bool transparent_submit_in_progress;
	bool first_person_render_in_progress;
	bool render_light_in_progress;
};


/* public code */

s_render_layer_globals* render_layer_globals_get(void);

bool __cdecl render_layer_begin(e_render_layer layer);

void __cdecl render_layer_draw(void);

void __cdecl render_layer_end(void);
