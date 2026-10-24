/*
PROBABILITY.C
*/

/* ---------- headers */

#include "cseries.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

long factorial(
	short n)
{
	long result = 0;

	if (n>=0)
	{
		result = 1;
		while (n>1)
		{
			result*= n;
			n--;
		}
	}

	return result;
}

long permutations(
	short n,
	short r)
{
	long result = 0;

	if (n>=r && r>=0)
	{
		result = 1;
		while (r--)
		{
			result*= n;
			n--;
		}
	}

	return result;
}

long combinations(
	short n,
	short r)
{
	long result = 0;

	if (n>=r && r>=0)
	{
		if (r>n-r)
		{
			r = n-r;
		}

		result = permutations(n, r);
		while (r>1)
		{
			result/= r;
			r--;
		}
	}

	return result;
}

boolean permute(
	short base,
	short count,
	short *indices)
{
	short index;

	match_assert("c:\\halo\\SOURCE\\math\\probability.c", 77, base>0);
	match_assert("c:\\halo\\SOURCE\\math\\probability.c", 78, count>0);
	match_assert("c:\\halo\\SOURCE\\math\\probability.c", 79, indices);

	for (index = 0; index<count; index++)
	{
		if (indices[index]<0 || indices[index]>=base)
		{
			for (index = 0; index<count; index++)
			{
				indices[index] = 0;
			}

			return TRUE;
		}
	}

	for (index = count-1; index>=0; index--)
	{
		if (indices[index]<base-1)
		{
			indices[index]++;
			while (++index<count)
			{
				indices[index] = 0;
			}

			return TRUE;
		}
	}

	return FALSE;
}

boolean combine(
	short base,
	short count,
	short *indices)
{
	short index;

	match_assert("c:\\halo\\SOURCE\\math\\probability.c", 119, base>=count);
	match_assert("c:\\halo\\SOURCE\\math\\probability.c", 120, count>0);
	match_assert("c:\\halo\\SOURCE\\math\\probability.c", 121, indices);

	for (index = 0; index<count; index++)
	{
		if (indices[index]<0 || indices[index]>=base || (index>0 && indices[index]<=indices[index-1]))
		{
			for (index = 0; index<count; index++)
			{
				indices[index] = index;
			}

			return TRUE;
		}
	}

	for (index = count-1; index>=0; index--)
	{
		if (indices[index]<base-(count-index))
		{
			indices[index]++;
			while (++index<count)
			{
				indices[index] = indices[index-1]+1;
			}

			return TRUE;
		}
	}

	return FALSE;
}

/* ---------- private code */
