/*
BINK.H

stand-in for the RAD Game Tools bink 1.x sdk header (xbox, 2001).
only declares what bink_playback.c uses; structures are truncated after the last field we touch.
*/

#ifndef __BINK_H
#define __BINK_H
#pragma once

/* ---------- constants */

#define RADLINK __stdcall
#define RADEXPLINK __stdcall
#define RADEXPFUNC

// BinkOpen flags
#define BINKIOSIZE 0x01000000L

// BinkCopyToBuffer surface types and flags
#define BINKSURFACE32 3
#define BINKCOPYALL 0x80000000L

/* ---------- macros */

#define BinkSoundUseDirectSound(lpDS) BinkSetSoundSystem(BinkOpenDirectSound, (U32)(lpDS))

/* ---------- structures */

typedef unsigned long U32;
typedef long S32;

typedef struct BINK
{
	U32 Width;
	U32 Height;
	U32 Frames;
	U32 FrameNum;
	// ... (truncated)
} BINK, *HBINK;

typedef struct BINKREALTIME
{
	U32 FrameNum;
	U32 FrameRate;
	U32 FrameRateDiv;
	U32 Frames;
	U32 FramesTime;
	U32 FramesVideoDecompTime;
	U32 FramesAudioDecompTime;
	U32 FramesReadTime;
	U32 FramesIdleReadTime;
	U32 FramesThreadReadTime;
	U32 FramesBlitTime;
	U32 ReadBufferSize;
	U32 ReadBufferUsed;
	U32 FramesDataRate;
} BINKREALTIME;

typedef struct BINKSUMMARY
{
	U32 Width;
	U32 Height;
	U32 TotalTime;
	U32 FileFrameRate;
	U32 FileFrameRateDiv;
	U32 FrameRate;
	U32 FrameRateDiv;
	U32 TotalOpenTime;
	U32 TotalFrames;
	U32 TotalPlayedFrames;
	U32 SkippedFrames;
	U32 SkippedBlits;
	U32 SoundSkips;
	U32 TotalBlitTime;
	U32 TotalReadTime;
	U32 TotalVideoDecompTime;
	U32 TotalAudioDecompTime;
	U32 TotalIdleReadTime;
	U32 TotalBackgroundReadTime;
	U32 TotalReadSpeed;
	U32 SlowestFrameTime;
	U32 Slowest2FrameTime;
	U32 SlowestFrameNum;
	U32 Slowest2FrameNum;
	U32 AverageDataRate;
	U32 AverageFrameSize;
	U32 HighestMemAmount;
	U32 TotalIOMemory;
	U32 HighestIOUsed;
	U32 Highest1SecRate;
	U32 Highest1SecFrame;
} BINKSUMMARY;

typedef void *BINKSNDOPEN; // really a function pointer, never called directly
typedef BINKSNDOPEN (RADLINK *BINKSNDSYSOPEN)(U32 param);

typedef void *(RADLINK *RADMEMALLOC)(U32 bytes);
typedef void (RADLINK *RADMEMFREE)(void *ptr);

/* ---------- prototypes */

RADEXPFUNC HBINK RADEXPLINK BinkOpen(char const *name, U32 flags);
RADEXPFUNC void RADEXPLINK BinkClose(HBINK bink);
RADEXPFUNC S32 RADEXPLINK BinkDoFrame(HBINK bink);
RADEXPFUNC void RADEXPLINK BinkNextFrame(HBINK bink);
RADEXPFUNC S32 RADEXPLINK BinkWait(HBINK bink);
RADEXPFUNC S32 RADEXPLINK BinkCopyToBuffer(HBINK bink, void *dest, S32 destpitch, U32 destheight, U32 destx, U32 desty, U32 flags);
RADEXPFUNC void RADEXPLINK BinkGetSummary(HBINK bink, BINKSUMMARY *summary);
RADEXPFUNC void RADEXPLINK BinkGetRealtime(HBINK bink, BINKREALTIME *run, U32 frames);
RADEXPFUNC void RADEXPLINK BinkSetIOSize(U32 iosize);
RADEXPFUNC S32 RADEXPLINK BinkSetSoundSystem(BINKSNDSYSOPEN open, U32 param);
RADEXPFUNC BINKSNDOPEN RADEXPLINK BinkOpenDirectSound(U32 param);
RADEXPFUNC void RADEXPLINK RADSetMemory(RADMEMALLOC a, RADMEMFREE f);

#endif // __BINK_H
