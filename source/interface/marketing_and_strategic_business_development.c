/*
MARKETING_AND_STRATEGIC_BUSINESS_DEVELOPMENT.C
*/

/* ---------- headers */

#include "cseries.h"
#include "marketing_and_strategic_business_development.h"
#include "errors.h"
#include "files.h"
#include "ui_widget.h"
#include "cache_files.h"
#include "saved_game_files.h"

/* ---------- prototypes */

static void clean_up_for_image_launch(void);

/* ---------- globals */

static boolean bss_00453ccc;
static boolean data_002fd5a0= TRUE;

/* ---------- public code */

boolean xbox_demos_available(
	void)
{
	struct file_reference file;
	
	if (data_002fd5a0==TRUE)
	{
		if (file_reference_create_from_path(&file, "d:\\XDemos\\XDemos.xbe", FALSE))
		{
			if (file_exists(&file))
			{
				bss_00453ccc= TRUE;
			}
		}

		data_002fd5a0= FALSE;
	}

	return bss_00453ccc;
}

/* must include here to preserve order */
#include "cseries_windows.h"
#include "rasterizer/xbox/rasterizer_xbox.h"

void xbox_demos_launch(
	void)
{
	if (xbox_demos_available())
	{
		LAUNCH_DATA launch_data= {0};

		clean_up_for_image_launch();
		XLaunchNewImageA("d:\\XDemos\\XDemos.xbe", &launch_data);

		XLaunchNewImageA(NULL, &launch_data);

		while (TRUE)
		{
			;
		}
	}
	else
	{
		error(_error_silent, "XDEMOS not found!");
	}

	return;
}

void xbox_dashboard_launch(
	void)
{
	LAUNCH_DATA launch_data= {0};
	long abort_error;

	clean_up_for_image_launch();

	abort_error= dashboard_abort_error;
	switch (abort_error)
	{
	case 0x21:
	case 0x22:
	{
		((PLD_LAUNCH_DASHBOARD)&launch_data)->dwReason= XLD_LAUNCH_DASHBOARD_MEMORY;
		break;
	}
	case 0x23:
	{
		((PLD_LAUNCH_DASHBOARD)&launch_data)->dwReason= XLD_LAUNCH_DASHBOARD_MAIN_MENU;
		break;
	}
	}

	XLaunchNewImageA(NULL, &launch_data);

	while (TRUE)
	{
		;
	}

	return;
}

/* ---------- private code */

static void clean_up_for_image_launch(
	void)
{
	error(_error_silent, "preparing for XLaunchNewImage()...");
	saved_game_files_take_mutex();

	if (cache_files_precache_in_progress())
	{
		error(_error_silent, "stopping map file precaching...");
		cache_files_precache_map_end();
	}

	match_vassert("c:\\halo\\SOURCE\\interface\\marketing_and_strategic_business_development.c", 64,
		SUCCEEDED(IDirect3DDevice8_PersistDisplay(&global_d3d_device)),
		"IDirect3DDevice8_PersistDisplay() failed in clean_up_for_image_launch()");
}
