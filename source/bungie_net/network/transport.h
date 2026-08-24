/*
TRANSPORT.H

header included in hcex build.
*/

#ifndef __TRANSPORT_H
#define __TRANSPORT_H
#pragma once

/* ---------- headers */

#ifdef xbox
#include <xtl.h>
#endif

/* ---------- constants */

// a datagram must fit the buffer network_game_server_handle_public_endpoint() reads into
enum
{
	IPV4_ADDRESS_LENGTH= 4,
	IPV4_LOOPBACK_ADDRESS= 0x7F000001,
	IPV4_BROADCAST_ADDRESS= 0xFFFFFFFF,

	DATAGRAM_MAXIMUM_SIZE= 400
};

// the codes the game tests are -3 (a read on a lost connection), -4 (a write that would block),
// -13 (poll_endpoint_set timeout) and -23 (connect_endpoint_async still in progress)
enum
{
	_transport_result_connect_in_progress= -23,
	_transport_result_dns_lookup_in_progress= -22,
	_transport_error_poll_error= -21,
	_transport_error_endpoint_set_full= -20,
	_transport_error_endpoint_not_in_set= -19,
	_transport_error_options_failed= -18,
	_transport_error_listen_failed= -17,
	_transport_error_connect_failed= -16,
	_transport_error_address_unknown= -15,
	_transport_error_bind_endpoint= -14,
	_transport_result_poll_timeout= -13,
	_transport_error_bad_endpoint= -12,
	_transport_error_buffers_full= -11,
	_transport_error_seg_fault= -10,
	_transport_error_out_of_memory= -9,
	_transport_error_dns_lookup_failure= -8,
	_transport_error_bad_input_parameters= -7,
	_transport_result_already_initialized= -6,
	_transport_error_not_initialized= -5,
	_transport_result_operation_would_block= -4,
	_transport_error_connection_lost= -3,
	_transport_error_endpoint_io= -2,
	_transport_error_unknown= -1,
	_transport_error_none= 0
};

// the only types create_transport_endpoint() takes; bind and connect turn them into SOCK_DGRAM
// and SOCK_STREAM
enum
{
	_transport_type_udp= 17,
	_transport_type_tcp= 18
};

/* ---------- macros */

/* ---------- structures */

union transport_address_data
{
	unsigned long ipv4_address;
	byte bytes[16];
};

struct transport_address
{
	union transport_address_data address; // 0x00
	word address_length; // 0x10
	word port; // 0x12
	long address_type; // 0x14, always zero; nothing above the transport layer reads it
}; // 0x18

typedef struct transport_endpoint *transport_endpoint_ref;
typedef struct transport_connect_process *transport_connect_process_ref;

/* ---------- prototypes/TRANSPORT.C */

boolean transport_network_available(void);
short transport_server_initialize(void);
short transport_server_terminate(void);
struct transport_address *create_transport_address(union transport_address_data *address, word address_length, word port);
void delete_transport_address(struct transport_address *address);
long transport_address_equivalent(struct transport_address *a, struct transport_address *b);
char *transport_address_to_string(struct transport_address *address);
char *transport_error_to_string(short error);
void transport_get_nonce(byte *nonce, long nonce_size);
#ifdef xbox
XNKID transport_get_key_id(void);
XNKEY transport_get_key(void);
XNADDR transport_get_xnaddr(void);
#endif
boolean transport_nonce_is_equal(byte *nonce1, byte *nonce2);
boolean transport_is_nonce(byte *nonce, long nonce_size);
long write_endpoint(transport_endpoint_ref endpoint, void *buffer, long buffer_size);
void cancel_connect_process(transport_connect_process_ref process);

/* ---------- globals */

extern boolean transport_initialized;

/* ---------- public code */

#endif // __TRANSPORT_H
