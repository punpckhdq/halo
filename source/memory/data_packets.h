/*
DATA_PACKETS.H

header included in hcex build.
*/

#ifndef __DATA_PACKETS_H
#define __DATA_PACKETS_H
#pragma once

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes/DATA_PACKETS.C */

void data_packet_verify(struct data_packet_definition *packet_definition);
boolean data_packet_encode(struct data_packet_definition *packet_definition, long packet_version, void const *decoded_packet, void *buffer, short *buffer_size, short maximum_buffer_size);
boolean data_packet_decode(struct data_packet_definition *packet_definition, void const *encoded_packet, short encoded_packet_size, void *decoded_packet, short *packet_version, short *decoded_packet_size);

/* ---------- globals */

/* ---------- public code */

#endif // __DATA_PACKETS_H
