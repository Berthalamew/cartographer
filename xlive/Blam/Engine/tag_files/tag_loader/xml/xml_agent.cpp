#include "stdafx.h"
#include "xml_agent.h"

#include "xml_definition_block.h"
#include "tag_files/tag_groups.h"
#include "tinyxml/tinyxml2.h"


c_xml_definition_agent::c_xml_definition_agent(tag_group type, const char* plugin_path)
{
	this->init(type, plugin_path);
}

c_xml_definition_agent::~c_xml_definition_agent()
{
	this->m_definition.clear();
}

void c_xml_definition_agent::init(tag_group type, const char* plugin_path)
{
	tinyxml2::XMLDocument document;
	tinyxml2::XMLError error = document.LoadFile(plugin_path);
	this->type = type.group;
	if (error != tinyxml2::XML_SUCCESS)
	{
		char err_buff[MAX_PATH + 40];
		sprintf(err_buff, "[xml_definition_agent] failed to load plugin %s for type %s", plugin_path, type.string);
		LOG_ERROR_GAME("{}", err_buff);
		throw err_buff;
	}

	tinyxml2::XMLElement* root = document.RootElement();
	uint32 root_size = strtoul(root->Attribute("baseSize"), nullptr, 16);

	this->m_definition = c_xml_definition_block(root, 0, root_size);
}

c_xml_definition_block* c_xml_definition_agent::get_definition()
{
	return &this->m_definition;
}
