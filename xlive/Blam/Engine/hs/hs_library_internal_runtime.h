#pragma once

/*
* This file contains the "internal" functions and logic used by the hs_runtime code
* NOTE: This header should NEVER EVER EVER be included in any other file except hs_runtime.cpp
*/

#include "hs.h"
#include "hs_globals_external.h"
#include "hs_runtime.h"
#include "object_lists.h"

#include "cache/cache_files.h"
#include "memory/data.h"

#ifdef TERMINAL_ENABLED
#include "main/console.h"
#endif

#include "objects/objects.h"

/* prototypes */

static void hs_inspect_boolean(int16 type, int32 value, char* buffer, int32 buffer_size);

static void hs_inspect_real(int16 type, int32 value, char* buffer, int32 buffer_size);

static void hs_inspect_short_integer(int16 type, int32 value, char* buffer, int32 buffer_size);

static void hs_inspect_long_integer(int16 type, int32 value, char* buffer, int32 buffer_size);

static void hs_inspect_string(int16 type, int32 value, char* buffer, int32 buffer_size);

static void hs_inspect_enum(int16 type, int32 enum_value, char* buffer, int32 buffer_size);

/* globals */

#ifdef TERMINAL_ENABLED

static void(*hs_type_inspectors[NUMBER_OF_HS_NODE_TYPES])(int16, int32, char*, int32) =
{
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	hs_inspect_boolean,
	hs_inspect_real,
	hs_inspect_short_integer,
	hs_inspect_long_integer,
	hs_inspect_string,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	hs_inspect_enum,
	hs_inspect_enum,
	hs_inspect_enum,
	hs_inspect_enum,
	hs_inspect_enum,
	hs_inspect_enum,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL
};

#endif

/* forward declarations */

extern int32 hs_global_evaluate(int16 global_designator);

extern int32 hs_global_designator_get_index(int16 global_designator);

extern void hs_global_reconcile_write(int16 global_designator);

extern data_array* hs_global_data_get(void);

/* public code */

void __cdecl hs_evaluate_wake(int16 function_index, int32 thread_index, bool initialize)
{
	INVOKE(0x953B8, 0x0, hs_evaluate_wake, function_index, thread_index, initialize);
	return;
}

void __cdecl hs_evaluate_begin(int16 function_index, int32 thread_index, bool initialize)
{
	INVOKE(0x95845, 0x0, hs_evaluate_begin, function_index, thread_index, initialize);
	return;
}

void __cdecl hs_evaluate_begin_random(int16 function_index, int32 thread_index, bool initialize)
{
	INVOKE(0x9590C, 0x0, hs_evaluate_begin_random, function_index, thread_index, initialize);
	return;
}

void __cdecl hs_evaluate_if(int16 function_index, int32 thread_index, bool initialize)
{
	INVOKE(0x95B05, 0x0, hs_evaluate_if, function_index, thread_index, initialize);
	return;
}

void hs_evaluate_set(int16 function_index, int32 thread_index, bool initialize)
{
	//INVOKE(0x95C80, 0x0, hs_evaluate_set, function_index, thread_index, initialize);

	const hs_thread* thread = hs_thread_get(thread_index);
	
	const hs_syntax_node* expression = hs_syntax_get(thread->stack->expression_index);
	
	const int32 global_expression_index = hs_syntax_get(expression->long_value)->next_node_index;
	const hs_syntax_node* global_expression = hs_syntax_get(global_expression_index);

	hs_stack_allocate(thread_index, 4);
	const e_hs_type type = (e_hs_type)hs_global_get_type(global_expression->short_value);
	if (initialize)
	{
		if (type == _hs_type_object_list)
		{
			const int32 object_list_index = hs_global_evaluate(global_expression->short_value);
			object_list_remove_reference(object_list_index);
		}
		const int32 runtime_global_abs_index = hs_global_designator_get_index(global_expression->short_value);
		hs_global_runtime* global = (hs_global_runtime*)datum_get_absolute(hs_global_data_get(), runtime_global_abs_index);
		
		hs_evaluate(thread_index, hs_syntax_get(global_expression_index)->next_node_index, &global->long_value);
	}
	else
	{
		hs_global_reconcile_write(global_expression->short_value);
		if (type == _hs_type_object_list)
		{
			const int32 object_list_index = hs_global_evaluate(global_expression->short_value);
			object_list_add_reference(object_list_index);
		}

		const int32 value = hs_global_evaluate(global_expression->short_value);
		hs_return(thread_index, value);
	}
	return;
}

void __cdecl hs_evaluate_logical(int16 function_index, int32 thread_index, bool initialize)
{
	INVOKE(0x95DBA, 0x0, hs_evaluate_logical, function_index, thread_index, initialize);
	return;
}

void __cdecl hs_evaluate_arithmetic(int16 function_index, int32 thread_index, bool initialize)
{
	INVOKE(0x95EE0, 0x0, hs_evaluate_arithmetic, function_index, thread_index, initialize);
	return;
}

void __cdecl hs_evaluate_equality(int16 function_index, int32 thread_index, bool initialize)
{
	INVOKE(0x9608C, 0x0, hs_evaluate_equality, function_index, thread_index, initialize);
	return;
}

void __cdecl hs_evaluate_inequality(int16 function_index, int32 thread_index, bool initialize)
{
	INVOKE(0x961B6, 0x0, hs_evaluate_equality, function_index, thread_index, initialize);
	return;
}

void __cdecl hs_evaluate_sleep(int16 function_index, int32 thread_index, bool initialize)
{
	INVOKE(0x96414, 0x0, hs_evaluate_sleep, function_index, thread_index, initialize);
	return;
}

void __cdecl hs_evaluate_sleep_forever(int16 function_index, int32 thread_index, bool initialize)
{
	INVOKE(0x96384, 0x0, hs_evaluate_sleep_forever, function_index, thread_index, initialize);
	return;
}

void __cdecl hs_evaluate_sleep_until(int16 function_index, int32 thread_index, bool initialize)
{
	INVOKE(0x965CA, 0x0, hs_evaluate_sleep_until, function_index, thread_index, initialize);
	return;
}

void hs_evaluate_inspect(int16 function_index, int32 thread_index, bool initialize)
{
	const hs_thread* thread = hs_thread_get(thread_index);
	int32* stack = (int32*)hs_stack_allocate(thread_index, sizeof(int32));

	ASSERT(function_index == _hs_function_inspect);

	hs_syntax_node* expression = hs_syntax_get(thread->stack->expression_index);
	hs_syntax_node* value = hs_syntax_get(expression->long_value);
	if (initialize)
	{
		hs_evaluate(thread_index, value->next_node_index, stack);
	}
	else
	{
#ifdef TERMINAL_ENABLED
		hs_syntax_node* global_value = hs_syntax_get(value->next_node_index);

		if (TEST_BIT(thread->flags, _hs_thread_display_expression_bit))
		{
			if (hs_type_inspectors[global_value->type])
			{
				char string[1024];
				hs_type_inspectors[global_value->type](global_value->type, *stack, string, NUMBEROF(string));
				console_printf(string);
			}
		}
#endif
		hs_return(thread_index, 0);
	}
	return;
}

void hs_evaluate_object_cast_up(int16 function_index, int32 thread_index, bool initialize)
{
	hs_thread* thread = hs_thread_get(thread_index);
	int32* stack = (int32*)hs_stack_allocate(thread_index, sizeof(int32));

	ASSERT(function_index == _hs_function_object_to_unit);

	if (initialize)
	{
		hs_syntax_node* expression = hs_syntax_get(thread->stack->expression_index);
		hs_syntax_node* value = hs_syntax_get(expression->long_value);
		hs_evaluate(thread_index, value->next_node_index, stack);
	}
	else if (*stack == NONE)
	{
		hs_return(thread_index, NONE);
	}
	else
	{
		const object_datum* object = object_get(stack[0]);
		const int16 type_index = (function_index - (_hs_function_object_to_unit - 1));
		if (TEST_BIT(hs_object_type_masks[type_index], object->object.object_identifier.get_type()))
		{
			hs_return(thread_index, stack[0]);
		}
		else
		{
#ifdef ERRORS_ENABLED
			const char* type_name = hs_type_names[type_index + FIRST_HS_OBJECT_TYPE];
			const char* tag_name = tag_get_name(object->definition_index);
			error(_error_delayed, "attempt to convert object %s to type %s", tag_name, type_name);
#endif
			hs_return(thread_index, NONE);
		}
	}
	return;
}

/* private code */

#ifdef HS_COMPILER_ENABLED

static void hs_inspect_boolean(
	int16 type,
	int32 value,
	char* buffer,
	int32 buffer_size)
{
	ASSERT(type == _hs_type_boolean);

	// We need to cast to a bool or else the compiler will give us a bad value
	// Note: added (& BYTE_MAX) to make sure we only get the byte of the value
	const bool result = (bool)(value & BYTE_MAX);	
	const char* string = result ? "true" : "false";

	csstrncpy(buffer, string, buffer_size);
	return;
}

static void hs_inspect_real(int16 type, int32 value, char* buffer, int32 buffer_size)
{
	ASSERT(type == _hs_type_real);

	csprintf(buffer, buffer_size, "%f", *(real32*)&value);
	return;
}

static void hs_inspect_short_integer(int16 type, int32 value, char* buffer, int32 buffer_size)
{
	ASSERT(type == _hs_type_short_integer);

	csprintf(buffer, buffer_size, "%d", (int16)value);
	return;
}

static void hs_inspect_long_integer(int16 type, int32 value, char* buffer, int32 buffer_size)
{
	ASSERT(type == _hs_type_long_integer);

	csprintf(buffer, buffer_size, "%ld", (int32)value);
	return;
}

static void hs_inspect_string(int16 type, int32 value, char* buffer, int32 buffer_size)
{
	ASSERT(type == _hs_type_string);

	csstrncpy(buffer, (const char*)value, buffer_size);
	return;
}

static void hs_inspect_enum(int16 type, int32 enum_value, char* buffer, int32 buffer_size)
{
	ASSERT(HS_TYPE_IS_ENUM(type));

	const hs_enum_definition* enum_definition = &hs_enum_table[type];
	ASSERT(VALID_INDEX(enum_value, enum_definition->count));

	csstrncpy(buffer, enum_definition->identifiers[enum_value], buffer_size);
	return;
}

#endif
