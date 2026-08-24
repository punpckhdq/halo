/*
MESSAGE_ENCRYPTION.H

header included in hcex build.
*/

#ifndef __MESSAGE_ENCRYPTION_H
#define __MESSAGE_ENCRYPTION_H
#pragma once

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes/MESSAGE_ENCRYPTION.C */

void reversible_crypt(byte *data, long data_size, byte const *key, long key_size);
void tea_encipher(unsigned long const input[2], unsigned long output[2], long const key[4]);
void tea_decipher(unsigned long const input[2], unsigned long output[2], long const key[4]);
void message_encrypt(word *msgptr, unsigned long const key[2]);
void message_decrypt(word *msgptr, unsigned long const key[2]);

/* ---------- globals */

/* ---------- public code */

#endif // __MESSAGE_ENCRYPTION_H
