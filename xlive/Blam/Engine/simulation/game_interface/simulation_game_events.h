#pragma once

enum e_simulation_event_type
{
	_simulation_event_type_damage_aftermath_event = 0,
	_simulation_event_type_damage_section_response,
	_simulation_event_type_breakable_surface_damage_event,
	_simulation_event_type_projectile_attached_event,
	_simulation_event_type_projectile_effect_event,
	_simulation_event_type_projectile_impact_effect_event,
	_simulation_event_type_projectile_object_impact_effect_event,
	_simulation_event_type_game_engine_event,
	_simulation_event_type_unit_board_vehicle_event,
	_simulation_event_type_unit_pickup_event,
	_simulation_event_type_weapon_effect_event,
	_simulation_event_type_unit_exit_vehicle_event,
	_simulation_event_type_weapon_fire_event,
	_simulation_event_type_weapon_reload_event,
	_simulation_event_type_unit_grenade_initiate_event,
	_simulation_event_type_unit_melee_initiate_event,
	_simulation_event_type_weapon_pickup_event,
	_simulation_event_type_weapon_put_away_event,
	_simulation_event_type_weapon_drop_event,
	_simulation_event_type_vehicle_flip_event,
	_simulation_event_type_vehicle_trick_event,
	_simulation_event_type_device_touch_event,
	_simulation_event_type_unit_grenade_release_event,
	_simulation_event_type_unit_melee_damage_event,
	_simulation_event_type_unit_enter_vehicle_event,
	_simulation_event_type_request_boot_player_event,
	k_simulation_event_type_count,
};

class c_simulation_event_definition;

struct c_simulation_event_definition_vtbl
{
	e_simulation_event_type (__thiscall* event_type)(c_simulation_event_definition* thisx);
	const char* (__thiscall* event_type_name)(c_simulation_event_definition* thisx);
	uint32 (__thiscall* payload_size)(c_simulation_event_definition* thisx);
	int32(__thiscall* number_of_entity_references)(c_simulation_event_definition* thisx);
	bool(__thiscall* reference_delays_entity_deletion)(c_simulation_event_definition* thisx);
	bool(__thiscall* event_can_be_transmitted)(c_simulation_event_definition* thisx, int a1, int a2);
	int32(__thiscall* minimum_required_bits)(c_simulation_event_definition* thisx, int a1, int a2, uint8* a3);
	void(__thiscall* calculate_relevance)(c_simulation_event_definition* thisx, int a2, int a3, float a4);
	int32(__thiscall* write_description_to_string)(c_simulation_event_definition* thisx, int a2, int a3, uint32 a4, int a5, char* a6);
	bool(__thiscall* encode)(c_simulation_event_definition* thisx, int32 payload_size, void* payload, class c_bitstream* packet);
	bool(__thiscall* decode)(c_simulation_event_definition* thisx, int32 payload_size, void* payload, class c_bitstream* packet);
	bool(__thiscall* perform)(c_simulation_event_definition* thisx, int32 entity_reference_count, int32* entity_references, uint32 payload_size, uint8* data);
};

class c_simulation_event_definition
{
public:
	virtual e_simulation_event_type event_type();
	virtual const char* event_type_name();
	virtual uint32 payload_size();
	virtual int32 number_of_entity_references();
	virtual bool reference_delays_entity_deletion();
	virtual bool event_can_be_transmitted(int a1, int a2);
	virtual int32 minimum_required_bits(int a1, int a2, uint32* a3);
	virtual void calculate_relevance(int a2, int a3, float a4);
	virtual int32 write_description_to_string(int a2, int a3, uint32 a4, int a5, char* a6);
	virtual bool encode(int32 payload_size, void* data, class c_bitstream* packet);
	virtual bool decode(int32 payload_size, void* data, class c_bitstream* packet);
	virtual bool apply_game_event(int32 entity_reference_count, const int32* gamestate_indices, int32 payload_size, uint8* data);
};
