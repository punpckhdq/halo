/*
BYTE_SWAPPING.H
*/

#ifndef __BYTE_SWAPPING_H
#define __BYTE_SWAPPING_H
#pragma once

/* ---------- constants */

enum
{
    _1byte= 1,
    _2byte= -2,
    _4byte= -4,
    _8byte= -8,
    _begin_bs_array= -100,
    _end_bs_array= -101,
    _extern_bs_definition= -102,
    BYTE_SWAP_DEFINITION_SIGNATURE= 'bysw'
};

/* ---------- macros */

#define SWAP2(q) (((q)>>8) | (((q)<<8)&0xff00))
#define SWAP4(q) (((q)>>24) | (((q)>>8)&0xff00) | (((q)<<8)&0x00ff0000) | (((q)<<24)&0xff000000))

/* ---------- types */

typedef long byte_swap_code;

struct byte_swap_definition
{
	char *name;
	long size;
	byte_swap_code *codes;
	unsigned long signature;
	boolean verified;
};

/* ---------- prototypes/BYTE_SWAPPING.C */

void byte_swap_memory(void *memory, long count, byte_swap_code code);
long byte_swap_codes_size(char *name, byte_swap_code *codes);
void byte_swap_data(struct byte_swap_definition *definition, void *data, long data_count);
void byte_swap_data_explicit(char *name, long size, byte_swap_code *codes, long data_count, void *data);

#endif // __BYTE_SWAPPING_H
