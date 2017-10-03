#include "stdafx.h"
#include "textures.h"

FastcallFunctionPointer(void, Direct3D_PVRToD3D, (NJS_TEXMEMLIST*, void*), 0x0078CBD0);
DataPointer(NJS_TEXMEMLIST*, CurrentTexMemList, 0x03CE7128);

static std::deque<NJS_TEXMEMLIST> global_textures;

inline void reset(NJS_TEXMEMLIST* memlist)
{
	memlist->globalIndex                      = -1;
	memlist->bank                             = -1;
	memlist->tspparambuffer                   = 0;
	memlist->texparambuffer                   = 0;
	memlist->texaddr                          = 0;
	memlist->count                            = 0;
	memlist->dummy                            = -1;
	memlist->texinfo.texaddr                  = nullptr;
	memlist->texinfo.texsurface.Type          = 0;
	memlist->texinfo.texsurface.BitDepth      = 0;
	memlist->texinfo.texsurface.PixelFormat   = 0;
	memlist->texinfo.texsurface.nWidth        = 0;
	memlist->texinfo.texsurface.nHeight       = 0;
	memlist->texinfo.texsurface.TextureSize   = 0;
	memlist->texinfo.texsurface.fSurfaceFlags = 0;
	memlist->texinfo.texsurface.pSurface      = nullptr;
	memlist->texinfo.texsurface.pVirtual      = nullptr;
	memlist->texinfo.texsurface.pPhysical     = nullptr;
}

static NJS_TEXMEMLIST* __cdecl GetCachedTexture_r(Uint32 gbix)
{
	if (!global_textures.empty())
	{
		// First, check if an entry with this global index already exists and return it.
		for (auto& i : global_textures)
		{
			if (i.globalIndex == gbix)
			{
				return &i;
			}
		}

		// If none, find the first empty slot and return that.
		for (auto& i : global_textures)
		{
			if (!i.count)
			{
				return &i;
			}
		}
	}

	// If all else fails, add a new slot to the texture deque
	NJS_TEXMEMLIST memlist;
	reset(&memlist);
	global_textures.push_back(memlist);

	return &global_textures.back();
}

static Sint32 __cdecl sub_77FA10(Uint32 gbix, void* texture)
{
	auto result = -1;

	for (auto& i : global_textures)
	{
		if (i.globalIndex == gbix)
		{
			DoSomethingWithPalette(*(NJS_TEXPALETTE**)&i.bank);
			Direct3D_PVRToD3D(&i, texture);
			result = 1;
			break;
		}
	}

	return result;
}

static void __cdecl njReleaseTextureAll_r()
{
	for (auto& i : global_textures)
	{
		njReleaseTextureLow(&i);
	}

	global_textures.clear();
}

static Sint32 __fastcall njSetTextureNumG_r(Uint32 gbix)
{
	for (auto& i : global_textures)
	{
		if (i.globalIndex == gbix)
		{
			CurrentTexMemList = &i;
			Direct3D_SetNJSTexture(&i);
			return 1;
		}
	}

	return -1;
}

void Textures_Init()
{
	WriteJump((void*)0x0077FA10, sub_77FA10);
	WriteJump(GetCachedTexture, GetCachedTexture_r);
	WriteJump(njReleaseTextureAll, njReleaseTextureAll_r);
	WriteJump(njSetTextureNumG, njSetTextureNumG_r);
}

void Textures_OnFrame()
{
#ifdef _DEBUG
	DisplayDebugStringFormatted(NJM_LOCATION(1, 2), "TEXTURES: %u", global_textures.size());
#endif
}
