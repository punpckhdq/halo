/*
D3D_INTIMACY.CPP

symbols in this file:
001BF370 0080:
	_d3d_find_flipcount (0000)
002B0030 0054:
	??_C@_0FE@HADBCKPF@?$CD?$CD?$CD?5WARNING?3?5direct3d?5context?5un@ (0000)
002B0088 0048:
	??_C@_0EI@EPFLNHAM@?$CD?$CD?$CD?5WARNING?3?5direct3d?5context?5un@ (0000)
002B00D0 0042:
	??_C@_0EC@KGOGDCPF@?$CD?$CD?$CD?5FATAL?5ERROR?5LOCATING?5DIRECT3@ (0000)
002B0114 0025:
	??_C@_0CF@FADDPKIM@c?3?2halo?2SOURCE?2main?2d3d_intimacy@ (0000)
002B0140 004e:
	??_C@_0EO@MHMEBONI@?$CD?$CD?$CD?5WARNING?3?5direct3d?5context?5un@ (0000)
*/

/* ---------- headers */

extern "C"
{
	#include "cseries.h"
	#include "d3d_intimacy.h"
	#include "errors.h"
	#include "main.h"
}

#include "cseries_windows.h"
#include "d3d_private.h"

/* ---------- public code */

inline bool check_device_interrupts(
	void)
{
	bool success = true;
	if (D3D::g_Device.m_Miniport.m_InterruptsEnabled!=TRUE)
	{
		error(_error_silent, "### WARNING: direct3d context unreadable (interrupts 0x%08X != 0x00000001)...", D3D::g_Device.m_Miniport.m_InterruptsEnabled);
		success = false;
	}
	return success;
}

inline bool check_device_callback(
	void)
{
	bool success = true;
	if (D3D::g_Device.m_Miniport.m_pVerticalBlankCallback!=main_vertical_blank_interrupt_handler)
	{
		error(_error_silent, "### WARNING: direct3d context unreadable (callback 0x%08X != 0x%08X)...", D3D::g_Device.m_Miniport.m_pVerticalBlankCallback, main_vertical_blank_interrupt_handler);
		success = false;
	}
	return success;
}

inline bool check_device_vblank(
	void)
{
	bool success = true;
	if (D3D::g_Device.m_Miniport.m_GammaCurrentIndex>0x10000)
	{
		error(_error_silent, "### WARNING: direct3d context unreadable (vblank 0x%08X greater than 0x00010000)...", D3D::g_Device.m_Miniport.m_GammaCurrentIndex);
		success = false;
	}
	return success;
}

extern "C" volatile unsigned int* d3d_find_flipcount(
	void)
{
	volatile unsigned int *flipcount = NULL;

	if (check_device_interrupts()&&check_device_callback()&&check_device_vblank())
	{
		flipcount = &D3D::g_Device.m_Miniport.m_VBlankFlipCount;
	}
	else
	{
		match_vassert("c:\\halo\\SOURCE\\main\\d3d_intimacy.cpp", 53, FALSE, "### FATAL ERROR LOCATING DIRECT3D FLIPCOUNT, THIS IS HORRIBLY BAD");
	}

	return flipcount;
}
