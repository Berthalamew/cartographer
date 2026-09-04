#include "stdafx.h"
#include "text_group.h"

#include "cache/cache_files.h"
#include "game/game_globals.h"
#include "main/game_preferences.h"
#include "memory/data.h"
#include "networking/network_event.h"

/* public code */

bool c_language_pack::unload_data(void)
{
	if (m_data_loaded)
	{
		CSERIES_FREE(m_string_references);
		CSERIES_FREE(m_string_data);
		m_string_references = NULL;
		m_string_data = NULL;
		m_data_loaded = false;
	}

	return true;
}

bool c_language_pack::try_find_string_exists(
	string_id id,
	int32 starting_index,
	int32 max_count)
{
	return INVOKE_TYPE(0x3DF4C, 0x0, bool(__thiscall*)(c_language_pack*, string_id, int32, int32), this, id, starting_index, max_count);
}

utf8* c_language_pack::get_string_utf8(
	string_id id,
	int32 starting_index,
	int32 max_count)
{
	int32 end_string_index = starting_index + max_count;

	for (int32 str_index = starting_index; str_index < end_string_index; ++str_index)
	{
		if (!VALID_INDEX(str_index, this->m_num_of_strings))
			continue;

		const s_string_reference* reference = &this->m_string_references[str_index];

		if (reference->string_id == id)
			return &this->m_string_data[reference->buffer_offset];
	}

	return "";

	//return INVOKE_TYPE(0x3DEFD, 0x0, utf8*(__thiscall*)(c_language_pack*, string_id, int32, int32), this, id, starting_index, max_count);
}

void c_language_pack::string_list_get_normal_string(
	string_id id,
	c_maximum_interface_text* out_string,
	int32 strings_start_index,
	int32 strings_count)
{
	if (this->m_data_loaded)
	{
		c_maximum_interface_text string;
		utf8* string_data = this->get_string_utf8(id, strings_start_index, strings_count);
		utf8_string_to_wchar_string(string_data, string.get_buffer(), string.max_length());
		out_string->set(string.get_string());
	}
	//INVOKE_TYPE(0x3E332, 0x0, void(__thiscall*)(c_language_pack*, string_id, c_maximum_interface_text*, int32, int32), this, id, out_string, strings_start_index, strings_count);

	return;
}

void c_language_pack::get_string_ids(
	string_id* out_ids,
	int32 out_count,
	int32 starting_index,
	int32 max_count)
{
	csmemset(out_ids, _string_id_invalid, sizeof(*out_ids) * out_count);
	
	if (m_data_loaded)
	{
		for (int32 itr = starting_index; itr < max_count + starting_index; itr++)
		{
			int32 index = itr - starting_index;
			if (index < out_count)//prevent buffer overrun
			{
				out_ids[index] = m_string_references[itr].string_id;
			}
		}
	}

	return;
}

void c_language_pack::append_strings(s_string_reference* string_references, utf8* string_buffer, uint32 string_buffer_size, uint32 string_count, uint16* out_index)
{
	ASSERT(string_references);
	ASSERT(string_buffer);
	ASSERT(out_index);
	ASSERT(string_count > 0);
	ASSERT(string_buffer_size > 0);
	
	// calculate, allocate and copy new buffers for string data
	int32 new_string_count = this->m_num_of_strings + string_count;
	int32 new_string_data_size = this->m_string_data_size + string_buffer_size;

	s_string_reference* new_string_references = (s_string_reference*)datum_header_allocate(new_string_count * sizeof(s_string_reference), 12);
	utf8* new_string_data = (utf8*)datum_header_allocate(new_string_data_size, 12);

	ASSERT(new_string_references);
	ASSERT(new_string_data);

	csmemcpy(new_string_references, this->m_string_references, sizeof(s_string_reference) * this->m_num_of_strings);
	csmemcpy(new_string_data, this->m_string_data, this->m_string_data_size);

	// update the newly allocated string reference data and write the new out index to the return buffer
	for (uint32 str_index = 0; str_index < string_count; ++str_index)
	{
		s_string_reference* new_reference = &string_references[str_index];
			
		new_string_references[this->m_num_of_strings + str_index].string_id = new_reference->string_id;
		new_string_references[this->m_num_of_strings + str_index].buffer_offset = new_reference->buffer_offset + this->m_string_data_size;
	}

	// deallocate old buffers
	datum_header_deallocate(this->m_string_references);
	datum_header_deallocate(this->m_string_data);

	// copy and set pointers and data sizes for new strings
	*out_index = (uint16)this->m_num_of_strings;
	
	csmemcpy(new_string_data + this->m_string_data_size, string_buffer, string_buffer_size);

	this->m_string_references = new_string_references;
	this->m_string_data = new_string_data;
	this->m_num_of_strings = new_string_count;
	this->m_string_data_size = new_string_data_size;


	//s_string_reference* string_reference = &this->m_string_references[*out_index];

	//utf8* str = &this->m_string_data[string_reference->buffer_offset];

	//event(_event_verbose, "%s == %s", string_buffer, str);
	//event(_event_verbose, "%s", str);
}

s_string_reference* c_language_pack::get_string_references() const
{
	return this->m_string_references;
}

utf8* c_language_pack::get_string_data() const
{
	return this->m_string_data;
}

int32 c_language_pack::get_number_of_strings() const
{
	return this->m_num_of_strings;
}

int32 c_language_pack::get_string_data_size() const
{
	return this->m_string_data_size;
}

int32 c_language_pack::get_string_reference_cache_offset() const
{
	return this->m_string_reference_cache_offset;
}

int32 c_language_pack::get_string_data_cache_offset() const
{
	return this->m_string_data_cache_offset;
}

bool c_language_pack::get_data_loaded() const
{
	return this->m_data_loaded;
}

void __cdecl string_list_get_normal_string(
	datum unic_datum,
	string_id id,
	c_maximum_interface_text* out_string)
{
	if (unic_datum == NONE)
		return;

	e_language current_language = get_current_language();

	s_multilingual_unicode_string_list_group_header* unic = (s_multilingual_unicode_string_list_group_header*)tag_get_fast(unic_datum);

	c_language_pack* pack = language_pack_get();

	pack->string_list_get_normal_string(
		id, 
		out_string, 
		unic->strings[current_language].strings_index, 
		unic->strings[current_language].strings_count);


	//INVOKE(0x3E3AC, 0x0, string_list_get_normal_string, unic_datum, id, out_string);

	return;
}

void __cdecl string_list_get_string_id_list(
	datum unic_datum,
	string_id* out_ids,
	int32 out_count)
{
	ASSERT(out_ids);
	ASSERT(out_count > 0);

	if (unic_datum != NONE)
	{
		e_language current_language = get_current_language();
		int8* unic_tag = (int8*)tag_get_fast(unic_datum);
		s_unicode_string_list_reference* list_reference = &((s_unicode_string_list_reference*)(unic_tag + 0x10))[current_language];

		s_game_globals* game_globals = scenario_get_game_globals();
		game_globals->language_pack[current_language].get_string_ids(out_ids, out_count, list_reference->strings_index, list_reference->strings_count);
	}

	return;
}

c_language_pack* language_pack_get()
{
	return language_pack_get_language(get_current_language());
}

c_language_pack* language_pack_get_language(e_language language)
{
	s_game_globals* globals = scenario_get_game_globals();

	return &globals->language_pack[language];
}

