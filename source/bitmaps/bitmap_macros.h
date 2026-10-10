/*
BITMAP_MACROS.H

header included in hcex build.
*/

#ifndef __BITMAP_MACROS_H
#define __BITMAP_MACROS_H
#pragma once

/* ---------- constants */

enum
{
	ALPHA_BITS = 8,
	ALPHA_ONE = MASK(ALPHA_BITS),
	PIXEL8_MAXIMUM_COLORS = 1<<8,
	PIXEL16_555_MAXIMUM_COLORS = 1<<15,
	PIXEL16_565_MAXIMUM_COLORS = 1<<16,
	PIXEL32_MAXIMUM_COLORS = 1<<24,
	PIXEL8_BITS = 8,
	PIXEL16_555_COMPONENT_BITS = 5,
	PIXEL16_565_RED_COMPONENT_BITS = 5,
	PIXEL16_565_GREEN_COMPONENT_BITS = 6,
	PIXEL16_565_BLUE_COMPONENT_BITS = 5,
	PIXEL32_COMPONENT_BITS = 8,
	PIXEL16_4444_COMPONENT_BITS = 4,
	PIXEL16_4444_MAXIMUM_COMPONENT = 1<<PIXEL16_4444_COMPONENT_BITS,
	PIXEL16_4444_COMPONENT_MASK = MASK(PIXEL16_4444_COMPONENT_BITS),
	NUMBER_OF_COLOR_COMPONENTS = 3,
	PIXEL16_555_MAXIMUM_COMPONENT = 1<<PIXEL16_555_COMPONENT_BITS,
	PIXEL16_565_MAXIMUM_RED_COMPONENT = 1<<PIXEL16_565_RED_COMPONENT_BITS,
	PIXEL16_565_MAXIMUM_GREEN_COMPONENT = 1<<PIXEL16_565_GREEN_COMPONENT_BITS,
	PIXEL16_565_MAXIMUM_BLUE_COMPONENT = 1<<PIXEL16_565_BLUE_COMPONENT_BITS,
	PIXEL32_MAXIMUM_COMPONENT = 1<<PIXEL32_COMPONENT_BITS,
	PIXEL8_MASK = MASK(PIXEL8_BITS),
	PIXEL16_555_COMPONENT_MASK = MASK(PIXEL16_555_COMPONENT_BITS),
	PIXEL16_565_RED_COMPONENT_MASK = MASK(PIXEL16_565_RED_COMPONENT_BITS),
	PIXEL16_565_GREEN_COMPONENT_MASK = MASK(PIXEL16_565_GREEN_COMPONENT_BITS),
	PIXEL16_565_BLUE_COMPONENT_MASK = MASK(PIXEL16_565_BLUE_COMPONENT_BITS),
	PIXEL32_COMPONENT_MASK = MASK(PIXEL32_COMPONENT_BITS),
	PIXEL32_ONE_HALF = PIXEL32_COMPONENT_MASK/2,
	RGB_COLOR_BITS = 16
};

/* ---------- macros */

#define PIXEL32_ALPHA(pixel) ((pixel)>>24) /* fake name */
#define PIXEL32_TO_PIXEL16_565(pixel) ((word)((((pixel)&0xff)>>3) | (((((pixel)>>8)&0xff)>>2)<<5) | (((((pixel)>>16)&0xff)>>3)<<11))) /* fake name */
#define REAL_ARGB_COLOR_TO_PIXEL32(color) ((((((pixel32)((color)->alpha*255.f)<<8) | (pixel32)((color)->red*255.f))<<8 | (pixel32)((color)->green*255.f))<<8) | (pixel32)((color)->blue*255.f)) /* fake name */

#define PIXEL32_COMPONENT(pixel, shift) (((pixel)>>(shift))&0xff) /* fake name */
#define PIXEL32_RED(pixel) PIXEL32_COMPONENT(pixel, 16) /* fake name */
#define PIXEL32_GREEN(pixel) PIXEL32_COMPONENT(pixel, 8) /* fake name */
#define PIXEL32_BLUE(pixel) ((pixel)&0xff) /* fake name */

#define PIXEL32_ALPHA_MASK 0xff000000 /* fake name */
#define PIXEL32_RGB_MASK 0x00ffffff /* fake name */
#define PIXEL32_ALPHA_BITS(pixel) ((pixel)&PIXEL32_ALPHA_MASK) /* fake name */
#define PIXEL32_RGB_BITS(pixel) ((pixel)&PIXEL32_RGB_MASK) /* fake name */

#define PIXEL32_FROM_ARGB(alpha, red, green, blue) (((alpha)<<24) | ((red)<<16) | ((green)<<8) | (blue)) /* fake name */
#define ROUNDED_SHIFT_RIGHT(value, shift) (((value)+(1<<((shift)-1)))>>(shift)) /* fake name */

#define PIXEL32_TO_PIXEL16_A8Y8(pixel) ((word)((pixel)>>16)) /* fake name */
#define PIXEL16_A8Y8_ALPHA(pixel) ((pixel)>>8) /* fake name */

#define PIXEL32_MODULATE_COMPONENT(pixel, color, shift) ((PIXEL32_COMPONENT(pixel, shift)*PIXEL32_COMPONENT(color, shift)>>8)<<(shift)) /* fake name */
#define PIXEL32_MODULATE(pixel, color) \
	(PIXEL32_MODULATE_COMPONENT(pixel, color, 24) | \
	PIXEL32_MODULATE_COMPONENT(pixel, color, 16) | \
	PIXEL32_MODULATE_COMPONENT(pixel, color, 8) | \
	(PIXEL32_BLUE(pixel)*PIXEL32_BLUE(color)>>8)) /* fake name */

#define PIXEL32_BLEND_COMPONENT(source, destination, alpha, inverse_alpha, shift) \
	((((PIXEL32_COMPONENT(source, shift)*(alpha))>>8) + ((PIXEL32_COMPONENT(destination, shift)*(inverse_alpha))>>8))<<(shift)) /* fake name */
#define PIXEL32_BLEND(source, destination, alpha, inverse_alpha) \
	(PIXEL32_BLEND_COMPONENT(source, destination, alpha, inverse_alpha, 16) | \
	PIXEL32_BLEND_COMPONENT(source, destination, alpha, inverse_alpha, 8) | \
	(((PIXEL32_BLUE(source)*(alpha))>>8) + ((PIXEL32_BLUE(destination)*(inverse_alpha))>>8)) | \
	MAX(alpha, (destination)>>24)<<24) /* fake name */

#define PIXEL16_1555_BLEND(source, destination, alpha, inverse_alpha) \
	((word) \
	(((((destination)&0x1f)*(inverse_alpha) + ((source)&0x1f)*(alpha))>>8)&0x1f) | \
	(((((destination)&0x3ff)*(inverse_alpha) + ((source)&0x3ff)*(alpha))>>8)&0x3e0) | \
	((((destination)*(inverse_alpha) + (source)*(alpha))>>8)&0x7c00)) /* fake name */
#define PIXEL16_565_BLEND(source, destination, alpha, inverse_alpha) \
	((word) \
	(((((destination)&0x1f)*(inverse_alpha) + ((source)&0x1f)*(alpha))>>8)&0x1f) | \
	(((((destination)&0x7ff)*(inverse_alpha) + ((source)&0x7ff)*(alpha))>>8)&0x7e0) | \
	((((destination)*(inverse_alpha) + (source)*(alpha))>>8)&0xf800)) /* fake name */

#define PIXEL32_TO_PIXEL16_1555(pixel) ((word)(((PIXEL32_ALPHA(pixel) ? 0x80 : 0)<<8) | ((PIXEL32_RED(pixel)>>3)<<10) | ((PIXEL32_GREEN(pixel)>>3)<<5) | (PIXEL32_BLUE(pixel)>>3))) /* fake name */
#define PIXEL32_TO_PIXEL16_4444(pixel) ((word)(((PIXEL32_ALPHA(pixel)>>4)<<12) | ((PIXEL32_RED(pixel)>>4)<<8) | ((PIXEL32_GREEN(pixel)>>4)<<4) | (PIXEL32_BLUE(pixel)>>4))) /* fake name */

#define PIXEL32_TO_PIXEL16_565_COLOR(color) (((((color)>>16)&0xf8)<<5 | (((color)>>8)&0xfc))<<3 | (((color)>>3)&0x1f)) /* fake name */
#define PIXEL32_TO_PIXEL16_4444_COLOR(color) ((((color)>>16)&0xf000) | (((color)>>12)&0xf00) | (((color)>>8)&0xf0) | (((color)>>4)&0xf)) /* fake name */

#define PIXEL16_1555_TO_PIXEL16_565(pixel) ((word)((((pixel)<<1)&0xffc0) | ((pixel)&0x3f))) /* fake name */
#define PIXEL16_565_TO_PIXEL16_1555(pixel) ((word)((((pixel)>>1)&0x7fe0) | ((pixel)&0x1f))) /* fake name */

#define PIXEL16_1555_RED(pixel) ((((pixel)>>10)<<2) | (((pixel)>>10)<<3)) /* fake name */
#define PIXEL16_1555_GREEN(pixel) (((((pixel)>>5)&0x1f)<<2) | ((((pixel)>>5)&0x1f)<<3)) /* fake name */
#define PIXEL16_1555_BLUE(pixel) ((((pixel)&0x1f)<<2) | (((pixel)&0x1f)<<3)) /* fake name */
#define PIXEL16_1555_TO_PIXEL32(pixel) \
	(((((pixel)>>15)&0x1)*0xff)<<24 | \
	(((((pixel)>>10)&0x1f)<<3 | (((pixel)>>10)&0x1f)>>2)<<16) | \
	(((((pixel)>>5)&0x1f)<<3 | (((pixel)>>5)&0x1f)>>2)<<8) | \
	((((pixel)&0x1f)<<3) | (((pixel)&0x1f)>>2))) /* fake name */

#define PIXEL16_565_RED(pixel) (((pixel)>>13) | (((pixel)>>11)<<3)) /* fake name */
#define PIXEL16_565_GREEN(pixel) (((((pixel)>>5)&0x3f)>>4) | ((((pixel)>>5)&0x3f)<<2)) /* fake name */
#define PIXEL16_565_BLUE(pixel) ((((pixel)&0x1f)>>2) | (((pixel)&0x1f)<<3)) /* fake name */
#define PIXEL16_565_TO_PIXEL32(pixel) \
	(0xff000000 | \
	(((((pixel)>>11)&0x1f)<<3 | (((pixel)>>11)&0x1f)>>2)<<16) | \
	(((((pixel)>>5)&0x3f)<<2 | (((pixel)>>5)&0x3f)>>4)<<8) | \
	((((pixel)&0x1f)<<3) | (((pixel)&0x1f)>>2))) /* fake name */

#define PIXEL16_4444_COMPONENT(pixel, shift) (((pixel)>>(shift))&0xf) /* fake name */
#define PIXEL16_4444_ALPHA(pixel) (PIXEL16_4444_COMPONENT(pixel, 12) | (PIXEL16_4444_COMPONENT(pixel, 12)<<4)) /* fake name */
#define PIXEL16_4444_RED(pixel) (PIXEL16_4444_COMPONENT(pixel, 8) | (PIXEL16_4444_COMPONENT(pixel, 8)<<4)) /* fake name */
#define PIXEL16_4444_GREEN(pixel) (PIXEL16_4444_COMPONENT(pixel, 4) | (PIXEL16_4444_COMPONENT(pixel, 4)<<4)) /* fake name */
#define PIXEL16_4444_BLUE(pixel) (((pixel)&0xf) | (((pixel)&0xf)<<4)) /* fake name */
#define PIXEL16_4444_COMPONENT_TO_PIXEL32(pixel, shift) (((PIXEL16_4444_COMPONENT(pixel, shift)<<4) | PIXEL16_4444_COMPONENT(pixel, shift))<<((shift)*2)) /* fake name */
#define PIXEL16_4444_TO_PIXEL32(pixel) \
	(PIXEL16_4444_COMPONENT_TO_PIXEL32(pixel, 12) | \
	PIXEL16_4444_COMPONENT_TO_PIXEL32(pixel, 8) | \
	PIXEL16_4444_COMPONENT_TO_PIXEL32(pixel, 4) | \
	((((pixel)&0xf)<<4) | ((pixel)&0xf))) /* fake name */
#define PIXEL16_4444_MODULATE(pixel, color) \
	((PIXEL16_4444_COMPONENT(pixel, 12)*PIXEL16_4444_COMPONENT(color, 12))<<24 | \
	(PIXEL16_4444_COMPONENT(pixel, 8)*PIXEL16_4444_COMPONENT(color, 8))<<16 | \
	(PIXEL16_4444_COMPONENT(pixel, 4)*PIXEL16_4444_COMPONENT(color, 4))<<8 | \
	(((pixel)&0xf)*((color)&0xf))) /* fake name */

/* ---------- structures */

/* ---------- prototypes/EXAMPLE.C */

/* ---------- globals */

/* ---------- public code */

#endif // __BITMAP_MACROS_H
