/*
RASTERIZER_XBOX_ERRORS.C
*/

/* ---------- headers */

#include "cseries.h"
#include "rasterizer_xbox.h"
#include "objects.h"
#include "render.h"
#include "bungie_net/common/message_header.h"
#include "unicode.h"
#include "rasterizer.h"
#include "input.h"
#include "bitmaps_inlines.h"
#include "rasterizer_console_vars.h"
#include "light_definitions.h"
#include "shaders.h"
#include "rasterizer/common/rasterizer_common.h"
#include "collisions.h"
#include "input_windows.h"
#include "texture_cache.h"
#include "strings/resource.h"
#include "shell.h"
#include "decals.h"
#include "fog_definitions.h"
#include "rasterizer_cinematics.h"
#include "shell_windows.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

void rasterizer_error(
	HRESULT hr,
	char const *format,
	...)
{
	char const *error_name = "<unknown error>";
	char error_line_string[1024];
	char error_desc_string[1024];
	va_list arglist;

	va_start(arglist, format);
	vsprintf(error_line_string, format, arglist);
	va_end(arglist);

	if (FAILED(D3DXGetErrorStringA(hr, error_desc_string, sizeof(error_desc_string) - 1)))
	{
		strcpy(error_desc_string, "<can't get description>");
	}

	switch (hr)
	{
	case D3DERR_WRONGTEXTUREFORMAT:
		error_name = "D3DERR_WRONGTEXTUREFORMAT";
		break;
	case D3DERR_UNSUPPORTEDCOLOROPERATION:
		error_name = "D3DERR_UNSUPPORTEDCOLOROPERATION";
		break;
	case D3DERR_UNSUPPORTEDCOLORARG:
		error_name = "D3DERR_UNSUPPORTEDCOLORARG";
		break;
	case D3DERR_UNSUPPORTEDALPHAOPERATION:
		error_name = "D3DERR_UNSUPPORTEDALPHAOPERATION";
		break;
	case D3DERR_UNSUPPORTEDALPHAARG:
		error_name = "D3DERR_UNSUPPORTEDALPHAARG";
		break;
	case D3DERR_TOOMANYOPERATIONS:
		error_name = "D3DERR_TOOMANYOPERATIONS";
		break;
	case D3DERR_CONFLICTINGTEXTUREFILTER:
		error_name = "D3DERR_CONFLICTINGTEXTUREFILTER";
		break;
	case D3DERR_UNSUPPORTEDFACTORVALUE:
		error_name = "D3DERR_UNSUPPORTEDFACTORVALUE";
		break;
	case D3DERR_CONFLICTINGRENDERSTATE:
		error_name = "D3DERR_CONFLICTINGRENDERSTATE";
		break;
	case D3DERR_UNSUPPORTEDTEXTUREFILTER:
		error_name = "D3DERR_UNSUPPORTEDTEXTUREFILTER";
		break;
	case D3DERR_CONFLICTINGTEXTUREPALETTE:
		error_name = "D3DERR_CONFLICTINGTEXTUREPALETTE";
		break;
	case D3DERR_DRIVERINTERNALERROR:
		error_name = "D3DERR_DRIVERINTERNALERROR";
		break;
	case D3DERR_NOTFOUND:
		error_name = "D3DERR_NOTFOUND";
		break;
	case D3DERR_MOREDATA:
		error_name = "D3DERR_MOREDATA";
		break;
	case D3DERR_DEVICELOST:
		error_name = "D3DERR_DEVICELOST";
		break;
	case D3DERR_DEVICENOTRESET:
		error_name = "D3DERR_DEVICENOTRESET";
		break;
	case D3DERR_NOTAVAILABLE:
		error_name = "D3DERR_NOTAVAILABLE";
		break;
	case D3DERR_OUTOFVIDEOMEMORY:
		error_name = "D3DERR_OUTOFVIDEOMEMORY";
		break;
	case D3DERR_INVALIDDEVICE:
		error_name = "D3DERR_INVALIDDEVICE";
		break;
	case D3DERR_INVALIDCALL:
		error_name = "D3DERR_INVALIDCALL";
		break;
	case E_FAIL:
		error_name = "E_FAIL";
		break;
	case E_INVALIDARG:
		error_name = "E_INVALIDARG";
		break;
	case E_OUTOFMEMORY:
		error_name = "E_OUTOFMEMORY";
		break;
	}

	error(
		_error_silent,
		"%s in %s (code=%d, error=%s)",
		error_name,
		error_line_string,
		hr,
		error_desc_string);

	return;
}

/* ---------- private code */
