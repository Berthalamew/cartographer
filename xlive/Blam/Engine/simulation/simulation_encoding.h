#pragma once

/* prototypes */

void simulation_write_quantized_position(class c_bitstream* packet, real_point3d const* position, int32 axis_encoding_bit_count, bool fixup_quantized_position_inside_bsp);
void simulation_read_quantized_position(class c_bitstream* packet, real_point3d* position, int32 axis_encoding_bit_count);

void __cdecl simulation_player_update_encode(class c_bitstream* packet, const struct simulation_player_update* player_update);
bool __cdecl simulation_player_update_decode(class c_bitstream* packet, struct simulation_player_update* player_update);

void __cdecl player_action_encode(class c_bitstream* packet, const struct player_action* action);
bool __cdecl player_action_decode(class c_bitstream* packet, struct player_action* action);
void __cdecl simulation_machine_update_encode(class c_bitstream* packet, const struct simulation_machine_update* machine_update);
bool __cdecl simulation_machine_update_decode(class c_bitstream* packet, struct simulation_machine_update* machine_update);

bool player_action_compare(struct player_action const* action1, struct player_action* action2);
bool simulation_update_compare(struct simulation_update const* update1, struct simulation_update* update2);

void __cdecl simulation_update_encode(class c_bitstream* packet, const struct simulation_update* update);
bool __cdecl simulation_update_decode(class c_bitstream* packet, struct simulation_update* update);
