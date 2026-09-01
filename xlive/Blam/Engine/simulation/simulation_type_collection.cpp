#include "stdafx.h"
#include "simulation_type_collection.h"

/* public code */

void c_simulation_type_collection::clear_types(void)
{
	m_entity_type_count = NONE;
	csmemset(m_entity_definitions, 0, sizeof(m_entity_definitions));
	
	m_event_type_count = NONE;
	csmemset(m_event_definitions, 0, sizeof(m_event_definitions));

	return;
}

void c_simulation_type_collection::finish_types(
	int32 entity_type_count,
	int32 event_type_count)
{
	ASSERT(entity_type_count<=k_simulation_entity_type_maximum_count);
	ASSERT(m_entity_type_count==NONE);

	m_entity_type_count = entity_type_count;
	m_event_type_count = event_type_count;

	return;
}

c_simulation_entity_definition* c_simulation_type_collection::get_entity_definition(
	e_simulation_entity_type entity_type) const
{
	c_simulation_entity_definition* entity_definition;

	ASSERT(entity_type>=0 && entity_type<m_entity_type_count);
	
	entity_definition = m_entity_definitions[entity_type];

	ASSERT(entity_definition!=NULL);
	ASSERT(entity_definition->entity_type()==entity_type);

	return entity_definition;
}

c_simulation_event_definition* c_simulation_type_collection::get_event_definition(
	e_simulation_event_type event_type) const
{
	c_simulation_event_definition* event_definition;

	ASSERT(event_type>=0 && event_type<m_event_type_count);

	event_definition = m_event_definitions[event_type];
	
	ASSERT(event_definition!=NULL);
	ASSERT(event_definition->event_type()==event_type);

	return event_definition;
}

int32 c_simulation_type_collection::get_event_definition_count(
	void) const
{
	ASSERT(m_event_type_count >= 0);

	return m_event_type_count;
}

void c_simulation_type_collection::register_entity_definition(
	e_simulation_entity_type entity_type,
	c_simulation_entity_definition* definition)
{
	m_entity_definitions[entity_type] = definition;
	return;
}

void c_simulation_type_collection::register_event_definition(
	e_simulation_event_type type,
	c_simulation_event_definition* definition)
{
	m_event_definitions[type] = definition;
	return;
}

char const* c_simulation_type_collection::get_entity_type_name(
	e_simulation_entity_type entity_type) const
{
	char const* type_name = "unknown";

	if (entity_type>=0 && entity_type<= m_entity_type_count)
	{
		type_name = get_entity_definition(entity_type)->entity_type_name();
	}
	
	return type_name;
}
