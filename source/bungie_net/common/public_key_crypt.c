/*
PUBLIC_KEY_CRYPT.C
*/

/* ---------- headers */

#include "cseries.h"
#include "public_key_crypt.h"

#include "64bit_math.h"
#include "random_numbers.h"

/* ---------- constants */

enum
{
	KEY_DWORD_COUNT= 2,
	MINIMUM_PRIME= 0xFFFFFF,
	MAXIMUM_PRIME_FACTOR= 0xFFFF,
	MINIMUM_EXPONENT= 255
};

/* ---------- macros */

// the private key is stored byte-swapped
#define SWAP_KEY(q) (((q)>>24) | (((q)>>8)&0xff00) | (((q)<<8)&0x00ff0000) | (((q)<<24)&0xff000000))

/* ---------- prototypes */

static unsigned long x_exp_y_mod_n(unsigned long x, unsigned long y, unsigned long n);
static unsigned long generate_diffie_hellman_public_key(unsigned long p, unsigned long x, unsigned long g);
static unsigned long generate_diffie_hellman_private_key(unsigned long p, unsigned long x, unsigned long public_key);

/* ---------- private code */

static unsigned long x_exp_y_mod_n(
	unsigned long x,
	unsigned long y,
	unsigned long n)
{
	struct qword_value s;
	struct qword_value xx;
	struct qword_value nn;
	struct qword_value product;

	s.qword= 1;
	xx.qword= x;
	nn.qword= n;

	while (y)
	{
		if (y&1)
		{
			multiply64(&s, &xx, &product);
			divide64(&product, &nn, NULL, &s);
		}
		y>>= 1;
		multiply64(&xx, &xx, &product);
		divide64(&product, &nn, NULL, &xx);
	}

	match_assert("c:\\halo\\SOURCE\\bungie_net\\common\\public_key_crypt.c", 95, s.qword <= 0xFFFFFFFF);

	return (unsigned long)s.qword;
}

static unsigned long generate_diffie_hellman_public_key(
	unsigned long p,
	unsigned long x,
	unsigned long g)
{
	match_assert("c:\\halo\\SOURCE\\bungie_net\\common\\public_key_crypt.c", 112, p>2);
	match_assert("c:\\halo\\SOURCE\\bungie_net\\common\\public_key_crypt.c", 113, x<(p-1));
	match_assert("c:\\halo\\SOURCE\\bungie_net\\common\\public_key_crypt.c", 114, g<p);

	return x_exp_y_mod_n(g, x, p);
}

static unsigned long generate_diffie_hellman_private_key(
	unsigned long p,
	unsigned long x,
	unsigned long public_key)
{
	match_assert("c:\\halo\\SOURCE\\bungie_net\\common\\public_key_crypt.c", 133, p>2);
	match_assert("c:\\halo\\SOURCE\\bungie_net\\common\\public_key_crypt.c", 134, x<(p-1));

	return x_exp_y_mod_n(public_key, x, p);
}

/* ---------- public code */

void generate_key_parameters(
	struct public_key *p,
	struct public_key *x,
	struct public_key *g)
{
	long i= 0;
	while (i<KEY_DWORD_COUNT)
	{
		do
		{
			unsigned long factor= randomprime(MAXIMUM_PRIME_FACTOR);

			p->dwords[i]= factor*randomprime(MAXIMUM_PRIME_FACTOR)+2;
		}
		while (p->dwords[i]<MINIMUM_PRIME);
		x->dwords[i]= randomrange(MINIMUM_EXPONENT, p->dwords[i]-2);
		g->dwords[i]= randomrange(MINIMUM_EXPONENT, p->dwords[i]-1);
		match_assert("c:\\halo\\SOURCE\\bungie_net\\common\\public_key_crypt.c", 162, x->dwords[i] < (p->dwords[i] - 2));
		match_assert("c:\\halo\\SOURCE\\bungie_net\\common\\public_key_crypt.c", 163, g->dwords[i] < (p->dwords[i] - 1));
		i++;
	}

	return;
}

void generate_public_key(
	struct public_key const *p,
	struct public_key const *x,
	struct public_key const *g,
	struct public_key *public_key)
{
	long i;

	for (i= 0; i<KEY_DWORD_COUNT; i++)
	{
		public_key->dwords[i]= generate_diffie_hellman_public_key(p->dwords[i], x->dwords[i], g->dwords[i]);
	}

	error(2, "p= %8lX%8lX\nx= %8lX%8lX\ng= %8lX%8lX\npublic key= %8lX%8lX\n\n",
		p->dwords[0], p->dwords[1], x->dwords[0], x->dwords[1], g->dwords[0], g->dwords[1],
		public_key->dwords[0], public_key->dwords[1]);

	return;
}

void generate_private_key(
	struct public_key const *public_key,
	struct public_key const *p,
	struct public_key const *x,
	struct public_key *private_key)
{
	long i;

	for (i= 0; i<KEY_DWORD_COUNT; i++)
	{
		unsigned long x_dword= x->dwords[i];
		unsigned long p_dword= p->dwords[i];
		unsigned long public_key_dword= public_key->dwords[i];

		private_key->dwords[i]= generate_diffie_hellman_private_key(p_dword, x_dword, public_key_dword);
		private_key->dwords[i]= SWAP_KEY(private_key->dwords[i]);
	}

	error(2, "public_key= %8lX%8lX\np= %8lX%8lX\nx= %8lX%8lX\nprivate key= %8lX%8lX\n\n",
		public_key->dwords[0], public_key->dwords[1], p->dwords[0], p->dwords[1], x->dwords[0], x->dwords[1],
		private_key->dwords[0], private_key->dwords[1]);

	return;
}
