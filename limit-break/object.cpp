#include "stdafx.h"
#include "object.h"
 #include <cmath>
#include "misc.h"

static Uint32 object_count = 0;
static std::deque<ObjectMaster> MasterObjectArray_r = {};
static bool queue_initialized = false;

static constexpr Uint32 POINTS_LENGTH = 60;
static Uint32 points[POINTS_LENGTH] = {};
static Uint32 points_i = 0;
static Uint32 object_average = 0;

VoidFunc(sub_51A740, 0x51A740);

// ReSharper disable once CppDeclaratorNeverUsed
static ObjectMaster* __cdecl AllocateObjectMaster_r(int index, ObjectFuncPtr LoadSub)
{
	if (index < 0 || index > 8)
	{
		return nullptr;
	}

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

	result->MainSub = LoadSub ? LoadSub : nullptr;

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
			result->Next = last;
			last->Previous = result;

			ObjectListThing[index] = result;
		}
		else
		{
			ObjectListThing[index] = result;

			result->Next = nullptr;
			result->Previous = nullptr;
		}
	}

	result->SETData.SETData = nullptr;
	result->Data1 = nullptr;
	result->Data2 = nullptr;
	result->UnknownA_ptr = nullptr;
	result->UnknownB_ptr = nullptr;
	result->DisplaySub = nullptr;
	result->DeleteSub = nullptr;
	result->Child = nullptr;
	result->Parent = nullptr;
	result->field_30 = 0;

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
	MasterObjectArray = ptr;
	CurrentObject = nullptr;
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
	{
		ObjectListThing[i] = nullptr;
	}
}

static void __cdecl DeleteObjectMaster_r(ObjectMaster* _this)
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
	auto next = _this->Next;

	_this->MainSub = nullptr;
	_this->DisplaySub = nullptr;
	_this->DeleteSub = nullptr;

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

	auto set = _this->SETData.SETData;
	if (set)
	{
		set->ObjInstance = nullptr;
		_LOBYTE(_this->SETData.SETData->Flags) &= 0xFEu;
		_this->SETData.SETData = nullptr;
	}

	auto master = MasterObjectArray;
	MasterObjectArray = _this;

	if (master == nullptr)
	{
		_this->Next = nullptr;
		_this->Previous = nullptr;
		_this->Child = nullptr;
		_this->Parent = nullptr;
	}
	else
	{
		master->Previous = _this;
		_this->Previous = nullptr;
		_this->Child = nullptr;
		_this->Parent = nullptr;
		_this->Next = master;
	}

	// Clear the object queue if there are no objects checked out
	if (!object_count)
	{
		MasterObjectArray_r.clear();
		InitObjectQueue();
	}
}

void object_init()
{
	WriteJump(InitMasterObjectArray, InitMasterObjectArray_r);
	WriteJump((void*)AllocateObjectMasterPtr, AllocateObjectMaster_asm);
	WriteJump(DeleteObjectMaster, DeleteObjectMaster_r);
	WriteCall((void*)0x0040B011, PrintDebug);
}

void object_OnFrame()
{
	if (GameState == 15)
	{
		const auto p = points_i;
		points[p] = object_count;

		if (++points_i >= POINTS_LENGTH)
		{
			float result = 0.0f;

			for (size_t i = 0; i < p; i++)
			{
				result += points[i];
			}

			object_average = static_cast<Uint32>(std::ceil(result / (float)POINTS_LENGTH));
		}
	}

	points_i %= POINTS_LENGTH;

#ifdef _DEBUG
	DisplayDebugStringFormatted(NJM_LOCATION(1, 4), "COUNT: REAL/AVG/MAX: %03u / %03u / %03u", object_count, object_average, MasterObjectArray_r.size());
#endif
}
