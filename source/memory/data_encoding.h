/*
DATA_ENCODING.H

header included in hcex build.
*/

#ifndef __DATA_ENCODING_H
#define __DATA_ENCODING_H
#pragma once

/* ---------- structures */

struct data_encoding_state
{
	void *buffer;
	long offset;
	long buffer_size;
	boolean overflow_flag;
};

/* ---------- prototypes/DATA_ENCODING.C */

void data_encode_new(struct data_encoding_state *state, void *buffer, long buffer_size);
boolean data_encode_memory(struct data_encoding_state *state, void *buffer, short count, long code);
boolean data_encode_integer(struct data_encoding_state *state, long value, long maximum_value);
boolean data_encode_structures(struct data_encoding_state *state, void *source_structures, short structure_count, struct byte_swap_definition *bs_definition);
boolean data_encode_array(struct data_encoding_state *state, long element_count_size, void *source_array, long element_count, struct byte_swap_definition *bs_definition);
boolean data_encode_string(struct data_encoding_state *state, char *source_string, short maximum_string_length);
void data_decode_new(struct data_encoding_state *state, void *buffer, long buffer_size);
void *data_decode_structures(struct data_encoding_state *state, short structure_count, struct byte_swap_definition *bs_definition);
void *data_decode_memory(struct data_encoding_state *state, short count, long code);
byte data_decode_byte(struct data_encoding_state *state);
short data_decode_short(struct data_encoding_state *state);
long data_decode_long(struct data_encoding_state *state);
unsigned __int64 data_decode_int64(struct data_encoding_state *state);
long data_decode_integer(struct data_encoding_state *state, long maximum_value);
void *data_decode_array(struct data_encoding_state *state, long element_count_size, long *element_count_reference, long maximum_element_count, struct byte_swap_definition *bs_definition);
char *data_decode_string(struct data_encoding_state *state, short maximum_string_length);

#endif // __DATA_ENCODING_H
