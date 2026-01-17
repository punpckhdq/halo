/*
SHELL_XBOX.C

symbols in this file:
00180A00 0060:
	_shell_platform_initialize (0000)
00180A60 0010:
	_shell_platform_verify (0000)
00180A70 0010:
	_shell_platform_dispose (0000)
00180A80 0010:
	_shell_screen_pause (0000)
00180A90 0010:
	_shell_get_command_line (0000)
00180AA0 0010:
	_shell_idle (0000)
00180AB0 00d0:
	_fuck_code_in_the_eye (0000)
00180B80 0090:
	_main (0000)
002A0A78 0007:
	??_C@_06GAOAIMNN@xdemo?5?$AA@ (0000)
002A0A80 0007:
	??_C@_06PFFNFMJI@XDEMOS?$AA@ (0000)
002A0A88 000c:
	_rdata_002a0a88 (0000)
004C08A8 0008:
	_bss_004c08a8 (0000)
*/

/* ---------- headers */

#include "cseries.h"
#include "cseries_windows.h"
#include "shell.h"
#include "physical_memory_map.h"
#include "xbox/rasterizer_xbox.h"
#include "main.h"

/* ---------- prototypes */

static void fuck_code_in_the_eye(void);

/* ---------- globals */

static char bss_004c08a8[8];

/* ---------- public code */

boolean shell_platform_initialize(
	void)
{
	LAUNCH_DATA launch_data;
	DWORD launch_data_type;

	if (!XGetLaunchInfo(&launch_data_type, &launch_data) &&
		launch_data_type==LDT_TITLE &&
		!strcmp((const char *)launch_data.Data, "XDEMOS"))
	{
		strncat(bss_004c08a8, "xdemo ", NUMBEROF(bss_004c08a8)-1);
		bss_004c08a8[NUMBEROF(bss_004c08a8)-1]= '\0';
	}

	return TRUE;
}

void shell_platform_verify(
	void)
{
	physical_memory_verify();

	return;
}

void shell_platform_dispose(
	void)
{
	return;
}

void shell_screen_pause(
	boolean paused)
{
	return;
}

const char *shell_get_command_line(
	void)
{
	return bss_004c08a8;
}

void shell_idle(
	void)
{
	return;
}

int main(
	void)
{
	fuck_code_in_the_eye();
	rasterizer_preinitialize__fill_you_up_with_the_devils_cock();
	physical_memory_allocate();

	__try
	{
		if (shell_initialize())
		{
			main_loop();
			shell_dispose();
		}
	}
	__except (generic_exception_filter(GetExceptionCode(), GetExceptionInformation()))
	{
		halt_and_catch_fire();
	}

	return EXIT_SUCCESS;
}

/* ---------- private code */

static void fuck_code_in_the_eye(
	void)
{
	DWORD flOldProtect;

	PDM_WALK_MODSECT walk_modsect= NULL;
	DMN_SECTIONLOAD section_load;

	PDM_WALK_MODULES walk_modules= NULL;
	DMN_MODLOAD module_load;

	while (DmWalkLoadedModules(&walk_modules, &module_load)!=XBDM_ENDOFLIST)
	{
		walk_modsect= NULL;
		while (DmWalkModuleSections(&walk_modsect, module_load.Name, &section_load)!=XBDM_ENDOFLIST)
		{
			if (fuck_code_in_the_eye>section_load.BaseAddress && fuck_code_in_the_eye<(void *)((byte *)section_load.BaseAddress + section_load.Size))
			{
				VirtualProtect(section_load.BaseAddress, section_load.Size, PAGE_READONLY, &flOldProtect);
				
				DmCloseModuleSections(walk_modsect);
				break;
			}
		}
	}
}
