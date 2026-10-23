/*
DATA_PACKETS.C
*/

/* ---------- headers */

#include "cseries.h"
#include "data_packets.h"

/* ---------- prototypes */

static void _data_packet_verify(struct data_packet_definition *packet_definition, short *byte_count_reference, struct data_packet_field *first_field, short *field_count_reference);
static void _data_packet_encode(struct data_packet_definition *packet_definition, struct data_encoding_state *encode_state, short version, void *original_buffer, short *byte_count_reference, struct data_packet_field *first_field, short *field_count_reference);
static void _data_packet_decode(struct data_packet_definition *packet_definition, struct data_encoding_state *decode_state, short version, void *original_buffer, short *byte_count_reference, struct data_packet_field *first_field, short *field_count_reference);

/* ---------- public code */

void data_packet_verify(
	struct data_packet_definition *packet_definition)
{
	match_assert("c:\\halo\\SOURCE\\memory\\data_packets.c", 32, packet_definition);
	match_assert("c:\\halo\\SOURCE\\memory\\data_packets.c", 33, packet_definition->size>=0);
	match_assert("c:\\halo\\SOURCE\\memory\\data_packets.c", 34, packet_definition->version>=0);
	match_assert("c:\\halo\\SOURCE\\memory\\data_packets.c", 35, packet_definition->name && packet_definition->fields);

	if (!packet_definition->initialized_flag)
	{
		short byte_count;
		short field_count;

		_data_packet_verify(packet_definition, &byte_count, packet_definition->fields, &field_count);
		match_vassert("c:\\halo\\SOURCE\\memory\\data_packets.c", 43, byte_count==packet_definition->size, csprintf(temporary, "packet '%s' fields added up to #%d bytes but should have been #%d bytes.", packet_definition->name, byte_count, packet_definition->size));
		packet_definition->initialized_flag = TRUE;
	}

	return;
}

boolean data_packet_encode(
	struct data_packet_definition *packet_definition,
	short version,
	void *packet,
	void *buffer,
	short *buffer_size,
	short maximum_buffer_size)
{
	struct data_encoding_state encode_state;

	match_assert("c:\\halo\\SOURCE\\memory\\data_packets.c", 61, packet_definition);
	match_assert("c:\\halo\\SOURCE\\memory\\data_packets.c", 62, buffer && buffer_size);
	match_assert("c:\\halo\\SOURCE\\memory\\data_packets.c", 63, maximum_buffer_size>=0);

	data_packet_verify(packet_definition);
	data_encode_new(&encode_state, buffer, maximum_buffer_size);

	if (version==NONE)
	{
		version = packet_definition->version;
	}

	if (packet_definition->version>0)
	{
		byte temp = (byte)version;

		data_encode_memory(&encode_state, &temp, 1, _1byte);
	}

	_data_packet_encode(packet_definition, &encode_state, version, packet, NULL, packet_definition->fields, NULL);
	*buffer_size = (short)encode_state.offset;

	return !encode_state.overflow_flag;
}

boolean data_packet_decode(
	struct data_packet_definition *packet_definition,
	void *encoded_packet,
	short encoded_packet_size,
	void *decoded_packet,
	short *version_reference,
	short *actual_encoded_packet_size_reference)
{
	struct data_encoding_state decode_state;
	boolean success = FALSE;
	short version;

	match_assert("c:\\halo\\SOURCE\\memory\\data_packets.c", 95, encoded_packet);
	match_assert("c:\\halo\\SOURCE\\memory\\data_packets.c", 96, decoded_packet);
	match_assert("c:\\halo\\SOURCE\\memory\\data_packets.c", 97, encoded_packet_size>=0);

	data_packet_verify(packet_definition);
	data_decode_new(&decode_state, encoded_packet, encoded_packet_size);

	version = !packet_definition->version ? 0 : data_decode_byte(&decode_state);
	if (version<=packet_definition->version)
	{
		_data_packet_decode(packet_definition, &decode_state, version, decoded_packet, NULL, packet_definition->fields, NULL);
		if (!decode_state.overflow_flag)
		{
			success = TRUE;
		}
	}

	if (version_reference)
	{
		*version_reference = version;
	}

	if (actual_encoded_packet_size_reference)
	{
		*actual_encoded_packet_size_reference = (short)decode_state.offset;
	}

	return success;
}

/* ---------- private code */

static void _data_packet_verify(
	struct data_packet_definition *packet_definition,
	short *byte_count_reference,
	struct data_packet_field *first_field,
	short *field_count_reference)
{
	struct data_packet_field *field = first_field;
	short byte_count = 0;

	while (field->type!=__pack_end)
	{
		short size;

		match_vassert("c:\\halo\\SOURCE\\memory\\data_packets.c", 142, field->type>=0 && field->type<NUMBER_OF_PACKET_FIELD_TYPES, csprintf(temporary, "unknown field type in packet '%s' (probably missing '__pack_pack_end')", packet_definition->name));
		match_vassert("c:\\halo\\SOURCE\\memory\\data_packets.c", 144, field->count>0, csprintf(temporary, "field has negative or zero count in packet '%s'", packet_definition->name));

		if (packet_definition->version>=field->first_version && (packet_definition->version<=field->last_version || field->last_version==0))
		{
			switch (field->type)
			{
			case __pack_pad:
				size = field->count;
				break;
			case __pack_char:
				size = field->count;
				break;
			case __pack_short:
				size = field->count*sizeof(short);
				break;
			case __pack_long:
				size = field->count*sizeof(long);
				break;
			case __pack_int64:
				size = field->count*sizeof(_uint64);
				break;
			case __pack_string:
				size = field->count + 1;
				break;
			case __pack_data:
				size = field->count + sizeof(short);
				break;
			case __pack_array:
				{
					short element_byte_count;
					short element_field_count;

					_data_packet_verify(packet_definition, &element_byte_count, field + 1, &element_field_count);
					size = field->count*element_byte_count + sizeof(short);
					field += element_field_count;
				}
				break;
			case __pack_fixed_data:
				size = field->count;
				break;
			case __pack_end:
				size = 0;
				break;
			default:
				match_halt("c:\\halo\\SOURCE\\memory\\data_packets.c", 193);
			}
		}

		field->size = size;
		byte_count += size;
		field++;
	}

	if (field_count_reference)
	{
		*field_count_reference = field - first_field + 1;
	}

	if (byte_count_reference)
	{
		*byte_count_reference = byte_count;
	}

	return;
}

static void _data_packet_encode(
	struct data_packet_definition *packet_definition,
	struct data_encoding_state *encode_state,
	short version,
	void *original_buffer,
	short *byte_count_reference,
	struct data_packet_field *first_field,
	short *field_count_reference)
{
	struct data_packet_field *field = first_field;
	byte *buffer = (byte *)original_buffer;

	while (field->type!=__pack_end)
	{
		if (version>=field->first_version && (version<=field->last_version || field->last_version==0))
		{
			switch (field->type)
			{
			case __pack_pad:
				break;
			case __pack_char:
				data_encode_memory(encode_state, buffer, field->count, _1byte);
				break;
			case __pack_short:
				data_encode_memory(encode_state, buffer, field->count, _2byte);
				break;
			case __pack_long:
				data_encode_memory(encode_state, buffer, field->count, _4byte);
				break;
			case __pack_int64:
				data_encode_memory(encode_state, buffer, field->count, _8byte);
				break;
			case __pack_string:
				data_encode_string(encode_state, (char *)buffer, field->count);
				break;
			case __pack_data:
				{
					byte *data = buffer;
					short data_size = *(short *)data;

					data += sizeof(short);
					match_assert("c:\\halo\\SOURCE\\memory\\data_packets.c", 253, data_size>=0 && data_size<=field->count);
					if (data_size<0 || data_size>field->count)
					{
						data_size = 0;
					}

					data_encode_integer(encode_state, data_size, field->count);
					data_encode_memory(encode_state, data, data_size, _1byte);
				}
				break;
			case __pack_fixed_data:
				data_encode_memory(encode_state, buffer, field->count, _1byte);
				break;
			case __pack_array:
				{
					short element_count = *(short *)buffer;
					short element_index;
					byte *element = buffer + sizeof(short);
					short byte_count;
					short field_count;

					_data_packet_verify(packet_definition, NULL, field + 1, &field_count);
					match_assert("c:\\halo\\SOURCE\\memory\\data_packets.c", 281, element_count>=0 && element_count<=field->count);
					if (element_count<0 || element_count>field->count)
					{
						element_count = 0;
					}

					data_encode_integer(encode_state, element_count, field->count);
					for (element_index = 0; element_index<element_count; element_index++)
					{
						_data_packet_encode(packet_definition, encode_state, version, element, &byte_count, field + 1, NULL);
						element += byte_count;
					}

					field += field_count;
				}
				break;
			default:
				match_halt("c:\\halo\\SOURCE\\memory\\data_packets.c", 297);
			}
		}
		else
		{
			switch (field->type)
			{
			case __pack_pad:
				break;
			case __pack_char:
			case __pack_short:
			case __pack_long:
			case __pack_int64:
			case __pack_fixed_data:
				data_encode_memory(encode_state, NULL, field->count, _1byte);
				break;
			case __pack_string:
				{
					byte temp = 0;

					data_encode_memory(encode_state, &temp, 1, _1byte);
				}
				break;
			case __pack_data:
			case __pack_array:
				data_encode_integer(encode_state, 0, field->count);
				break;
			default:
				match_halt("c:\\halo\\SOURCE\\memory\\data_packets.c", 324);
			}
		}

		buffer += field->size;
		field++;
	}

	if (field_count_reference)
	{
		*field_count_reference = field - first_field + 1;
	}

	if (byte_count_reference)
	{
		*byte_count_reference = buffer - (byte *)original_buffer;
	}

	return;
}

static void _data_packet_decode(
	struct data_packet_definition *packet_definition,
	struct data_encoding_state *decode_state,
	short version,
	void *original_buffer,
	short *byte_count_reference,
	struct data_packet_field *first_field,
	short *field_count_reference)
{
	struct data_packet_field *field = first_field;
	byte *buffer = (byte *)original_buffer;

	while (field->type!=__pack_end)
	{
		if (version>=field->first_version && (version<=field->last_version || field->last_version==0))
		{
			switch (field->type)
			{
			case __pack_char:
				{
					void *data = data_decode_memory(decode_state, field->count, _1byte);

					if (data)
					{
						memcpy(buffer, data, field->count);
					}
				}
				break;
			case __pack_short:
				{
					void *data = data_decode_memory(decode_state, field->count, _2byte);

					if (data)
					{
						memcpy(buffer, data, field->count*sizeof(short));
					}
				}
				break;
			case __pack_long:
				{
					void *data = data_decode_memory(decode_state, field->count, _4byte);

					if (data)
					{
						memcpy(buffer, data, field->count*sizeof(long));
					}
				}
				break;
			case __pack_int64:
				{
					void *data = data_decode_memory(decode_state, field->count, _8byte);

					if (data)
					{
						memcpy(buffer, data, field->count*sizeof(_uint64));
					}
				}
				break;
			case __pack_string:
				{
					char *string = data_decode_string(decode_state, field->count);

					if (string)
					{
						strcpy((char *)buffer, string);
					}
				}
				break;
			case __pack_data:
				{
					short data_size = (short)data_decode_integer(decode_state, field->count);
					void *data;

					*(short *)buffer = data_size;
					data = data_decode_memory(decode_state, data_size, _1byte);
					if (data)
					{
						memcpy(buffer + sizeof(short), data, data_size);
					}
				}
				break;
			case __pack_fixed_data:
				{
					void *data = data_decode_memory(decode_state, field->count, _1byte);

					if (data)
					{
						memcpy(buffer, data, field->count);
					}
				}
				break;
			case __pack_array:
				{
					short element_count = (short)data_decode_integer(decode_state, field->count);
					byte *element;
					short byte_count;
					short field_count;
					short element_index;

					_data_packet_verify(packet_definition, NULL, field + 1, &field_count);
					if (element_count<0 || element_count>field->count)
					{
						element_count = 0;
					}

					*(short *)buffer = element_count;
					element = buffer + sizeof(short);
					for (element_index = 0; element_index<element_count; element_index++)
					{
						_data_packet_decode(packet_definition, decode_state, version, element, &byte_count, field + 1, NULL);
						element += byte_count;
					}

					field += field_count;
				}
				break;
			}
		}
		else
		{
			memset(buffer, 0, field->size);
		}

		buffer += field->size;
		field++;
	}

	if (field_count_reference)
	{
		*field_count_reference = field - first_field + 1;
	}

	if (byte_count_reference)
	{
		*byte_count_reference = buffer - (byte *)original_buffer;
	}

	return;
}
