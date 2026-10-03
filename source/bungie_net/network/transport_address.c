/*
TRANSPORT_ADDRESS.C
*/

/* ---------- headers */

#include "cseries.h"
#include "transport.h"

/* ---------- constants */

enum
{
	MAXIMUM_TRANSPORT_ADDRESS_STRING_LENGTH = 256, /* fake name */
};

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

static char transport_address_string[MAXIMUM_TRANSPORT_ADDRESS_STRING_LENGTH]; /* fake name */

/* ---------- public code */

struct transport_address *create_transport_address(
	struct transport_address_data const *address,
	word address_length,
	word port)
{
	struct transport_address *result;

	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_address.c", 30, transport_initialized);
	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_address.c", 31, address);

	result = (struct transport_address *)match_malloc("c:\\halo\\SOURCE\\bungie_net\\network\\transport_address.c", 33, sizeof(struct transport_address));

	if (result)
	{
		result->address = *address;
		result->address_length = address_length;
		result->port = port;
		result->address_type = 0;
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
	struct transport_address const *a,
	struct transport_address const *b)
{
	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_address.c", 59, a);
	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_address.c", 60, b);
	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_address.c", 61, transport_initialized);
	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_address.c", 63, IPV4_ADDRESS_LENGTH == a->address_length);
	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_address.c", 64, IPV4_ADDRESS_LENGTH == b->address_length);

	return !memcmp(a->address.bytes, b->address.bytes, MAX(a->address_length, b->address_length)) && a->port == b->port;
}

char *transport_address_to_string(
	struct transport_address const *addr)
{
	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_address.c", 74, addr);
	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_address.c", 75, IPV4_ADDRESS_LENGTH == addr->address_length);

	transport_address_string[0] = 0;

	if (addr->address_length == IPV4_ADDRESS_LENGTH)
	{
		_snprintf(
			transport_address_string,
			sizeof(transport_address_string),
			"%hd.%hd.%hd.%hd:%hd",
			addr->address.bytes[3],
			addr->address.bytes[2],
			addr->address.bytes[1],
			addr->address.bytes[0],
			addr->port);
	}
	else if (addr->address_length == IPV6_ADDRESS_LENGTH)
	{
		_snprintf(
			transport_address_string,
			sizeof(transport_address_string),
			"%4X.%4X.%4X.%4X.%4X.%4X.%4X.%4X:%hd",
			addr->address.words[0],
			addr->address.words[1],
			addr->address.words[2],
			addr->address.words[3],
			addr->address.words[4],
			addr->address.words[5],
			addr->address.words[6],
			addr->address.words[7],
			addr->port);
	}

	return transport_address_string;
}

char const *transport_error_to_string(
	short error)
{
	char const *result;

	switch (error)
	{
	case _transport_error_none:
		result = "_transport_error_none";
		break;
	case _transport_error_unknown:
		result = "_transport_error_unknown";
		break;
	case _transport_error_endpoint_io:
		result = "_transport_error_endpoint_io";
		break;
	case _transport_error_connection_lost:
		result = "_transport_error_connection_lost";
		break;
	case _transport_result_operation_would_block:
		result = "_transport_result_operation_would_block";
		break;
	case _transport_error_not_initialized:
		result = "_transport_error_not_initialized";
		break;
	case _transport_result_already_initialized:
		result = "_transport_result_already_initialized";
		break;
	case _transport_error_bad_input_parameters:
		result = "_transport_error_bad_input_parameters";
		break;
	case _transport_error_dns_lookup_failure:
		result = "_transport_error_dns_lookup_failure";
		break;
	case _transport_error_out_of_memory:
		result = "_transport_error_out_of_memory";
		break;
	case _transport_error_seg_fault:
		result = "_transport_error_seg_fault";
		break;
	case _transport_error_buffers_full:
		result = "_transport_error_buffers_full";
		break;
	case _transport_error_bad_endpoint:
		result = "_transport_error_bad_endpoint";
		break;
	case _transport_result_poll_timeout:
		result = "_transport_result_poll_timeout";
		break;
	case _transport_error_bind_endpoint:
		result = "_transport_error_bind_endpoint";
		break;
	case _transport_error_address_unknown:
		result = "_transport_error_address_unknown";
		break;
	case _transport_error_connect_failed:
		result = "_transport_error_connect_failed";
		break;
	case _transport_error_listen_failed:
		result = "_transport_error_listen_failed";
		break;
	case _transport_error_options_failed:
		result = "_transport_error_options_failed";
		break;
	case _transport_error_endpoint_not_in_set:
		result = "_transport_error_endpoint_not_in_set";
		break;
	case _transport_error_endpoint_set_full:
		result = "_transport_error_endpoint_set_full";
		break;
	case _transport_error_poll_error:
		result = "_transport_error_poll_error";
		break;
	case _transport_result_dns_lookup_in_progress:
		result = "_transport_result_dns_lookup_in_progress";
		break;
	case _transport_result_connect_in_progress:
		result = "_transport_result_connect_in_progress";
		break;
	default:
		result = "<unknown transport error>";
		break;
	}

	return result;
}
