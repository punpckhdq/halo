/*
BITMAPS_INLINES.H

file has inline function assertions.
*/

#ifndef __BITMAPS_INLINES_H
#define __BITMAPS_INLINES_H
#pragma once

/* ---------- headers */

#include "bitmap_macros.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes/BITMAPS_INLINES.H */

pixel32 real_rgb_color_to_pixel32(union real_rgb_color const *color);
pixel32 real_argb_color_to_pixel32(real_argb_color const *color);
pixel32 real_a_rgb_color_to_pixel32(real alpha, union real_rgb_color const *color);

/* ---------- globals */

/* ---------- public code */

// these are ASM blocks in sapien as well
__inline pixel32 real_argb_color_to_pixel32(
	real_argb_color const *color)
{
	real scale = 255.f;
	pixel32 result;

	match_vassert("..\\bitmaps\\bitmaps_inlines.h", 89, valid_real_argb_color(color), csprintf(temporary, "%s: assert_valid_real_argb_color(%f, %f, %f, %f)", "color", color->alpha, color->red, color->green, color->blue));

	{
		long alpha;
		long red;
		long green;
		long blue;

		__asm
		{
			mov edx, color
			fld dword ptr [edx]			// alpha
			fld dword ptr [edx + 4]		// red
			fld dword ptr [edx + 8]		// green
			fld dword ptr [edx + 12]	// blue
			fld scale
			fmul st(4), st				// scale all four by 255
			fmul st(3), st
			fmul st(2), st
			fmulp st(1), st
			fistp blue					// round each to an integer
			fistp green
			fistp red
			fistp alpha
			mov edx, blue				// pack as 0xAARRGGBB
			mov ebx, green
			mov ecx, red
			mov eax, alpha
			shl ebx, 8
			shl ecx, 16
			shl eax, 24
			or edx, ebx
			or edx, ecx
			or edx, eax
			mov result, edx
		}
	}

	{
		pixel32 verify;

		// same conversion again, masking each channel to a byte
		__asm
		{
			mov edx, color
			fld dword ptr [edx]			// alpha
			fld dword ptr [edx + 4]		// red
			fld dword ptr [edx + 8]		// green
			fld dword ptr [edx + 12]	// blue
			fld scale
			fmul st(4), st				// scale all four by 255
			fmul st(3), st
			fmul st(2), st
			fmulp st(1), st
			fistp verify				// blue
			and verify, UNSIGNED_CHAR_MAX
			mov edx, verify
			fistp verify				// green
			and verify, UNSIGNED_CHAR_MAX
			shl verify, 8
			or edx, verify
			fistp verify				// red
			and verify, UNSIGNED_CHAR_MAX
			shl verify, 16
			or edx, verify
			fistp verify				// alpha
			shl verify, 24
			or edx, verify
			mov verify, edx
		}

		match_assert("..\\bitmaps\\bitmaps_inlines.h", 188, verify == result);
	}

	return result;
}

__inline pixel32 real_rgb_color_to_pixel32(
	real_rgb_color const *color)
{
	pixel32 result;
	real scale = 255.f;

	match_assert_valid_real_rgb_color("..\\bitmaps\\bitmaps_inlines.h", 201, color);

	__asm
	{
		mov edx, color
		fld dword ptr [edx]			// red
		fld dword ptr [edx + 4]		// green
		fld dword ptr [edx + 8]		// blue
		fld scale
		fmul st(3), st				// scale all three by 255
		fmul st(2), st
		fmulp st(1), st
		fistp result				// blue
		and result, UNSIGNED_CHAR_MAX
		mov edx, result
		fistp result				// green
		and result, UNSIGNED_CHAR_MAX
		shl result, 8
		or edx, result
		fistp result				// red
		and result, UNSIGNED_CHAR_MAX
		shl result, 16
		or edx, result				// 0x00RRGGBB, no alpha
		mov result, edx
	}

	return result;
}

__inline pixel32 real_alpha_intensity_to_pixel32(
	real alpha,
	real intensity)
{
	real_argb_color color;

	match_assert("..\\bitmaps\\bitmaps_inlines.h", 319, alpha>=0.0f && alpha<=1.0f);
	match_assert("..\\bitmaps\\bitmaps_inlines.h", 320, intensity>=0.0f && intensity<=1.0f);

	color.alpha = alpha;
	color.red = intensity;
	color.green = intensity;
	color.blue = intensity;

	return real_argb_color_to_pixel32(&color);
}

#endif // __BITMAPS_INLINES_H
