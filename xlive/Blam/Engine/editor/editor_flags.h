#pragma once

/* constants */

enum
{
	MAXIMUM_EDITOR_COMMENTS = 65536
};

/* enums */

enum e_comment_type : int32
{
	_comment_type_generic = 0
};

/* structures */

// max count: MAXIMUM_EDITOR_COMMENTS 65536
struct editor_comment_definition
{
	real_point3d position;
	e_comment_type type;
	char name[k_tag_string_length];
	char comment[256];
};
ASSERT_STRUCT_SIZE(editor_comment_definition, 304);
