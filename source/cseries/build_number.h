/*
BUILD_NUMBER.H

header included in hcex build.
*/

#ifndef __BUILD_NUMBER_H
#define __BUILD_NUMBER_H
#pragma once

/* ---------- macros */

#define BUILD_NAME "halobeta xbox"

#if CONFIG_VERSION == 2276
	#define BUILD_NUMBER "01.10.12.2276"
#else
	#define BUILD_NUMBER "01.01.14.2342"
#endif

#define BUILD_STRING BUILD_NAME " " BUILD_NUMBER

#ifdef NON_MATCHING
	#define BUILD_DATE __DATE__
	#define BUILD_TIME __TIME__
#elif CONFIG_VERSION == 2276
	#define BUILD_DATE "Oct 12 2001"
	#define BUILD_TIME "16:07:48"
#else
	#define BUILD_DATE "Jan 14 2002"
	#define BUILD_TIME "12:49:20"
#endif

#endif // __BUILD_NUMBER_H
