/*
PROGRESS_BAR.C
*/

/* ---------- headers */

#include "cseries.h"
#include "progress_bar.h"
#include "ui_widget.h"
#include "objects.h"
#include "collision_bsp.h"
#include "render.h"
#include "network_game_globals.h"
#include "unicode.h"
#include "rasterizer.h"
#include "input.h"
#include "player_profile.h"
#include "saved_game_files.h"
#include "ima_adpcm.h"
#include "cache_files.h"
#include "rasterizer/xbox/rasterizer_xbox.h"

/* ---------- constants */

enum
{
	NUMBER_OF_MONK_VOICES = 4,							/* fake name */
	PROGRESS_BAR_MASK_TEXTURE_WIDTH = 128,				/* fake name */
	PROGRESS_BAR_MASK_TEXTURE_HEIGHT = 16,				/* fake name */
	PROGRESS_BAR_RELEASE_DELAY = MILLISECONDS_PER_SECOND	/* fake name */
};

enum
{
	_progress_bar_texture_stage_image = 0,	/* fake name */
	_progress_bar_texture_stage_mask,		/* fake name */
	_progress_bar_texture_stage_noise,		/* fake name */
	_progress_bar_texture_stage_unused,		/* fake name */
	NUMBER_OF_TEXTURE_STAGES				/* fake name */
};

/* ---------- macros */

/* ---------- structures */

typedef struct
{
	int status;
	unsigned char type;
	unsigned char pixelDepth;
	short int width;
	short int height;
	unsigned long *imageData;
} tgaInfo;

struct gravy_layer	/* fake name */
{
	real width;
	real height;
	real x;
	real y;
	real half_width;
	real half_height;
	real distance;
};

struct gravy_rect	/* fake name */
{
	real x0;
	real y0;
	real x1;
	real y1;
};

/* ---------- prototypes */

static boolean progress_bar_is_stuff_ready(void);
static void progress_bar_render(real t);
static void progress_bar_uncompress_grayscale_buffer(IDirect3DTexture8 **texture);
static void progress_bar_create_noise_texture(void);
static void progress_bar_create_mask_texture(void);
static void progress_bar_make_stuff_ready(void);
static void draw_layer_int(struct gravy_layer const *layer, real_rgb_color const *color, real alpha, real t);
static void draw_layer(real distance, real alpha, real_rgb_color const *color, real t);
static void draw_initial_layer(real_rgb_color const *color, real t);
static void this_is_awful(D3DBaseTexture *texture, D3DSurface const *surface);
static void do_convoluation_coords(real x, real y);
static void draw_fade_layer(real x_offset, real y_offset, real alpha);

/* ---------- globals */

D3DPIXELSHADERDEF blur_shader;
D3DPIXELSHADERDEF regular_shader;
unsigned long current_time;

static boolean progress_bar_enabled = TRUE;	/* fake name */

#include "progress_bar_monk_tone.h"

real delta_z = 5.f;
real num_layers = 40.f;

static real convolution_offset = .1f;	/* fake name */

static unsigned long progress_bar_start_time = 0;						/* fake name */
static real progress_bar_start_progress = 0.f;							/* fake name */
static D3DMATRIX saved_projection = {0};								/* fake name */
static D3DMATRIX saved_world = {0};										/* fake name */
static D3DMATRIX saved_view = {0};										/* fake name */
static D3DMATRIX gravy_projection = {0};								/* fake name */
static D3DMATRIX progress_bar_projection = {0};							/* fake name */
static D3DMATRIX progress_bar_world = {0};								/* fake name */
static D3DMATRIX progress_bar_view = {0};								/* fake name */
static IDirectSoundBuffer *monk_voices[NUMBER_OF_MONK_VOICES] = {0};	/* fake name */

long progress_bar_mode = 0;

static boolean progress_bar_capture_frame = FALSE;			/* fake name */
static IDirect3DTexture8 *progress_bar_texture = NULL;		/* fake name */
static IDirect3DTexture8 *progress_bar_mask_texture = NULL;	/* fake name */
static boolean progress_bar_active = FALSE;					/* fake name */

real last_t = 0.f;

/* ---------- public code */

void tgaLoadHeader(
	FILE *file,
	tgaInfo *info)
{
	unsigned char cGarbage;
	short int iGarbage;

	fread(&cGarbage, sizeof(unsigned char), 1, file);
	fread(&cGarbage, sizeof(unsigned char), 1, file);
	fread(&info->type, sizeof(unsigned char), 1, file);
	fread(&iGarbage, sizeof(short int), 1, file);
	fread(&iGarbage, sizeof(short int), 1, file);
	fread(&cGarbage, sizeof(unsigned char), 1, file);
	fread(&iGarbage, sizeof(short int), 1, file);
	fread(&iGarbage, sizeof(short int), 1, file);
	fread(&info->width, sizeof(short int), 1, file);
	fread(&info->height, sizeof(short int), 1, file);
	fread(&info->pixelDepth, sizeof(unsigned char), 1, file);
	fread(&cGarbage, sizeof(unsigned char), 1, file);

	return;
}

void tgaLoadImageData(
	FILE *file,
	tgaInfo *info)
{
	int i;

	for (i = 0; i < info->height; i++)
	{
		int j;
		unsigned long *pixel = info->imageData + (info->height - i - 1) * info->width;

		for (j = 0; j < info->width; j++)
		{
			unsigned char color[3];

			fread(color, sizeof(unsigned char), 3, file);
			*pixel++ = (color[0] << 16) | ((color[0] >> 1) << 8) | (color[0] >> 2);
		}
	}

	return;
}

void tgaLoad(
	char *filename,
	unsigned long *pixels)
{
	FILE *file = fopen(filename, "rb");

	if (file)
	{
		tgaInfo info;

		tgaLoadHeader(file, &info);
		info.imageData = pixels;
		tgaLoadImageData(file, &info);
		fclose(file);
	}

	return;
}

static boolean progress_bar_is_stuff_ready(
	void)
{
	boolean result = progress_bar_texture != NULL;

	return result;
}

void progress_bar_initialize(
	void)
{
	char source_path[40];
	char destination_path[40];
	char map_directory[40];
	char destination_directory[40];

	strcpy(map_directory, cache_files_map_directory());
	sprintf(source_path, "d%sloading.tga", map_directory + 1);
	sprintf(destination_path, "z%sloading.tga", map_directory + 1);
	sprintf(destination_directory, "z%s", map_directory + 1);
	destination_directory[strlen(destination_directory) - 1] = '\0';
	CreateDirectory(destination_directory, NULL);
	CopyFile(source_path, destination_path, TRUE);

	return;
}

void progress_bar_dispose(
	void)
{
	return;
}

void progress_bar_begin(
	boolean ui_is_up)
{
	progress_bar_capture_frame = !ui_is_up;
	progress_bar_active = TRUE;
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);

	return;
}

void progress_bar_end(
	void)
{
	progress_bar_active = FALSE;
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL);

	return;
}

boolean progress_bar_is_active(
	void)
{
	return progress_bar_active;
}

__inline void SetRenderStateSmart(
	D3DRENDERSTATETYPE State,
	DWORD Value)
{
	D3DDevice_SetRenderState(State, Value);

	return;
}

__inline void SetTextureStageStateSmart(
	DWORD Stage,
	D3DTEXTURESTAGESTATETYPE Type,
	DWORD Value)
{
	D3DDevice_SetTextureStageState(Stage, Type, Value);

	return;
}

static void progress_bar_render(
	real t)
{
	real_rgb_color color;
	D3DSurface *front_buffer;
	D3DBaseTexture front_buffer_texture;
	long voice_index;
	long stage;
	real ranges[NUMBER_OF_MONK_VOICES][2] = {{ 0.f, 1.f }, { .4f, 1.f }, { .5f, 1.f }, { .55f, 1.f }};
	real volumes[NUMBER_OF_MONK_VOICES] = { 3500.f, 4500.f, 3500.f, 4500.f };
	static real wobble = 0.f;

	wobble += .01f;
	t = (real)((sine(wobble) * .05 + t - last_t) * .2 + last_t);

	if (t > 1.0)
	{
		t = 1.f;
	}

	last_t = t;

	color.blue = (real)pow(sin(t * 3.14159), .9f);
	color.green = color.blue;
	color.red = color.blue;

	// Mix the monk voices. Monk0 is always audible, but the rest fade in at loading % -- (ranges[])
	for (voice_index = 0; voice_index < NUMBER_OF_MONK_VOICES; voice_index++)
	{
		real fraction;

		if (t < ranges[voice_index][0])
		{
			fraction = 0.f;
		}
		else if (t > ranges[voice_index][1])
		{
			fraction = 0.f;
		}
		else
		{
			fraction = (t - ranges[voice_index][0]) / (ranges[voice_index][1] - ranges[voice_index][0]);
		}

		if (monk_voices[voice_index])
		{
			IDirectSoundBuffer_SetVolume(
				monk_voices[voice_index],
				(long)((real)pow(sin(fraction * 3.1415), .3f) * volumes[voice_index] - 5000.f));
		}
	}

	current_time = GetTickCount();

	// Backup current View Matrix states
	IDirect3DDevice8_GetTransform(global_d3d_device, D3DTS_WORLD, &saved_world);
	IDirect3DDevice8_GetTransform(global_d3d_device, D3DTS_VIEW, &saved_view);
	IDirect3DDevice8_GetTransform(global_d3d_device, D3DTS_PROJECTION, &saved_projection);

	IDirect3DDevice8_SetTransform(global_d3d_device, D3DTS_WORLD, &progress_bar_world);
	IDirect3DDevice8_SetTransform(global_d3d_device, D3DTS_VIEW, &progress_bar_view);
	IDirect3DDevice8_SetTransform(global_d3d_device, D3DTS_PROJECTION, &progress_bar_projection);

	IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_ALPHABLENDENABLE, TRUE);
	IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_DESTBLEND, D3DBLEND_ONE);
	IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_CULLMODE, D3DCULL_NONE);
	IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_ZENABLE, FALSE);
	IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_LIGHTING, FALSE);

	IDirect3DDevice8_SetTextureStageState(global_d3d_device, _progress_bar_texture_stage_image,D3DTSS_MINFILTER, D3DTEXF_LINEAR);
	IDirect3DDevice8_SetTextureStageState(global_d3d_device, _progress_bar_texture_stage_image,D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
	IDirect3DDevice8_SetTextureStageState(global_d3d_device, _progress_bar_texture_stage_image,D3DTSS_MIPFILTER, D3DTEXF_POINT);
	IDirect3DDevice8_SetTextureStageState(global_d3d_device, _progress_bar_texture_stage_image,D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
	IDirect3DDevice8_SetTextureStageState(global_d3d_device, _progress_bar_texture_stage_image,D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);

	IDirect3DDevice8_SetTextureStageState(global_d3d_device, _progress_bar_texture_stage_mask,D3DTSS_MINFILTER, D3DTEXF_LINEAR);
	IDirect3DDevice8_SetTextureStageState(global_d3d_device, _progress_bar_texture_stage_mask,D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
	IDirect3DDevice8_SetTextureStageState(global_d3d_device, _progress_bar_texture_stage_mask,D3DTSS_MIPFILTER, D3DTEXF_POINT);
	IDirect3DDevice8_SetTextureStageState(global_d3d_device, _progress_bar_texture_stage_mask,D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
	IDirect3DDevice8_SetTextureStageState(global_d3d_device, _progress_bar_texture_stage_mask,D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);

	IDirect3DDevice8_SetTextureStageState(global_d3d_device, _progress_bar_texture_stage_noise,D3DTSS_MINFILTER, D3DTEXF_LINEAR);
	IDirect3DDevice8_SetTextureStageState(global_d3d_device, _progress_bar_texture_stage_noise,D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
	IDirect3DDevice8_SetTextureStageState(global_d3d_device, _progress_bar_texture_stage_noise,D3DTSS_MIPFILTER, D3DTEXF_POINT);
	IDirect3DDevice8_SetTextureStageState(global_d3d_device, _progress_bar_texture_stage_noise,D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
	IDirect3DDevice8_SetTextureStageState(global_d3d_device, _progress_bar_texture_stage_noise,D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);

	IDirect3DDevice8_SetTexture(global_d3d_device, _progress_bar_texture_stage_image, (IDirect3DBaseTexture8 *)progress_bar_texture);

	IDirect3DDevice8_Clear(global_d3d_device, 0, NULL, D3DCLEAR_TARGET, 0, 1.f, 0);

	IDirect3DDevice8_SetVertexShader(global_d3d_device, 0);

	// Set Blur Effect, used on the previous frame (front buffer, the current surface)
	IDirect3DDevice8_SetPixelShaderProgram(global_d3d_device, &blur_shader);

	// On XBox, You can call GetBackBuffer() with -1, and it returns the FrontBuffer.
	IDirect3DDevice8_GetBackBuffer(global_d3d_device, -1, D3DBACKBUFFER_TYPE_MONO, &front_buffer);
	// Data hack on the front_buffer to make it a usable texture
	this_is_awful(&front_buffer_texture, front_buffer);

	// Bind this front_buffer to all the texture stages
	for (stage = 0; stage < NUMBER_OF_TEXTURE_STAGES; stage++)
	{
		IDirect3DDevice8_SetTextureStageState(global_d3d_device, stage, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
		IDirect3DDevice8_SetTextureStageState(global_d3d_device, stage, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
		IDirect3DDevice8_SetTextureStageState(global_d3d_device, stage, D3DTSS_MIPFILTER, D3DTEXF_POINT);
		IDirect3DDevice8_SetTextureStageState(global_d3d_device, stage, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
		IDirect3DDevice8_SetTextureStageState(global_d3d_device, stage, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
		IDirect3DDevice8_SetTexture(global_d3d_device, stage, &front_buffer_texture);
	}

	// Draw the fade effect with the blur
	if (t < .9f)
	{
		draw_fade_layer(0.f, 0.f, .9f);
	}
	else
	{
		draw_fade_layer(0.f, 0.f, .9f - (t - .9f) * 9.f);
	}

	// Setup and draw the map's `loading.tga`
	IDirect3DDevice8_SetTexture(global_d3d_device, _progress_bar_texture_stage_image, (IDirect3DBaseTexture8 *)progress_bar_texture);
	IDirect3DDevice8_SetTexture(global_d3d_device, _progress_bar_texture_stage_mask, (IDirect3DBaseTexture8 *)progress_bar_mask_texture);
	IDirect3DDevice8_SetTextureStageState(global_d3d_device, _progress_bar_texture_stage_mask,D3DTSS_BORDERCOLOR, 0x05050505);
	IDirect3DDevice8_SetTextureStageState(global_d3d_device, _progress_bar_texture_stage_mask,D3DTSS_ADDRESSU, D3DTADDRESS_BORDER);
	IDirect3DDevice8_SetTextureStageState(global_d3d_device, _progress_bar_texture_stage_mask,D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);

	IDirect3DDevice8_SetPixelShaderProgram(global_d3d_device, &regular_shader);

	draw_initial_layer(&color, t);

	// unbind each Texture stage
	for (stage = 0; stage < NUMBER_OF_TEXTURE_STAGES; stage++)
	{
		IDirect3DDevice8_SetTexture(global_d3d_device, stage, NULL);
	}

	// Restore saved matrixes
	IDirect3DDevice8_SetTransform(global_d3d_device, D3DTS_WORLD, &saved_world);
	IDirect3DDevice8_SetTransform(global_d3d_device, D3DTS_VIEW, &saved_view);
	IDirect3DDevice8_SetTransform(global_d3d_device, D3DTS_PROJECTION, &saved_projection);

	IDirect3DDevice8_BlockUntilVerticalBlank(global_d3d_device);

	return;
}

static void progress_bar_uncompress_grayscale_buffer(
	IDirect3DTexture8 **texture)
{
	D3DLOCKED_RECT locked_rect;
	D3DSURFACE_DESC description;
	unsigned long *pixels;
	char map_directory[40];
	char path[40];

	IDirect3DDevice8_CreateTexture(
		global_d3d_device,
		RASTERIZER_TARGET_RENDER_PRIMARY_WIDTH / 2,
		RASTERIZER_TARGET_RENDER_PRIMARY_HEIGHT / 2,
		1,
		0,
		D3DFMT_LIN_A8B8G8R8,
		0,
		texture);
	IDirect3DTexture8_LockRect(*texture, 0, &locked_rect, NULL, 0);
	IDirect3DTexture8_GetLevelDesc(*texture, 0, &description);
	pixels = locked_rect.pBits;
	strcpy(map_directory, cache_files_map_directory());
	sprintf(path, "z%sloading.tga", map_directory + 1);
	tgaLoad(path, pixels);
	IDirect3DTexture8_UnlockRect(*texture, 0);

	return;
}

static void progress_bar_create_mask_texture(
	void)
{
	D3DLOCKED_RECT locked_rect;
	unsigned long *pixel;
	long y;

	IDirect3DDevice8_CreateTexture(global_d3d_device, PROGRESS_BAR_MASK_TEXTURE_WIDTH, PROGRESS_BAR_MASK_TEXTURE_HEIGHT, 1, 0, D3DFMT_LIN_A8B8G8R8, 0, &progress_bar_mask_texture);
	IDirect3DTexture8_LockRect(progress_bar_mask_texture, 0, &locked_rect, NULL, 0);
	pixel = locked_rect.pBits;

	for (y = 0; y < PROGRESS_BAR_MASK_TEXTURE_HEIGHT; y++)
	{
		long x;
		double v = pow(sin(3.1415f * (y * (1.f / (PROGRESS_BAR_MASK_TEXTURE_HEIGHT - 1)))), .75);

		for (x = 0; x < PROGRESS_BAR_MASK_TEXTURE_WIDTH; x++)
		{
			byte intensity = (byte)(((real)(sin(3.1415f * (x * (1.f / (PROGRESS_BAR_MASK_TEXTURE_WIDTH - 1)))) * v) * (250.f / 255.f) + 5.f / 255.f) * 255.f);

			*pixel++ = (intensity << 24) | (intensity << 16) | (intensity << 8) | intensity;
		}
	}

	IDirect3DTexture8_UnlockRect(progress_bar_mask_texture, 0);

	return;
}

void progress_bar_eachframe(
	void)
{
	static boolean first_frame = TRUE;
	unsigned long time = GetTickCount();

	if (first_frame)
	{
		progress_bar_start_time = GetTickCount();
		first_frame = FALSE;
		progress_bar_mode = 1;
	}

	if (time - progress_bar_start_time > PROGRESS_BAR_RELEASE_DELAY)
	{
		long voice_index;

		last_t = 0.f;

		if (progress_bar_texture)
		{
			IDirect3DTexture8_Release(progress_bar_texture);
			progress_bar_texture = NULL;
		}

		if (progress_bar_mask_texture)
		{
			IDirect3DTexture8_Release(progress_bar_mask_texture);
			progress_bar_mask_texture = NULL;
		}

		if (progress_bar_texture)
		{
			IDirect3DTexture8_Release(progress_bar_texture);
			progress_bar_texture = NULL;
		}

		for (voice_index = 0; voice_index < NUMBER_OF_MONK_VOICES; voice_index++)
		{
			if (monk_voices[voice_index])
			{
				IDirectSoundBuffer_Stop(monk_voices[voice_index]);
				IDirectSoundBuffer_Release(monk_voices[voice_index]);
				monk_voices[voice_index] = NULL;
			}
		}
	}

	return;
}

static void progress_bar_make_stuff_ready(
	void)
{
	XBOXADPCMWAVEFORMAT format;
	DSBUFFERDESC description;
	D3DSurface *depth_buffer;
	D3DSurface *back_buffer;
	D3DSurface *front_buffer;
	long voice_index;

	D3DXMatrixIdentity(&progress_bar_world);
	D3DXMatrixIdentity(&progress_bar_view);
	D3DXMatrixOrthoLH(&progress_bar_projection, 2.f, 2.f, -1.f, 2.f);
	D3DXMatrixPerspectiveLH(&gravy_projection, .64f, .48f, 1.f, 1000.f);
	progress_bar_uncompress_grayscale_buffer(&progress_bar_texture);
	progress_bar_create_noise_texture();
	progress_bar_create_mask_texture();

	SetRenderStateSmart(D3DRS_ALPHABLENDENABLE, TRUE);
	SetRenderStateSmart(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	SetRenderStateSmart(D3DRS_DESTBLEND, D3DBLEND_ONE);
	SetRenderStateSmart(D3DRS_CULLMODE, D3DCULL_NONE);
	SetRenderStateSmart(D3DRS_ZENABLE, FALSE);
	SetRenderStateSmart(D3DRS_LIGHTING, FALSE);
	IDirect3DDevice8_SetTexture(global_d3d_device, _progress_bar_texture_stage_image, (IDirect3DBaseTexture8 *)progress_bar_texture);
	SetTextureStageStateSmart(_progress_bar_texture_stage_image,D3DTSS_MINFILTER, D3DTEXF_LINEAR);
	SetTextureStageStateSmart(_progress_bar_texture_stage_image,D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
	SetTextureStageStateSmart(_progress_bar_texture_stage_image,D3DTSS_MIPFILTER, D3DTEXF_POINT);
	SetTextureStageStateSmart(_progress_bar_texture_stage_image,D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
	SetTextureStageStateSmart(_progress_bar_texture_stage_image,D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);

	memset(&blur_shader, 0, sizeof(blur_shader));
	blur_shader.PSTextureModes = PS_TEXTUREMODES(
		PS_TEXTUREMODES_PROJECT2D,
		PS_TEXTUREMODES_PROJECT2D,
		PS_TEXTUREMODES_PROJECT2D,
		PS_TEXTUREMODES_PROJECT2D);
	blur_shader.PSCombinerCount = PS_COMBINERCOUNT(
		3,
		PS_COMBINERCOUNT_MUX_LSB | PS_COMBINERCOUNT_SAME_C0 | PS_COMBINERCOUNT_SAME_C1);
	blur_shader.PSRGBInputs[0] = PS_COMBINERINPUTS(
		PS_REGISTER_T0 | PS_CHANNEL_RGB | PS_INPUTMAPPING_UNSIGNED_IDENTITY,
		PS_REGISTER_ONE_HALF,
		PS_REGISTER_T1 | PS_CHANNEL_RGB | PS_INPUTMAPPING_UNSIGNED_IDENTITY,
		PS_REGISTER_ONE_HALF);
	blur_shader.PSRGBOutputs[0] = PS_COMBINEROUTPUTS(
		PS_REGISTER_DISCARD,
		PS_REGISTER_DISCARD,
		PS_REGISTER_R0,
		PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_CD_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
	blur_shader.PSRGBInputs[1] = PS_COMBINERINPUTS(
		PS_REGISTER_T2 | PS_CHANNEL_RGB | PS_INPUTMAPPING_UNSIGNED_IDENTITY,
		PS_REGISTER_ONE_HALF,
		PS_REGISTER_T3 | PS_CHANNEL_RGB | PS_INPUTMAPPING_UNSIGNED_IDENTITY,
		PS_REGISTER_ONE_HALF);
	blur_shader.PSRGBOutputs[1] = PS_COMBINEROUTPUTS(
		PS_REGISTER_DISCARD,
		PS_REGISTER_DISCARD,
		PS_REGISTER_R1,
		PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_CD_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
	blur_shader.PSRGBInputs[2] = PS_COMBINERINPUTS(
		PS_REGISTER_R0 | PS_CHANNEL_RGB | PS_INPUTMAPPING_UNSIGNED_IDENTITY,
		PS_REGISTER_ONE_HALF,
		PS_REGISTER_R1 | PS_CHANNEL_RGB | PS_INPUTMAPPING_UNSIGNED_IDENTITY,
		PS_REGISTER_ONE_HALF);
	blur_shader.PSRGBOutputs[2] = PS_COMBINEROUTPUTS(
		PS_REGISTER_DISCARD,
		PS_REGISTER_DISCARD,
		PS_REGISTER_R0,
		PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_CD_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
	blur_shader.PSAlphaInputs[0] = PS_COMBINERINPUTS(
		PS_REGISTER_V0 | PS_CHANNEL_ALPHA | PS_INPUTMAPPING_UNSIGNED_IDENTITY,
		PS_REGISTER_ONE,
		PS_REGISTER_ZERO,
		PS_REGISTER_ZERO);
	blur_shader.PSAlphaOutputs[0] = PS_COMBINEROUTPUTS(
		PS_REGISTER_R0,
		PS_REGISTER_DISCARD,
		PS_REGISTER_DISCARD,
		PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_CD_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
	blur_shader.PSFinalCombinerInputsABCD = PS_COMBINERINPUTS(
		PS_REGISTER_ZERO,
		PS_REGISTER_ZERO,
		PS_REGISTER_ZERO,
		PS_REGISTER_R0 | PS_CHANNEL_RGB | PS_INPUTMAPPING_UNSIGNED_IDENTITY);
	blur_shader.PSFinalCombinerInputsEFG = PS_COMBINERINPUTS(
		PS_REGISTER_ZERO,
		PS_REGISTER_ZERO,
		PS_REGISTER_R0 | PS_CHANNEL_ALPHA | PS_INPUTMAPPING_UNSIGNED_IDENTITY,
		0);

	memset(&regular_shader, 0, sizeof(regular_shader));
	regular_shader.PSTextureModes = PS_TEXTUREMODES(
		PS_TEXTUREMODES_PROJECT2D,
		PS_TEXTUREMODES_PROJECT2D,
		PS_TEXTUREMODES_NONE,
		PS_TEXTUREMODES_NONE);
	regular_shader.PSCombinerCount = PS_COMBINERCOUNT(
		1,
		PS_COMBINERCOUNT_MUX_LSB | PS_COMBINERCOUNT_SAME_C0 | PS_COMBINERCOUNT_SAME_C1);
	regular_shader.PSRGBInputs[0] = PS_COMBINERINPUTS(
		PS_REGISTER_T0 | PS_CHANNEL_RGB | PS_INPUTMAPPING_UNSIGNED_IDENTITY,
		PS_REGISTER_V0 | PS_CHANNEL_RGB | PS_INPUTMAPPING_UNSIGNED_IDENTITY,
		PS_REGISTER_ZERO,
		PS_REGISTER_ZERO);
	regular_shader.PSRGBOutputs[0] = PS_COMBINEROUTPUTS(
		PS_REGISTER_R0,
		PS_REGISTER_DISCARD,
		PS_REGISTER_DISCARD,
		PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_CD_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
	regular_shader.PSAlphaInputs[0] = PS_COMBINERINPUTS(
		PS_REGISTER_V0 | PS_CHANNEL_ALPHA | PS_INPUTMAPPING_UNSIGNED_IDENTITY,
		PS_REGISTER_T1 | PS_CHANNEL_ALPHA | PS_INPUTMAPPING_UNSIGNED_IDENTITY,
		PS_REGISTER_ZERO,
		PS_REGISTER_ZERO);
	regular_shader.PSAlphaOutputs[0] = PS_COMBINEROUTPUTS(
		PS_REGISTER_R0,
		PS_REGISTER_DISCARD,
		PS_REGISTER_DISCARD,
		PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_CD_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
	regular_shader.PSFinalCombinerInputsABCD = PS_COMBINERINPUTS(
		PS_REGISTER_ONE,
		PS_REGISTER_R0 | PS_CHANNEL_RGB | PS_INPUTMAPPING_UNSIGNED_IDENTITY,
		PS_REGISTER_ZERO,
		PS_REGISTER_ZERO);
	regular_shader.PSFinalCombinerInputsEFG = PS_COMBINERINPUTS(
		PS_REGISTER_ZERO,
		PS_REGISTER_ZERO,
		PS_REGISTER_R0 | PS_CHANNEL_ALPHA | PS_INPUTMAPPING_UNSIGNED_IDENTITY,
		0);

	format.wfx.wFormatTag = WAVE_FORMAT_XBOX_ADPCM;
	format.wfx.wBitsPerSample = 4;
	format.wfx.nChannels = 1;
	format.wfx.nBlockAlign = 36;
	format.wfx.nSamplesPerSec = 22050;
	format.wfx.nAvgBytesPerSec = 12384;
	format.wfx.cbSize = 2;
	format.wSamplesPerBlock = 64;

	memset(&description, 0, sizeof(description));
	description.dwSize = sizeof(description);
	description.dwFlags = DSBCAPS_LOCDEFER | DSBCAPS_CTRLPOSITIONNOTIFY;
	description.dwBufferBytes = 0;
	description.lpwfxFormat = &format.wfx;

	for (voice_index = 0; voice_index < NUMBER_OF_MONK_VOICES; voice_index++)
	{
		if (FAILED(DirectSoundCreateBuffer(&description, &monk_voices[voice_index])))
		{
			return;
		}

		IDirectSoundBuffer_SetBufferData(monk_voices[voice_index], compressed_monk_tone, sizeof(compressed_monk_tone));
		IDirectSoundBuffer_SetLoopRegion(monk_voices[voice_index], 0, sizeof(compressed_monk_tone));
		IDirectSoundBuffer_SetCurrentPosition(monk_voices[voice_index], 0);
		IDirectSoundBuffer_SetVolume(monk_voices[voice_index], -5000);
		IDirectSoundBuffer_Play(monk_voices[voice_index], 0, 0, DSBPLAY_LOOPING);
	}

	IDirectSoundBuffer_SetPitch(monk_voices[0], -4681);
	IDirectSoundBuffer_SetPitch(monk_voices[1], -6386);
	IDirectSoundBuffer_SetPitch(monk_voices[2], -3999);
	IDirectSoundBuffer_SetPitch(monk_voices[3], -8773);

	if (progress_bar_capture_frame)
	{
		progress_bar_capture_frame = FALSE;
		IDirect3DDevice8_GetDepthStencilSurface(global_d3d_device, &depth_buffer);
		IDirect3DDevice8_GetBackBuffer(global_d3d_device, 0, D3DBACKBUFFER_TYPE_MONO, &back_buffer);
		IDirect3DDevice8_GetBackBuffer(global_d3d_device, -1, D3DBACKBUFFER_TYPE_MONO, &front_buffer);
		IDirect3DDevice8_SetRenderTarget(global_d3d_device, front_buffer, depth_buffer);
		IDirect3DDevice8_Clear(global_d3d_device, 0, NULL, D3DCLEAR_TARGET, 0, 0.f, 0);
		IDirect3DDevice8_SetRenderTarget(global_d3d_device, back_buffer, depth_buffer);
	}

	return;
}

void progress_bar_display(
	real progress)
{
	match_assert("c:\\halo\\SOURCE\\interface\\progress_bar.c", 827, (progress>=0.f) && (progress<=1.f));

	if (progress_bar_active && progress > 0.f)
	{
		if (!progress_bar_is_stuff_ready())
		{
			progress_bar_make_stuff_ready();
			progress_bar_start_progress = progress;
		}

		if (progress_bar_enabled)
		{
			progress_bar_render((progress - progress_bar_start_progress) / (1.f - progress_bar_start_progress));
		}
	}

	return;
}

void to_screen(
	real x,
	real y,
	real z,
	real *screen_x,
	real *screen_y)
{
	D3DXVECTOR4 point;
	D3DXVECTOR4 screen_point;

	point.x = x;
	point.y = y;
	point.z = z;
	point.w = 1.f;
	D3DXVec4Transform(&screen_point, &point, &gravy_projection);
	*screen_x = screen_point.x / screen_point.w;
	*screen_y = screen_point.y / screen_point.w;

	return;
}

void generate_gravy_rect(
	struct gravy_layer const *layer,
	struct gravy_rect *rect)
{
	to_screen(
		layer->x - layer->half_width,
		layer->y - layer->half_height,
		1000.f - layer->distance,
		&rect->x0,
		&rect->y0);
	to_screen(
		layer->x + layer->half_width,
		layer->y + layer->half_height,
		1000.f - layer->distance,
		&rect->x1,
		&rect->y1);
	match_assert("c:\\halo\\SOURCE\\interface\\progress_bar.c", 898, rect->x0 < rect->x1);
	match_assert("c:\\halo\\SOURCE\\interface\\progress_bar.c", 899, rect->y0 < rect->y1);

	return;
}

static void progress_bar_create_noise_texture(
	void)
{
	return;
}

void gen_cloud_coord(
	real x,
	real y,
	real offset)
{
	IDirect3DDevice8_SetVertexData2f(global_d3d_device, D3DVSDE_TEXCOORD1, x * 32.f + offset, y * 20.f);

	return;
}

void gen_mask_coord(
	real x,
	real y,
	real t)
{
	IDirect3DDevice8_SetVertexData2f(
		global_d3d_device,
		D3DVSDE_TEXCOORD1,
		x - (t * 768.f - 64.f - 64.f),
		y * (1.f / 30.f));

	return;
}

static void draw_layer_int(
	struct gravy_layer const *layer,
	real_rgb_color const *color,
	real alpha,
	real t)
{
	volatile struct gravy_rect rect;
	real x0;
	real y0;
	real x1;
	real y1;
	real u = layer->width * .5f;
	real v = (real)(layer->height * .5f);

	generate_gravy_rect(layer, (struct gravy_rect *)&rect);
	IDirect3DDevice8_Begin(global_d3d_device, D3DPT_TRIANGLEFAN);
	IDirect3DDevice8_SetVertexData4f(global_d3d_device, D3DVSDE_DIFFUSE, color->red, color->green, color->blue, alpha);
	gen_mask_coord(0.f, 0.f, t);
	IDirect3DDevice8_SetVertexData2f(global_d3d_device, D3DVSDE_TEXCOORD0, 0.f, v);
	y0 = rect.y0;
	x0 = rect.x0;
	IDirect3DDevice8_SetVertexData4f(global_d3d_device, D3DVSDE_VERTEX, x0, y0, .5f, 1.f);
	gen_mask_coord(RASTERIZER_TARGET_RENDER_PRIMARY_WIDTH, 0.f, t);
	IDirect3DDevice8_SetVertexData2f(global_d3d_device, D3DVSDE_TEXCOORD0, u, v);
	x1 = rect.x1;
	IDirect3DDevice8_SetVertexData4f(global_d3d_device, D3DVSDE_VERTEX, x1, y0, .5f, 1.f);
	gen_mask_coord(RASTERIZER_TARGET_RENDER_PRIMARY_WIDTH, RASTERIZER_TARGET_RENDER_PRIMARY_HEIGHT, t);
	IDirect3DDevice8_SetVertexData2f(global_d3d_device, D3DVSDE_TEXCOORD0, u, 0.f);
	y1 = rect.y1;
	IDirect3DDevice8_SetVertexData4f(global_d3d_device, D3DVSDE_VERTEX, x1, y1, .5f, 1.f);
	gen_mask_coord(0.f, RASTERIZER_TARGET_RENDER_PRIMARY_HEIGHT, t);
	IDirect3DDevice8_SetVertexData2f(global_d3d_device, D3DVSDE_TEXCOORD0, 0.f, 0.f);
	IDirect3DDevice8_SetVertexData4f(global_d3d_device, D3DVSDE_VERTEX, x0, y1, .5f, 1.f);
	IDirect3DDevice8_End(global_d3d_device);

	return;
}

static void draw_layer(
	real distance,
	real alpha,
	real_rgb_color const *color,
	real t)
{
	struct gravy_layer layer;

	layer.width = RASTERIZER_TARGET_RENDER_PRIMARY_WIDTH;
	layer.height = RASTERIZER_TARGET_RENDER_PRIMARY_HEIGHT;
	layer.x = 0.f;
	layer.y = 0.f;
	layer.half_width = (real)fabs(RASTERIZER_TARGET_RENDER_PRIMARY_WIDTH / 2.0);
	layer.half_height = (real)fabs(RASTERIZER_TARGET_RENDER_PRIMARY_HEIGHT / 2.0);
	layer.distance = distance;
	draw_layer_int(&layer, color, alpha, t);

	return;
}

static void draw_initial_layer(
	real_rgb_color const *color,
	real t)
{
	draw_layer(0.f, 1.f, color, t);

	return;
}

// This is a hack used to make a usable texture struct out of a surface.
static void this_is_awful(
	D3DBaseTexture *texture,
	D3DSurface const *surface)
{
	// (NUMBER_OF_TEXTURE_STAGES + 1) = refcount
	texture->Common = D3DCOMMON_TYPE_TEXTURE | (NUMBER_OF_TEXTURE_STAGES + 1);
	texture->Data = surface->Data;
	texture->Lock = 0;
	texture->Size =
		(RASTERIZER_TARGET_RENDER_PRIMARY_WIDTH - 1) |
		((RASTERIZER_TARGET_RENDER_PRIMARY_HEIGHT - 1) << D3DSIZE_HEIGHT_SHIFT) |
		((RASTERIZER_TARGET_RENDER_PRIMARY_WIDTH * sizeof(D3DCOLOR) / D3DTEXTURE_PITCH_ALIGNMENT - 1) << D3DSIZE_PITCH_SHIFT);
	texture->Format =
		D3DFORMAT_DMACHANNEL_A |
		D3DFORMAT_BORDERSOURCE_COLOR |
		(2 << D3DFORMAT_DIMENSION_SHIFT) |
		(D3DFMT_LIN_A8R8G8B8 << D3DFORMAT_FORMAT_SHIFT) |
		(1 << D3DFORMAT_MIPMAP_SHIFT);

	return;
}

static void do_convoluation_coords(
	real x,
	real y)
{
	IDirect3DDevice8_SetVertexData2f(global_d3d_device, D3DVSDE_TEXCOORD0, x - convolution_offset, convolution_offset + y);
	IDirect3DDevice8_SetVertexData2f(global_d3d_device, D3DVSDE_TEXCOORD1, x - convolution_offset, y - convolution_offset);
	IDirect3DDevice8_SetVertexData2f(global_d3d_device, D3DVSDE_TEXCOORD2, convolution_offset + x, y - convolution_offset);
	IDirect3DDevice8_SetVertexData2f(global_d3d_device, D3DVSDE_TEXCOORD3, convolution_offset + x, convolution_offset + y);

	return;
}

static void draw_fade_layer(
	real x_offset,
	real y_offset,
	real alpha)
{
	real x0;
	real y0;
	real x1;
	real y1;

	IDirect3DDevice8_Begin(global_d3d_device, D3DPT_TRIANGLEFAN);
	IDirect3DDevice8_SetVertexData4f(global_d3d_device, D3DVSDE_DIFFUSE, 254.f / 255.f, 254.f / 255.f, 1.f, alpha);
	do_convoluation_coords(3.f, 0.f);
	y0 = (1.f - y_offset) / 128.f + 1.f;
	x0 = (-1.f - x_offset) / 128.f - 1.f;
	IDirect3DDevice8_SetVertexData4f(global_d3d_device, D3DVSDE_VERTEX, x0, y0, .5f, 1.f);
	do_convoluation_coords(637.f, 0.f);
	x1 = (1.f - x_offset) / 128.f + 1.f;
	IDirect3DDevice8_SetVertexData4f(global_d3d_device, D3DVSDE_VERTEX, x1, y0, .5f, 1.f);
	do_convoluation_coords(637.f, RASTERIZER_TARGET_RENDER_PRIMARY_HEIGHT);
	y1 = (-1.f - y_offset) / 128.f - 1.f;
	IDirect3DDevice8_SetVertexData4f(global_d3d_device, D3DVSDE_VERTEX, x1, y1, .5f, 1.f);
	do_convoluation_coords(3.f, RASTERIZER_TARGET_RENDER_PRIMARY_HEIGHT);
	IDirect3DDevice8_SetVertexData4f(global_d3d_device, D3DVSDE_VERTEX, x0, y1, .5f, 1.f);
	IDirect3DDevice8_End(global_d3d_device);

	return;
}

void progress_bar_enable(
	boolean enable)
{
	progress_bar_enabled = enable;

	return;
}
