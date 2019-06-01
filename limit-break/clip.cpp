#include "stdafx.h"
#include "clip.h"

enum class ClipType
{
	Clip, Draw
};

const float clip_default = 168100.0f;

float clip_current = clip_default;
float clip_limit   = 0.0f; // this is written to externally
float clip_min     = FLT_MAX;
float clip_max     = 0.0f;

static void set_clip(float r, ClipType type)
{
	if (r < clip_min && r > clip_default)
	{
		clip_min = r;
	}

	if (r > clip_max)
	{
		clip_max = r;
	}

	// If the upper limit is 0, or if the provided distance is less than the limit,
	// and if it exceeds the current clip distance, update it.
	/*if ((clip_limit == 0.0f || r <= clip_limit) && r > clip_current)
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
	}*/

	// Otherwise if the clip limit is 0, just abort
	if (clip_limit == 0.0f)
	{
		return;
	}

	// If the value exceeds the limit, use the limit as the current distance.
	clip_current = clip_limit;
}

static int __cdecl ClipSetObject_r(ObjectMaster* a1)
{
	auto set = a1->SETData.SETData;

	if (set)
	{
		set_clip(set->Distance, ClipType::Clip);
	}

	if (!(ControllerPointers[0]->HeldButtons & Buttons_Z))
	{
		return ClipObject(a1, set != nullptr ? std::max(clip_current, set->Distance) : clip_current);
	}

	if (set)
	{
		return ClipObject(a1, set->Distance);
	}

	return ClipObject(a1, clip_default);
}

// ReSharper disable once CppDeclaratorNeverUsed
static int __cdecl ObjectInRange_r(NJS_VECTOR* from, float x, float y, float z, float range)
{
	set_clip(range, ClipType::Draw);
	return ObjectInRange(from, x, y, z, (ControllerPointers[0]->HeldButtons & Buttons_Z) ? range : std::max(range, clip_current));
}

static void __declspec(naked) ObjectInRange_asm()
{
	__asm
	{
		push [esp + 10h] // range
		push [esp + 10h] // z
		push [esp + 10h] // y
		push [esp + 10h] // x
		push ecx // from

		call ObjectInRange_r

		pop ecx // from
		add esp, 10h
		retn
	}
}

void clip_init()
{
	WriteJump(ClipSetObject, ClipSetObject_r);
	WriteJump(ClipSetObject_Min, ClipSetObject_r);
	WriteCall((void*)0x0046BBB9, ObjectInRange_asm);
	WriteData((float**)0x0046B6F8, &clip_current);
	WriteData((float**)0x0046B713, &clip_current);
	WriteData((float**)0x0046B72D, &clip_current);
}

void clip_reset(float limit)
{
	// LevelDrawDistance
	const auto f = -*(float*)0x03ABDC74 * 0.5f;

	clip_current = f * f;
	clip_limit = limit;
	clip_min = clip_current;
	clip_max = 0.0f;
}

bool clip_increase(float inc)
{
	if (inc <= 0.0f || clip_limit >= clip_max)
	{
		return false;
	}

	clip_limit = std::min(clip_limit + inc, clip_max);
	return true;
}

bool clip_decrease(float dec)
{
	if (dec <= 0.0f)
	{
		return false;
	}

	bool result;

	if (clip_limit > clip_min)
	{
		clip_limit -= dec;

		if (clip_limit < clip_min)
		{
			clip_limit = clip_min;
		}

		result = true;
	}
	else
	{
		clip_limit = clip_default;
		result = false;
	}

	clip_current = std::min(clip_current, clip_limit);
	return result;
}
