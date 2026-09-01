#include "stdafx.h"
#include "simulation_game_generics.h"

#include "cache/cache_files.h"
#include "memory/bitstream.h"
#include "models/models.h"
#include "objects/objects.h"
#include "objects/object_definition.h"

/* prototypes */

CLASS_HOOK_DECLARE_LABEL(c_simulation_generic_entity_definition__entity_creation_encode, c_simulation_generic_entity_definition::entity_creation_encode);
static void __declspec(naked) jmp_c_simulation_generic_entity_definition__entity_creation_encode(void)
{
	CLASS_HOOK_JMP(c_simulation_generic_entity_definition__entity_creation_encode, c_simulation_generic_entity_definition::entity_creation_encode);
}

CLASS_HOOK_DECLARE_LABEL(c_simulation_generic_entity_definition__entity_creation_decode, c_simulation_generic_entity_definition::entity_creation_decode);
static void __declspec(naked) jmp_c_simulation_generic_entity_definition__entity_creation_decode(void)
{
	CLASS_HOOK_JMP(c_simulation_generic_entity_definition__entity_creation_decode, c_simulation_generic_entity_definition::entity_creation_decode);
}

CLASS_HOOK_DECLARE_LABEL(c_simulation_generic_entity_definition__build_object_creation_data, c_simulation_generic_entity_definition::build_object_creation_data);
static void __declspec(naked) jmp_c_simulation_generic_entity_definition__build_object_creation_data(void)
{
	CLASS_HOOK_JMP(c_simulation_generic_entity_definition__build_object_creation_data, c_simulation_generic_entity_definition::build_object_creation_data);
}

/* public code */

void simulation_game_generics_apply_patches(
	void)
{
	WritePointer(Memory::GetAddress(0x3CA694, 0x0), jmp_c_simulation_generic_entity_definition__entity_creation_encode);
	WritePointer(Memory::GetAddress(0x3CA698, 0x0), jmp_c_simulation_generic_entity_definition__entity_creation_decode);
	WritePointer(Memory::GetAddress(0x3CA6CC, 0x0), jmp_c_simulation_generic_entity_definition__build_object_creation_data);

	return;
}

void c_simulation_generic_entity_definition::entity_creation_encode(
	int32 creation_data_size,
	void const* creation_data,
	struct s_simulation_view_telemetry_data const* telemetry_data,
	class c_bitstream* packet,
	bool encode_for_network)
{
	s_simulation_generic_creation_data const* generic_creation_data = (s_simulation_generic_creation_data const*)creation_data;

	ASSERT(creation_data_size==sizeof(struct s_simulation_generic_creation_data));
	ASSERT(creation_data);
	ASSERT(packet);

	packet->push_structure("generic-creation", NONE, 0);

	c_simulation_object_entity_definition::object_creation_encode(&generic_creation_data->object, packet, encode_for_network);
	
	packet->write_raw_data("variant-name", &generic_creation_data->variant, SIZEOF_BITS(generic_creation_data->variant));

	packet->pop_structure("generic-creation", NONE);

	return;
}

bool c_simulation_generic_entity_definition::entity_creation_decode(
	int32 creation_data_size,
	void* creation_data,
	class c_bitstream* packet, 
	bool decode_for_network)
{
	bool object_success;
	bool decode_success;

	s_simulation_generic_creation_data* generic_creation_data = (s_simulation_generic_creation_data *)creation_data;

	ASSERT(creation_data_size == sizeof(struct s_simulation_generic_creation_data));
	ASSERT(creation_data);
	ASSERT(packet);

	packet->push_structure("generic-creation", NONE, 0);

	object_success = c_simulation_object_entity_definition::object_creation_decode(&generic_creation_data->object, packet, decode_for_network);

	packet->read_raw_data("variant-name", &generic_creation_data->variant, SIZEOF_BITS(generic_creation_data->variant));

	packet->pop_structure("generic-creation", NONE);

	decode_success = !packet->overflowed() && object_success;

	return decode_success;
}

void c_simulation_generic_entity_definition::build_object_creation_data(
	int32 generic_index,
	int32 creation_data_size,
	void* creation_data)
{
	object_datum* object = object_get(generic_index);
	s_simulation_generic_creation_data* generic_creation_data = (s_simulation_generic_creation_data*)creation_data;

	ASSERT(creation_data_size==sizeof(struct s_simulation_generic_creation_data));
	ASSERT(creation_data);

	csmemset(generic_creation_data, 0, sizeof(*generic_creation_data));
	c_simulation_object_entity_definition::object_build_creation_data(generic_index, &generic_creation_data->object);

	struct object_definition const* object_definition = (struct object_definition const*)tag_get_fast(object->definition_index);

	if (object->object.variant_index!=NONE && object_definition->object.model.index!=NONE)
	{
		s_model_definition const* model_definition = (s_model_definition const*)tag_get_fast(object_definition->object.model.index);
		s_model_variant const* model_variant = TAG_BLOCK_GET_ELEMENT(&model_definition->variants, object->object.variant_index, s_model_variant);

		generic_creation_data->variant = model_variant->name;
	}
	else
	{
		generic_creation_data->variant = _string_id_empty_string;
	}

	return;
}
