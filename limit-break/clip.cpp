#include "stdafx.h"
#include <Trampoline.h>
#include "clip.h"

const float clip_default = 168100.0f;

float clip_current = clip_default;
float clip_limit   = 0.0f; // this is written to externally
float clip_min     = FLT_MAX;
float clip_max     = 0.0f;

static int __cdecl ClipObject_r(ObjectMaster* a1, float dist);
static Trampoline ClipObject_t(0x0046C010, 0x0046C018, ClipObject_r);

int __cdecl clip_object(ObjectMaster* a1, float dist)
{
	const auto set = a1->SETData.SETData;

	if (set && set->Flags & 8 || std::abs(dist) < std::numeric_limits<float>::epsilon())
	{
		return 0;
	}

	const auto entity = a1->Data1;
	const auto y      = entity->Position.y;
	const auto z      = entity->Position.z;
	const auto x      = entity->Position.x;

	/*
	// this is the original behavior which is hard-coded to the first two player entities;
	// using the for loop below opens up support for sadx-multitap, sadx-online, etc
	if (Camera_Data1 && ObjectInRange(&Camera_Data1->Position, entity->Position.x, y, entity->Position.z, dist)
	    || EntityData1Ptrs[0] && ObjectInRange(&EntityData1Ptrs[0]->Position, x, y, z, dist)
	    || EntityData1Ptrs[1]
	    && !EntityData1Ptrs[1]->CharIndex.SByte[1]
	    && ObjectInRange(&EntityData1Ptrs[1]->Position, x, y, z, 1600.0))
	{
		return 0;
	}
	 */

	if (Camera_Data1 && ObjectInRange(&Camera_Data1->Position, x, y, z, dist))
	{
		return 0;
	}

	for (size_t i = 0; i < EntityData1Ptrs_Length; i++)
	{
		auto player = EntityData1Ptrs[i];

		if (player == nullptr)
		{
			continue;
		}

		if (ObjectInRange(&player->Position, x, y, z, dist))
		{
			return 0;
		}
	}

	a1->MainSub = DeleteObjectMaster;
	return 1;
}

static void set_clip(float r)
{
	if (r < clip_min && r > clip_default)
	{
		clip_min = r;
	}

	if (r > clip_max)
	{
		clip_max = r;
	}

	if (clip_limit == 0.0f)
	{
		return;
	}

	// If the value exceeds the limit, use the limit as the current distance.
	clip_current = clip_limit;
}

static int do_clip(ObjectMaster* a1, float distance = clip_default)
{
	const auto set = a1->SETData.SETData;

	if (set)
	{
		set_clip(set->Distance);
	}

	if (!(ControllerPointers[0]->HeldButtons & Buttons_Z))
	{
		return clip_object(a1, set != nullptr ? std::max(clip_current, set->Distance) : clip_current);
	}

	if (set)
	{
		return clip_object(a1, set->Distance);
	}

	return clip_object(a1, distance);
}

static int __cdecl ClipObject_r(ObjectMaster* a1, float dist)
{
	return do_clip(a1);
}

static int __cdecl ClipSetObject_r(ObjectMaster* a1)
{
	return do_clip(a1);
}

// ReSharper disable once CppDeclaratorNeverUsed
static int __cdecl ObjectInRange_r(NJS_VECTOR* from, float x, float y, float z, float range)
{
	set_clip(range);
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
	// HACK: this halving here "fixes" (covers up) the platforms in red mountain disappearing, etc
	const auto f = std::min(-LevelDrawDistance.Maximum, -SkyboxDrawDistance.Maximum) * 0.5f;

	clip_current = f * f;
	clip_limit   = limit;
	clip_min     = clip_current;
	clip_max     = 0.0f;
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
