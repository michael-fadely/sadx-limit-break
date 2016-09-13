#include "stdafx.h"
#include <SADXModLoader.h>
#include "clip.h"

enum class ClipType
{
	Clip, Draw
};

const float clip_default = 168100.0f;

float clip_current = clip_default;
float clip_max     = 0.0f; // this is written to externally

static void __stdcall set_clip(float r, ClipType type)
{
	if ((clip_max == 0.0f || r < clip_max) && r > clip_current)
	{
		if (type == ClipType::Clip)
		{
			PrintDebug("CLIP: %f -> %f\n", clip_current, r);
		}
		else if (type == ClipType::Draw)
		{
			PrintDebug("DRAW: %f -> %f\n", clip_current, r);
		}

		clip_current = r;
		return;
	}

	if (clip_max == 0.0f)
	{
		return;
	}

	clip_current = clip_max;
}

static int __cdecl ClipSetObject_r(ObjectMaster* a1)
{
	auto set = a1->SETData;

	if (set)
	{
		set_clip(set->Distance, ClipType::Clip);
	}

	if (!(ControllerPointers[0]->HeldButtons & Buttons_Z))
	{
		return ClipObject(a1, set != nullptr ? max(clip_current, set->Distance) : clip_current);
	}

	if (set)
	{
		return ClipObject(a1, set->Distance);
	}

	return ClipObject(a1, clip_default);
}

static int __cdecl ObjectInRange_r(NJS_VECTOR* v, float r, float z, float y, float x /*float x, float y, float z, float r*/)
{
	set_clip(r, ClipType::Draw);
	return ObjectInRange(v, x, y, z, (ControllerPointers[0]->HeldButtons & Buttons_Z) ? r : max(r, clip_current));
}

static void __declspec(naked) ObjectInRange_asm()
{
	__asm
	{
		push[esp + 04h] // range
		push[esp + 0Ch] // z
		push[esp + 14h] // y
		push[esp + 1Ch] // x
		push ecx // from
		call ObjectInRange_r
		pop ecx // from
		add esp, 4 // x
		add esp, 4 // y
		add esp, 4 // z
		add esp, 4 // range
		retn
	}
}

void Clip_Init()
{
	WriteJump((void*)ClipSetObject, ClipSetObject_r);
	WriteJump((void*)0x0046C390, ClipSetObject_r);
	WriteCall((void*)0x0046BBB9, ObjectInRange_asm);
	WriteData((float**)0x0046B6F8, &clip_current);
	WriteData((float**)0x0046B713, &clip_current);
	WriteData((float**)0x0046B72D, &clip_current);
}
