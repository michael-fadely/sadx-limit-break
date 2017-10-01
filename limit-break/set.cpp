#include "stdafx.h"
#include "set.h"

#define SHIBYTE(x)   (*((int8_t*)&(x)+1))

static std::vector<SETObjData> set_table {};
static std::vector<MissionSETData> mission_set_table {};

// TODO: dynamic SetData
//DataArray(SETEntry*, SetFiles, 0x03ABDF40, 6);
DataArray(short, word_3C52468, 0x3C52468, 0);
DataPointer(Uint8*, MissionFlagsPtr, 0x03C70140);
DataPointer(int16_t, SETTable_Count, 0x03C4E454);
DataPointer(SETEntry*, CurrentSetFile, 0x03C4E45C);
DataPointer(SETEntry*, CurrentSetFileBase, 0x03C4E458);
DataPointer(int, dword_91BA00, 0x0091BA00);
DataPointer(short, CurrentStageAndAct, 0x03C4E450);

FunctionPointer(Sint32, IsSwitchPressed, (int index), 0x004CB4F0);

static Trampoline* SetObjList_t = nullptr;
static Trampoline* ReleaseSetFile_t = nullptr;

static ObjectMaster *__cdecl GetSetObjInstance_r(ObjectMaster *caller, __int16 index)
{
	ObjectMaster *result = nullptr;

	if (index != -1)
	{
		if (SHIBYTE(index) >= 0)
		{
			result = set_table[index & 0x7FFF].ObjInstance;
		}
		else
		{
			result = mission_set_table[index & 0x7FFF].ObjInstance;
		}
	}

	return result;
}

static _BOOL1 __cdecl IsMissionSETObj_r(void *a1)
{
	if (mission_set_table.empty())
	{
		return false;
	}

	const auto size = mission_set_table.size();

	if (size == 1)
	{
		return a1 == mission_set_table.data();
	}

	return a1 >= mission_set_table.data() && a1 <= &mission_set_table[size - 1];
}

static _BOOL1 __cdecl IsActiveMissionObject_r(ObjectMaster *a1)
{
	MissionSETData *v1 = a1->SETData.MissionSETData;

	return !IsMissionSETObj_r(v1) || !((v1->PRMEntry->Display ^ MissionFlagsPtr[v1->PRMEntry->Mission]) & 1);
}

static void __cdecl SetObjList_r()
{
	set_table.clear();
	const auto original = (decltype(SetObjList_r)*)SetObjList_t->Target();
	original();
}

static void __cdecl CountSetItemsMaybe_r()
{
	SETEntry *entry = CurrentSetFile;
	const int stagething = ((char)CurrentStageAndAct + (SHIBYTE(CurrentStageAndAct) << 8)) << 8;
	int j = 0;

	if (CurrentSetFile)
	{
		//int max = 1023;
		//if ((unsigned int)(SETTable_Count + *(_DWORD *)&CurrentSetFileBase->ObjectType) <= 1023)
		//{
			int max = SETTable_Count + *(_DWORD *)&CurrentSetFileBase->ObjectType;
		//}

		if (!max)
		{
			set_table.clear();
		}
		else if (set_table.size() != max && max > 0)
		{
			set_table.resize(max);
		}

		__int16 max_ = max;
		SETObjData *obj_data = &set_table[SETTable_Count];
		if (SETTable_Count < max)
		{
			int i = max - SETTable_Count;
			do
			{
				obj_data->Flags |= 0x8000u;
				const int bullshit = dword_91BA00;
				obj_data->f1 = 0;
				if (bullshit == stagething + (entry->ObjectType & 0xFFF))
				{
					obj_data->Flags |= word_3C52468[j++ + 7 * GetCurrentCharacterID()];
					obj_data->f1 = j;
				}
				obj_data->SETEntry = entry;
				obj_data->LoadCount = 0;
				++entry;
				++obj_data;
				--i;
			} while (i);
			max_ = max;
		}
		SETTable_Count = max_;
	}

	SETTable_Count = static_cast<int16_t>(set_table.size());
}

static void __cdecl CountSetItems_r()
{
	//const auto original = (decltype(CountSetItems_r)*)CountSetItems_t->Target();
	//original()

	ObjectMaster *v4; // eax@4

	signed int v0 = 0;
	__int16 v6 = 0;
	if (SETTable_Count <= 0)
	{
		SETTable_Count = 0;
		set_table.clear();
		return;
	}

	signed int v1 = -1;
	char *v2 = (char *)&set_table[0].Flags;
	SETObjData *v3 = set_table.data();
	do
	{
		if (!(*(short *)v2 & 0x4000)
			|| (*(short *)v2 &= ~0x4000u,
				*(_DWORD *)&v3->LoadCount = *(_DWORD *)(v2 - 2),
				v3->ObjInstance = *(ObjectMaster **)(v2 + 2),
				v3->SETEntry = *(SETEntry **)(v2 + 6),
				v3->Distance = *(float *)(v2 + 10),
				(v4 = v3->ObjInstance) != nullptr)
			&& (v4->SETData.SETData = v3, ++v3, ++v1, ++v6, v0 != v1))
		{
			const char v5 = (*v2 & 2) == 0;
			*(_DWORD *)(v2 + 6) = 0;
			*(short *)v2 = 0;
			if (!v5)
			{
				FreeMemory(*(void **)(v2 + 10));
				*(_DWORD *)(v2 + 10) = 0;
			}
		}
		++v0;
		v2 += 0x10;
	} while (v0 < SETTable_Count);
	SETTable_Count = v6;

	if (SETTable_Count != set_table.size())
	{
		set_table.resize(SETTable_Count);
	}
}

#define __HIBYTE(x)   (*((int8_t*)&(x)+1))
#define __LOBYTE(x)   (*((int8_t*)&(x)))   // low byte

static char __cdecl sub_5922A0(ObjectMaster *a1)
{
	__int16 v3; // ax@6
	__int16 v5; // ax@11

	MissionSETData *v1 = a1->SETData.MissionSETData;
	if (IsMissionSETObj_r(v1))
	{
		PRM_Entry *v2 = v1->PRMEntry;
		if (v2->Display == 5 && !IsSwitchPressed(v2->field_A + 32))
		{
			return 0;
		}
		if (v2->Display == 3)
		{
			__HIBYTE(v3) = v2->Appearance;
			__LOBYTE(v3) = v2->field_9;
			if (v3 != -1)
			{
				SETObjData *v4 = SHIBYTE(v3) >= 0 ? &set_table[v3 & 0x7FFF] : &mission_set_table[v3 & 0x7FFF];
				if (v4)
				{
					v5 = v4->Flags;
					if (SHIBYTE(v5) >= 0 || !(v5 & 0x20))
					{
						return 0;
					}
				}
			}
		}
		if (v2->field_3)
		{
			SETObjData *v6 = a1->SETData.SETData;
			const char v7 = v6->f1;
			if (v7)
			{
				v6->f1 = v7 - 1;
				return 0;
			}
		}
	}
	return 1;
}

static void __cdecl sub_46BE00()
{
	signed int v0 = 0;
	if (SETTable_Count > 0)
	{
		char *v1 = (char *)&set_table[0].Flags;
		do
		{
			if (!(*(WORD *)v1 & 0x4000))
			{
				const __int16 v2 = *(WORD *)v1 & 0x7FE9;
				*(WORD *)v1 = v2;
				if (v2 & 1)
				{
					*(ObjectFuncPtr *)(*(_DWORD *)(v1 + 2) + 16) = DeleteObjectMaster;
				}
			}
			++v0;
			v1 += 16;
		} while (v0 < SETTable_Count);
	}
}

static void __cdecl sub_46BF20()
{
	if (!CurrentSetFile)
	{
		return;
	}

	/*int v0 = *(_DWORD *)&CurrentSetFileBase->ObjectType + SETTable_Count;
		if ((unsigned int)v0 > 1023)
		{
			v0 = 1023;
		}*/

	int v0 = static_cast<int>(set_table.size());
	if (v0 > 0)
	{
		char *v1 = (char *)&set_table[0].Flags;
		do
		{
			if (!(*v1 & 0x10))
			{
				*(WORD *)v1 = 0x8000u;
				*(v1 - 1) = 0;
			}
			v1 += 16;
			--v0;
		} while (v0);
	}
}

// ReSharper disable once CppDeclaratorNeverUsed
static void* __cdecl get_data()
{
	return set_table.data();
}

// ReSharper disable once CppDeclaratorNeverUsed
static const auto loc_46BA2E = (void*)0x46BA2E;
// ReSharper disable once CppDeclaratorNeverUsed
static const auto loc_46B821 = (void*)0x46B821;
static void __declspec(naked) reference_46B816()
{
	__asm
	{
		push eax
		call get_data
		mov esi, eax
		pop eax

		js _loc_46BA2E
		jmp loc_46B821
	_loc_46BA2E:
		jmp loc_46BA2E
	}
}

// ReSharper disable once CppDeclaratorNeverUsed
static const auto loc_46BCD2 = (void*)0x46BCD2;
// ReSharper disable once CppDeclaratorNeverUsed
static const auto loc_46BAD2 = (void*)0x46BAD2;
static void __declspec(naked) reference_46BAC7()
{
	__asm
	{
		push eax
		call get_data
		mov esi, eax
		pop eax

		js _loc_46BCD2
		jmp loc_46BAD2
	_loc_46BCD2:
		jmp loc_46BCD2
	}
}

static void __cdecl ReleaseSetFile_r()
{
	const auto original = (decltype(ReleaseSetFile_r)*)ReleaseSetFile_t->Target();
	original();
	set_table.clear();
}

void Set_Init()
{
	WriteJump(GetSetObjInstance, GetSetObjInstance_r);
	WriteJump(IsMissionSETObj, IsMissionSETObj_r);
	WriteJump(IsActiveMissionObject, IsActiveMissionObject_r);
	WriteJump((void*)0x5922A0, sub_5922A0);
	WriteJump((void*)0x46BE00, sub_46BE00);
	WriteJump((void*)0x46BF20, sub_46BF20);

	WriteJump((void*)0x0046B816, reference_46B816);
	WriteJump((void*)0x0046BAC7, reference_46BAC7);

	SetObjList_t = new Trampoline(0x0046C1D0, 0x0046C1D8, SetObjList_r);
	ReleaseSetFile_t = new Trampoline(0x00422440, 0x00422447, ReleaseSetFile_r);
	WriteJump((void*)0x0046BE50, CountSetItems_r);
	WriteJump((void*)0x0046BD20, CountSetItemsMaybe_r);
}
