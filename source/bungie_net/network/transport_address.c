/*
TRANSPORT_ADDRESS.C
*/

/* ---------- headers */

#include "cseries.h"
#include "transport.h"
#include "transport_endpoint_winsock.h"

#include <stdio.h>

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

// transport_address_to_string() hands this back, so a string is only good until the next call
static char bss_0031cd30[256];

/* ---------- public code */

struct transport_address *create_transport_address(
	union transport_address_data *address,
	word address_length,
	word port)
{
	struct transport_address *result;

	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_address.c", 30, transport_initialized);
	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_address.c", 31, address);

	result= match_malloc("c:\\halo\\SOURCE\\bungie_net\\network\\transport_address.c", 33, sizeof(struct transport_address));
	if (result)
	{
		result->address= *address;
		result->address_length= address_length;
		result->port= port;
		result->address_type= 0;
	}

	return result;
}

void delete_transport_address(
	struct transport_address *address)
{
	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_address.c", 47, transport_initialized);
	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_address.c", 48, address);

	match_free("c:\\halo\\SOURCE\\bungie_net\\network\\transport_address.c", 50, address);

	return;
}

long transport_address_equivalent(
	struct transport_address *a,
	struct transport_address *b)
{
	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_address.c", 59, a);
	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_address.c", 60, b);
	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_address.c", 61, transport_initialized);

	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_address.c", 63, IPV4_ADDRESS_LENGTH == a->address_length);
	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_address.c", 64, IPV4_ADDRESS_LENGTH == b->address_length);

	if (csmemcmp(a, b, MAX(a->address_length, b->address_length))==0 && a->port==b->port)
	{
		return TRUE;
	}

	return FALSE;
}

char *transport_address_to_string(
	struct transport_address *addr)
{
	word address_length;

	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_address.c", 74, addr);
	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_address.c", 75, IPV4_ADDRESS_LENGTH == addr->address_length);

	bss_0031cd30[0]= 0;
	address_length= addr->address_length;
	if (address_length==IPV4_ADDRESS_LENGTH)
	{
		_snprintf(bss_0031cd30, sizeof(bss_0031cd30), "%hd.%hd.%hd.%hd:%hd",
			addr->address.bytes[3], addr->address.bytes[2], addr->address.bytes[1], addr->address.bytes[0],
			addr->port);
	}
	// the sixteen byte form prints as eight words; the union has no word member to index
	else if (address_length==sizeof(addr->address))
	{
		_snprintf(bss_0031cd30, sizeof(bss_0031cd30), "%4X.%4X.%4X.%4X.%4X.%4X.%4X.%4X:%hd",
			((word *)&addr->address)[0], ((word *)&addr->address)[1],
			((word *)&addr->address)[2], ((word *)&addr->address)[3],
			((word *)&addr->address)[4], ((word *)&addr->address)[5],
			((word *)&addr->address)[6], ((word *)&addr->address)[7],
			addr->port);
	}

	return bss_0031cd30;
}

char *transport_error_to_string(
	short error)
{
	switch (error)
	{
	case _transport_error_none: return "_transport_error_none";
	case _transport_error_unknown: return "_transport_error_unknown";
	case _transport_error_endpoint_io: return "_transport_error_endpoint_io";
	case _transport_error_connection_lost: return "_transport_error_connection_lost";
	case _transport_result_operation_would_block: return "_transport_result_operation_would_block";
	case _transport_error_not_initialized: return "_transport_error_not_initialized";
	case _transport_result_already_initialized: return "_transport_result_already_initialized";
	case _transport_error_bad_input_parameters: return "_transport_error_bad_input_parameters";
	case _transport_error_dns_lookup_failure: return "_transport_error_dns_lookup_failure";
	case _transport_error_out_of_memory: return "_transport_error_out_of_memory";
	case _transport_error_seg_fault: return "_transport_error_seg_fault";
	case _transport_error_buffers_full: return "_transport_error_buffers_full";
	case _transport_error_bad_endpoint: return "_transport_error_bad_endpoint";
	case _transport_result_poll_timeout: return "_transport_result_poll_timeout";
	case _transport_error_bind_endpoint: return "_transport_error_bind_endpoint";
	case _transport_error_address_unknown: return "_transport_error_address_unknown";
	case _transport_error_connect_failed: return "_transport_error_connect_failed";
	case _transport_error_listen_failed: return "_transport_error_listen_failed";
	case _transport_error_options_failed: return "_transport_error_options_failed";
	case _transport_error_endpoint_not_in_set: return "_transport_error_endpoint_not_in_set";
	case _transport_error_endpoint_set_full: return "_transport_error_endpoint_set_full";
	case _transport_error_poll_error: return "_transport_error_poll_error";
	case _transport_result_dns_lookup_in_progress: return "_transport_result_dns_lookup_in_progress";
	case _transport_result_connect_in_progress: return "_transport_result_connect_in_progress";
	}

	return "<unknown transport error>";
}

/* ---------- private code */
