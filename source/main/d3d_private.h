/*
D3D_PRIVATE.H
*/

#ifndef __D3D_PRIVATE_H
#define __D3D_PRIVATE_H
#pragma once

/* ---------- structures */

namespace D3D
{

class CMiniport
{
	public:
	byte __unknown00[0xA4];
	BOOL m_InterruptsEnabled;
	byte __unknownA8[0x13C];
	D3DCALLBACK m_pVerticalBlankCallback;
	byte __unknown1E8[0x28];
	volatile unsigned int m_VBlankFlipCount;
	byte __unknown214[0x608];
	unsigned long m_GammaCurrentIndex;
	byte __unknown824[0x38];
};

class CDevice :
	public D3DDevice
{
	public:
	byte __unknown00[0x2308];
	CMiniport m_Miniport;
	byte __unknown2B60[0x30];
};

extern CDevice g_Device;

}

#endif // __D3D_PRIVATE_H
