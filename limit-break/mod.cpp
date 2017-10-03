#include "stdafx.h"

static short last_level = 0;
static short last_act = 0;

static const Uint32 sprite_count = 4096;
static Uint8 table[80 * sprite_count];

#ifdef _DEBUG
DataPointer(int, Display_SPR_TASK, 0x03B28118);
#endif

static void __cdecl InitSpriteTable_r(void*, Uint32)
{
	InitSpriteTable((QueuedModelParticle*)table, sprite_count);

#ifdef _DEBUG
	Display_SPR_TASK = 1;
#endif
}

extern "C"
{
	__declspec(dllexport) ModInfo SADXModInfo = { ModLoaderVer };

	__declspec(dllexport) void __cdecl Init()
	{
		WriteCall((void*)0x00415A60, InitSpriteTable_r);

		Object_Init();
		Collision_Init();
		Clip_Init();
		Textures_Init();
		Set_Init();
	}

	__declspec(dllexport) void __cdecl OnFrame()
	{
		const auto pad = ControllerPointers[0];

		if (last_level != CurrentLevel || last_act != CurrentAct
			|| pad && pad->PressedButtons & Buttons_C)
		{
			last_level = CurrentLevel;
			last_act = CurrentAct;
			Clip_Reset();
		}

		DisplayDebugStringFormatted(NJM_LOCATION(1, 5), "CLIP: %f", clip_current);

		Object_OnFrame();
		Textures_OnFrame();
	}
}
