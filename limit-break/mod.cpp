#include "stdafx.h"

static const Uint32 SPRITE_COUNT = 4096;
static Uint8 table[80 * SPRITE_COUNT];

static void __cdecl InitSpriteTable_r(void*, Uint32)
{
	InitSpriteTable(reinterpret_cast<QueuedModelParticle*>(table), SPRITE_COUNT);

#ifdef _DEBUG
	Display_SPR_TASK = 1;
#endif
}

static ObjectFuncPtr skybox_mainsub = nullptr;

DataArray(Rotation3, stru_90BFE8, 0x90BFE8, 0);

static void __cdecl clip_mainsub(ObjectMaster* _this)
{
	skybox_mainsub(_this);

	if (_this->MainSub != skybox_mainsub && _this->MainSub != clip_mainsub)
	{
		skybox_mainsub = _this->MainSub;
		_this->MainSub = clip_mainsub;
		return;
	}

	clip_reset();
}

void __cdecl LoadSkyboxObject_r()
{
	SetGlobalPoint2Col_Colors(stru_90BFE8[CurrentLevel].x,
	                          stru_90BFE8[CurrentLevel].y,
	                          stru_90BFE8[CurrentLevel].z);

	if (SkyboxObjects[CurrentLevel])
	{
		skybox_mainsub = SkyboxObjects[CurrentLevel];
		LoadObject(LoadObj_Data1, 1, clip_mainsub);
	}
}

static constexpr auto LATEALLOCA_SIZE = 1024 * 1024;

void __cdecl late_alloca_init_r()
{
	if (late_alloca_buffer)
	{
		delete[] late_alloca_buffer;
	}

	late_alloca_size = LATEALLOCA_SIZE;
	late_alloca_buffer = new uint8_t[LATEALLOCA_SIZE];
	late_alloca_end = late_alloca_buffer + LATEALLOCA_SIZE;

	memcpy(reinterpret_cast<void*>(0x90084C), reinterpret_cast<void*>(0x900858), sizeof(NJS_VECTOR));
	late_alloca_reset();
}

extern "C"
{
	__declspec(dllexport) ModInfo SADXModInfo = { ModLoaderVer, nullptr, nullptr, 0, nullptr, 0, nullptr, 0, nullptr, 0 };

	__declspec(dllexport) void __cdecl Init()
	{
		WriteCall(reinterpret_cast<void*>(0x00415A60), InitSpriteTable_r);
		WriteJump(LoadSkyboxObject, LoadSkyboxObject_r);
		WriteJump(late_alloca_init, late_alloca_init_r);

		dynacol_init();
		object_init();
		collision_init();
		clip_init();
		textures_init();
		set_init();

		// Bridge in EC01
		WriteData((float**)(0x00501D3A + 2), &clip_current);
		WriteData((float**)(0x00501D53 + 2), &clip_current);
		WriteData((float**)(0x00501D90 + 2), &clip_current);
	}

	__declspec(dllexport) void __cdecl OnFrame()
	{
		DisplayDebugStringFormatted(NJM_LOCATION(1, 5), "CLIP: %f", clip_current);
		object_OnFrame();
		textures_OnFrame();
	}
}
