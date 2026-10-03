/*
S3TC.H

header included in hcex build.
*/

#ifndef __S3TC_H
#define __S3TC_H
#pragma once

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

struct S3TC_COLOR
{
	byte rgba[4];
};

struct S3TCBlockRGB
{
	word rgb0;
	word rgb1;
	unsigned long pixbm;
};

struct S3TCBlockAlpha4
{
	word alphabm[4];
	struct S3TCBlockRGB rgb;
};

struct S3TCBlockAlpha3
{
	byte alpha0;
	byte alpha1;
	byte alphabm[6];
	struct S3TCBlockRGB rgb;
};

/* ---------- prototypes/S3TC.C */

void EncodeBlockRGBColorKey(
	struct S3TC_COLOR *colorSrc,
	struct S3TCBlockRGB *pblockDst,
	byte alphaKey);
void DecodeBlockRGB(
	struct S3TCBlockRGB *pblockSrc,
	struct S3TC_COLOR *colorDst);
void DecodeBlockRGB__single_pixel(
	struct S3TCBlockRGB const *pblockSrc,
	struct S3TC_COLOR *colorDst,
	short u,
	short v);
void DecodeBlockAlpha4(
	struct S3TCBlockAlpha4 *pblockSrc,
	struct S3TC_COLOR *colorDst);
void DecodeBlockAlpha4__single_pixel(
	struct S3TCBlockAlpha4 const *pblockSrc,
	struct S3TC_COLOR *colorDst,
	short u,
	short v);
void DecodeBlockAlpha3(
	struct S3TCBlockAlpha3 *pblockSrc,
	struct S3TC_COLOR *colorDst);
void DecodeBlockAlpha3__single_pixel(
	struct S3TCBlockAlpha3 const *pblockSrc,
	struct S3TC_COLOR *colorDst,
	short u,
	short v);
void EncodeBlockRGB(
	struct S3TC_COLOR *colorSrc,
	struct S3TCBlockRGB *pblockDst);
void EncodeBlockAlpha4(
	struct S3TC_COLOR *colorSrc,
	struct S3TCBlockAlpha4 *pblockDst);
void EncodeBlockAlpha3(
	struct S3TC_COLOR *colorSrc,
	struct S3TCBlockAlpha3 *pblockDst);

/* ---------- globals */

/* ---------- public code */

#endif // __S3TC_H
