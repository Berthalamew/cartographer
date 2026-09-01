#pragma once
#include "replication_scheduler.h"
#include "networking/player_motion.h"
#include "networking/player_prediction.h"

/* structures */

struct s_replication_control_request
{
	int32 unknown_count;
	int32 control_index;
	int8 gap_8[52];
	int32 block_count;
	s_replication_allocation_block blocks[2];
	int8 gap_50[48];
};

/* classes */

class c_replication_control_view : c_replication_scheduler_client
{
private:
	bool m_initialized;
	char _pad09[3];
	void* m_telemetry_provider;
	uint32 m_motion_available_send;
	uint32 m_motion_available_receive;
	s_player_motion m_motion[32];
	uint32 m_motion_timestamp[32];
	uint32 m_prediction_available_send;
	uint32 m_prediction_available_receive;
	s_prediction m_prediction[32];

public:
	bool has_data_to_transmit(void) override;
	bool build_outgoing_requests(void const* telemetry_data, int32 maximum_number_of_requests, struct s_replication_incoming_request* requests) override;
	int32 terminator_required_bits() override;
	void write_to_packet(void* request_identifier, int32 request_type, void const* telemetry_data, int32 packet_sequence_number, class c_bitstream* packet, int32 must_leave_space_bits) override;
	void write_terminator_to_packet(class c_bitstream* packet) override;
	e_network_read_result read_from_packet(int32 packet_sequence_number, class c_bitstream* packet, int32 maximum_number_of_requests, struct s_replication_incoming_request* requests, int32* out_number_of_requests) override;
	void process_incoming_request(void* request) override;
	void notify_packet_acknowledged(void) override;
	void mark_packet_delivered(bool delivered) override;
};
ASSERT_STRUCT_SIZE(c_replication_control_view, 0xF20);
