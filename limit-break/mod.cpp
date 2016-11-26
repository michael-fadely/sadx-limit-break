#include "stdafx.h"
#include <SADXModLoader.h>
#include <thread>
#include "clip.h"
#include "collision.h"
#include "misc.h"
#include "textures.h"
#include "../sadx-mod-loader/libmodutils/Trampoline.h"

using namespace std;
using namespace chrono;

using FrameRatio = duration<double, ratio<1, 60>>;

static auto last_level  = (short)0;
static auto last_act    = (short)0;
static auto last_multi  = (Uint32)0;
static auto dec_time    = (Uint32)0;
static auto frame_start = system_clock::now();
static auto frame_ratio = FrameRatio(1);
static auto frame_dur   = 0.0;
static auto frame_max   = 0.0;
static auto frame_min   = DBL_MAX;
static auto perf_inc    = 1.75f; // 105 FPS - Draw distance increase threshold
static auto perf_dec    = 1.30f; //  78 FPS - Draw distance decrease threshold

static const Uint32 points_length   = 60;
static Uint32 points[points_length] = {};
static Uint32 points_i              = 0;
static Uint32 object_average        = 0;

static const Uint32 sprite_count = 4096;
static Uint8 table[80 * sprite_count];

static Uint32 object_count = 0;
static deque<ObjectMaster> MasterObjectArray_r = {};
static bool queue_initialized = false;

VoidFunc(sub_51A740, 0x51A740);

DataArray(ObjectMaster*, ObjectListThing, 0x03ABDBC4, 8);
DataPointer(ObjectMaster*, MasterObjectArray, 0x03ABDBEC);
DataPointer(ObjectMaster*, MasterObjectArray_Base, 0x03ABDBE4);
DataPointer(ObjectMaster*, CurrentObject, 0x03ABDBF8);
DataPointer(int, Display_SPR_TASK, 0x03B28118);

int inline FadeColor(Uint32 color_to, Uint32 color_from, float m)
{
	NJS_COLOR from = { color_from };
	NJS_COLOR to   = { color_to };
	NJS_COLOR result;

	result.argb.a = from.argb.a + (Uint8)((to.argb.a - from.argb.a) * m);
	result.argb.r = from.argb.r + (Uint8)((to.argb.r - from.argb.r) * m);
	result.argb.g = from.argb.g + (Uint8)((to.argb.g - from.argb.g) * m);
	result.argb.b = from.argb.b + (Uint8)((to.argb.b - from.argb.b) * m);

	return result.color;
}

static ObjectMaster* __cdecl AllocateObjectMaster_r(int index, ObjectFuncPtr LoadSub)
{
	if (index < 0 || index > 8)
		return nullptr;

	auto result = MasterObjectArray;

	if (++object_count == MasterObjectArray_r.size())
	{
		MasterObjectArray_r.push_back({});
		result->Next = &MasterObjectArray_r.back();
	}

	auto next = result->Next;
	if (next)
	{
		next->Previous = nullptr;
	}
	MasterObjectArray = next;

	result->MainSub = LoadSub ? LoadSub : nullsub;

	if (index != 7)
	{
		if (index == 8)
		{
			index = 7;
		}

		auto last = ObjectListThing[index];
		if (last)
		{
			result->Previous = nullptr;
			result->Next     = last;
			last->Previous   = result;

			ObjectListThing[index] = result;
		}
		else
		{
			ObjectListThing[index] = result;

			result->Next     = nullptr;
			result->Previous = nullptr;
		}
	}

	result->SETData      = nullptr;
	result->Data1        = nullptr;
	result->Data2        = nullptr;
	result->UnknownA_ptr = nullptr;
	result->UnknownB_ptr = nullptr;
	result->DisplaySub   = nullptr;
	result->DeleteSub    = nullptr;
	result->Child        = nullptr;
	result->Parent       = nullptr;
	result->field_30     = 0;

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

static void __cdecl InitObjectQueue()
{
	MasterObjectArray_r.push_back({});

	auto ptr = &MasterObjectArray_r.back();

	MasterObjectArray_Base = ptr;
	MasterObjectArray      = ptr;
	CurrentObject          = nullptr;
}

static void __cdecl InitMasterObjectArray_r()
{
	sub_51A740();

	if (!queue_initialized)
	{
		InitObjectQueue();
		queue_initialized = true;
	}

	for (auto i = 0; i < ObjectListThing_Length; i++)
		ObjectListThing[i] = nullptr;
}

static void __cdecl DeleteObjectMaster_r(ObjectMaster *_this)
{
	if (!NaiZoGola(_this))
	{
		return;
	}

	if (_this->DeleteSub)
	{
		_this->DeleteSub(_this);
	}

	--object_count;

	auto previous = _this->Previous;
	auto next     = _this->Next;

	_this->MainSub    = nullptr;
	_this->DisplaySub = nullptr;
	_this->DeleteSub  = nullptr;

	if (_this->Child != nullptr)
	{
		DeleteChildObjects(_this);
	}

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
			{
				next->Previous = nullptr;
			}

			_this->Parent->Child = next;
		}
		else
		{
			if (next)
			{
				next->Previous = nullptr;
			}

			auto i = 0;
			while (_this != ObjectListThing[i])
			{
				++i;
				if (i >= ObjectListThing_Length)
				{
					goto FREE_DATA;
				}
			}

			ObjectListThing[i] = next;
		}
		goto FREE_DATA;
	}

	if (!previous)
	{
		goto LABEL_11;
	}

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
		_LOBYTE(_this->SETData->Flags) &= 0xFEu;
		_this->SETData = nullptr;
	}

	auto master = MasterObjectArray;
	MasterObjectArray = _this;

	if (master == nullptr)
	{
		_this->Next     = nullptr;
		_this->Previous = nullptr;
		_this->Child    = nullptr;
		_this->Parent   = nullptr;
	}
	else
	{
		master->Previous = _this;
		_this->Previous  = nullptr;
		_this->Child     = nullptr;
		_this->Parent    = nullptr;
		_this->Next      = master;
	}

	// Clear the object queue if there are no objects checked out
	if (!object_count)
	{
		MasterObjectArray_r.clear();
		InitObjectQueue();
	}
}

static void __cdecl InitSpriteTable_r(void*, Uint32)
{
	InitSpriteTable(table, sprite_count);
#ifdef _DEBUG
	Display_SPR_TASK = 1;
#endif
}

static void __cdecl SetFrameMultiplier(int a1)
{
	if (a1 != last_multi)
	{
		last_multi = a1;
		frame_ratio = FrameRatio(a1);
		duration<double, milli> temp = frame_ratio;
		frame_dur = temp.count();

		frame_max = 0.0f;
		frame_min = FLT_MAX;
	}
}

static Average<float> perf_average(15);
static Average<float> frame_average(60);
static duration<double, milli> present = {};

static const auto curve_max = 100'000.0f; // 168'100.0f is default
static auto curve           = 1.0f;
static auto curve_multi     = 1.0f;
static auto delta           = 1.0;

inline void curve_reset()
{
	curve = 1.0f;
}

inline void curve_inc(float multiplier = 1.0f)
{
	curve_multi = max(0.1f, multiplier);
	auto _max = (curve_max * curve_multi) * (float)delta;

	if (curve < _max)
	{
		curve *= max(1.0f, 1.25f * curve_multi) * (float)delta;
	}

	if (curve > _max)
	{
		curve = _max;
	}

	if (curve < 1.0f)
		curve_reset();
}

static void __cdecl Direct3D_Present_r();
static Trampoline Direct3D_Present_trampoline(0x0078BA30, 0x0078BA35, Direct3D_Present_r);
static void __cdecl Direct3D_Present_r()
{
	VoidFunc(original, Direct3D_Present_trampoline.Target());
	auto start = system_clock::now();
	original();
	present = system_clock::now() - start;
}

static bool enableFrameLimit = true;
static void __cdecl CustomDeltaSleep()
{
	auto now = system_clock::now();

	duration<double, milli> dur = now - frame_start - present;
	auto perf_ratio = frame_dur / dur.count();

	if (enableFrameLimit && present <= frame_ratio)
	{
		while ((now = system_clock::now()) - frame_start < frame_ratio)
		{
		}
	}

	now = system_clock::now();
	dur = now - frame_start;
	delta = dur.count() / frame_dur;
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

	auto last = DebugFontColor;
	SetDebugFontColor(FadeColor(0xFF00FF00, 0xFFFF0000, (float)min(perf_ratio, 1.0)));
	DisplayDebugStringFormatted(NJM_LOCATION(1, 11), "FRAME TIME NOW: %07.4f", frame_time);
	DisplayDebugStringFormatted(NJM_LOCATION(1, 12), "FRAME TIME MIN: %07.4f", frame_min);
	DisplayDebugStringFormatted(NJM_LOCATION(1, 13), "FRAME TIME MAX: %07.4f", frame_max);
	DisplayDebugStringFormatted(NJM_LOCATION(1, 15), "FRAME PERF NOW: %07.4f", perf_ratio);

	if (frame_average.add_point((float)frame_time))
	{
		auto avg = frame_average.get_average();
		SetDebugFontColor(FadeColor(0xFF00FF00, 0xFFFF0000, (float)min(frame_dur / avg, 1.0)));
		DisplayDebugStringFormatted(NJM_LOCATION(1, 14), "FRAME TIME AVG: %07.4f (%05.2f FPS)", avg, 1000.0 / avg);
	}

	SetDebugFontColor(last);

	if (GameState == 21 || !perf_average.add_point((float)perf_ratio))
	{
		dec_time = 0;
		curve_reset();
		return;
	}

	auto average = perf_average.get_average();

	SetDebugFontColor(FadeColor(0xFF00FF00, 0xFFFF0000, (float)min(average, 1.0)));
	DisplayDebugStringFormatted(NJM_LOCATION(1, 16), "FRAME PERF AVG: %07.4f", average);
	SetDebugFontColor(last);

	if (clip_limit == 0.0f)
		clip_limit = clip_current;

	if (average < perf_dec || perf_ratio < 1.0)
	{
		// it's been a second, so let's just start over
		// from the min draw distance and hopefully increase
		// from there.
		if (++dec_time >= (60 / last_multi))
		{
			dec_time = 0;
			Clip_Reset(clip_current);
		}
		else
		{
			curve_inc(1.0f - (float)(average / perf_dec));

			if (!Clip_Decrease(curve))
			{
				curve_reset();
				return;
			}

			SetDebugFontColor(0xFFFF0000);
			DisplayDebugStringFormatted(NJM_LOCATION(1, 10), "- DECREASING: %07.4f", curve);
			SetDebugFontColor(last);
		}
	}
	else
	{
		curve_inc((1.0f - clip_limit / clip_max) * (float)(perf_ratio / perf_inc));

		if (average < perf_inc || !Clip_Increase(curve))
		{
			curve_reset();
			return;
		}

		SetDebugFontColor(0xFF00FF00);
		DisplayDebugStringFormatted(NJM_LOCATION(1, 10), "+ INCREASING: %07.4f", curve);
		SetDebugFontColor(last);
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

		WriteJump(InitMasterObjectArray, InitMasterObjectArray_r);
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
		auto pad = ControllerPointers[0];
		if (pad && pad->HeldButtons & Buttons_R && pad->PressedButtons & Buttons_Y)
		{
			enableFrameLimit = !enableFrameLimit;
		}

		if (last_level != CurrentLevel || last_act != CurrentAct
			|| pad && pad->PressedButtons & Buttons_C)
		{
			last_level = CurrentLevel;
			last_act   = CurrentAct;
			Clip_Reset();
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

#ifdef _DEBUG
		DisplayDebugStringFormatted(NJM_LOCATION(1, 4), "COUNT: REAL/AVG/MAX: %03u / %03u / %03u", object_count, object_average, MasterObjectArray_r.size());
#endif
		DisplayDebugStringFormatted(NJM_LOCATION(1, 5), "CLIP: %f", clip_current);
		Textures_OnFrame();
	}
}
