#include "stdafx.h"
#include <SADXModLoader.h>
#include <deque>
#include "textures.h"

FastcallFunctionPointer(void, Direct3D_PVRToD3D, (NJS_TEXMEMLIST*, void*), 0x0078CBD0);
DataPointer(NJS_TEXMEMLIST*, CurrentTexMemList, 0x03CE7128);

static std::deque<NJS_TEXMEMLIST> GlobalTextures;

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

static NJS_TEXMEMLIST* __cdecl _GetCachedTexture(Uint32 gbix)
{
	if (GlobalTextures.size())
	{
		for (auto& i : GlobalTextures)
		{
			if (i.globalIndex == gbix)
			{
				//PrintDebug("/!\\\tCACHE SLOT FOR %u\n", gbix);
				return &i;
			}
		}

		for (auto& i : GlobalTextures)
		{
			if (!i.count)
			{
				//PrintDebug("/!\\\tFREE SLOT FOR %u\n", gbix);
				return &i;
			}
		}
	}

	NJS_TEXMEMLIST memlist;
	reset(&memlist);
	GlobalTextures.push_back(memlist);

	//PrintDebug("/!\\\tNEW SLOT FOR %u, %u COUNT\n", gbix, GlobalTextures.size());
	return &GlobalTextures.back();
}

static Sint32 __cdecl sub_77FA10(Uint32 gbix, void* texture)
{
	auto result = -1;

	for (auto& i : GlobalTextures)
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

static void __cdecl _njReleaseTextureAll()
{
	for (auto& i : GlobalTextures)
		njReleaseTextureLow(&i);

	GlobalTextures.clear();
}

Sint32 __cdecl _njSetTextureNumG(Uint32 gbix, void* asdf)
{
	for (auto& i : GlobalTextures)
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

void __declspec(naked) _njSetTextureNumG_asm()
{
	__asm
	{
		push ecx
		call _njSetTextureNumG
		pop ecx
		retn
	}
}

void Textures_Init()
{
	WriteJump((void*)0x0077FA10, sub_77FA10);
	WriteJump(GetCachedTexture, _GetCachedTexture);
	WriteJump(njReleaseTextureAll, _njReleaseTextureAll);
	WriteJump(njSetTextureNumG, _njSetTextureNumG_asm);
}

void Textures_OnFrame()
{
	DisplayDebugStringFormatted(NJM_LOCATION(1, 2), "TEXTURES: %u", GlobalTextures.size());
}
