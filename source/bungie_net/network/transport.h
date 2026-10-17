/*
TRANSPORT.H

header included in hcex build.
*/

#ifndef __TRANSPORT_H
#define __TRANSPORT_H
#pragma once

/* ---------- constants */

enum
{
	IPV4_ADDRESS_LENGTH = 4,
	IPV6_ADDRESS_LENGTH = 16, /* fake name */
};

enum
{
	_transport_type_udp = 17,
	_transport_type_tcp = 18, /* fake name */
};

enum
{
	_transport_error_none = 0,
	_transport_error_unknown = -1,
	_transport_error_endpoint_io = -2,
	_transport_error_connection_lost = -3,
	_transport_result_operation_would_block = -4,
	_transport_error_not_initialized = -5,
	_transport_result_already_initialized = -6,
	_transport_error_bad_input_parameters = -7,
	_transport_error_dns_lookup_failure = -8,
	_transport_error_out_of_memory = -9,
	_transport_error_seg_fault = -10,
	_transport_error_buffers_full = -11,
	_transport_error_bad_endpoint = -12,
	_transport_result_poll_timeout = -13,
	_transport_error_bind_endpoint = -14,
	_transport_error_address_unknown = -15,
	_transport_error_connect_failed = -16,
	_transport_error_listen_failed = -17,
	_transport_error_options_failed = -18,
	_transport_error_endpoint_not_in_set = -19,
	_transport_error_endpoint_set_full = -20,
	_transport_error_poll_error = -21,
	_transport_result_dns_lookup_in_progress = -22,
	_transport_result_connect_in_progress = -23,
};

/* ---------- macros */

/* ---------- structures */

struct transport_address_data /* fake name */
{
	union
	{
		unsigned long ipv4_address; /* fake name */
		word words[IPV6_ADDRESS_LENGTH/sizeof(word)]; /* fake name */
		byte bytes[IPV6_ADDRESS_LENGTH]; /* fake name */
	};
};

struct transport_address
{
	struct transport_address_data address; /* fake name */
	word address_length;
	word port;
	long address_type; /* fake name */
};

/* ---------- prototypes/TRANSPORT_ADDRESS.C */

struct transport_address *create_transport_address(struct transport_address_data const *address, word address_length, word port);
void delete_transport_address(struct transport_address *address);
long transport_address_equivalent(struct transport_address const *a, struct transport_address const *b);
char *transport_address_to_string(struct transport_address const *addr);
char const *transport_error_to_string(short error);

/* ---------- prototypes/TRANSPORT_ENDPOINT_WINSOCK.C */

struct transport_endpoint *create_transport_endpoint(long type);
void delete_transport_endpoint(struct transport_endpoint *ep);
short get_endpoint_address(struct transport_endpoint *ep, struct transport_address *address);
long get_endpoint_type(struct transport_endpoint const *ep);
short set_endpoint_blocking(struct transport_endpoint *ep, long blocking);
short bind_endpoint(struct transport_endpoint *ep, struct transport_address const *address);
short connect_endpoint(struct transport_endpoint *ep, struct transport_address const *address);
void disconnect_endpoint(struct transport_endpoint *ep);
short connect_endpoint_async(struct transport_endpoint *ep, struct transport_address const *address, struct transport_connect_process **process_ref_ptr);
void cancel_connect_process(struct transport_connect_process *input);
short listen_endpoint(struct transport_endpoint *ep);
struct transport_endpoint *accept_endpoint(struct transport_endpoint *listening_endpoint);
short reject_endpoint(struct transport_endpoint *ep);
long read_endpoint(struct transport_endpoint *ep, void *buffer, long length);
long write_endpoint(struct transport_endpoint *ep, void const *buffer, long length);
long read_from_endpoint(struct transport_endpoint *ep, void *buffer, long length, struct transport_address *src_addr);
long write_to_endpoint(struct transport_endpoint *ep, void const *buffer, long length, struct transport_address const *dest_addr);
boolean endpoint_readable(struct transport_endpoint *ep, word timeout);
boolean endpoint_writeable(struct transport_endpoint *ep, word timeout);
boolean endpoint_connected(struct transport_endpoint const *ep);
long endpoint_listening(struct transport_endpoint const *ep);
long endpoint_blocking(struct transport_endpoint const *ep);
short get_endpoint_error(struct transport_endpoint const *ep);
long endpoint_equivalent(struct transport_endpoint const *a, struct transport_endpoint const *b);
char const *winsock_error_to_string(long error_number);

/* ---------- prototypes/TRANSPORT_ENDPOINT_SET_WINSOCK.C */

void net_startup_debug(void);
void transport_push_key(XNKEY const *key, XNKID const *key_id);
void transport_pop_key(void);
short transport_server_initialize(void);
short transport_server_terminate(void);
void transport_get_nonce(void *dst, long bytes);
boolean transport_nonce_is_equal(void const *src, void const *dst);
boolean transport_is_nonce(void const *src, long bytes);
void transport_client_stop(void);
XNADDR transport_get_xnaddr(void);
XNKID transport_get_key_id(void);
XNKEY transport_get_key(void);
void transport_client_start(XNADDR const *address, XNKEY const *key, XNKID const *key_id, word port, struct transport_address *result);
long transport_initialize(void);
short transport_dispose(void);
boolean transport_network_available(void);
struct transport_endpoint_set *create_endpoint_set(short max_endpoints);
short delete_endpoint_set(struct transport_endpoint_set *set);
short poll_endpoint_set(struct transport_endpoint_set *set, word timeout);
short add_endpoint_to_set(struct transport_endpoint *ep, struct transport_endpoint_set *set);
short remove_endpoint_from_set(struct transport_endpoint *ep, struct transport_endpoint_set *set);
void rewind_endpoint_set(struct transport_endpoint_set *set);
struct transport_endpoint *get_next_endpoint_from_set(struct transport_endpoint_set *set);
long count_endpoints_in_set(struct transport_endpoint_set const *set);

/* ---------- globals */

extern boolean transport_initialized;

/* ---------- public code */

#endif // __TRANSPORT_H
