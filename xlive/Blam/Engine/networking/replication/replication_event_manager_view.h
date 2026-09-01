#pragma once
#include "replication_scheduler.h"

/* structures */

struct s_replication_event_manager_view_statistics
{
	uint32 events_sent;
	uint32 events_pending;
	uint32 events_in_transit;
};
ASSERT_STRUCT_SIZE(s_replication_event_manager_view_statistics, 12);

/* classes */

class c_replication_event_manager_view : c_replication_scheduler_client
{
public:
	bool has_data_to_transmit() override;
	bool build_outgoing_requests(void const* telemetry_data, int32 maximum_number_of_requests, s_replication_incoming_request* requests) override;
	int32 terminator_required_bits() override;
	void write_to_packet(void* request_identifier, int32 request_type, void const* telemetry_data, int32 packet_sequence_number, class c_bitstream* packet, int32 must_leave_space_bits) override;
	void write_terminator_to_packet(c_bitstream* packet) override;
	e_network_read_result read_from_packet(int32 packet_sequence_number, class c_bitstream* packet, int32 maximum_number_of_requests, s_replication_incoming_request* requests, int32* out_number_of_requests) override;
	void process_incoming_request(void* request) override;
	void notify_packet_acknowledged() override;
	void mark_packet_delivered(bool delivered) override;

	bool has_fatal_error(
		void) const
	{
		return m_fatal_error;
	}

	class c_event_record
	{
	public:
		class c_replication_outgoing_event* m_event;
		class c_event_record* m_next;
	};

	class c_packet_record
	{
		int32 m_packet_sequence_number;
		class c_event_record* m_event_list;
		class c_packet_record* m_next;
	};

private:
	bool m_initialized;
	bool m_fatal_error;
	uint32 m_view_index;
	class c_packet_record* m_packet_list;
	uint32 m_packet_list_length;
	class c_replication_event_manager* m_event_manager;
	s_replication_event_manager_view_statistics m_statistics;
};
ASSERT_STRUCT_SIZE(c_replication_event_manager_view, 40);

/* prototypes */

void replication_event_manager_view_apply_patches(void);
