/*
RASTERIZER_XBOX.C
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
#include "main.h"

/* ---------- constants */

enum
{
	RASTERIZER_TARGET_SHADOW_PRIMARY_SIZE = 128,
	RASTERIZER_TARGET_SHADOW_SECONDARY_SIZE = 128,
	RASTERIZER_TARGET_SUN_GLOW_SIZE = 64,
	RASTERIZER_TARGET_WATER_SIZE = 128,
	RASTERIZER_TARGET_WATER_MAX_MIPMAP_LEVELS = 4,
	NUMBER_OF_RENDER_PRIMARY_TEXTURES = 2, /* fake name */
	DEFAULT_TEXTURE_SIZE = 4, /* fake name */
	NUMBER_OF_CUBE_MAP_FACES = 6, /* fake name */
	SAFE_FRAME_HORIZONTAL_INSET = 48, /* fake name */
	SAFE_FRAME_VERTICAL_INSET = 36 /* fake name */
};

/* ---------- macros */

#define RASTERIZER_DEVICE_CREATION_FLAGS D3DCREATE_HARDWARE_VERTEXPROCESSING
#define RASTERIZER_TARGET_RENDER_PRIMARY_FORMAT (D3DFORMAT_DMACHANNEL_A | D3DFORMAT_BORDERSOURCE_COLOR | (2<<D3DFORMAT_DIMENSION_SHIFT) | (D3DFMT_LIN_A8R8G8B8<<D3DFORMAT_FORMAT_SHIFT) | (1<<D3DFORMAT_MIPMAP_SHIFT)) /* fake name */
#define RASTERIZER_TARGET_RENDER_PRIMARY_SIZE ((RASTERIZER_TARGET_RENDER_PRIMARY_WIDTH-1) | ((RASTERIZER_TARGET_RENDER_PRIMARY_HEIGHT-1)<<D3DSIZE_HEIGHT_SHIFT) | ((RASTERIZER_TARGET_RENDER_PRIMARY_WIDTH*sizeof(pixel32)/64-1)<<D3DSIZE_PITCH_SHIFT)) /* fake name */

/* ---------- structures */

struct rasterizer_point_light_constants
{
	real_point3d position;
	real inverse_radius_squared;
	real_vector3d forward;
	real spot_falloff_coefficient_A;
	real_rgb_color color;
	real spot_falloff_coefficient_B;
};

struct rasterizer_distant_light_constants
{
	real_vector3d forward;
	real pad0;
	real_rgb_color color;
	real pad1;
};

struct rasterizer_model_lighting_constants
{
	struct rasterizer_point_light_constants point_lights[MAXIMUM_RENDERED_POINT_LIGHTS];
	struct rasterizer_distant_light_constants distant_lights[MAXIMUM_RENDERED_DISTANT_LIGHTS];
	real_rgb_color ambient;
	real pad;
};

/* ---------- prototypes */

static void rasterizer_filthy_bitmap_defaults_initialize(void);

/* ---------- globals */

static IDirect3D8 *d3d = NULL;
static IDirect3DBaseTexture8 *global_d3d_texture_render_primary[NUMBER_OF_RENDER_PRIMARY_TEXTURES] = { NULL, NULL };
static IDirect3DSurface8 *global_d3d_surface_render_primary = NULL;
static IDirect3DSurface8 *global_d3d_surface_render_primary_z = NULL;
static IDirect3DTexture8 *global_d3d_texture_render_secondary = NULL;
static IDirect3DTexture8 *global_d3d_texture_render_secondary_z = NULL; /* fake name */
static IDirect3DSurface8 *global_d3d_surface_render_secondary = NULL;
static IDirect3DSurface8 *global_d3d_surface_render_secondary_z = NULL; /* fake name */
static IDirect3DTexture8 *global_d3d_texture_shadow_primary = NULL;
static IDirect3DSurface8 *global_d3d_surface_shadow_primary = NULL;
static IDirect3DTexture8 *global_d3d_texture_shadow_secondary = NULL;
static IDirect3DSurface8 *global_d3d_surface_shadow_secondary = NULL;
static IDirect3DTexture8 *global_d3d_texture_sun_glow_primary = NULL;
static IDirect3DSurface8 *global_d3d_surface_sun_glow_primary = NULL;
static IDirect3DTexture8 *global_d3d_texture_sun_glow_secondary = NULL;
static IDirect3DSurface8 *global_d3d_surface_sun_glow_secondary = NULL;
static IDirect3DTexture8 *global_d3d_texture_water = NULL;
static IDirect3DSurface8 *global_d3d_surface_water[RASTERIZER_TARGET_WATER_MAX_MIPMAP_LEVELS] = { NULL, NULL, NULL, NULL };
static IDirect3DBaseTexture8 *global_d3d_texture_z = NULL; /* fake name */
static IDirect3DSurface8 *global_d3d_surface_z = NULL; /* fake name */
IDirect3DDevice8 *global_d3d_device = NULL;
static IDirect3DPalette8 *d3d_palette = NULL;
static boolean window_continues_previous_window = FALSE; /* fake name */
static short previous_window_index = 0; /* fake name */

static short current_vertex_shader_index = NONE; /* fake name */
static short previous_stencil_mode = NONE;

/* ---------- public code */

__inline void SetRenderStateSmart(
	D3DRENDERSTATETYPE state,
	unsigned long value)
{
	IDirect3DDevice8_SetRenderState(global_d3d_device, state, value);

	return;
}

__inline void SetTextureStageStateSmart(
	unsigned long stage,
	D3DTEXTURESTAGESTATETYPE type,
	unsigned long value)
{
	IDirect3DDevice8_SetTextureStageState(global_d3d_device, stage, type, value);

	return;
}

boolean rasterizer_preinitialize__fill_you_up_with_the_devils_cock(
	void)
{
	boolean success = TRUE;

	d3d = Direct3DCreate8(D3D_SDK_VERSION);

	if (!d3d)
	{
		error(_error_silent, "### ERROR failed to create D3D object");
		success = FALSE;
	}
	else
	{
		D3DPRESENT_PARAMETERS d3d_present_parameters;

		memset(&d3d_present_parameters, 0, sizeof(d3d_present_parameters));
		d3d_present_parameters.Flags = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
		d3d_present_parameters.Windowed = FALSE;
		d3d_present_parameters.SwapEffect = D3DSWAPEFFECT_DISCARD;
		d3d_present_parameters.EnableAutoDepthStencil = TRUE;
		d3d_present_parameters.AutoDepthStencilFormat = D3DFMT_D24S8;
		d3d_present_parameters.BackBufferFormat = D3DFMT_A8R8G8B8;
		d3d_present_parameters.BackBufferWidth = RASTERIZER_TARGET_RENDER_PRIMARY_WIDTH;
		d3d_present_parameters.BackBufferHeight = RASTERIZER_TARGET_RENDER_PRIMARY_HEIGHT;
		d3d_present_parameters.FullScreen_PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;

		D3DCALL(success, IDirect3D8_CreateDevice(d3d,
			D3DADAPTER_DEFAULT,
			D3DDEVTYPE_HAL,
			NULL,
			RASTERIZER_DEVICE_CREATION_FLAGS,
			&d3d_present_parameters,
			&global_d3d_device));

		if (!global_d3d_device)
		{
			success = FALSE;
		}

		if (!success)
		{
			global_d3d_device = NULL;
			error(_error_silent, "### ERROR failed to create D3D device");
		}
		else
		{
			IDirect3DDevice8_GetDeviceCaps(global_d3d_device, &global_d3d_caps);
			success = SUCCEEDED(IDirect3DDevice8_Present(global_d3d_device, NULL, NULL, NULL, NULL));

			if (global_d3d_device)
			{
				IDirect3DDevice8_Release(global_d3d_device);
				global_d3d_device = NULL;
			}

			if (d3d)
			{
				IDirect3D8_Release(d3d);
				d3d = NULL;
			}
		}
	}

	if (!success)
	{
		error(_error_silent, "### ERROR rasterizer_preinitialize failed");
	}

	return success;
}

void *rasterizer_get_bitmap_default_hardware_format(
	struct bitmap_data const *bitmap)
{
	void *hardware_format;

	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox.c", 209, bitmap);

	switch (bitmap->type)
	{
	case _bitmap_type_2d:
		hardware_format = rasterizer_globals.default_2d_hardware_format;
		break;
	case _bitmap_type_3d:
		hardware_format = rasterizer_globals.default_2d_hardware_format;
		break;
	case _bitmap_type_cube_map:
		hardware_format = rasterizer_globals.default_cm_hardware_format;
		break;
	default:
		match_vhalt("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox.c", 223, "### ERROR unsupported bitmap type");
	}

	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox.c", 226, hardware_format);

	return hardware_format;
}

/* ---------- private code */

static void rasterizer_filthy_bitmap_defaults_initialize(
	void)
{
	D3DLOCKED_RECT d3d_locked_rect;
	D3DLOCKED_BOX d3d_locked_box;
	void *default_2d_hardware_format;
	void *default_3d_hardware_format;
	void *default_cm_hardware_format;
	boolean success = TRUE;

	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox.c", 239, global_d3d_device);

	D3DCALL(success, IDirect3DDevice8_CreateTexture(global_d3d_device,
		4,
		4,
		1,
		0,
		D3DFMT_A4R4G4B4,
		D3DPOOL_MANAGED,
		&(IDirect3DTexture8*)default_2d_hardware_format));
	D3DCALL(success, IDirect3DDevice8_CreateVolumeTexture(global_d3d_device,
		4,
		4,
		4,
		1,
		0,
		D3DFMT_A4R4G4B4,
		D3DPOOL_MANAGED,
		&(IDirect3DVolumeTexture8*)default_3d_hardware_format));
	D3DCALL(success, IDirect3DDevice8_CreateCubeTexture(global_d3d_device,
		4,
		1,
		0,
		D3DFMT_A4R4G4B4,
		D3DPOOL_MANAGED,
		&(IDirect3DCubeTexture8*)default_cm_hardware_format));

	success = success && default_2d_hardware_format && default_3d_hardware_format && default_cm_hardware_format;

	if (success)
	{
		short pixel_index;
		short face_index;
		unsigned short const checkers[2] = { 0x0f00, 0xf0f0 };

		IDirect3DTexture8_LockRect((IDirect3DTexture8*)default_2d_hardware_format, 0, &d3d_locked_rect, NULL, 0);

		for (pixel_index = 0; pixel_index < DEFAULT_TEXTURE_SIZE*DEFAULT_TEXTURE_SIZE; pixel_index++)
		{
			((unsigned short *)d3d_locked_rect.pBits)[pixel_index] = checkers[pixel_index & 1];
		}

		IDirect3DTexture8_UnlockRect((IDirect3DTexture8*)default_2d_hardware_format, 0);

		IDirect3DVolumeTexture8_LockBox((IDirect3DVolumeTexture8*)default_3d_hardware_format, 0, &d3d_locked_box, NULL, 0);

		for (pixel_index = 0; pixel_index < DEFAULT_TEXTURE_SIZE*DEFAULT_TEXTURE_SIZE*DEFAULT_TEXTURE_SIZE; pixel_index++)
		{
			((unsigned short *)d3d_locked_box.pBits)[pixel_index] = checkers[pixel_index & 1];
		}

		success = SUCCEEDED(IDirect3DVolumeTexture8_UnlockBox((IDirect3DVolumeTexture8*)default_3d_hardware_format, 0));

		for (face_index = 0; face_index < NUMBER_OF_CUBE_MAP_FACES; face_index++)
		{
			D3DCALL(success, IDirect3DCubeTexture8_LockRect((IDirect3DCubeTexture8*)default_cm_hardware_format, face_index, 0, &d3d_locked_rect, NULL, 0));

			for (pixel_index = 0; pixel_index < DEFAULT_TEXTURE_SIZE*DEFAULT_TEXTURE_SIZE; pixel_index++)
			{
				((unsigned short *)d3d_locked_rect.pBits)[pixel_index] = checkers[pixel_index & 1];
			}

			D3DCALL(success, IDirect3DCubeTexture8_UnlockRect((IDirect3DCubeTexture8*)default_cm_hardware_format, face_index, 0));
		}
	}

	match_vassert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox.c", 311, success, "### ERROR rasterizer_filthy_bitmap_default_initialize failed");

	rasterizer_globals.default_2d_hardware_format = default_2d_hardware_format;
	rasterizer_globals.default_3d_hardware_format = default_3d_hardware_format;
	rasterizer_globals.default_cm_hardware_format = default_cm_hardware_format;

	return;
}

/* ---------- public code */

boolean _rasterizer_initialize(
	void)
{
	boolean success = TRUE;

	if (!rasterizer_globals.pushbuffer_size)
	{
		rasterizer_globals.pushbuffer_size = 512;
	}

	if (!rasterizer_globals.pushbuffer_kickoff_size)
	{
		rasterizer_globals.pushbuffer_kickoff_size = 32;
	}

	Direct3D_SetPushBufferSize(rasterizer_globals.pushbuffer_size*1024, rasterizer_globals.pushbuffer_kickoff_size*1024);

	d3d = Direct3DCreate8(D3D_SDK_VERSION);

	if (!d3d)
	{
		error(_error_silent, "### ERROR failed to create D3D object");
		success = FALSE;
	}
	else
	{
		D3DPRESENT_PARAMETERS d3d_present_parameters;

		rasterizer_globals.screen_bounds.y0 = 0;
		rasterizer_globals.screen_bounds.x0 = 0;
		rasterizer_globals.screen_bounds.x1 = RASTERIZER_TARGET_RENDER_PRIMARY_WIDTH;
		rasterizer_globals.screen_bounds.y1 = RASTERIZER_TARGET_RENDER_PRIMARY_HEIGHT;
		rasterizer_globals.frame_bounds.x0 = SAFE_FRAME_HORIZONTAL_INSET;
		rasterizer_globals.frame_bounds.x1 = RASTERIZER_TARGET_RENDER_PRIMARY_WIDTH - SAFE_FRAME_HORIZONTAL_INSET;
		rasterizer_globals.frame_bounds.y0 = SAFE_FRAME_VERTICAL_INSET;
		rasterizer_globals.frame_bounds.y1 = RASTERIZER_TARGET_RENDER_PRIMARY_HEIGHT - SAFE_FRAME_VERTICAL_INSET;
		rasterizer_globals.frame_index = 1;

		memset(&d3d_present_parameters, 0, sizeof(d3d_present_parameters));
		d3d_present_parameters.Flags = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
		d3d_present_parameters.Windowed = FALSE;
		d3d_present_parameters.SwapEffect = D3DSWAPEFFECT_DISCARD;
		d3d_present_parameters.EnableAutoDepthStencil = TRUE;
		d3d_present_parameters.AutoDepthStencilFormat = rasterizer_globals.use_floating_point_zbuffer ? D3DFMT_F24S8 : D3DFMT_D24S8;
		d3d_present_parameters.BackBufferFormat = D3DFMT_A8R8G8B8;
		d3d_present_parameters.BackBufferWidth = rasterizer_globals.screen_bounds.x1 - rasterizer_globals.screen_bounds.x0;
		d3d_present_parameters.BackBufferHeight = rasterizer_globals.screen_bounds.y1 - rasterizer_globals.screen_bounds.y0;

		switch (rasterizer_globals.refresh_rate)
		{
		case NONE:
			d3d_present_parameters.FullScreen_PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
			global_frame_rate_throttle = FALSE;
			break;
		case 30:
			d3d_present_parameters.FullScreen_PresentationInterval = D3DPRESENT_INTERVAL_TWO;
			global_frame_rate_throttle = TRUE;
			break;
		case 60:
			d3d_present_parameters.FullScreen_PresentationInterval = D3DPRESENT_INTERVAL_ONE;
			global_frame_rate_throttle = TRUE;
			break;
		default:
			if (rasterizer_globals.refresh_rate)
			{
				error(_error_silent, "### ERROR unsupported refresh rate (%dHz), switching to default", rasterizer_globals.refresh_rate);
				rasterizer_globals.refresh_rate = 0;
			}

			d3d_present_parameters.FullScreen_PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;
			global_frame_rate_throttle = TRUE;
		}

		D3DCALL(success, IDirect3D8_CreateDevice(d3d,
			D3DADAPTER_DEFAULT,
			D3DDEVTYPE_HAL,
			NULL,
			RASTERIZER_DEVICE_CREATION_FLAGS,
			&d3d_present_parameters,
			&global_d3d_device));

		if (!global_d3d_device)
		{
			success = FALSE;
		}

		if (!success)
		{
			global_d3d_device = NULL;
			error(_error_silent, "### ERROR failed to create D3D device");
		}
		else
		{
			D3DCOLOR *palette_data;

			IDirect3DDevice8_GetDeviceCaps(global_d3d_device, &global_d3d_caps);
			SetupSmartStates();

			palette_data = NULL;
			D3DCALL(success, IDirect3DDevice8_CreatePalette(global_d3d_device, D3DPALETTE_256, &d3d_palette));
			D3DCALL(success, IDirect3DPalette8_Lock(d3d_palette, &palette_data, 0));
			memcpy(palette_data, global_vector_palette, sizeof(global_vector_palette));
			D3DCALL(success, IDirect3DPalette8_Unlock(d3d_palette));
			D3DCALL(success, IDirect3DDevice8_SetPalette(global_d3d_device, 0, d3d_palette));
			D3DCALL(success, IDirect3DDevice8_SetPalette(global_d3d_device, 1, d3d_palette));
			D3DCALL(success, IDirect3DDevice8_SetPalette(global_d3d_device, 2, d3d_palette));
			D3DCALL(success, IDirect3DDevice8_SetPalette(global_d3d_device, 3, d3d_palette));

			if (success)
			{
				short mipmap_index;

				IDirect3DDevice8_GetBackBuffer(global_d3d_device, 0, D3DBACKBUFFER_TYPE_MONO, &global_d3d_surface_render_primary);
				D3DCALL(success, IDirect3DDevice8_GetDepthStencilSurface(global_d3d_device, &global_d3d_surface_render_primary_z));

				global_d3d_texture_render_primary[0] = match_malloc("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox.c", 562, sizeof(IDirect3DBaseTexture8));
				global_d3d_texture_render_primary[1] = match_malloc("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox.c", 563, sizeof(IDirect3DBaseTexture8));

				if (global_d3d_texture_render_primary[0] && global_d3d_texture_render_primary[1])
				{
					short texture_index;

					for (texture_index = 0; texture_index < NUMBER_OF_RENDER_PRIMARY_TEXTURES; texture_index++)
					{
						IDirect3DBaseTexture8 *texture = global_d3d_texture_render_primary[texture_index];

						texture->Common = D3DCOMMON_TYPE_TEXTURE | 1;
						texture->Data = (texture_index == 1) ? global_d3d_surface_render_primary->Data : 0;
						texture->Lock = 0;
						texture->Size = RASTERIZER_TARGET_RENDER_PRIMARY_SIZE;
						texture->Format = RASTERIZER_TARGET_RENDER_PRIMARY_FORMAT;
					}
				}
				else
				{
					success = FALSE;
				}

				global_d3d_surface_z = match_malloc("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox.c", 610, sizeof(IDirect3DSurface8));

				if (global_d3d_surface_z)
				{
					*global_d3d_surface_z = *global_d3d_surface_render_primary;
					global_d3d_surface_z->Data = global_d3d_surface_render_primary_z->Data;
				}
				else
				{
					success = FALSE;
				}

				global_d3d_texture_z = match_malloc("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox.c", 623, sizeof(IDirect3DBaseTexture8));

				if (global_d3d_texture_z)
				{
					global_d3d_texture_z->Common = D3DCOMMON_TYPE_TEXTURE | 1;
					global_d3d_texture_z->Data = global_d3d_surface_render_primary_z->Data;
					global_d3d_texture_z->Format = RASTERIZER_TARGET_RENDER_PRIMARY_FORMAT;
					global_d3d_texture_z->Lock = 0;
					global_d3d_texture_z->Size = RASTERIZER_TARGET_RENDER_PRIMARY_SIZE;
				}
				else
				{
					success = FALSE;
				}

				D3DCALL(success, IDirect3DDevice8_CreateTexture(global_d3d_device,
					RASTERIZER_TARGET_RENDER_SECONDARY_WIDTH,
					RASTERIZER_TARGET_RENDER_SECONDARY_HEIGHT,
					1,
					D3DUSAGE_RENDERTARGET,
					D3DFMT_LIN_A8R8G8B8,
					D3DPOOL_DEFAULT,
					&global_d3d_texture_render_secondary));
				D3DCALL(success, IDirect3DTexture8_GetSurfaceLevel(global_d3d_texture_render_secondary, 0, &global_d3d_surface_render_secondary));

				if (!global_d3d_texture_render_secondary || !global_d3d_surface_render_secondary)
				{
					success = FALSE;
				}

				global_d3d_texture_render_secondary_z = NULL;
				global_d3d_surface_render_secondary_z = NULL;

				D3DCALL(success, IDirect3DDevice8_CreateTexture(global_d3d_device,
					RASTERIZER_TARGET_WATER_SIZE,
					RASTERIZER_TARGET_WATER_SIZE,
					RASTERIZER_TARGET_WATER_MAX_MIPMAP_LEVELS,
					D3DUSAGE_RENDERTARGET,
					D3DFMT_A8R8G8B8,
					D3DPOOL_DEFAULT,
					&global_d3d_texture_water));

				if (!global_d3d_texture_water)
				{
					success = FALSE;
				}

				for (mipmap_index = 0; success && mipmap_index < RASTERIZER_TARGET_WATER_MAX_MIPMAP_LEVELS; mipmap_index++)
				{
					D3DCALL(success, IDirect3DTexture8_GetSurfaceLevel(global_d3d_texture_water, mipmap_index, &global_d3d_surface_water[mipmap_index]));

					if (!global_d3d_surface_water[mipmap_index])
					{
						success = FALSE;
					}
				}

				D3DCALL(success, IDirect3DDevice8_CreateTexture(global_d3d_device,
					RASTERIZER_TARGET_SHADOW_PRIMARY_SIZE,
					RASTERIZER_TARGET_SHADOW_PRIMARY_SIZE,
					1,
					D3DUSAGE_RENDERTARGET,
					D3DFMT_R5G6B5,
					D3DPOOL_DEFAULT,
					&global_d3d_texture_shadow_primary));
				D3DCALL(success, IDirect3DTexture8_GetSurfaceLevel(global_d3d_texture_shadow_primary, 0, &global_d3d_surface_shadow_primary));

				if (!global_d3d_texture_shadow_primary || !global_d3d_surface_shadow_primary)
				{
					success = FALSE;
				}

				D3DCALL(success, IDirect3DDevice8_CreateTexture(global_d3d_device,
					RASTERIZER_TARGET_SHADOW_SECONDARY_SIZE,
					RASTERIZER_TARGET_SHADOW_SECONDARY_SIZE,
					1,
					D3DUSAGE_RENDERTARGET,
					D3DFMT_R5G6B5,
					D3DPOOL_DEFAULT,
					&global_d3d_texture_shadow_secondary));
				D3DCALL(success, IDirect3DTexture8_GetSurfaceLevel(global_d3d_texture_shadow_secondary, 0, &global_d3d_surface_shadow_secondary));

				if (!global_d3d_texture_shadow_secondary || !global_d3d_surface_shadow_secondary)
				{
					success = FALSE;
				}

				D3DCALL(success, IDirect3DDevice8_CreateTexture(global_d3d_device,
					RASTERIZER_TARGET_SUN_GLOW_SIZE,
					RASTERIZER_TARGET_SUN_GLOW_SIZE,
					1,
					D3DUSAGE_RENDERTARGET,
					D3DFMT_A8R8G8B8,
					D3DPOOL_DEFAULT,
					&global_d3d_texture_sun_glow_primary));
				D3DCALL(success, IDirect3DTexture8_GetSurfaceLevel(global_d3d_texture_sun_glow_primary, 0, &global_d3d_surface_sun_glow_primary));

				if (!global_d3d_texture_sun_glow_primary || !global_d3d_surface_sun_glow_primary)
				{
					success = FALSE;
				}

				D3DCALL(success, IDirect3DDevice8_CreateTexture(global_d3d_device,
					RASTERIZER_TARGET_SUN_GLOW_SIZE,
					RASTERIZER_TARGET_SUN_GLOW_SIZE,
					1,
					D3DUSAGE_RENDERTARGET,
					D3DFMT_A8R8G8B8,
					D3DPOOL_DEFAULT,
					&global_d3d_texture_sun_glow_secondary));
				D3DCALL(success, IDirect3DTexture8_GetSurfaceLevel(global_d3d_texture_sun_glow_secondary, 0, &global_d3d_surface_sun_glow_secondary));

				if (!global_d3d_texture_sun_glow_secondary || !global_d3d_surface_sun_glow_secondary)
				{
					success = FALSE;
				}

				if (!success)
				{
					error(_error_silent, "### ERROR failed to create offscreen surface(s)");
				}
				else
				{
					success = SUCCEEDED(IDirect3DDevice8_SetShaderConstantMode(global_d3d_device, D3DSCM_192CONSTANTS));
					IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_ZENABLE, D3DZB_TRUE);
					IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_ZWRITEENABLE, TRUE);
					IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
					IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_ZBIAS, 0);
					IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_SOLIDOFFSETENABLE, TRUE);
					IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_ALPHATESTENABLE, FALSE);
					IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_ALPHAFUNC, D3DCMP_GREATER);
					IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_ALPHAREF, 0);
					IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_ALPHABLENDENABLE, FALSE);
					IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_SRCBLEND, D3DBLEND_ONE);
					IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_DESTBLEND, D3DBLEND_ZERO);
					IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_BLENDOP, D3DBLENDOP_ADD);
					IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_FOGENABLE, TRUE);
					IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_LIGHTING, FALSE);
					IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_SPECULARENABLE, TRUE);
					IDirect3DDevice8_SetTextureStageState(global_d3d_device, 0, D3DTSS_TEXCOORDINDEX, 0);
					IDirect3DDevice8_SetTextureStageState(global_d3d_device, 1, D3DTSS_TEXCOORDINDEX, 1);
					IDirect3DDevice8_SetTextureStageState(global_d3d_device, 2, D3DTSS_TEXCOORDINDEX, 2);
					IDirect3DDevice8_SetTextureStageState(global_d3d_device, 3, D3DTSS_TEXCOORDINDEX, 3);
					D3DDevice_SetFlickerFilter(5);
					D3DDevice_SetSoftDisplayFilter(FALSE);
				}
			}
		}
	}

	rasterizer_filthy_bitmap_defaults_initialize();

	success = success &&
		rasterizer_memory_pool_initialize() &&
		rasterizer_dynamic_geometry_initialize() &&
		rasterizer_transparent_geometry_initialize() &&
		rasterizer_vertex_shaders_initialize() &&
		rasterizer_debug_initialize() &&
		rasterizer_frame_statistics_initialize() &&
		rasterizer_text_cache_initialize() &&
		rasterizer_detail_objects_initialize() &&
		rasterizer_profile_initialize() &&
		rasterizer_environment_fog_screen_initialize();

	rasterizer_screen_effects_initialize();
	texture_cache_new();
	rasterizer_debug_begin();

	if (success)
	{
		rasterizer_globals.active = TRUE;
	}
	else
	{
		error(_error_silent, "### ERROR failed to initialize rasterizer");
	}

	return success;
}

void _rasterizer_reset_state(
	void)
{
	return;
}

void rasterizer_spin_begin(
	void)
{
	return;
}

void rasterizer_spin_end(
	void)
{
	return;
}

void _rasterizer_frame_begin(
	struct rasterizer_frame_begin_parameters const *parameters)
{
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox.c", 1256, global_d3d_device);

	rasterizer_globals.z_near = rasterizer_get_near_clip_distance();
	global_frame_parameters.game_time_sec = parameters->game_time_sec;

	rasterizer_profile_frame_begin();
	rasterizer_frame_statistics_begin();
	rasterizer_water_set_visibility_for_frame(FALSE);
	rasterizer_lights_begin_for_new_frame();
	texture_cache_idle();

	SetRenderStateSmart(D3DRS_DXT1NOISEENABLE, rasterizer_debug_options.DXTC_noise_enabled);

	if (rasterizer_debug_options.draw_environment_decals)
	{
		rasterizer_decal_vertices_begin_update();
		decals_update();
		rasterizer_decal_vertices_end_update();
	}

	return;
}

boolean _rasterizer_windows_begin(
	void)
{
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox.c", 1331, global_d3d_device);

	IDirect3DDevice8_BeginScene(global_d3d_device);
}

void _rasterizer_window_begin(
	struct rasterizer_window_begin_parameters const *parameters)
{
	pixel32 clear_color;

	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox.c", 1351, parameters);
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox.c", 1352, global_d3d_device);

	global_window_parameters = *parameters;

	window_continues_previous_window = previous_window_index != NONE && parameters->window_index == NONE;
	previous_window_index = parameters->window_index;

	if (!window_continues_previous_window)
	{
		rasterizer_memory_pool_begin();
		rasterizer_dynamic_geometry_begin();
		rasterizer_transparent_geometry_begin();
		rasterizer_environment_fog_screen_window_begin();
		rasterizer_lights_begin();
	}

	rasterizer_water_set_visibility_for_window(FALSE);
	rasterizer_active_camouflage_set_visibility(FALSE);
	rasterizer_profile_window_begin();
	rasterizer_set_stencil_mode(_rasterizer_stencil_mode_none);
	rasterizer_window_set_fog(&parameters->fog);

	if (rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_overdraw)
	{
		clear_color = 0;
	}
	else
	{
		clear_color = real_rgb_color_to_pixel32(&global_window_parameters.fog.atmospheric_color);
	}

	if (parameters->rasterizer_target == _rasterizer_target_render_primary || parameters->rasterizer_target == _rasterizer_target_render_secondary)
	{
		rasterizer_profile_begin(_rasterizer_profile_clear);
		rasterizer_set_target(parameters->rasterizer_target, 0, clear_color, !parameters->suppress_clear, TRUE);
		rasterizer_profile_end(_rasterizer_profile_clear);

		if (parameters->rasterizer_target == _rasterizer_target_render_primary)
		{
			match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox.c", 1415, parameters->camera.z_near!=0.0f);
		}
	}
	else
	{
		match_vhalt("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox.c", 1420, "### ERROR unsupported rasterizer target for scene rendering");
	}

	rasterizer_set_frustum_z(-1.f, -1.f);
	IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_FILLMODE, rasterizer_debug_options.wireframe_enabled ? D3DFILL_WIREFRAME : D3DFILL_SOLID);

	return;
}

void _rasterizer_window_get_fog(
	struct render_fog *fog)
{
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox.c", 1440, fog);

	*fog = global_window_parameters.fog;

	return;
}

void _rasterizer_window_set_fog(
	struct render_fog const *fog)
{
	real vsh_constants__fog[VSH_CONSTANTS__FOG_COUNT*4];

	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox.c", 1452, fog);

	global_window_parameters.fog = *fog;

	if (global_window_parameters.fog.atmospheric_maximum_density <= 0.f)
	{
		global_window_parameters.fog.atmospheric_maximum_density = 1.f;
	}

	if (global_window_parameters.fog.atmospheric_maximum_distance == 0.f || !rasterizer_debug_options.fog_atmospheric_enabled)
	{
		global_window_parameters.fog.atmospheric_maximum_density = 0.f;
		global_window_parameters.fog.atmospheric_minimum_distance = global_window_parameters.camera.z_far;
		global_window_parameters.fog.atmospheric_maximum_distance = 2.f*global_window_parameters.camera.z_far;
	}

	if (global_window_parameters.fog.planar_maximum_density <= 0.f)
	{
		global_window_parameters.fog.planar_maximum_density = 1.f;
	}

	if (global_window_parameters.fog.planar_mode && !TEST_FLAG(fog->fog_definition_flags, _fog_definition_screen_effect_only_bit) && rasterizer_debug_options.fog_planar_enabled)
	{
		if (global_window_parameters.fog.planar_mode == _render_planar_fog_mode_fully_fogged)
		{
			match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox.c", 1498, global_window_parameters.fog.planar_maximum_distance!=0.0f);

			global_window_parameters.fog.planar_maximum_depth = 1.f;
			plane3d_from_point_and_normal(&global_window_parameters.fog.plane, &global_window_parameters.camera.position, &global_window_parameters.camera.forward);
			global_window_parameters.fog.plane.d += global_window_parameters.camera.z_far;
		}
	}
	else
	{
		global_window_parameters.fog.planar_mode = _render_planar_fog_mode_off;
		global_window_parameters.fog.planar_maximum_density = 0.f;
		global_window_parameters.fog.planar_maximum_distance = 1.f;
		global_window_parameters.fog.planar_maximum_depth = 1.f;
		global_window_parameters.fog.planar_color = *global_real_rgb_white;
		global_window_parameters.fog.plane.n = global_window_parameters.camera.forward;
		global_window_parameters.fog.plane.d = dot_product3d((real_vector3d const *)&global_window_parameters.camera.position, &global_window_parameters.camera.forward);
	}

	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox.c", 1511, global_window_parameters.fog.atmospheric_maximum_distance>=global_window_parameters.fog.atmospheric_minimum_distance);
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox.c", 1512, global_window_parameters.fog.planar_maximum_distance>0.0f);
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox.c", 1513, global_window_parameters.fog.planar_maximum_depth>0.0f);

	{
		real one_over_atmospheric_range = 1.f/(global_window_parameters.fog.atmospheric_maximum_distance - global_window_parameters.fog.atmospheric_minimum_distance);
		real camera_plane_distance = plane3d_distance_to_point(&global_window_parameters.fog.plane, &global_window_parameters.camera.position);
		real camera_forward_distance = dot_product3d((real_vector3d const *)&global_window_parameters.camera.position, &global_window_parameters.camera.forward);
		real one_over_planar_maximum_distance = 1.f/global_window_parameters.fog.planar_maximum_distance;
		real one_over_planar_maximum_depth = 1.f/global_window_parameters.fog.planar_maximum_depth;

		vsh_constants__fog[0] = global_window_parameters.camera.forward.i*one_over_atmospheric_range;
		vsh_constants__fog[1] = global_window_parameters.camera.forward.j*one_over_atmospheric_range;
		vsh_constants__fog[2] = global_window_parameters.camera.forward.k*one_over_atmospheric_range;
		vsh_constants__fog[3] = -((global_window_parameters.fog.atmospheric_minimum_distance + camera_forward_distance)*one_over_atmospheric_range);
		vsh_constants__fog[4] = -(global_window_parameters.fog.plane.n.i*one_over_planar_maximum_depth);
		vsh_constants__fog[5] = -(global_window_parameters.fog.plane.n.j*one_over_planar_maximum_depth);
		vsh_constants__fog[6] = -(global_window_parameters.fog.plane.n.k*one_over_planar_maximum_depth);
		vsh_constants__fog[7] = global_window_parameters.fog.plane.d*one_over_planar_maximum_depth;
		vsh_constants__fog[8] = global_window_parameters.camera.forward.i*one_over_planar_maximum_distance;
		vsh_constants__fog[9] = global_window_parameters.camera.forward.j*one_over_planar_maximum_distance;
		vsh_constants__fog[10] = global_window_parameters.camera.forward.k*one_over_planar_maximum_distance;
		vsh_constants__fog[11] = -(camera_forward_distance*one_over_planar_maximum_distance);
		vsh_constants__fog[12] = PIN(global_window_parameters.fog.atmospheric_maximum_density, 0.f, 1.f);
		vsh_constants__fog[13] = PIN(-(one_over_planar_maximum_depth*camera_plane_distance), 0.f, 1.f);
		vsh_constants__fog[14] = PIN(global_window_parameters.fog.planar_maximum_density, 0.f, 1.f);
		vsh_constants__fog[15] = 0.f;
	}

	IDirect3DDevice8_SetVertexShaderConstant(global_d3d_device, VSH_CONSTANTS__FOG_OFFSET, vsh_constants__fog, VSH_CONSTANTS__FOG_COUNT);
	IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_FOGCOLOR, real_rgb_color_to_pixel32(&global_window_parameters.fog.atmospheric_color));

	return;
}

void _rasterizer_window_end(
	void)
{
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox.c", 1567, global_d3d_device);

	if (main_get_window_count() > 1 && !rasterizer_debug_options.pad3)
	{
		IDirect3DDevice8_Clear(global_d3d_device, 0, NULL, D3DCLEAR_TARGET_A, 0, 1.f, 0);
	}

	if (global_window_parameters.has_mirror)
	{
		rectangle2d bounds;

		bounds.x0 = 512;
		bounds.x1 = 640;
		bounds.y0 = 0;
		bounds.y1 = 96;
		rasterizer_secondary_render_target_debug(&bounds);
	}

	if (global_window_parameters.window_index == NONE)
	{
		rasterizer_frame_statistics_get_fps(&rasterizer_frame_statistics);
		rasterizer_frame_statistics_draw();
	}

	if (!window_continues_previous_window)
	{
		rasterizer_memory_pool_end();
		rasterizer_dynamic_geometry_end();
		rasterizer_transparent_geometry_end();
		rasterizer_environment_fog_screen_window_end();
		rasterizer_lights_end();
		rasterizer_debug_end();
		rasterizer_debug_begin();
	}

	rasterizer_profile_window_end();

	return;
}

void _rasterizer_windows_end(
	void)
{
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox.c", 1629, global_d3d_device);

	IDirect3DDevice8_EndScene(global_d3d_device);

	return;
}

void _rasterizer_frame_end(
	void)
{
	short index;
	boolean success = TRUE;

	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox.c", 1648, global_d3d_device);

	rasterizer_profile_frame_end();
	rasterizer_frame_statistics_end();

	for (index = 0; index < RASTERIZER_MAXIMUM_TEXTURE_STAGES; index++)
	{
		D3DCALL(success, IDirect3DDevice8_SetTexture(global_d3d_device, index, NULL));
	}

	for (index = 0; index < 16; index++)
	{
		D3DCALL(success, IDirect3DDevice8_SetStreamSource(global_d3d_device, index, NULL, 0));
	}

	D3DCALL(success, IDirect3DDevice8_SetIndices(global_d3d_device, NULL, 0));

	if (!success)
	{
		error(_error_silent, "### ERROR rasterizer_frame_end failed");
	}

	return;
}

void _rasterizer_present(
	struct bitmap_data *screenshot_bitmap,
	point2d const *screenshot_index)
{
	boolean success = TRUE;

	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox.c", 1689, global_d3d_device);

	if (screenshot_bitmap && screenshot_bitmap->base_address)
	{
		rectangle2d bounds = rasterizer_globals.screen_bounds;

		if (screenshot_index)
		{
			short width = rasterizer_globals.screen_bounds.x1 - rasterizer_globals.screen_bounds.x0;
			short height = rasterizer_globals.screen_bounds.y1 - rasterizer_globals.screen_bounds.y0;

			bounds.x0 = screenshot_index->x*width;
			bounds.y0 = screenshot_index->y*height;
			bounds.x1 = bounds.x0 + width;
			bounds.y1 = bounds.y0 + height;
		}

		if ((screenshot_bitmap->format == _bitmap_format_a8r8g8b8 || screenshot_bitmap->format == _bitmap_format_x8r8g8b8) &&
			screenshot_bitmap->mipmap_count == 0 &&
			bounds.x0 >= 0 &&
			bounds.y0 >= 0 &&
			bounds.x1 <= screenshot_bitmap->width &&
			bounds.y1 <= screenshot_bitmap->height)
		{
			D3DSURFACE_DESC d3d_surface_desc;
			D3DLOCKED_RECT d3d_locked_rect;
			IDirect3DSurface8 *d3d_backbuffer = NULL;

			IDirect3DDevice8_GetBackBuffer(global_d3d_device, 0, D3DBACKBUFFER_TYPE_MONO, &d3d_backbuffer);
			IDirect3DSurface8_GetDesc(d3d_backbuffer, &d3d_surface_desc);

			if (d3d_surface_desc.Size == d3d_surface_desc.Width*d3d_surface_desc.Height*sizeof(pixel32))
			{
				IDirect3DSurface8_LockRect(d3d_backbuffer, &d3d_locked_rect, NULL, D3DLOCK_READONLY | D3DLOCK_TILED);

				if (d3d_locked_rect.pBits)
				{
					short row_index;
					short screen_width = rasterizer_globals.screen_bounds.x1 - rasterizer_globals.screen_bounds.x0;
					short screen_height = rasterizer_globals.screen_bounds.y1 - rasterizer_globals.screen_bounds.y0;

					match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox.c", 1732, d3d_locked_rect.Pitch==bitmap_format_get_bits_per_pixel(screenshot_bitmap->format)*screen_width/CHAR_BITS);

					for (row_index = 0; row_index < screen_height; row_index++)
					{
						void *destination = bitmap_2d_address(screenshot_bitmap, bounds.x0, (short)(bounds.y0 + row_index), 0);

						memcpy(destination, (byte *)d3d_locked_rect.pBits + row_index*d3d_locked_rect.Pitch, d3d_locked_rect.Pitch);
					}

					success = SUCCEEDED(IDirect3DSurface8_UnlockRect(d3d_backbuffer));
				}
				else
				{
					error(_error_silent, "### ERROR rasterizer_present: failed to lock backbuffer surface");
					success = FALSE;
				}
			}
			else
			{
				error(_error_silent, "### ERROR rasterizer_present: failed to get backbuffer surface");
				success = FALSE;
			}
		}
		else
		{
			error(_error_silent, "### ERROR rasterizer_present: invalid bitmap");
			success = FALSE;
		}
	}

	D3DCALL(success, IDirect3DDevice8_Present(global_d3d_device, NULL, NULL, window_globals.hWndPresentTarget, NULL));
	rasterizer_globals.frame_index++;

	if (!success)
	{
		error(_error_silent, "### ERROR rasterizer_present failed");
	}

	return;
}

void _rasterizer_dispose(
	void)
{
	rasterizer_memory_pool_dispose();
	rasterizer_dynamic_geometry_dispose();
	rasterizer_transparent_geometry_dispose();
	rasterizer_vertex_shaders_dispose();
	rasterizer_debug_dispose();
	rasterizer_frame_statistics_dispose();
	rasterizer_text_cache_dispose();
	rasterizer_detail_objects_dispose();
	rasterizer_profile_dispose();
	rasterizer_environment_fog_screen_dispose();
	rasterizer_screen_effects_dispose();
	texture_cache_delete();

	if (global_d3d_device)
	{
		IDirect3DDevice8_Release(global_d3d_device);
		global_d3d_device = NULL;
	}

	if (d3d)
	{
		IDirect3D8_Release(d3d);
		d3d = NULL;
	}

	return;
}

void _rasterizer_set_vblank_callback(
	void (*callback)(unsigned long))
{
	IDirect3DDevice8_SetVerticalBlankCallback(global_d3d_device, callback);

	return;
}

void rasterizer_set_framebuffer_blend_function(
	short framebuffer_blend_function)
{
	static unsigned long const srcblend_table[NUMBER_OF_SHADER_FRAMEBUFFER_BLEND_FUNCTIONS + 1] =
	{
		D3DBLEND_SRCALPHA,
		D3DBLEND_DESTCOLOR,
		D3DBLEND_DESTCOLOR,
		D3DBLEND_ONE,
		D3DBLEND_ONE,
		D3DBLEND_ONE,
		D3DBLEND_ONE,
		D3DBLEND_ONE,
		NONE
	};
	static unsigned long const destblend_table[NUMBER_OF_SHADER_FRAMEBUFFER_BLEND_FUNCTIONS + 1] =
	{
		D3DBLEND_INVSRCALPHA,
		D3DBLEND_ZERO,
		D3DBLEND_SRCCOLOR,
		D3DBLEND_ONE,
		D3DBLEND_ONE,
		D3DBLEND_ONE,
		D3DBLEND_ONE,
		D3DBLEND_INVSRCALPHA,
		NONE
	};
	static unsigned long const blendop_table[NUMBER_OF_SHADER_FRAMEBUFFER_BLEND_FUNCTIONS + 1] =
	{
		D3DBLENDOP_ADD,
		D3DBLENDOP_ADD,
		D3DBLENDOP_ADD,
		D3DBLENDOP_ADD,
		D3DBLENDOP_REVSUBTRACT,
		D3DBLENDOP_MIN,
		D3DBLENDOP_MAX,
		D3DBLENDOP_ADD,
		NONE
	};

	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox.c", 1907, framebuffer_blend_function>=0 && framebuffer_blend_function<NUMBER_OF_SHADER_FRAMEBUFFER_BLEND_FUNCTIONS);

	SetRenderStateSmart(D3DRS_SRCBLEND, srcblend_table[framebuffer_blend_function]);
	SetRenderStateSmart(D3DRS_DESTBLEND, destblend_table[framebuffer_blend_function]);
	SetRenderStateSmart(D3DRS_BLENDOP, blendop_table[framebuffer_blend_function]);

	return;
}

boolean rasterizer_set_texture_bitmap_data(
	short stage,
	struct bitmap_data const *bitmap)
{
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox.c", 1935, stage>=0 && stage<RASTERIZER_MAXIMUM_TEXTURE_STAGES);

	if (bitmap)
	{
		IDirect3DBaseTexture8 *hardware_format;

		profile_texture_start();
		hardware_format = _texture_cache_bitmap_get_hardware_format(bitmap, TRUE, TRUE);
		profile_texture_end();

		IDirect3DDevice8_SetTexture(global_d3d_device, stage, hardware_format);
	}
	else
	{
		display_assert("### YOU GOT FUCKED in rasterizer_set_texture_bitmap_data", MATCH_FILE("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox.c"), MATCH_LINE(1943), TRUE);
		error(_error_silent, "### ERROR direct texture not found (stage=%d)", stage);
	}

	return TRUE;
}

boolean rasterizer_set_texture_direct(
	short stage,
	long bitmap_group_index,
	short bitmap_index)
{
	boolean success = FALSE;
	struct bitmap_data *bitmap = NULL;

	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox.c", 1958, stage>=0 && stage<RASTERIZER_MAXIMUM_TEXTURE_STAGES);

	if (bitmap_group_index != NONE)
	{
		long bitmap_count = bitmap_group_get(bitmap_group_index)->bitmaps.count;

		if (bitmap_count > 0)
		{
			bitmap = bitmap_group_try_and_get_bitmap(bitmap_group_index, (short)(bitmap_index % bitmap_count));
		}
	}

	if (bitmap)
	{
		rasterizer_set_texture_bitmap_data(stage, bitmap);
		success = TRUE;
	}
	else
	{
		display_assert("### YOU GOT FUCKED in rasterizer_set_texture_direct", MATCH_FILE("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox.c"), MATCH_LINE(1982), TRUE);
		error(_error_silent, "### ERROR direct texture not found (stage=%d)", stage);
	}

	return success;
}

boolean rasterizer_set_texture_direct_non_blocking(
	short stage,
	long bitmap_group_index,
	short bitmap_index)
{
	boolean blocked = FALSE;
	struct bitmap_data *bitmap = NULL;

	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox.c", 2003, stage>=0 && stage<RASTERIZER_MAXIMUM_TEXTURE_STAGES);

	if (bitmap_group_index != NONE)
	{
		long bitmap_count = bitmap_group_get(bitmap_group_index)->bitmaps.count;

		if (bitmap_count > 0)
		{
			bitmap = bitmap_group_try_and_get_bitmap(bitmap_group_index, (short)(bitmap_index % bitmap_count));
		}
	}

	if (bitmap)
	{
		if (_texture_cache_bitmap_get_hardware_format(bitmap, FALSE, TRUE))
		{
			rasterizer_set_texture_bitmap_data(stage, bitmap);
		}
		else
		{
			blocked = TRUE;
		}
	}
	else
	{
		display_assert("### YOU GOT FUCKED in rasterizer_set_texture_direct_non_blocking", MATCH_FILE("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox.c"), MATCH_LINE(2035), TRUE);
		error(_error_silent, "### ERROR direct texture not found (stage=%d)", stage);
	}

	return blocked;
}

point2d *rasterizer_set_texture(
	short stage,
	short type,
	short usage,
	long bitmap_group_index,
	short bitmap_index)
{
	static point2d dimensions;
	struct bitmap_data *bitmap;
	boolean success = FALSE;

	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox.c", 2058, stage>=0 && stage<RASTERIZER_MAXIMUM_TEXTURE_STAGES);
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox.c", 2059, type>=0 && type<NUMBER_OF_BITMAP_TYPES);
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox.c", 2060, usage>=0 && usage<NUMBER_OF_BITMAP_USAGES);

	if ((rasterizer_debug_options.bump_mapping_enabled || usage != _bitmap_usage_vector) && bitmap_group_index != NONE)
	{
		long bitmap_count = bitmap_group_get(bitmap_group_index)->bitmaps.count;

		if (bitmap_count > 0)
		{
			bitmap = bitmap_group_try_and_get_bitmap(bitmap_group_index, (short)(bitmap_index % bitmap_count));

			if (bitmap->type == type)
			{
				rasterizer_set_texture_bitmap_data(stage, bitmap);
				success = TRUE;
				dimensions.x = bitmap->width;
				dimensions.y = bitmap->height;
			}
			else
			{
				error(_error_silent, "### ERROR incompatible bitmap type in shader got %s expected %s", bitmap_type_get_string(bitmap->type), bitmap_type_get_string(type));
			}
		}
	}

	if (!success)
	{
		long default_bitmap_group_index = global_rasterizer_data->default_textures[type].index;

		if (default_bitmap_group_index != NONE)
		{
			bitmap = bitmap_group_try_and_get_bitmap(default_bitmap_group_index, usage);

			if (bitmap)
			{
				rasterizer_set_texture_bitmap_data(stage, bitmap);
				success = TRUE;
				dimensions.x = bitmap->width;
				dimensions.y = bitmap->height;
			}
		}
	}

	if (!success)
	{
		display_assert("### YOU GOT FUCKED in rasterizer_set_texture", MATCH_FILE("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox.c"), MATCH_LINE(2117), TRUE);
		error(_error_silent, "### ERROR default texture not found (stage=%d, type=%d, usage=%d)", stage, type, usage);
	}

	return success ? &dimensions : NULL;
}

boolean rasterizer_set_texture_non_blocking(
	short stage,
	short type,
	short usage,
	long bitmap_group_index,
	short bitmap_index)
{
	static point2d dimensions;
	struct bitmap_data *bitmap;
	boolean success = FALSE;
	boolean blocked = FALSE;

	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox.c", 2142, stage>=0 && stage<RASTERIZER_MAXIMUM_TEXTURE_STAGES);
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox.c", 2143, type>=0 && type<NUMBER_OF_BITMAP_TYPES);
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox.c", 2144, usage>=0 && usage<NUMBER_OF_BITMAP_USAGES);

	if ((rasterizer_debug_options.bump_mapping_enabled || usage != _bitmap_usage_vector) && bitmap_group_index != NONE)
	{
		long bitmap_count = bitmap_group_get(bitmap_group_index)->bitmaps.count;

		if (bitmap_count > 0)
		{
			bitmap = bitmap_group_try_and_get_bitmap(bitmap_group_index, (short)(bitmap_index % bitmap_count));

			if (_texture_cache_bitmap_get_hardware_format(bitmap, FALSE, TRUE))
			{
				if (bitmap->type == type)
				{
					rasterizer_set_texture_bitmap_data(stage, bitmap);
					dimensions.x = bitmap->width;
					dimensions.y = bitmap->height;
					success = TRUE;
				}
				else
				{
					error(_error_silent, "### ERROR incompatible bitmap type in shader got %s expected %s", bitmap_type_get_string(bitmap->type), bitmap_type_get_string(type));
				}
			}
			else
			{
				blocked = TRUE;
				success = TRUE;
			}
		}
	}

	if (!success)
	{
		long default_bitmap_group_index = global_rasterizer_data->default_textures[type].index;

		if (default_bitmap_group_index != NONE)
		{
			bitmap = bitmap_group_try_and_get_bitmap(default_bitmap_group_index, usage);

			if (bitmap)
			{
				rasterizer_set_texture_bitmap_data(stage, bitmap);
				dimensions.x = bitmap->width;
				dimensions.y = bitmap->height;
				success = TRUE;
			}
		}
	}

	if (!success)
	{
		display_assert("### YOU GOT FUCKED in rasterizer_set_texture_non_blocking", MATCH_FILE("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox.c"), MATCH_LINE(2210), TRUE);
		error(_error_silent, "### ERROR default texture not found (stage=%d, type=%d, usage=%d)", stage, type, usage);
	}

	return blocked;
}

IDirect3DSurface8 *rasterizer_get_target(
	short rasterizer_target,
	short mipmap_index)
{
	IDirect3DSurface8 *d3d_surface = NULL;

	switch (rasterizer_target)
	{
	case _rasterizer_target_render_primary:
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox.c", 2232, mipmap_index==0);
		d3d_surface = global_d3d_surface_render_primary;
		break;
	case _rasterizer_target_render_secondary:
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox.c", 2236, mipmap_index==0);
		d3d_surface = global_d3d_surface_render_secondary;
		break;
	case _rasterizer_target_shadow_primary:
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox.c", 2240, mipmap_index==0);
		d3d_surface = global_d3d_surface_shadow_primary;
		break;
	case _rasterizer_target_shadow_secondary:
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox.c", 2244, mipmap_index==0);
		d3d_surface = global_d3d_surface_shadow_secondary;
		break;
	case _rasterizer_target_sun_glow_primary:
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox.c", 2248, mipmap_index==0);
		d3d_surface = global_d3d_surface_sun_glow_primary;
		break;
	case _rasterizer_target_sun_glow_secondary:
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox.c", 2252, mipmap_index==0);
		d3d_surface = global_d3d_surface_sun_glow_secondary;
		break;
	case _rasterizer_target_water:
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox.c", 2256, mipmap_index>=0 && mipmap_index<RASTERIZER_TARGET_WATER_MAX_MIPMAP_LEVELS);
		d3d_surface = global_d3d_surface_water[mipmap_index];
		break;
	default:
		match_vhalt("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox.c", 2260, "### ERROR unsupported rasterizer target");
	}

	return d3d_surface;
}

void rasterizer_set_target(
	short rasterizer_target,
	short mipmap_index,
	pixel32 clear_color,
	boolean clear,
	boolean zbuffer)
{
	D3DSURFACE_DESC d3d_surface_desc;
	D3DVIEWPORT8 d3d_viewport;
	IDirect3DSurface8 *d3d_surface = NULL;
	IDirect3DSurface8 *d3d_surface_z = NULL;

	switch (rasterizer_target)
	{
	case _rasterizer_target_render_primary:
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox.c", 2280, mipmap_index==0);
		d3d_surface = global_d3d_surface_render_primary;
		d3d_surface_z = global_d3d_surface_render_primary_z;
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox.c", 2284, d3d_surface);
		break;
	case _rasterizer_target_render_secondary:
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox.c", 2287, mipmap_index==0);
		d3d_surface = global_d3d_surface_render_secondary;
		d3d_surface_z = global_d3d_surface_render_secondary_z ? global_d3d_surface_render_secondary_z : global_d3d_surface_render_primary_z;
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox.c", 2290, d3d_surface);
		break;
	case _rasterizer_target_shadow_primary:
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox.c", 2293, mipmap_index==0);
		d3d_surface = global_d3d_surface_shadow_primary;
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox.c", 2295, d3d_surface);
		break;
	case _rasterizer_target_shadow_secondary:
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox.c", 2298, mipmap_index==0);
		d3d_surface = global_d3d_surface_shadow_secondary;
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox.c", 2300, d3d_surface);
		break;
	case _rasterizer_target_sun_glow_primary:
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox.c", 2303, mipmap_index==0);
		d3d_surface = global_d3d_surface_sun_glow_primary;
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox.c", 2305, d3d_surface);
		break;
	case _rasterizer_target_sun_glow_secondary:
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox.c", 2308, mipmap_index==0);
		d3d_surface = global_d3d_surface_sun_glow_secondary;
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox.c", 2310, d3d_surface);
		break;
	case _rasterizer_target_water:
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox.c", 2313, mipmap_index>=0 && mipmap_index<RASTERIZER_TARGET_WATER_MAX_MIPMAP_LEVELS);
		d3d_surface = global_d3d_surface_water[mipmap_index];
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox.c", 2315, d3d_surface);
		break;
	case _rasterizer_target_z:
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox.c", 2331, mipmap_index==0);
		d3d_surface = global_d3d_surface_z;
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox.c", 2333, d3d_surface);
		break;
	default:
		match_vhalt("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox.c", 2336, "### ERROR unsupported rasterizer target");
	}

	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox.c", 2340, !zbuffer||d3d_surface_z);

	IDirect3DDevice8_SetRenderTarget(global_d3d_device, d3d_surface, zbuffer ? d3d_surface_z : NULL);

	if (rasterizer_target == _rasterizer_target_render_primary)
	{
		d3d_viewport.X = global_window_parameters.camera.viewport_bounds.x0;
		d3d_viewport.Y = global_window_parameters.camera.viewport_bounds.y0;
		d3d_viewport.Width = global_window_parameters.camera.viewport_bounds.x1 - global_window_parameters.camera.viewport_bounds.x0;
		d3d_viewport.Height = global_window_parameters.camera.viewport_bounds.y1 - global_window_parameters.camera.viewport_bounds.y0;
	}
	else
	{
		IDirect3DSurface8_GetDesc(d3d_surface, &d3d_surface_desc);
		d3d_viewport.X = 0;
		d3d_viewport.Y = 0;
		d3d_viewport.Width = d3d_surface_desc.Width;
		d3d_viewport.Height = d3d_surface_desc.Height;
	}

	d3d_viewport.MinZ = 0.f;
	d3d_viewport.MaxZ = 1.f;
	IDirect3DDevice8_SetViewport(global_d3d_device, &d3d_viewport);

	if (clear)
	{
		unsigned long clear_flags = D3DCLEAR_TARGET;

		if (rasterizer_target == _rasterizer_target_render_primary || rasterizer_target == _rasterizer_target_render_secondary)
		{
			clear_flags = D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL;
		}

		IDirect3DDevice8_Clear(global_d3d_device, 0, NULL, clear_flags, clear_color, 1.f, 0);
	}

	return;
}

void rasterizer_set_target_as_texture(
	short stage,
	short rasterizer_target,
	short max_mipmap)
{
	IDirect3DBaseTexture8 *water_texture;
	boolean success = TRUE;
	IDirect3DTexture8 *d3d_texture = NULL;

	switch (rasterizer_target)
	{
	case _rasterizer_target_render_primary:
	{
		IDirect3DBaseTexture8 *render_primary_texture = global_d3d_texture_render_primary[rasterizer_globals.frame_index & 1];

		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox.c", 2415, max_mipmap==0);

		if (!render_primary_texture->Data)
		{
			IDirect3DSurface8 *d3d_backbuffer;

			IDirect3DDevice8_GetBackBuffer(global_d3d_device, 0, D3DBACKBUFFER_TYPE_MONO, &d3d_backbuffer);
			render_primary_texture->Common = D3DCOMMON_TYPE_TEXTURE | 1;
			render_primary_texture->Data = d3d_backbuffer->Data;
			render_primary_texture->Lock = 0;
			render_primary_texture->Size = RASTERIZER_TARGET_RENDER_PRIMARY_SIZE;
			render_primary_texture->Format = RASTERIZER_TARGET_RENDER_PRIMARY_FORMAT;
			D3DCALL(success, IDirect3DSurface8_Release(d3d_backbuffer));
		}

		d3d_texture = (IDirect3DTexture8 *)global_d3d_texture_render_primary[rasterizer_globals.frame_index & 1];
		break;
	}
	case _rasterizer_target_render_secondary:
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox.c", 2457, max_mipmap==0);
		d3d_texture = global_d3d_texture_render_secondary;
		break;
	case _rasterizer_target_shadow_primary:
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox.c", 2461, max_mipmap==0);
		d3d_texture = global_d3d_texture_shadow_primary;
		break;
	case _rasterizer_target_shadow_secondary:
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox.c", 2465, max_mipmap==0);
		d3d_texture = global_d3d_texture_shadow_secondary;
		break;
	case _rasterizer_target_sun_glow_primary:
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox.c", 2469, max_mipmap==0);
		d3d_texture = global_d3d_texture_sun_glow_primary;
		break;
	case _rasterizer_target_sun_glow_secondary:
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox.c", 2473, max_mipmap==0);
		d3d_texture = global_d3d_texture_sun_glow_secondary;
		break;
	case _rasterizer_target_water:
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox.c", 2477, max_mipmap>=0 && max_mipmap<=RASTERIZER_TARGET_WATER_MAX_MIPMAP_LEVELS);
		d3d_texture = global_d3d_texture_water;
		break;
	case _rasterizer_target_z:
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox.c", 2492, max_mipmap==0);
		d3d_texture = (IDirect3DTexture8 *)global_d3d_texture_z;
		break;
	default:
		match_vhalt("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox.c", 2496, "### ERROR unsupported rasterizer target");
	}

	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox.c", 2499, d3d_texture);

	water_texture = (IDirect3DBaseTexture8 *)global_d3d_texture_water;

	if (max_mipmap == 0)
	{
		max_mipmap = RASTERIZER_TARGET_WATER_MAX_MIPMAP_LEVELS;
	}
	else
	{
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox.c", 2508, max_mipmap>=0 && max_mipmap<=RASTERIZER_TARGET_WATER_MAX_MIPMAP_LEVELS);
	}

	water_texture->Format = (water_texture->Format & ~D3DFORMAT_MIPMAP_MASK) | (max_mipmap<<D3DFORMAT_MIPMAP_SHIFT);

	D3DCALL(success, IDirect3DDevice8_SetTexture(global_d3d_device, stage, (IDirect3DBaseTexture8*)d3d_texture));

	if (!success)
	{
		error(_error_silent, "### ERROR rasterizer_set_target_as_texture failed");
	}

	return;
}

void rasterizer_secondary_render_target_debug(
	rectangle2d const *bounds)
{
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox.c", 2532, bounds);
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox.c", 2533, global_d3d_device);

	if (rasterizer_debug_options.secondary_render_target_debug_enabled && global_window_parameters.rasterizer_target == _rasterizer_target_render_primary)
	{
		rasterizer_set_target_as_texture(0, _rasterizer_target_render_secondary, 0);
		IDirect3DDevice8_SetTextureStageState(global_d3d_device, 0, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
		IDirect3DDevice8_SetTextureStageState(global_d3d_device, 0, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
		IDirect3DDevice8_SetTextureStageState(global_d3d_device, 0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
		IDirect3DDevice8_SetTextureStageState(global_d3d_device, 0, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
		IDirect3DDevice8_SetTextureStageState(global_d3d_device, 0, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);
		IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_CULLMODE, D3DCULL_CCW);
		IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE);
		IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_ALPHABLENDENABLE, FALSE);
		IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_ALPHATESTENABLE, FALSE);
		IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_ZENABLE, D3DZB_FALSE);
		IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_ZBIAS, 0);
		rasterizer_set_vertex_shader_permutation(4, _rasterizer_vertex_type_dynamic_screen, 0);

		{
			short viewport_width = global_window_parameters.camera.viewport_bounds.x1 - global_window_parameters.camera.viewport_bounds.x0;
			short viewport_height = global_window_parameters.camera.viewport_bounds.y1 - global_window_parameters.camera.viewport_bounds.y0;
			real vsh_constants__screenproj[5][4] =
			{
				{2.f/viewport_width, 0.f, 0.f, -1.f - 1.f/viewport_width},
				{0.f, -2.f/viewport_height, 0.f, 1.f/viewport_height + 1.f},
				{0.f, 0.f, 0.f, 0.5f},
				{0.f, 0.f, 0.f, 1.f},
				{RASTERIZER_TARGET_RENDER_SECONDARY_WIDTH, RASTERIZER_TARGET_RENDER_SECONDARY_HEIGHT, 0.f, 1.f}
			};

			IDirect3DDevice8_SetVertexShaderConstant(global_d3d_device,
				VSH_CONSTANTS__SCREENPROJ_OFFSET,
				vsh_constants__screenproj,
				VSH_CONSTANTS__SCREENPROJ_COUNT);
		}

		memset(&pixel_shader, 0, sizeof(pixel_shader));
		pixel_shader.PSTextureModes = PS_TEXTUREMODES(PS_TEXTUREMODES_PROJECT2D, PS_TEXTUREMODES_NONE, PS_TEXTUREMODES_NONE, PS_TEXTUREMODES_NONE);
		pixel_shader.PSCombinerCount = PS_COMBINERCOUNT(1, PS_COMBINERCOUNT_MUX_LSB | PS_COMBINERCOUNT_SAME_C0 | PS_COMBINERCOUNT_SAME_C1);
		pixel_shader.PSFinalCombinerInputsABCD = PS_COMBINERINPUTS(PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_T0);
		rasterizer_set_pixel_shader(&pixel_shader);

		IDirect3DDevice8_Begin(global_d3d_device, D3DPT_TRIANGLEFAN);
		IDirect3DDevice8_SetVertexData2s(global_d3d_device, 4, 0, 1);
		IDirect3DDevice8_SetVertexData2s(global_d3d_device, VSDE_VERTEX, bounds->x0, bounds->y0);
		IDirect3DDevice8_SetVertexData2s(global_d3d_device, 4, 1, 1);
		IDirect3DDevice8_SetVertexData2s(global_d3d_device, VSDE_VERTEX, bounds->x1, bounds->y0);
		IDirect3DDevice8_SetVertexData2s(global_d3d_device, 4, 1, 0);
		IDirect3DDevice8_SetVertexData2s(global_d3d_device, VSDE_VERTEX, bounds->x1, bounds->y1);
		IDirect3DDevice8_SetVertexData2s(global_d3d_device, 4, 0, 0);
		IDirect3DDevice8_SetVertexData2s(global_d3d_device, VSDE_VERTEX, bounds->x0, bounds->y1);
		IDirect3DDevice8_End(global_d3d_device);
	}

	return;
}

void rasterizer_set_vertex_shader(
	short vertex_shader_index)
{
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox.c", 2613, vertex_shader_index>=0);
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox.c", 2614, vertex_shader_index<NUMBER_OF_VERTEX_SHADERS);

	if (vertex_shader_index != current_vertex_shader_index)
	{
		boolean success;

		if (vertex_shader_table[vertex_shader_index].handle != NONE)
		{
			IDirect3DDevice8_SetVertexShader(global_d3d_device, vertex_shader_table[vertex_shader_index].handle);
			success = TRUE;

			if (rasterizer_debug_options.statistics_mode)
			{
				rasterizer_frame_statistics.vertex_shader_bytes += vertex_shader_table[vertex_shader_index].size;
			}
		}
		else
		{
			error(_error_silent, "### ERROR vertex shader not valid (#%d)", vertex_shader_index);
			success = FALSE;
		}

		current_vertex_shader_index = vertex_shader_index;

		if (!success)
		{
			error(_error_silent, "### ERROR rasterizer_set_vertex_shader failed");
		}
	}

	return;
}

void rasterizer_set_pixel_shader(
	D3DPIXELSHADERDEF const *pixel_shader)
{
	short combiner_count;

	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox.c", 2652, pixel_shader);

	combiner_count = (short)(pixel_shader->PSCombinerCount & MASK(4));

	if (combiner_count < 6)
	{
		short stage;
		boolean unique_c0 = (pixel_shader->PSCombinerCount & (PS_COMBINERCOUNT_UNIQUE_C0<<8)) != 0;
		boolean unique_c1 = (pixel_shader->PSCombinerCount & (PS_COMBINERCOUNT_UNIQUE_C1<<8)) != 0;

		IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_PSTEXTUREMODES, pixel_shader->PSTextureModes);
		IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_PSCOMBINERCOUNT, pixel_shader->PSCombinerCount);
		IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_PSINPUTTEXTURE, pixel_shader->PSInputTexture);
		IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_PSDOTMAPPING, pixel_shader->PSDotMapping);
		IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_PSFINALCOMBINERINPUTSABCD, pixel_shader->PSFinalCombinerInputsABCD);
		IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_PSFINALCOMBINERINPUTSEFG, pixel_shader->PSFinalCombinerInputsEFG);
		IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_PSFINALCOMBINERCONSTANT0, pixel_shader->PSFinalCombinerConstant0);
		IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_PSFINALCOMBINERCONSTANT1, pixel_shader->PSFinalCombinerConstant1);

		for (stage = 0; stage < combiner_count; stage++)
		{
			D3DDevice_SetRenderStateNotInline(D3DRS_PSALPHAINPUTS0 + stage, pixel_shader->PSAlphaInputs[stage]);
			D3DDevice_SetRenderStateNotInline(D3DRS_PSALPHAOUTPUTS0 + stage, pixel_shader->PSAlphaOutputs[stage]);
			D3DDevice_SetRenderStateNotInline(D3DRS_PSRGBINPUTS0 + stage, pixel_shader->PSRGBInputs[stage]);
			D3DDevice_SetRenderStateNotInline(D3DRS_PSRGBOUTPUTS0 + stage, pixel_shader->PSRGBOutputs[stage]);

			if (unique_c0 || stage == 0)
			{
				D3DDevice_SetRenderStateNotInline(D3DRS_PSCONSTANT0_0 + stage, pixel_shader->PSConstant0[stage]);
			}

			if (unique_c1 || stage == 0)
			{
				D3DDevice_SetRenderStateNotInline(D3DRS_PSCONSTANT1_0 + stage, pixel_shader->PSConstant1[stage]);
			}
		}

		if (rasterizer_debug_options.statistics_mode == _rasterizer_statistics_mode_full)
		{
			rasterizer_frame_statistics.pixel_shader_bytes += sizeof(unsigned long)*(combiner_count*(4 + unique_c0 + unique_c1) + 8);
		}
	}
	else
	{
		IDirect3DDevice8_SetPixelShaderProgram(global_d3d_device, (D3DPIXELSHADERDEF *)pixel_shader);

		if (rasterizer_debug_options.statistics_mode == _rasterizer_statistics_mode_full)
		{
			rasterizer_frame_statistics.pixel_shader_bytes += 228;
		}
	}

	return;
}

void rasterizer_set_model_skinning(
	struct render_skinning const *skinning)
{
	static real vsh_constants__nodematrices[RASTERIZER_MAXIMUM_NODES_PER_MODEL][3][4];
	short node_index;

	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox.c", 2749, skinning);
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox.c", 2750, skinning->node_matrices);
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox.c", 2751, skinning->node_matrix_count>0 && skinning->node_matrix_count<RASTERIZER_MAXIMUM_NODES_PER_MODEL);

	for (node_index = 0; node_index < skinning->node_matrix_count; node_index++)
	{
		real scale = skinning->node_matrices[node_index].scale;
		real_matrix4x3 const *node_matrix = &skinning->node_matrices[node_index];
		real (*constants)[4] = vsh_constants__nodematrices[node_index];

		constants[0][0] = scale*node_matrix->forward.i;
		constants[0][1] = scale*node_matrix->left.i;
		constants[0][2] = scale*node_matrix->up.i;
		constants[0][3] = node_matrix->position.x;
		constants[1][0] = scale*node_matrix->forward.j;
		constants[1][1] = scale*node_matrix->left.j;
		constants[1][2] = scale*node_matrix->up.j;
		constants[1][3] = node_matrix->position.y;
		constants[2][0] = scale*node_matrix->forward.k;
		constants[2][1] = scale*node_matrix->left.k;
		constants[2][2] = scale*node_matrix->up.k;
		constants[2][3] = node_matrix->position.z;
	}

	IDirect3DDevice8_SetVertexShaderConstant(global_d3d_device, VSH_CONSTANTS__NODEMATRIX_OFFSET, vsh_constants__nodematrices, 3*skinning->node_matrix_count);

	if (rasterizer_debug_options.statistics_mode)
	{
		rasterizer_frame_statistics.skinning_bytes += sizeof(vsh_constants__nodematrices[0])*skinning->node_matrix_count;
	}

	return;
}

void rasterizer_set_model_lighting_point_light(
	long light_index,
	short light_num,
	struct rasterizer_model_lighting_constants *lighting_constants)
{
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox.c", 2826, lighting_constants);

	if (light_index != NONE)
	{
		struct rasterizer_light_submit_parameters const *light;
		struct rasterizer_point_light_constants *point_light;

		match_vassert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox.c", 2835, light_index>=0 && light_index<rasterizer_lights.light_count, csprintf(temporary, "### ERROR invalid light index #%d (count=#%d)", light_index, rasterizer_lights.light_count));

		light = &rasterizer_lights.lights[light_index];
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox.c", 2837, light->radius>0.0f);

		point_light = &lighting_constants->point_lights[light_num];
		point_light->position = light->position;
		point_light->inverse_radius_squared = 1.f/(light->radius*light->radius);
		point_light->forward = light->forward;
		point_light->color = light->color;

		if (light->definition->geometry.runtime_cosine_falloff_angle != -1.f)
		{
			point_light->spot_falloff_coefficient_A = 1.f/(light->definition->geometry.runtime_cosine_falloff_angle - light->definition->geometry.runtime_cosine_cutoff_angle);
			point_light->spot_falloff_coefficient_B = -(point_light->spot_falloff_coefficient_A*light->definition->geometry.runtime_cosine_cutoff_angle);
		}
		else
		{
			point_light->spot_falloff_coefficient_A = 0.f;
			point_light->spot_falloff_coefficient_B = 1.f;
		}
	}
	else
	{
		memset(&lighting_constants->point_lights[light_num], 0, sizeof(lighting_constants->point_lights[light_num]));
		lighting_constants->point_lights[light_num].spot_falloff_coefficient_A = 0.f;
		lighting_constants->point_lights[light_num].spot_falloff_coefficient_B = 1.f;
	}

	return;
}

void rasterizer_set_model_lighting_distant_light(
	struct render_distant_light const *light,
	short light_num,
	struct rasterizer_model_lighting_constants *lighting_constants)
{
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox.c", 2874, lighting_constants);

	if (light)
	{
		lighting_constants->distant_lights[light_num].forward = light->direction;
		lighting_constants->distant_lights[light_num].color = light->color;
	}
	else
	{
		memset(&lighting_constants->distant_lights[light_num], 0, sizeof(lighting_constants->distant_lights[light_num]));
	}

	return;
}

void rasterizer_set_model_lighting(
	struct render_lighting const *lighting)
{
	struct rasterizer_model_lighting_constants vsh_constants__vertexlight;

	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox.c", 2896, lighting);
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox.c", 2897, lighting->point_light_count>=0 && lighting->point_light_count<=MAXIMUM_RENDERED_POINT_LIGHTS);
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox.c", 2898, lighting->distant_light_count>=0 && lighting->distant_light_count<=MAXIMUM_RENDERED_DISTANT_LIGHTS);

	if (rasterizer_debug_options.model_lighting_ambient > 0.f)
	{
		real ambient;

		memset(&vsh_constants__vertexlight, 0, sizeof(vsh_constants__vertexlight));
		ambient = rasterizer_debug_options.model_lighting_ambient;
		vsh_constants__vertexlight.ambient.blue = ambient;
		vsh_constants__vertexlight.ambient.green = ambient;
		vsh_constants__vertexlight.ambient.red = ambient;
	}
	else
	{
		short light_num;

		for (light_num = 0; light_num < MAXIMUM_RENDERED_POINT_LIGHTS; light_num++)
		{
			rasterizer_set_model_lighting_point_light(lighting->point_light_count > light_num ? lighting->point_light_indices[light_num] : NONE, light_num, &vsh_constants__vertexlight);
		}

		for (light_num = 0; light_num < MAXIMUM_RENDERED_DISTANT_LIGHTS; light_num++)
		{
			rasterizer_set_model_lighting_distant_light(lighting->distant_light_count > light_num ? &lighting->distant_lights[light_num] : NULL, light_num, &vsh_constants__vertexlight);
		}

		vsh_constants__vertexlight.ambient = lighting->ambient_color;
	}

	IDirect3DDevice8_SetVertexShaderConstant(global_d3d_device, VSH_CONSTANTS__VERTEXLIGHT_OFFSET, &vsh_constants__vertexlight, VSH_CONSTANTS__VERTEXLIGHT_COUNT);

	if (rasterizer_debug_options.statistics_mode)
	{
		rasterizer_frame_statistics.lighting_bytes += sizeof(vsh_constants__vertexlight);
	}

	return;
}

void rasterizer_set_frustum_z(
	real z_near,
	real z_far)
{
	real vsh_constants__viewproj[VSH_CONSTANTS__VIEWPROJ_COUNT + VSH_CONSTANTS__EYEXFORM_COUNT][4];
	short row;

	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox.c", 2967, global_d3d_device);

	render_camera_hack_frustum_z(&global_window_parameters.frustum, z_near, z_far);

	for (row = 0; row < 4; row++)
	{
		short column;

		for (column = 0; column < 4; column++)
		{
			real const *vector = global_window_parameters.frustum.world_to_view.n[column];

			vsh_constants__viewproj[row][column] =
				global_window_parameters.frustum.projection_matrix[0][row]*vector[0] +
				global_window_parameters.frustum.projection_matrix[1][row]*vector[1] +
				global_window_parameters.frustum.projection_matrix[2][row]*vector[2];
		}

		vsh_constants__viewproj[row][3] += global_window_parameters.frustum.projection_matrix[3][row];
	}

	vsh_constants__viewproj[4][0] = global_window_parameters.camera.position.x;
	vsh_constants__viewproj[4][1] = global_window_parameters.camera.position.y;
	vsh_constants__viewproj[4][2] = global_window_parameters.camera.position.z;
	vsh_constants__viewproj[4][3] = 2.f;
	vsh_constants__viewproj[5][0] = global_window_parameters.camera.forward.i;
	vsh_constants__viewproj[5][1] = global_window_parameters.camera.forward.j;
	vsh_constants__viewproj[5][2] = global_window_parameters.camera.forward.k;
	vsh_constants__viewproj[5][3] = 0.5f;
	vsh_constants__viewproj[6][0] = global_window_parameters.frustum.view_to_world.forward.i;
	vsh_constants__viewproj[6][1] = global_window_parameters.frustum.view_to_world.forward.j;
	vsh_constants__viewproj[6][2] = global_window_parameters.frustum.view_to_world.forward.k;
	vsh_constants__viewproj[6][3] = 1.f;
	vsh_constants__viewproj[7][0] = global_window_parameters.frustum.view_to_world.left.i;
	vsh_constants__viewproj[7][1] = global_window_parameters.frustum.view_to_world.left.j;
	vsh_constants__viewproj[7][2] = global_window_parameters.frustum.view_to_world.left.k;
	vsh_constants__viewproj[7][3] = 255.9375f;

	IDirect3DDevice8_SetVertexShaderConstant(global_d3d_device, VSH_CONSTANTS__VIEWPROJ_OFFSET, vsh_constants__viewproj, VSH_CONSTANTS__VIEWPROJ_COUNT + VSH_CONSTANTS__EYEXFORM_COUNT);

	return;
}

void rasterizer_set_stencil_mode(
	short stencil_mode)
{
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox.c", 3034, global_d3d_device);

	if (!rasterizer_debug_options.stencil_mask_enabled)
	{
		stencil_mode = _rasterizer_stencil_mode_none;
	}

	if (stencil_mode != previous_stencil_mode)
	{
		switch (stencil_mode)
		{
		case _rasterizer_stencil_mode_none:
			SetRenderStateSmart(D3DRS_STENCILENABLE, FALSE);
			break;
		case _rasterizer_stencil_mode_write:
			SetRenderStateSmart(D3DRS_STENCILENABLE, TRUE);
			SetRenderStateSmart(D3DRS_STENCILFAIL, D3DSTENCILOP_KEEP);
			SetRenderStateSmart(D3DRS_STENCILZFAIL, D3DSTENCILOP_KEEP);
			SetRenderStateSmart(D3DRS_STENCILPASS, D3DSTENCILOP_REPLACE);
			SetRenderStateSmart(D3DRS_STENCILFUNC, D3DCMP_ALWAYS);
			SetRenderStateSmart(D3DRS_STENCILREF, 1);
			SetRenderStateSmart(D3DRS_STENCILMASK, 1);
			SetRenderStateSmart(D3DRS_STENCILWRITEMASK, 1);
			break;
		case _rasterizer_stencil_mode_reject:
			SetRenderStateSmart(D3DRS_STENCILENABLE, TRUE);
			SetRenderStateSmart(D3DRS_STENCILFAIL, D3DSTENCILOP_KEEP);
			SetRenderStateSmart(D3DRS_STENCILZFAIL, D3DSTENCILOP_KEEP);
			SetRenderStateSmart(D3DRS_STENCILPASS, D3DSTENCILOP_KEEP);
			SetRenderStateSmart(D3DRS_STENCILFUNC, D3DCMP_EQUAL);
			SetRenderStateSmart(D3DRS_STENCILREF, 0);
			SetRenderStateSmart(D3DRS_STENCILMASK, 1);
			SetRenderStateSmart(D3DRS_STENCILWRITEMASK, 0);
			break;
		case _rasterizer_stencil_mode_reject_invert:
			SetRenderStateSmart(D3DRS_STENCILENABLE, TRUE);
			SetRenderStateSmart(D3DRS_STENCILFAIL, D3DSTENCILOP_KEEP);
			SetRenderStateSmart(D3DRS_STENCILZFAIL, D3DSTENCILOP_KEEP);
			SetRenderStateSmart(D3DRS_STENCILPASS, D3DSTENCILOP_KEEP);
			SetRenderStateSmart(D3DRS_STENCILFUNC, D3DCMP_NOTEQUAL);
			SetRenderStateSmart(D3DRS_STENCILREF, 0);
			SetRenderStateSmart(D3DRS_STENCILMASK, 1);
			SetRenderStateSmart(D3DRS_STENCILWRITEMASK, 0);
			break;
		case _rasterizer_stencil_mode_write_alpha_tested_decal:
			SetRenderStateSmart(D3DRS_STENCILENABLE, TRUE);
			SetRenderStateSmart(D3DRS_STENCILFAIL, D3DSTENCILOP_KEEP);
			SetRenderStateSmart(D3DRS_STENCILZFAIL, D3DSTENCILOP_KEEP);
			SetRenderStateSmart(D3DRS_STENCILPASS, D3DSTENCILOP_REPLACE);
			SetRenderStateSmart(D3DRS_STENCILFUNC, D3DCMP_EQUAL);
			SetRenderStateSmart(D3DRS_STENCILREF, 2);
			SetRenderStateSmart(D3DRS_STENCILMASK, 1);
			SetRenderStateSmart(D3DRS_STENCILWRITEMASK, 2);
			break;
		case _rasterizer_stencil_mode_reject_alpha_tested_decal:
			SetRenderStateSmart(D3DRS_STENCILENABLE, TRUE);
			SetRenderStateSmart(D3DRS_STENCILFAIL, D3DSTENCILOP_KEEP);
			SetRenderStateSmart(D3DRS_STENCILZFAIL, D3DSTENCILOP_KEEP);
			SetRenderStateSmart(D3DRS_STENCILPASS, D3DSTENCILOP_KEEP);
			SetRenderStateSmart(D3DRS_STENCILFUNC, D3DCMP_EQUAL);
			SetRenderStateSmart(D3DRS_STENCILREF, 0);
			SetRenderStateSmart(D3DRS_STENCILMASK, 3);
			SetRenderStateSmart(D3DRS_STENCILWRITEMASK, 0);
			break;
		default:
			match_vhalt("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox.c", 3099, "### ERROR unsupported stencil mode");
		}

		previous_stencil_mode = stencil_mode;
	}

	return;
}

short rasterizer_get_stencil_mode(
	void)
{
	return previous_stencil_mode;
}

void SetupSmartStates(
	void)
{
	long type;
	long stage;

	for (type = 0; type < D3DRS_MAX; type++)
	{
		IDirect3DDevice8_GetRenderState(global_d3d_device, type, &renderstate_table[type]);
	}

	for (type = 0; type < D3DTSS_MAX; type++)
	{
		for (stage = 0; stage < D3DTSS_MAXSTAGES; stage++)
		{
			IDirect3DDevice8_GetTextureStageState(global_d3d_device, stage, type, &texturestagestate_table[stage][type]);
		}
	}

	for (stage = 0; stage < D3DTSS_MAXSTAGES; stage++)
	{
		texture_table[stage] = NULL;
	}

	return;
}
