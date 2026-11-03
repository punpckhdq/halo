/*
SHELL_WINDOWS.H

header included in hcex build.
*/

#ifndef __SHELL_WINDOWS_H
#define __SHELL_WINDOWS_H
#pragma once

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

struct window_data
{
	HINSTANCE hInstance;
	HWND hWnd;
	HWND hWndPresentTarget;
	int nCmdShow;
	void *MainWndProc;
	char class_name[64];
	char window_title[64];
};

/* ---------- prototypes/EXAMPLE.C */

/* ---------- globals */

extern struct window_data window_globals;

/* ---------- public code */

#endif // __SHELL_WINDOWS_H
