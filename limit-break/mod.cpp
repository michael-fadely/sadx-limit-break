#include "stdafx.h"
#include <SADXModLoader.h>
#include <thread>
#include "clip.h"
#include "collision.h"
#include "misc.h"
#include "textures.h"

static short last_level = 0;
static short last_act   = 0;

static Uint32 object_count = 0;

static const Uint32 points_length   = 60;
static Uint32 points[points_length] = {};
static Uint32 points_i              = 0;
static Uint32 object_average        = 0;

DataArray(ObjectMaster*, ObjectListThing, 0x03ABDBC4, 8);
DataPointer(ObjectMaster*, MasterObjectArray, 0x03ABDBEC);

static ObjectMaster* __cdecl AllocateObjectMaster_r(int index, ObjectFuncPtr LoadSub)
{
	if (index < 0 || index > 8)
		return nullptr;

	++object_count;
	auto result = new ObjectMaster{};
	result->MainSub = LoadSub;

	if (index != 7)
	{
		if (index > 7)
			index = 7;

		auto last = ObjectListThing[index];
		bool static_object = last >= MasterObjectArray && last <= &MasterObjectArray[4095];

		if (static_object)
		{
			PrintDebug("\a/!\\\tSTATIC OBJECT DETECTED\n");
			last = nullptr;
		}

		if (last)
		{
			last->Previous = result;
			result->Next = last;
		}

		ObjectListThing[index] = result;
	}

	return result;
}

static void __declspec(naked) AllocateObjectMaster_asm()
{
	__asm
	{
		push edi
		push edx
		call AllocateObjectMaster_r
		pop edx
		pop edi
		retn
	}
}

static void __cdecl DeleteObjectMaster_r(ObjectMaster *_this)
{
	if (!NaiZoGola(_this))
		return;

	--object_count;

	if (_this->DeleteSub)
		_this->DeleteSub(_this);

	auto previous     = _this->Previous;
	auto next         = _this->Next;
	_this->MainSub    = nullptr;
	_this->DisplaySub = nullptr;
	_this->DeleteSub  = nullptr;

	if (_this->Child != nullptr)
		DeleteChildObjects(_this);

	if (!next)
	{
		if (previous)
		{
			previous->Next = nullptr;
			goto FREE_DATA;
		}

	LABEL_11:
		if (_this->Parent)
		{
			if (next)
				next->Previous = nullptr;

			_this->Parent->Child = next;
		}
		else
		{
			if (next)
				next->Previous = nullptr;

			Uint32 i = 0;
			while (_this != ObjectListThing[i])
			{
				++i;
				if (i >= 8)
				{
					goto FREE_DATA;
				}
			}

			ObjectListThing[i] = next;
		}

		goto FREE_DATA;
	}

	if (!previous)
		goto LABEL_11;

	next->Previous = previous;
	previous->Next = next;

FREE_DATA:
	auto data1 = _this->Data1;
	if (data1)
	{
		if (data1->CollisionInfo)
		{
			FreeEntityCollision(_this->Data1);
		}
		if (*(DWORD *)&_this->Data1->field_3C)
		{
			FreeWhateverField3CIs(_this->Data1);
		}
		FreeMemory(_this->Data1);
		_this->Data1 = nullptr;
	}

	if (_this->UnknownA_ptr)
	{
		FreeMemory(_this->UnknownA_ptr);
		_this->UnknownA_ptr = nullptr;
	}

	if (_this->Data2)
	{
		FreeMemory(_this->Data2);
		_this->Data2 = nullptr;
	}

	if (_this->UnknownB_ptr)
	{
		FreeMemory(_this->UnknownB_ptr);
		_this->UnknownB_ptr = nullptr;
	}

	auto set = _this->SETData;
	if (set)
	{
		set->dword4 = 0;
		_LOBYTE(set->Flags) &= 0xFEu;
		_this->SETData = nullptr;
	}

	delete _this;
}

static const Uint32 sprite_count = 3072;
DataPointer(int, Display_SPR_TASK, 0x03B28118);
static Uint8 table[80 * sprite_count];

static void __cdecl InitSpriteTable_r(void*, Uint32)
{
	InitSpriteTable(table, sprite_count);
	Display_SPR_TASK = 1;
}

// TODO: average
using namespace std;
using namespace chrono;
using FrameRatio = duration<long, ratio<1, 60>>;

static auto frame_start = system_clock::now();
static auto frame_ratio = FrameRatio{ 1 };
static auto frame_max   = 0.0f;
static auto frame_min   = FLT_MAX;
static int last_multi   = 0;

static void __cdecl SetFrameMultiplier(int a1)
{
	if (a1 != last_multi)
	{
		last_multi = a1;
		frame_ratio = FrameRatio{ a1 };
	}
}

static void __cdecl CustomDeltaSleep()
{
	while (system_clock::now() - frame_start < frame_ratio)
		this_thread::yield();

	auto now = system_clock::now();
	duration<float, milli> dur = now - frame_start;
	frame_start = now;

	auto frame_time = dur.count();

	if (ControllerPointers[0] && ControllerPointers[0]->PressedButtons & Buttons_C)
	{
		frame_max = 0.0f;
		frame_min = FLT_MAX;
	}
	else
	{
		if (frame_time > frame_max)
			frame_max = frame_time;
		if (frame_time < frame_min)
			frame_min = frame_time;
	}

	DisplayDebugStringFormatted(NJM_LOCATION(1, 11), "FRAME TIME NOW: %f", frame_time);
	DisplayDebugStringFormatted(NJM_LOCATION(1, 12), "FRAME TIME MIN: %f", frame_min);
	DisplayDebugStringFormatted(NJM_LOCATION(1, 13), "FRAME TIME MAX: %f", frame_max);

	// TODO: this needs a threshold
	if (DemoPlaying || dur <= frame_ratio)
		return;

	DisplayDebugStringFormatted(NJM_LOCATION(1, 10), "REDUCING");

	if (clip_max == 0.0f)
		clip_max = clip_current;

	if (clip_max > clip_default)
	{
		clip_max -= clip_default;
		if (clip_max < clip_default)
			clip_max = clip_default;
	}
	else
	{
		clip_max = 0.0f;
		clip_current = clip_default;
	}
}
extern "C"
{
	__declspec(dllexport) ModInfo SADXModInfo = { ModLoaderVer };

	__declspec(dllexport) void __cdecl Init()
	{
		// Custom frame limiter
		WriteJump((void*)0x007899E0, CustomDeltaSleep);
		WriteJump((void*)0x007899A0, SetFrameMultiplier);

		WriteJump((void*)AllocateObjectMasterPtr, AllocateObjectMaster_asm);
		WriteJump(DeleteObjectMaster, DeleteObjectMaster_r);
		WriteCall((void*)0x00415A60, InitSpriteTable_r);
		WriteCall((void*)0x0040B011, PrintDebug);
		Collision_Init();
		Clip_Init();
		Textures_Init();
	}

	__declspec(dllexport) void __cdecl OnFrame()
	{
		if (last_level != CurrentLevel || last_act != CurrentAct)
		{
			last_level   = CurrentLevel;
			last_act     = CurrentAct;
			clip_current = clip_default;
			clip_max     = 0.0f;
		}

		if (GameState == 15)
		{
			Uint32 p = points_i;
			points[p] = object_count;

			if (++points_i >= points_length)
			{
				float result = 0.0f;

				for (Uint32 i = 0; i < p; i++)
					result += points[i];

				object_average = (Uint32)ceil(result / (float)points_length);
			}
		}

		points_i %= points_length;

		DisplayDebugStringFormatted(NJM_LOCATION(1, 4), "COUNT: REAL/AVG: %03u / %03u", object_count, object_average);
		DisplayDebugStringFormatted(NJM_LOCATION(1, 5), "CLIP: %f", clip_current);
		Textures_OnFrame();
	}
}
