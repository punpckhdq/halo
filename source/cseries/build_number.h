/*
BUILD_NUMBER.H

header included in hcex build.
*/

#ifndef __BUILD_NUMBER_H
#define __BUILD_NUMBER_H
#pragma once

/* ---------- macros */

#define BUILD_NAME "halobeta xbox"
#define BUILD_STRING "01.01.14.2342"

#ifdef NON_MATCHING
	#define BUILD_DATE __DATE__
	#define BUILD_TIME __TIME__
#else
	#define BUILD_DATE "Jan 14 2002"
	#define BUILD_TIME "12:49:20"
#endif

#endif // __BUILD_NUMBER_H
