/*
MESSAGE_ENCRYPTION.H

header included in hcex build.
*/

#ifndef __MESSAGE_ENCRYPTION_H
#define __MESSAGE_ENCRYPTION_H
#pragma once

/* ---------- headers */

#include "message_header.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes/MESSAGE_ENCRYPTION.C */

void message_encrypt(message_header *msgptr, unsigned long const *key);
void message_decrypt(message_header *msgptr, unsigned long const *key);
void reversible_crypt(byte *data, long data_size, byte const *key, long key_size);
void tea_encipher(unsigned long const *input, unsigned long *output, long const *key);
void tea_decipher(unsigned long const *input, unsigned long *output, long const *key);

/* ---------- globals */

/* ---------- public code */

#endif // __MESSAGE_ENCRYPTION_H
