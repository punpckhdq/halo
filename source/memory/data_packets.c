/*
DATA_PACKETS.C
*/

/* ---------- headers */

#include "cseries.h"
#include "data_packet_groups.h"
#include "data_packets.h"

#include "byte_swapping.h"
#include "data_encoding.h"

/* ---------- constants */

enum
{
	NUMBER_OF_DATA_PACKET_FIELD_TYPES= 10
};

/* ---------- prototypes */

static void _data_packet_verify(struct data_packet_definition *packet_definition, short *packet_size, struct data_packet_field *fields, short *field_count);
static void _data_packet_encode(struct data_packet_definition *packet_definition, struct data_encoding_state *state, short packet_version, void const *source, short *packet_size, struct data_packet_field *fields, short *field_count);
static void _data_packet_decode(struct data_packet_definition *packet_definition, struct data_encoding_state *state, short packet_version, void *destination, short *packet_size, struct data_packet_field *fields, short *field_count);

/* ---------- public code */

void data_packet_verify(
	struct data_packet_definition *packet_definition)
{
	match_assert("c:\\halo\\SOURCE\\memory\\data_packets.c", 32, packet_definition);
	match_assert("c:\\halo\\SOURCE\\memory\\data_packets.c", 33, packet_definition->size>=0);
	match_assert("c:\\halo\\SOURCE\\memory\\data_packets.c", 34, packet_definition->version>=0);
	match_assert("c:\\halo\\SOURCE\\memory\\data_packets.c", 35, packet_definition->name && packet_definition->fields);

	if (!packet_definition->initialized)
	{
		short size, field_count;

		_data_packet_verify(packet_definition, &size, packet_definition->fields, &field_count);

		match_vassert("c:\\halo\\SOURCE\\memory\\data_packets.c", 43, size==packet_definition->size, csprintf(temporary, "packet '%s' fields added up to #%d bytes but should have been #%d bytes.", packet_definition->name, size, packet_definition->size));

		packet_definition->initialized= TRUE;
	}
}

boolean data_packet_encode(
	struct data_packet_definition *packet_definition,
	long packet_version,
	void const *decoded_packet,
	void *buffer,
	short *buffer_size,
	short maximum_buffer_size)
{
	struct data_encoding_state state;
	short version;

	match_assert("c:\\halo\\SOURCE\\memory\\data_packets.c", 61, packet_definition);
	match_assert("c:\\halo\\SOURCE\\memory\\data_packets.c", 62, buffer && buffer_size);
	match_assert("c:\\halo\\SOURCE\\memory\\data_packets.c", 63, maximum_buffer_size>=0);

	data_packet_verify(packet_definition);

	data_encode_new(&state, buffer, maximum_buffer_size);

	version= (short)packet_version;
	if (version==NONE)
	{
		version= packet_definition->version;
	}

	// versioned packets carry the version they were encoded at in their first byte
	if (packet_definition->version>0)
	{
		byte encoded_version= (byte)version;

		data_encode_memory(&state, &encoded_version, 1, _1byte);
	}

	_data_packet_encode(packet_definition, &state, version, decoded_packet, NULL, packet_definition->fields, NULL);

	*buffer_size= (short)state.offset;

	return !state.overflow;
}

boolean data_packet_decode(
	struct data_packet_definition *packet_definition,
	void const *encoded_packet,
	short encoded_packet_size,
	void *decoded_packet,
	short *packet_version,
	short *decoded_packet_size)
{
	struct data_encoding_state state;
	boolean success= FALSE;
	short version;

	match_assert("c:\\halo\\SOURCE\\memory\\data_packets.c", 95, encoded_packet);
	match_assert("c:\\halo\\SOURCE\\memory\\data_packets.c", 96, decoded_packet);
	match_assert("c:\\halo\\SOURCE\\memory\\data_packets.c", 97, encoded_packet_size>=0);

	data_packet_verify(packet_definition);

	data_decode_new(&state, encoded_packet, encoded_packet_size);

	// an unversioned packet has no version byte in front of it
	if (packet_definition->version==0)
	{
		version= 0;
	}
	else
	{
		version= data_decode_byte(&state);
	}

	// we can read anything up to the version we were built against, but nothing beyond it
	if (version<=packet_definition->version)
	{
		_data_packet_decode(packet_definition, &state, version, decoded_packet, NULL, packet_definition->fields, NULL);

		if (!state.overflow)
		{
			success= TRUE;
		}
	}

	if (packet_version)
	{
		*packet_version= version;
	}

	if (decoded_packet_size)
	{
		*decoded_packet_size= (short)state.offset;
	}

	return success;
}

/* ---------- private code */

// walks one run of fields, filling in each field's size and handing back what the run adds up to;
// a nested array recurses and swallows the run between it and its terminator
static void _data_packet_verify(
	struct data_packet_definition *packet_definition,
	short *packet_size,
	struct data_packet_field *fields,
	short *field_count)
{
	struct data_packet_field *field= fields;
	short field_size;
	short size= 0;

	while (field->type!=_data_packet_field_end)
	{
		match_vassert("c:\\halo\\SOURCE\\memory\\data_packets.c", 142, field->type>=0 && field->type<NUMBER_OF_DATA_PACKET_FIELD_TYPES, csprintf(temporary, "unknown field type in packet '%s' (probably missing '__pack_pack_end')", packet_definition->name));
		match_vassert("c:\\halo\\SOURCE\\memory\\data_packets.c", 144, field->count>0, csprintf(temporary, "field has negative or zero count in packet '%s'", packet_definition->name));

		if (packet_definition->version>=field->minimum_version &&
			(packet_definition->version<=field->maximum_version || field->maximum_version==0))
		{
			switch (field->type)
			{
				case 0:
				case 1:
				case 8:
					field_size= field->count;
					break;

				case 2:
					field_size= field->count*2;
					break;

				case _data_packet_field_longs:
					field_size= field->count*4;
					break;

				case 4:
					field_size= field->count*8;
					break;

				case 5:
					field_size= field->count+1;
					break;

				case 6:
					field_size= field->count+sizeof(short);
					break;

				case 7:
				{
					short element_size, element_field_count;

					_data_packet_verify(packet_definition, &element_size, field+1, &element_field_count);

					field_size= field->count*element_size+sizeof(short);
					field += element_field_count;
				}
				break;

				case _data_packet_field_end:
					field_size= 0;
					break;

				default:
					match_halt("c:\\halo\\SOURCE\\memory\\data_packets.c", 193);
			}
		}

		field->size= field_size;
		size += field_size;

		field++;
	}

	if (field_count)
	{
		*field_count= field - fields + 1;
	}

	if (packet_size)
	{
		*packet_size= size;
	}
}

static void _data_packet_encode(
	struct data_packet_definition *packet_definition,
	struct data_encoding_state *state,
	short packet_version,
	void const *source,
	short *packet_size,
	struct data_packet_field *fields,
	short *field_count)
{
	struct data_packet_field *field= fields;
	byte const *data= source;

	while (field->type!=_data_packet_field_end)
	{
		if (packet_version>=field->minimum_version &&
			(packet_version<=field->maximum_version || field->maximum_version==0))
		{
			switch (field->type)
			{
				case 0:
					break;

				case 1:
					data_encode_memory(state, data, field->count, _1byte);
					break;

				case 2:
					data_encode_memory(state, data, field->count, _2byte);
					break;

				case _data_packet_field_longs:
					data_encode_memory(state, data, field->count, _4byte);
					break;

				case 4:
					data_encode_memory(state, data, field->count, _8byte);
					break;

				case 5:
					data_encode_string(state, (char const *)data, field->count);
					break;

				case 6:
				{
					short data_size= *(short const *)data;

					match_assert("c:\\halo\\SOURCE\\memory\\data_packets.c", 253, data_size>=0 && data_size<=field->count);

					if (data_size<0 || data_size>field->count)
					{
						data_size= 0;
					}

					data_encode_integer(state, data_size, field->count);
					data_encode_memory(state, (short const *)data + 1, data_size, _1byte);
				}
				break;

				case 8:
					data_encode_memory(state, data, field->count, _1byte);
					break;

				case 7:
				{
					short element_count= *(short const *)data;
					byte const *element_data= (byte const *)((short const *)data + 1);
					short element_index, element_field_count;

					_data_packet_verify(packet_definition, NULL, field+1, &element_field_count);

					match_assert("c:\\halo\\SOURCE\\memory\\data_packets.c", 281, element_count>=0 && element_count<=field->count);

					if (element_count<0 || element_count>field->count)
					{
						element_count= 0;
					}

					data_encode_integer(state, element_count, field->count);

					for (element_index= 0; element_index<element_count; element_index++)
					{
						short element_size;

						_data_packet_encode(packet_definition, state, packet_version, element_data, &element_size, field+1, NULL);

						element_data += element_size;
					}

					field += element_field_count;
				}
				break;

				default:
					match_halt("c:\\halo\\SOURCE\\memory\\data_packets.c", 297);
			}
		}
		else
		{
			// a field the far end still expects has to take up its room on the wire
			switch (field->type)
			{
				case 0:
					break;

				case 1:
				case 2:
				case _data_packet_field_longs:
				case 4:
				case 8:
					data_encode_memory(state, NULL, field->count, _1byte);
					break;

				case 5:
				{
					byte terminator= 0;

					data_encode_memory(state, &terminator, 1, _1byte);
				}
				break;

				case 6:
				case 7:
					data_encode_integer(state, 0, field->count);
					break;

				default:
					match_halt("c:\\halo\\SOURCE\\memory\\data_packets.c", 324);
			}
		}

		data += field->size;

		field++;
	}

	if (field_count)
	{
		*field_count= field - fields + 1;
	}

	if (packet_size)
	{
		*packet_size= (short)(data - (byte const *)source);
	}
}

static void _data_packet_decode(
	struct data_packet_definition *packet_definition,
	struct data_encoding_state *state,
	short packet_version,
	void *destination,
	short *packet_size,
	struct data_packet_field *fields,
	short *field_count)
{
	struct data_packet_field *field= fields;
	byte *data= destination;

	while (field->type!=_data_packet_field_end)
	{
		if (packet_version>=field->minimum_version &&
			(packet_version<=field->maximum_version || field->maximum_version==0))
		{
			switch (field->type)
			{
				case 1:
				{
					void *decoded= data_decode_memory(state, field->count, _1byte);

					if (decoded)
					{
						csmemcpy(data, decoded, field->count);
					}
				}
				break;

				case 2:
				{
					void *decoded= data_decode_memory(state, field->count, _2byte);

					if (decoded)
					{
						csmemcpy(data, decoded, field->count*2);
					}
				}
				break;

				case _data_packet_field_longs:
				{
					void *decoded= data_decode_memory(state, field->count, _4byte);

					if (decoded)
					{
						csmemcpy(data, decoded, field->count*4);
					}
				}
				break;

				case 4:
				{
					void *decoded= data_decode_memory(state, field->count, _8byte);

					if (decoded)
					{
						csmemcpy(data, decoded, field->count*8);
					}
				}
				break;

				case 5:
				{
					char *decoded= data_decode_string(state, field->count);

					if (decoded)
					{
						csstrcpy((char *)data, decoded);
					}
				}
				break;

				case 6:
				{
					short data_size= data_decode_integer(state, field->count);
					void *decoded;

					*(short *)data= data_size;

					decoded= data_decode_memory(state, data_size, _1byte);

					if (decoded)
					{
						csmemcpy((short *)data + 1, decoded, data_size);
					}
				}
				break;

				case 8:
				{
					void *decoded= data_decode_memory(state, field->count, _1byte);

					if (decoded)
					{
						csmemcpy(data, decoded, field->count);
					}
				}
				break;

				case 7:
				{
					short element_count= data_decode_integer(state, field->count);
					short element_index, element_field_count;
					byte *element_data;

					_data_packet_verify(packet_definition, NULL, field+1, &element_field_count);

					if (element_count<0 || element_count>field->count)
					{
						element_count= 0;
					}

					*(short *)data= element_count;
					element_data= (byte *)((short *)data + 1);

					for (element_index= 0; element_index<element_count; element_index++)
					{
						short element_size;

						_data_packet_decode(packet_definition, state, packet_version, element_data, &element_size, field+1, NULL);

						element_data += element_size;
					}

					field += element_field_count;
				}
				break;

				default:
					break;
			}
		}
		else
		{
			csmemset(data, 0, field->size);
		}

		data += field->size;

		field++;
	}

	if (field_count)
	{
		*field_count= field - fields + 1;
	}

	if (packet_size)
	{
		*packet_size= (short)(data - (byte *)destination);
	}
}
