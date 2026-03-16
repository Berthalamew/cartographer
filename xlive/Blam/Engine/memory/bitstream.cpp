#include "stdafx.h"

#include "bitstream.h"

#include "math/real_quantization.h"

void c_bitstream::set_data(uint8* data, int32 data_size)
{
	m_data = data;
	m_data_size_bytes = data_size;
	reset(_bitstream_state_none);
}

void c_bitstream::begin_reading()
{
	if (!writing())
	{
		c_bitstream::reset(_bitstream_state_reading);
	}

	int32 header = read_integer("bitstream-header", 32);
	if (header == 'debg')
	{
		// comment this line if you want to read debug streams
		// but probably isn't recommended
		m_data_error_detected = true;
	}
	else
	{
		m_current_bit_position = 0;
		m_data_error_detected = false;
	}
}

void c_bitstream::finish_reading()
{
	m_state = _bitstream_state_reading_finished;
}

void c_bitstream::begin_writing(int32 alignment)
{	
	typedef void(__thiscall* begin_writing_t)(c_bitstream*, uint32, int, int);
	auto p_begin_writing = Memory::GetAddress<begin_writing_t>(0xD24F5, 0xCEAAF);
	p_begin_writing(this, alignment, 0, 0);	// last 2 arguments are debug
}

void c_bitstream::finish_writing(int32* out_space_left_in_bits)
{
	typedef void(__thiscall* finish_writing_t)(c_bitstream*, void*);
	auto p_finish_writing = Memory::GetAddress<finish_writing_t>(0xD1205, 0xCD7BF);
	p_finish_writing(this, out_space_left_in_bits);
}

void c_bitstream::reset(e_bitstream_state state)
{
	m_state = state;
	m_current_bit_position = 0;
	m_position_stack_depth = 0;
	m_data_error_detected = false;
	if (writing())
	{
		// enable some debugging, not really required in release?
	}
	else if (reading())
	{
		// enable some debugging, not really required in release?
	}
}

void c_bitstream::write_string_wchar(const char* name, const void* string, int32 size_in_words)
{
	typedef void(__thiscall* write_string_wchar_t)(c_bitstream*, const char*, const void*, int32);
	INVOKE_TYPE(0xD18DF, 0xCDE99, write_string_wchar_t, this, name, string, size_in_words);
}

void c_bitstream::read_string_wchar(const char* name, void* string_buffer, int32 size_in_words)
{
	typedef void(__thiscall* read_string_wchar_t)(c_bitstream*, const char*, void*, int);
	INVOKE_TYPE(0xD1FA7, 0xCE561, read_string_wchar_t, this, name, string_buffer, size_in_words);
}

void c_bitstream::write_integer(const char* name, int32 value, uint32 size_in_bits)
{
	typedef void(__thiscall* write_integer_t)(c_bitstream*, const char*, int32, uint32);
	INVOKE_TYPE(0xD17C6, 0xCDD80, write_integer_t, this, name, value, size_in_bits);
}

int32 c_bitstream::read_integer(const char* name, uint32 size_in_bits)
{
	typedef int32(__thiscall* read_integer_t)(c_bitstream*, const char*, uint32);
	return INVOKE_TYPE(0xD1EE5, 0xCE49F, read_integer_t, this, name, size_in_bits);
}

void c_bitstream::read_point3d(
	char const* debug_string,
	long_point3d* point, 
	int32 axis_encoding_size_in_bits)
{
	ASSERT(reading());
	ASSERT(axis_encoding_size_in_bits>=0&&axis_encoding_size_in_bits<=SIZEOF_BITS(point->n[0]));

	for (int32 axis = 0; axis<NUMBEROF(point->n); ++axis)
	{
		point->n[axis]= read_value_internal(axis_encoding_size_in_bits);
		ASSERT(point->n[axis] < (1 << axis_encoding_size_in_bits));
	}

	return;
}

uint32 c_bitstream::read_value_internal(int32 size_in_bits)
{
	typedef uint32(__thiscall* read_value_internal_t)(c_bitstream*, int32);
	return INVOKE_TYPE(0xD167B, 0x0, read_value_internal_t, this, size_in_bits);
}

void c_bitstream::write_raw_data(const char* name, const void* data, uint32 size_in_bits)
{
	typedef void(__thiscall* write_raw_data_t)(c_bitstream*, const char*, const void*, uint32);
	INVOKE_TYPE(0xD18CD, 0xCDE87, write_raw_data_t, this, name, data, size_in_bits);
}

void c_bitstream::read_raw_data(const char* name, void* data, int size_in_bits)
{
	typedef void(__thiscall* read_raw_data_t)(c_bitstream*, const char*, void*, int);
	INVOKE_TYPE(0xD1F95, 0xCE54F, read_raw_data_t, this, name, data, size_in_bits);
}

void c_bitstream::write_bool(const char* name, bool value)
{
	typedef void(__thiscall* write_bool_t)(c_bitstream*, const char*, bool);
	INVOKE_TYPE(0xD1886, 0xCDE40, write_bool_t, this, name, value);
}

bool c_bitstream::read_bool(const char* name)
{
	ASSERT(reading());
	typedef bool(__thiscall* read_bool_t)(c_bitstream*, const char*);
	return INVOKE_TYPE(0xD1F47, 0xCE501, read_bool_t, this, name);
}

void c_bitstream::data_decode_address(const char* name, void* address)
{
	typedef void(__thiscall* data_decode_address_t)(c_bitstream*, const char*, void*);
	INVOKE_TYPE(0xD1FFD, 0xCE5B7, data_decode_address_t, this, name, address);
}

void c_bitstream::write_quantized_real(const char* name, real32 value, real32 min_value, real32 max_value, int32 size_in_bits, bool exact_midpoint)
{
	typedef void(__thiscall* data_encode_quantized_real_t)(c_bitstream*, const char*, real32, real32, real32, int32, bool);
	INVOKE_TYPE(0xD1B9B, 0xCE155, data_encode_quantized_real_t, this, name, value, min_value, max_value, size_in_bits, exact_midpoint);
}

real32 c_bitstream::read_quantized_real(
	const char* debug_string,
	real32 min_value,
	real32 max_value,
	int32 size_in_bits,
	bool exact_midpoint)
{
	typedef real32(__thiscall* data_decode_quantized_real_t)(c_bitstream*, const char*, real32, real32, int32, bool);
	return INVOKE_TYPE(0xD2039, 0xCE5F3, data_decode_quantized_real_t, this, debug_string, min_value, max_value, size_in_bits, exact_midpoint);
}

void c_bitstream::data_encode_unit_vector(const char* name, real_vector3d* vector)
{
	typedef void(__thiscall* data_encode_unit_vector_t)(c_bitstream*, const char*, real_vector3d*);
	INVOKE_TYPE(0xD1BD9, 0xCE193, data_encode_unit_vector_t, this, name, vector);
}

void c_bitstream::data_decode_unit_vector(const char* name, real_vector3d* out_vector)
{
	typedef void(__thiscall* data_decode_unit_vector_t)(c_bitstream*, const char*, real_vector3d*);
	INVOKE_TYPE(0xD20F4, 0xCE6AE, data_decode_unit_vector_t, this, name, out_vector);
}

void c_bitstream::data_encode_signed_integer(const char* name, int32 value, uint32 size_in_bits)
{
	typedef void(__thiscall* data_encode_signed_integer_t)(c_bitstream*, const char*, int32, uint32);
	INVOKE_TYPE(0xD183B, 0xCDDF5, data_encode_signed_integer_t, this, name, value, size_in_bits);
}

int32 c_bitstream::data_decode_signed_integer(const char* name, uint32 size_in_bits)
{
	typedef int(__thiscall* data_decode_signed_integer_t)(c_bitstream*, const char*, uint32);
	return INVOKE_TYPE(0xD1EF2, 0xCE4AC, data_decode_signed_integer_t, this, name, size_in_bits);
}

void c_bitstream::data_encode_axes(const char* name, real_vector3d* forward, real_vector3d* up)
{
	typedef void(__thiscall* data_encode_axes_t)(c_bitstream*, const char*, real_vector3d*, real_vector3d*);
	INVOKE_TYPE(0xD1D41, 0xCE2FB, data_encode_axes_t, this, name, forward, up);
}

void c_bitstream::read_axes(
	char const* debug_string,
	real_vector3d* forward,
	real_vector3d* up)
{
	typedef void(__thiscall* data_decode_axes_t)(c_bitstream*, char const*, real_vector3d*, real_vector3d*);
	INVOKE_TYPE(0xD21DB, 0xCE795, data_decode_axes_t, this, debug_string, forward, up);
	return;
}

void c_bitstream::data_encode_vector(const char *name, real_vector3d *vector, float min_magnitude_value, float max_magnitude_value, int magnitude_size_in_bits)
{
	typedef void(__thiscall* data_encode_vector_t)(c_bitstream*, const char*, real_vector3d*, float , float , int);
	INVOKE_TYPE(0xD1C4B, 0xCE205, data_encode_vector_t, this, name, vector, min_magnitude_value, max_magnitude_value, magnitude_size_in_bits);
}

void c_bitstream::read_vector(
	char const* debug_string,
	real_vector3d* value,
	real32 min_magnitude,
	real32 max_magnitude,
	int32 magnitude_size_in_bits)
{
	typedef void(__thiscall* data_decode_vector_t)(c_bitstream*, const char*, real_vector3d *, real32, real32, int32);
	INVOKE_TYPE(0xD210C, 0xCE6C6, data_decode_vector_t, this, debug_string, value, min_magnitude, max_magnitude, magnitude_size_in_bits);
	return;
}

void c_bitstream::write_long_integer(const char* name, uint64 value, int size_in_bits)
{
	typedef void(__thiscall* write_long_integer_t)(c_bitstream*, const char*, unsigned long long, int);
	INVOKE_TYPE(0xD171D, 0xCDCD7, write_long_integer_t, this, name, value, size_in_bits);
}

uint64 c_bitstream::read_long_integer(const char* name, int size_in_bits)
{
	typedef uint64(__thiscall* read_long_integer_t)(c_bitstream*, const char*, int);
	return INVOKE_TYPE(0xD1E9A, 0xCE454, read_long_integer_t, this, name, size_in_bits);
}

CLASS_HOOK_DECLARE_LABEL(c_bitstream__write_unit_vector, c_bitstream::write_unit_vector);
void c_bitstream::write_unit_vector(const char* name, const real_vector3d* unit_vector)
{
	int32 quantized_vector = quantize_unit_vector3d(unit_vector);
	write_integer("unit-vector", quantized_vector, 19);
}

CLASS_HOOK_DECLARE_LABEL(c_bitstream__read_unit_vector, c_bitstream::read_unit_vector);
void c_bitstream::read_unit_vector(const char* name, real_vector3d* out_unit_vector)
{
	int32 quantized_vector = read_integer("unit-vector", 19);
	dequantize_unit_vector3d(quantized_vector, out_unit_vector);
}

bool c_bitstream::begin_consistency_check(
	void)
{
	ASSERT(!writing());
	reset(_bitstream_state_none);

	// TODO: ifdef name
#ifdef NDEBUG
	return false;
#else
	return true;
#endif
}

void c_bitstream::finish_consistency_check(
	void)
{
	finish_reading();
	return;
}

bool c_bitstream::compare_quantized_reals(
	real32 value1,
	real32 value2,
	real32 min_value,
	real32 max_value,
	int32 size_in_bits,
	bool exact_midpoint,
	bool circular_comparison)
{
	int32 step_count = (1<<size_in_bits)-1;

	ASSERT(size_in_bits>0);
	ASSERT(max_value>min_value);
	ASSERT(!exact_midpoint || size_in_bits>1);
	
	if (exact_midpoint)
	{
		step_count -= step_count % 2;
	}
	
	ASSERT(step_count>0);

	const real32 range = max_value-min_value;
	const real32 delta= range / step_count;
	const real32 value_range = fabs(value2-value1);

	bool result = true;
	if (delta<=value_range)
	{
		if (circular_comparison)
		{
			const real32 range_difference = fabs(value_range-range);
			result = range_difference<delta;
		}
		else
		{
			result = false;
		}
	}

	return result;
}

__declspec(naked) void jmp_write_unit_vector()
{
	CLASS_HOOK_JMP(c_bitstream__write_unit_vector, c_bitstream::write_unit_vector);
}
__declspec(naked) void jmp_read_unit_vector()
{
	CLASS_HOOK_JMP(c_bitstream__read_unit_vector, c_bitstream::read_unit_vector);
}

void bitstream_serialization_apply_patches()
{
	WriteValue(Memory::GetAddress(0xD215C, 0xCE716) + 1, (int8)19);
	PatchCall(Memory::GetAddress(0xD216B, 0xCE725), (void*)dequantize_unit_vector3d);
	WriteJmpTo(Memory::GetAddress(0xD1BD9, 0xCE193), jmp_write_unit_vector);
	WriteJmpTo(Memory::GetAddress(0xD20F4, 0xCE6AE), jmp_read_unit_vector);

	// write_axes
	PatchCall(Memory::GetAddress(0xD1DF7, 0xCE3B1), (void*)quantize_unit_vector3d);
	WriteValue(Memory::GetAddress(0xD1E05, 0xCE3BF) + 2, (int32)(1 << 19));
	WriteValue(Memory::GetAddress(0xD1E0D, 0xCE3C7) + 1, (int32)(1 << 19));
	WriteValue(Memory::GetAddress(0xD1E2A, 0xCE3E4) + 1, (int8)19);
	PatchCall(Memory::GetAddress(0xD1E3A, 0xCE3F4), (void*)dequantize_unit_vector3d);

	// read_axes
	WriteValue(Memory::GetAddress(0xD2243, 0xCE7FD) + 1, (int8)19);
	PatchCall(Memory::GetAddress(0xD2252, 0xCE80C), (void*)dequantize_unit_vector3d);
}