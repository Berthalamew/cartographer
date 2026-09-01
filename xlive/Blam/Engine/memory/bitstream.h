#pragma once

enum
{
	k_bitstream_default_alignment = 1
};

enum e_bitstream_state : uint32
{
	_bitstream_state_none,
	_bitstream_state_writing,
	_bitstream_state_write_finished,
	_bitstream_state_reading,
	_bitstream_state_read_only_for_consistency,// read after write maybe???
	_bitstream_state_reading_finished,

	k_bitstream_state_count
};

class c_bitstream
{
public:
	c_bitstream(uint8* data, int32 data_size)
	{
		// field_15 = false;
		m_data_size_alignment = k_bitstream_default_alignment;
		set_data(data, data_size);
	}

	~c_bitstream() = default;

	void set_data(uint8* data, int32 data_size);
	void pop_position(bool reset_to_pushed_state);

	void reset(e_bitstream_state state);

	bool writing() const
	{
		return m_state == _bitstream_state_writing;
	}

	bool reading() const
	{
		return m_state == _bitstream_state_reading || m_state == _bitstream_state_read_only_for_consistency;
	}

	bool read_only_for_consistency() const
	{
		return m_state == _bitstream_state_read_only_for_consistency;
	}

	void begin_reading();
	void finish_reading();
	void begin_writing(int32 data_size_alignment);
	void finish_writing(int32* out_space_left_in_bits);

	bool would_overflow(int32 bit_count) const
	{
		return bit_count + m_current_bit_position > m_data_size_bytes * 8;
	}

	bool overflowed(void) const
	{
		ASSERT(reading() || writing());
		return m_current_bit_position > m_data_size_bytes * 8;
	}

	bool error_occurred() const
	{
		bool result = overflowed();
		// debug mode in release mode should be disabled !!!
		if (m_data_error_detected)
		{
			result = true;
		}

		return result;
	}

	int32 get_space_left_in_bits() const
	{
		ASSERT(writing());

		return 8 * m_data_size_bytes - m_current_bit_position;
	}

	int32 get_space_used_in_bits(
		void) const
	{
		ASSERT(writing());
		return m_current_bit_position;
	}

	int32 get_space_used_in_bytes(
		void) const
	{
		return (get_space_used_in_bits() + 7) / 8;
	}

	
	void write_point3d(char const* debug_string, long_point3d const* point, int32 axis_encoding_size_in_bits);

	void write_string_wchar(const char* name, const void* string, int32 size_in_words);
	void read_string_wchar(const char* name, void* string, int32 size_in_words);
	void write_integer(const char* name, int32 value, uint32 size_in_bits);
	int32 read_integer(const char* name, uint32 size_in_bits);
	void read_point3d(char const* debug_string, long_point3d* point, int32 axis_encoding_size_in_bits);
	uint32 read_value_internal(int32 size_in_bits);

	void write_bool(char const* debug_string, bool value);
	void write_raw_data(char const* debug_string, void const* raw_data, int32 size_in_bits);

	bool read_bool(char const* debug_string);
	void read_raw_data(char const* debug_string, void* raw_data, int32 size_in_bits);

	void data_decode_address(const char* name, void* address);
	void write_quantized_real(const char* name, real32 value, real32 min_value, real32 max_value, int32 size_in_bits, bool exact_midpoint);
	real32 read_quantized_real(const char* debug_string, real32 min_value, real32 max_value, int32 size_in_bits, bool exact_midpoint);
	void data_encode_unit_vector(const char* name, real_vector3d* vector);
	void data_decode_unit_vector(const char* name, real_vector3d* out_vector);
	void data_encode_signed_integer(const char* name, int32 value, uint32 size_in_bits);
	int32 data_decode_signed_integer(const char* name, uint32 size_in_bits);
	void write_axes(char const* debug_string, real_vector3d const* forward, real_vector3d const* up);
	void read_axes(char const* debug_string, real_vector3d* forward, real_vector3d* up);
	void write_vector(char const* debug_string, real_vector3d const* value, real32 min_magnitude, real32 max_magnitude, int32 magnitude_size_in_bits);
	void read_vector(char const* debug_string, real_vector3d* value, real32 min_magnitude, real32 max_magnitude, int32 magnitude_size_in_bits);
	void write_long_integer(const char *name, uint64 value, int size_in_bits);
	uint64 read_long_integer(const char *name, int size_in_bits);

	void write_unit_vector(const char* name, const real_vector3d* unit_vector);
	void read_unit_vector(const char* name, real_vector3d* out_unit_vector);

	bool begin_consistency_check(void);
	void finish_consistency_check(void);
	
	static bool compare_quantized_reals(
		real32 value1,
		real32 value2,
		real32 min_value,
		real32 max_value,
		int32 size_in_bits,
		bool exact_midpoint,
		bool circular_comparison);

	void push_structure(
		char const* block_name_string,
		int32 instance,
		uint32 attributes)
	{
		// TODO
		return;
	}

	void pop_structure(
		char const* block_name_string,
		int32 instance)
	{
		// TODO
		return;
	}

	void push_position(
		void)
	{
		ASSERT(reading() || writing());
		ASSERT(m_position_stack_depth<k_bitstream_maximum_position_stack_size);

		m_position_stack[m_position_stack_depth++] = m_current_bit_position;

		return;
	}

private:
	static long const k_bitstream_maximum_position_stack_size = 4;

	uint8* m_data;
	int32 m_data_size_bytes;
	int32 m_data_size_alignment;
	e_bitstream_state m_state;
	int32 m_current_bit_position;
	bool m_data_error_detected; // debug streams disabled in release mode
	bool gap_15[3]; // these are other types of errors, currently disabled in release mode
	int32 m_position_stack_depth;
	int32 m_position_stack[k_bitstream_maximum_position_stack_size];
	int32 m_number_of_bits_rewound;
	int32 m_number_of_position_resets;

	void write_value_internal(uint32 value, int32 size_in_bits);

};
ASSERT_STRUCT_SIZE(c_bitstream, 52);

void bitstream_serialization_apply_patches();
