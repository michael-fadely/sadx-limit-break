#include "stdafx.h"
#include "set.h"

#define SHIBYTE(x)   (*((int8_t*)&(x)+1))

static std::vector<SETObjData> set_table {};
static std::vector<MissionSETData> mission_set_table {};

#pragma pack(push, 1)
struct ObjectListHead
{
	int Count;
	int List;
};
#pragma pack(pop)

// TODO: dynamic SetFiles
//DataArray(SETEntry*, SetFiles, 0x03ABDF40, 6);
DataArray(short, word_3C52468, 0x3C52468, 0);
DataArray(ObjectListHead*, ObjLists, 0x00974AF8, 344);
DataPointer(Uint8*, MissionFlagsPtr, 0x03C70140);
DataPointer(int16_t, SETTable_Count, 0x03C4E454);
DataPointer(int16_t, MissionSetCount, 0x03C70158);
DataPointer(SETEntry*, CurrentSetFile, 0x03C4E45C);
DataPointer(SETEntry*, CurrentSetFileBase, 0x03C4E458);
DataPointer(SETEntry*, MissionSetObjects, 0x03C72968);
DataPointer(PRM_Entry*, MissionParameters, 0x03C72960);
DataPointer(int, dword_91BA00, 0x0091BA00);
DataPointer(short, CurrentStageAndAct, 0x03C4E450);
DataPointer(LPVOID, MissionSetFile, 0x03C70150);
DataPointer(LPVOID, MissionParameterFile, 0x03C70154);
DataPointer(ObjectListHead, Objs_Mission, 0x0170F854);

FunctionPointer(Sint32, IsSwitchPressed, (int index), 0x004CB4F0);
FunctionPointer(int, LoadFileWithMalloc_, (const char *name, LPVOID *data), 0x422310);
FunctionPointer(void, MissionSET_LoadCam, (int arg_0, int arg_4), 0x005931B0);

static Trampoline* SetObjList_t = nullptr;
static Trampoline* ReleaseSetFile_t = nullptr;

static ObjectMaster *__cdecl GetSetObjInstance_r(ObjectMaster*, short index)
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

		short max_ = max;
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
	short v6 = 0;
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
			short v3;
			__HIBYTE(v3) = v2->Appearance;
			__LOBYTE(v3) = v2->field_9;

			if (v3 != -1)
			{
				SETObjData *v4 = SHIBYTE(v3) >= 0 ? &set_table[v3 & 0x7FFF] : &mission_set_table[v3 & 0x7FFF];
				if (v4)
				{
					short v5 = v4->Flags;
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
	for (auto& it : set_table)
	{
		if (!(it.Flags & 0x4000))
		{
			continue;
		}

		it.Flags &= 0x7FE9;

		if (it.Flags & 1 && it.ObjInstance)
		{
			it.ObjInstance->MainSub = DeleteObjectMaster;
		}
	}
}

static void __cdecl sub_46BF20()
{
	if (!CurrentSetFile)
	{
		return;
	}

	for (auto& it : set_table)
	{
		if (it.Flags & 0x10)
		{
			it.Flags = 0x8000u;
			it.f1 = 0;
		}
	}
}

// ReSharper disable once CppDeclaratorNeverUsed
static void* __cdecl get_set_data()
{
	return set_table.data();
}

// ReSharper disable once CppDeclaratorNeverUsed
static const auto loc_46BA2E = (void*)0x46BA2E;
// ReSharper disable once CppDeclaratorNeverUsed
static const auto loc_46B821 = (void*)0x46B821;
static void __declspec(naked) set_reference_46B816()
{
	__asm
	{
		push eax
		call get_set_data
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
static void __declspec(naked) set_reference_46BAC7()
{
	__asm
	{
		push eax
		call get_set_data
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

// ReSharper disable once CppDeclaratorNeverUsed
static void* __cdecl get_mission_data()
{
	return mission_set_table.data();
}

// ReSharper disable once CppDeclaratorNeverUsed
static const auto loc_591D68 = (void*)0x591D68;
static void __declspec(naked) mission_reference_591D5D()
{
	__asm
	{
		push eax
		xor eax, eax

		mov eax, dword ptr [MissionSetCount]
		mov ax, word ptr [eax]
		cmp ax, 0

		jle zero

		// non-zero
		call get_mission_data

		mov esi, eax
		mov edi, [eax+10h]

		jmp end

	zero:
		mov esi, 0
		mov edi, 0

	end:
		pop eax
		jmp loc_591D68
	}
}

static void __cdecl MissionSET_Clear()
{
	for (auto& it : mission_set_table)
	{
		if (it.ObjInstance)
		{
			DeleteObjectMaster(it.ObjInstance);
		}
	}

	mission_set_table.clear();
}

static void __cdecl MissionSET_Load()
{
	short v1; // ax@1
	short v2; // ax@1
	int v3; // eax@2
	char dest[32]; // [sp+Ch] [bp-24h]@4

	__LOBYTE(v1) = 0;
	__HIBYTE(v1) = (int8_t)CurrentLevel;
	MissionSetObjects = nullptr;
	MissionParameters = nullptr;
	v2 = CurrentAct | v1;
	char v0 = __HIBYTE(v2);
	const uint8_t v10 = (uint8_t)v2;

	if (__HIBYTE(v2) < (uint8_t)LevelIDs_SSGarden)
	{
	LABEL_4:
		const int v5 = (uint8_t)v0;
		*(ObjectListHead**)0x3C72970 = ObjLists[v10 + 8 * (uint8_t)v0];
		*(ObjectListHead**)0x3C72974 = &Objs_Mission;
		MissionSET_LoadCam((uint8_t)v0, v10);
		char *v4 = (char*)GetCharIDString();
		sprintf(dest, "SetMi%02d%02d%s.bin", v5, v10, v4);
		if (LoadFileWithMalloc_(dest, &MissionSetFile)
			|| (sprintf(dest, "PrmMi%02d%02d%s.bin", v5, v10, v4), LoadFileWithMalloc_(dest, &MissionParameterFile)))
		{
			MissionSetCount = 0;
			mission_set_table.clear();
		}
		else
		{
			MissionSetCount = *(WORD *)MissionSetFile;
			PRM_Entry *params = (PRM_Entry *)((char *)MissionParameterFile + 32);
			SETEntry *entry = (SETEntry *)((char *)MissionSetFile + 32);
			MissionSetObjects = (SETEntry *)((char *)MissionSetFile + 32);
			MissionParameters = (PRM_Entry *)((char *)MissionParameterFile + 32);

			if (MissionSetCount > 0)
			{
				mission_set_table.resize(MissionSetCount);

				for (auto& it : mission_set_table)
				{
					it.Flags |= 0x8000u;
					it.SETEntry = entry++;
					it.PRMEntry = params++;
					it.LoadCount = 0;
					it.f1 = 0;
				}
			}
			else
			{
				mission_set_table.clear();
			}
		}
	}
	else
	{
		__LOBYTE(v3) = GetNextChaoStage();
		switch (v3 + 1)
		{
			case 5:
				v0 = 39;
				goto LABEL_4;
			case 0:
				goto LABEL_4;
			case 6:
				v0 = 40;
				goto LABEL_4;
			case 7:
				v0 = 41;
				goto LABEL_4;
			default:
				MissionSetCount = 0;
				break;
		}
	}
}

static void __cdecl DeactivateMission_r(char mission, char deleteobjects)
{
	for (auto& it : mission_set_table)
	{
		if (it.PRMEntry->Mission != mission)
		{
			continue;
		}

		if (deleteobjects && it.ObjInstance)
		{
			it.ObjInstance->MainSub = DeleteObjectMaster;
		}

		it.Flags = it.Flags & 0xFFDB | 0x8000;
	}
}

void Set_Init()
{
	WriteJump(GetSetObjInstance, GetSetObjInstance_r);
	WriteJump(IsMissionSETObj, IsMissionSETObj_r);
	WriteJump(IsActiveMissionObject, IsActiveMissionObject_r);
	WriteJump((void*)0x5922A0, sub_5922A0);
	WriteJump((void*)0x46BE00, sub_46BE00);
	WriteJump((void*)0x46BF20, sub_46BF20);
	WriteJump((void*)0x591A20, MissionSET_Clear);
	WriteJump(DeactivateMission, DeactivateMission_r);

	WriteJump((void*)0x00591A70, MissionSET_Load);

	WriteJump((void*)0x0046B816, set_reference_46B816);
	WriteJump((void*)0x0046BAC7, set_reference_46BAC7);

	WriteJump((void*)0x00591D5D, mission_reference_591D5D);

	SetObjList_t = new Trampoline(0x0046C1D0, 0x0046C1D8, SetObjList_r);
	ReleaseSetFile_t = new Trampoline(0x00422440, 0x00422447, ReleaseSetFile_r);
	WriteJump((void*)0x0046BE50, CountSetItems_r);
	WriteJump((void*)0x0046BD20, CountSetItemsMaybe_r);
}
