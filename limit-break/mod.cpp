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
DataArray(ObjectFuncPtr, SkyboxObjects, 0x0090C1F0, 44);

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

extern "C"
{
	__declspec(dllexport) ModInfo SADXModInfo = { ModLoaderVer, nullptr, nullptr, 0, nullptr, 0, nullptr, 0, nullptr, 0 };

	__declspec(dllexport) void __cdecl Init()
	{
		WriteCall(reinterpret_cast<void*>(0x00415A60), InitSpriteTable_r);
		WriteJump(LoadSkyboxObject, LoadSkyboxObject_r);

		object_init();
		collision_init();
		clip_init();
		textures_init();
		set_init();
		objdef_init();
	}

	__declspec(dllexport) void __cdecl OnFrame()
	{
#ifdef _DEBUG
		DisplayDebugStringFormatted(NJM_LOCATION(1, 5), "CLIP: %f", clip_current);
#endif
		object_OnFrame();
		textures_OnFrame();
	}
}
