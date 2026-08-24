/*
KEY_AGREEMENT.H

header included in hcex build.
*/

#ifndef __KEY_AGREEMENT_H
#define __KEY_AGREEMENT_H
#pragma once

/* ---------- headers */

#include "public_key_crypt.h"
#include "bungie_net/network/transport.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes/KEY_AGREEMENT.C */

long is_message_encryption_key_message(word *msgptr, word message_size, byte *packet_type);
boolean initiate_key_exchange(transport_endpoint_ref endpoint, struct public_key *public_key, struct public_key *p, struct public_key *x);
boolean complete_key_exchange(transport_endpoint_ref endpoint, word *msgptr, struct public_key *prime, struct public_key *secret, struct public_key *private_key);
void initialize_key_agreement_packets(void);

/* ---------- globals */

/* ---------- public code */

#endif // __KEY_AGREEMENT_H
