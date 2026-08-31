#pragma once
#include "cseries/language.h"

/* constants */

enum
{
	k_max_strings_per_language = 0x8000,
	k_maximum_multilingual_unicode_strings_per_string_list = 9216
};

/* structures */

struct s_unicode_string_list_reference
{
	uint16 strings_index;
	uint16 strings_count;
};

struct s_multilingual_unicode_string_reference
{
	string_id string_id;
	int32 language_offsets[k_language_count];
};
ASSERT_STRUCT_SIZE(s_multilingual_unicode_string_reference, 40);

struct s_multilingual_unicode_string_list_group_header
{
	// MAX: k_maximum_multilingual_unicode_strings_per_string_list
	s_tag_block string_references;		// s_multilingual_unicode_string_reference
	tag_data string_data;
	s_unicode_string_list_reference strings[k_language_count];
};
ASSERT_STRUCT_SIZE(s_multilingual_unicode_string_list_group_header, 52);

struct s_string_reference
{
	string_id string_id;
	uint32 buffer_offset;
};

/* classes */

// TODO add member functions
class c_language_pack
{
public:
	bool unload_data(void);
	
	bool try_find_string_exists(string_id id, int32 starting_index, int32 max_count);
	utf8* get_string_utf8(string_id id, int32 starting_index, int32 max_count);
	
	void string_list_get_normal_string(string_id id, c_maximum_interface_text* out_string, int32 strings_start_index, int32 strings_count);
	void get_string_ids(string_id* array, int32 array_size, int32 starting_index, int32 max_count);

	void append_strings(s_string_reference* string_references, utf8* string_buffer, uint32 string_buffer_size, uint32 string_count, uint16* out_index);

	s_string_reference* get_string_references() const;
	utf8* get_string_data() const;
	int32 get_number_of_strings() const;
	int32 get_string_data_size() const;
	int32 get_string_reference_cache_offset() const;
	int32 get_string_data_cache_offset() const;
	bool get_data_loaded() const;

private:
	s_string_reference* m_string_references;
	utf8* m_string_data;
	int32 m_num_of_strings;
	int32 m_string_data_size;
	int32 m_string_reference_cache_offset;
	int32 m_string_data_cache_offset;
	bool m_data_loaded;
};
ASSERT_STRUCT_SIZE(c_language_pack, 28);

/* prototypes */

void __cdecl string_list_get_normal_string(datum unic_datum, string_id id, c_maximum_interface_text* out_string);
void __cdecl string_list_get_string_id_list(datum unic_datum, string_id* out_ids, int32 out_count);

c_language_pack* language_pack_get();

c_language_pack* language_pack_get_language(e_language language);