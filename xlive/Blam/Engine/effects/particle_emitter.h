#pragma once

struct c_particle_emitter
{
	uint16 datum_salt;
	int16 m_particle_count;
	datum m_particle_index;
	datum m_next_emitter_index;
	real32 particles_to_emit;
	real_matrix3x3 m_matrix;
	real_point3d m_position;
	real_point3d m_previous_position;

	void adjust_matrix_and_vector_to_effect_camera(bool use_effect_camera, real_matrix3x3* out_matrix, real_vector3d* out_vector) const;

	void __stdcall pulse(
		c_particle_emitter* _this,
		real32 delta,
		class c_particle_system* particle_system,
		class c_particle_emitter_definition* emitter_definition,
		struct s_particle_state* particle_state,
		const real_matrix4x3* in_martix,
		real32 alpha
	);

	void spawn_particle(
		struct s_particle_state* particle_state,
		class c_particle_system* particle_system,
		class c_particle_emitter_definition* emitter_definition,
		real32 a5,
		real32 a6,
		real32 delta,
		real32 a7
	);

	void calc_matrix(
		class c_particle_emitter_definition* definition,
		class c_particle_system* particle_system,
		real32 scale,
		const real_matrix4x3* matrix
	);
};
ASSERT_STRUCT_SIZE(c_particle_emitter, 0x4C);

struct data_array* get_particle_emitter_table();
