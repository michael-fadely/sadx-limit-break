#include "stdafx.h"
#include <SADXModLoader.h>
#include "clip.h"

enum class ClipType
{
	Clip, Draw
};

const float clip_default = 168100.0f;

float clip_current = clip_default;
float clip_max     = 0.0f;

static void set_clip(float r, ClipType type)
{
	if ((clip_max == 0.0f || r < clip_max) && r > clip_current)
	{
		if (type == ClipType::Clip)
			PrintDebug("CLIP: %f -> %f\n", clip_current, r);
		else if (type == ClipType::Draw)
			PrintDebug("DRAW: %f -> %f\n", clip_current, r);

		clip_current = r;
		return;
	}

	if (clip_max == 0.0f)
		return;

	clip_current = clip_max;
}

static int __cdecl ClipSetObject_(ObjectMaster* a1)
{
	int result;
	auto v1 = a1->SETData;

	if (v1)
		set_clip(v1->Distance, ClipType::Clip);

	if (!(ControllerPointers[0]->HeldButtons & Buttons_Z))
	{
		result = ClipObject(a1, (v1 && v1->Distance > clip_current) ? v1->Distance : clip_current);
	}
	else if (v1)
	{
		result = ClipObject(a1, v1->Distance);
	}
	else
	{
		result = ClipObject(a1, clip_default);
	}

	return result;
}

static int __cdecl _ObjectInRangeHax(NJS_VECTOR* v, float r, float z, float y, float x /*float x, float y, float z, float r*/)
{
	set_clip(r, ClipType::Draw);
	return ObjectInRange(v, x, y, z, (ControllerPointers[0]->HeldButtons & Buttons_Z) ? r : (r > clip_current ? r : clip_current));
}

static void __declspec(naked) ObjectInRangeHax()
{
	__asm
	{
		push[esp + 04h] // range
		push[esp + 0Ch] // z
		push[esp + 14h] // y
		push[esp + 1Ch] // x
		push ecx // from
		call _ObjectInRangeHax
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
	WriteJump((void*)0x0046C360, ClipSetObject_);
	WriteJump((void*)0x0046C390, ClipSetObject_);
	WriteCall((void*)0x0046BBB9, ObjectInRangeHax);
	WriteData((float**)0x0046B6F8, &clip_current);
	WriteData((float**)0x0046B713, &clip_current);
	WriteData((float**)0x0046B72D, &clip_current);
}
