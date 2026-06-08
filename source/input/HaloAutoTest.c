/*
HALOAUTOTEST.C
*/

/* ---------- headers */

#include "cseries.h"
#include "cseries_windows.h"
#include "HaloAutoTest.h"
#include "integer_math.h"
#include "input.h"

/* ---------- globals */

static struct
{
	HANDLE __unknown00;
	unsigned long __unknown04;
} bss_004535b4;

/* ---------- public code */

void HATCleanup(
	void)
{
	if (bss_004535b4.__unknown00)
	{
		CloseHandle(bss_004535b4.__unknown00);
	}

	return;
}

void HATReadMain(
	void)
{
	if (GetFileAttributesA("d:\\write.xts")!=-1)
	{
		bss_004535b4.__unknown04 = 0x03;
	}
	else if (GetFileAttributesA("d:\\read.xts")!=-1)
	{
		bss_004535b4.__unknown04 = 0x04;
	}
	else if (GetFileAttributesA("d:\\loop.xts")!=-1)
	{
		bss_004535b4.__unknown04 = 0x05;
	}

	return;
}

void HATRawRead(
	struct gamepad_state *gamepad)
{
	DWORD number_of_bytes_read = 0;
	ReadFile(bss_004535b4.__unknown00, gamepad, 0x28, &number_of_bytes_read, 0);

	return;
}

void HATRawLoopRead(
	struct gamepad_state *gamepad)
{
	DWORD number_of_bytes_read = 0;
	ReadFile(bss_004535b4.__unknown00, gamepad, 0x28, &number_of_bytes_read, 0);

	if (number_of_bytes_read==0)
	{
		SetFilePointer(bss_004535b4.__unknown00, 0, 0, 0);
		ReadFile(bss_004535b4.__unknown00, gamepad, 0x28, &number_of_bytes_read, 0);
	}

	return;
}

void HATRawWrite(
	struct gamepad_state *gamepad)
{
	DWORD number_of_bytes_written = 0;
	WriteFile(bss_004535b4.__unknown00, gamepad, 0x28, &number_of_bytes_written, 0);

	return;
}

void HATInit(
	void)
{
	HATReadMain();

	if (bss_004535b4.__unknown04==0x03)
	{
		bss_004535b4.__unknown00 = CreateFileA(
			"d:\\state.data",
			GENERIC_WRITE,
			0,
			0,
			CREATE_ALWAYS,
			FILE_FLAG_SEQUENTIAL_SCAN,
			NULL);
	}
	else if (bss_004535b4.__unknown04==0x04||bss_004535b4.__unknown04==0x05)
	{
		bss_004535b4.__unknown00 = CreateFileA(
			"d:\\state.data",
			GENERIC_READ,
			0,
			0,
			OPEN_EXISTING,
			FILE_FLAG_SEQUENTIAL_SCAN,
			NULL);
	}

	return;
}

void HATRun(
	struct gamepad_state *gamepad)
{
	switch (bss_004535b4.__unknown04)
	{
	case 0x03:
	{
		DWORD number_of_bytes_written = 0;
		WriteFile(bss_004535b4.__unknown00, gamepad, 0x28, &number_of_bytes_written, 0);
		break;
	}
	case 0x04:
	{
		DWORD number_of_bytes_read = 0;
		ReadFile(bss_004535b4.__unknown00, gamepad, 0x28, &number_of_bytes_read, 0);
		break;
	}
	case 0x05:
	{
		HATRawLoopRead(gamepad);
		break;
	}
	default:
		break;
	}

	return;
}
