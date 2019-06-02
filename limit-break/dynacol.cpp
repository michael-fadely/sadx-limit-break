#include "stdafx.h"

#pragma region "ObjectArray"

struct UsedObject
{
	bool used;
	NJS_OBJECT* object;
};

std::deque<UsedObject> object_array;

NJS_OBJECT* __cdecl ObjectArray_GetFreeObject_r()
{
	auto pred = [](auto e) -> bool
	{
		return e.used == false;
	};

	const auto it = std::find_if(object_array.begin(), object_array.end(), pred);

	if (it != object_array.end())
	{
		return it->object;
	}

	const auto object = new NJS_OBJECT {};

	const UsedObject element
	{
		true,
		object
	};

	object_array.push_back(element);
	PrintDebug("object_array expanded to: %u\n", object_array.size());

	return object;
}

int __cdecl ObjectArray_Remove_r(NJS_OBJECT* a1)
{
	int result = 0;

	for (auto& element : object_array)
	{
		if (element.object == a1)
		{
			element.used = false;
			break;
		}

		++result;
	}

	return result;
}

#pragma endregion

FunctionPointer(/*NJS_OBJECT_LIST*/ void*, sub_456B40, (NJS_OBJECT* a1), 0x456B40);
DataPointer(int, LandTable_CollisionMeshCount, 0x03B36D3C);
//DataPointer(int, LandTableLoadedA, 0x0091545C);
DataPointer(int, LandTableLoadedB, 0x00915460);

constexpr auto ColFlags_WaterNoAlpha = 0x40002u;

std::vector<DynamicCOL> dynacol_a;
std::vector<DynamicCOL> dynacol_b;

void __cdecl DynamicCOL_RunA()
{
	float v1; // edx
	float v2; // edx
	float v3; // eax
	float v4; // eax
	DynamicCOL* v5; // ebx
	NJS_OBJECT* model; // esi
	NJS_MODEL_SADX* v7; // eax
	float v8; // st7
	float px; // st7
	float py; // st6
	float pz; // st5
	Angle v12; // eax
	Angle v13; // eax
	Angle v14; // eax
	int v15; // edx
	float v16; // st4
	float v18; // ST20_4
	float v19; // ST1C_4
	float v20; // ST18_4
	int v23; // edi
	float* v24 = nullptr; // esi
	float v25 = 0.0f; // ST20_4
	float v26 = 0.0f; // st7
	float v27 = 0.0f; // st6
	float v28 = 0.0f; // ST00_4
	int v34 = 0; // [esp+14h] [ebp-3Ch]
	COL* v35 = nullptr; // [esp+18h] [ebp-38h]
	float v36 = 0.0f; // [esp+1Ch] [ebp-34h]
	NJS_VECTOR position; // [esp+2Ch] [ebp-24h]

	//float v39; // [esp+38h] [ebp-18h]
	//float v40; // [esp+3Ch] [ebp-14h]
	//float v41; // [esp+40h] [ebp-10h]
	//float v42; // [esp+44h] [ebp-Ch]
	//float v43; // [esp+48h] [ebp-8h]
	//float v44; // [esp+4Ch] [ebp-4h]

	// shit's fucked
	NJS_VECTOR v[2] {};

	NJS_VECTOR& va = v[0];

	float& v42 = v[1].x;
	float& v43 = v[1].y;
	float& v44 = v[1].z;

	DynamicCOL* dynacol_landtable = DynamicCOLArray_LandTable;

	if (!Camera_Data1)
	{
		return;
	}

	va.x = Camera_Data1->Position.x;
	v1 = Camera_Data1->Position.y;
	va.z = Camera_Data1->Position.z;
	va.y = v1;
	v34 = 1;

	if (EntityData1Ptrs[0])
	{
		va.x = EntityData1Ptrs[0]->Position.x;
		v2 = EntityData1Ptrs[0]->Position.y;
		va.z = EntityData1Ptrs[0]->Position.z;
		va.y = v2;
		if (EntityData1Ptrs[1])
		{
			v42 = EntityData1Ptrs[1]->Position.x;
			v3 = EntityData1Ptrs[1]->Position.z;
			v43 = EntityData1Ptrs[1]->Position.y;
			v44 = v3;
			v34 = 2;
		}
	}
	else if (EntityData1Ptrs[1])
	{
		va.x = EntityData1Ptrs[1]->Position.x;
		v4 = EntityData1Ptrs[1]->Position.z;
		va.y = EntityData1Ptrs[1]->Position.y;
		va.z = v4;
	}

	DynamicCOLCount_B = 0;
	DynamicCOLCount_B_Again = 0;
	njPushMatrix(nullptr);

	if (!DynamicCOLCount || dynacol_a.empty())
	{
		goto WHAT_FUCK;
	}

	v5 = &dynacol_a.back();

	size_t landtable_i = 0;

	while (true)
	{
		model = v5->Model;
		if (!(v5->Entity->Data1->Status & Status_Ball))
		{
			goto CONTINUE;
		}

		if (v5->Flags & ColFlags_UseRotation)
		{
			njUnitMatrix(nullptr);
			v12 = model->ang[2];
			if (v12)
			{
				njRotateZ(nullptr, (unsigned __int16)v12);
			}
			v13 = model->ang[0];
			if (v13)
			{
				njRotateX(nullptr, (unsigned __int16)v13);
			}
			v14 = model->ang[1];
			if (v14)
			{
				njRotateY(nullptr, (unsigned __int16)v14);
			}
			njCalcPoint(nullptr, &model->basicdxmodel->center, &position);
			px = position.x + model->pos[0];
			py = position.y + model->pos[1];
			pz = position.z + model->pos[2];
		}
		else
		{
			v7 = model->basicdxmodel;
			v8 = v7->center.x;
			auto center = &v7->center;
			px = v8 + model->pos[0];
			py = center->y + model->pos[1];
			pz = center->z + model->pos[2];
		}

		v15 = v34;
		v16 = LandTable_MinimumRadius;

		if (v34 <= 0)
		{
			goto CONTINUE;
		}

		auto v17 = &va.y;

		while (true)
		{
			v18 = *(v17 - 1) - px;
			v19 = *v17 - py;
			v20 = v17[1] - pz;
			if ((v16 + model->basicdxmodel->r) * (v16 + model->basicdxmodel->r) > v20 * v20 + v19 * v19 + v18 * v18)
			{
				break;
			}
			v16 = 40.0;
			--v15;
			v17 += 3;
			if (v15 <= 0)
			{
				goto CONTINUE;
			}
		}

		/*v29 = (int*)((char*)DynamicCOLArray_B + (char*)dynacol_landtable - (char*)DynamicCOLArray_LandTable);
		*v29 = v5->Flags;
		v30 = v5->Entity;
		v29[1] = (int)v5->Model;
		v29[2] = (int)v30;*/

		auto b = &dynacol_b[landtable_i];

		b->Flags = v5->Flags;
		b->Model = v5->Model;
		b->Entity = v5->Entity;

		//*(NJS_OBJECT * *)((char*)& DynamicCOLArray_LandTable[0].Model + (char*)dynacol_landtable - (char*)DynamicCOLArray_LandTable) = v6;
		//*(ObjectMaster * *)((char*)& DynamicCOLArray_LandTable[0].Entity + (char*)dynacol_landtable - (char*)DynamicCOLArray_LandTable) = v31;

		DynamicCOLArray_LandTable[landtable_i].Model = model;
		DynamicCOLArray_LandTable[landtable_i].Entity = v5->Entity;

		dynacol_landtable->Flags = v5->Flags;
		++dynacol_landtable;
		++DynamicCOLCount_B;

		if (static_cast<size_t>(++DynamicCOLCount_B_Again) >= dynacol_b.size())
		{
			break;
		}

		if (DynamicCOLCount_B >= DynamicCOLArray_LandTable_Length)
		{
			njPopMatrix(1u);
			return;
		}

	CONTINUE:
		if (++landtable_i >= dynacol_a.size())
		{
			goto WHAT_FUCK;
		}

		--v5;
	}

	PrintDebug("overflow tears 0\n");

WHAT_FUCK:
	njPopMatrix(1u);
	LandTable_CollisionMeshCount = 0;
	v35 = ColList2;

	if (LandTableLoadedB != 1 || !CurrentLandTable)
	{
		return;
	}

	for (int i = 0; i < CurrentLandTable->COLCount; ++i)
	{
		auto v21 = &CurrentLandTable->Col[i];

		if (!(v21->Flags & (ColFlags_WaterNoAlpha | ColFlags_Water | ColFlags_Solid)))
		{
			continue;
		}

		v23 = v34;
		v36 = LandTable_MinimumRadius;
		if (v34 <= 0)
		{
			continue;
		}

		v24 = &va.y;
		while (true)
		{
			v25 = v21->Center.x - *(v24 - 1);
			v26 = v21->Center.y - *v24;
			v27 = v21->Center.z - v24[1];
			v28 = v27 * v27 + v26 * v26 + v25 * v25;
			if (squareroot(v28) - v36 < v21->Radius)
			{
				break;
			}
			--v23;
			v24 += 3;
			v36 = 40.0f;
			if (v23 <= 0)
			{
				goto LABEL_38;
			}
		}

		memcpy(v35, v21, sizeof(COL));
		dynacol_landtable->Flags  = v21->Flags;
		dynacol_landtable->Entity = nullptr;
		++v35;
		dynacol_landtable->Model = v21->Model;
		++dynacol_landtable;
		++DynamicCOLCount_B;

		if (static_cast<size_t>(++LandTable_CollisionMeshCount) >= dynacol_b.size() ||
		    DynamicCOLCount_B >= DynamicCOLArray_LandTable_Length)
		{
			return;
		}

	LABEL_38:
		continue;
	}
}

void __cdecl DynamicCOL_RunB()
{
	short v1 = 0;
	DynamicCOL* v2 = DynamicCOLArray_LandTable;
	DynamicCOLCount_B = 0;
	DynamicCOLCount_B_Again = 0;

	size_t landtable_i = 0;

	if (DynamicCOLCount && !dynacol_a.empty())
	{
		DynamicCOL* v0 = &dynacol_a.back();

		while (true)
		{
			//DynamicCOL* v3 = (DynamicCOL*)((char*)DynamicCOLArray_B + (char*)v2 - (char*)DynamicCOLArray_LandTable);
			// TODO: add
			auto v3 = &dynacol_b[landtable_i];

			v3->Flags = v0->Flags;
			ObjectMaster* v4 = v0->Entity;
			v3->Model = v0->Model;
			v3->Entity = v4;

			//*(NJS_OBJECT * *)((char*)& DynamicCOLArray_LandTable[0].Model + (char*)v2
			//				  - (char*)DynamicCOLArray_LandTable) = v0->Model;

			//*(ObjectMaster * *)((char*)& DynamicCOLArray_LandTable[0].Entity
			//					+ (char*)v2
			//					- (char*)DynamicCOLArray_LandTable) = v0->Entity;

			DynamicCOLArray_LandTable[landtable_i].Model = v0->Model;
			DynamicCOLArray_LandTable[landtable_i].Entity = v0->Entity;

			v2->Flags = v0->Flags;
			++v2;
			++v1;

			if (DynamicCOLCount_B_Again > 0 && static_cast<size_t>(++DynamicCOLCount_B_Again) >= dynacol_b.size())
			{
				break;
			}

			if (v1 < DynamicCOLArray_LandTable_Length)
			{
				--v0;
				if (landtable_i < dynacol_a.size())
				{
					continue;
				}
			}
			DynamicCOLCount_B = v1;
			goto WHAT_FUCK;
		}

		DynamicCOLCount_B = v1;
		PrintDebug("overflow tears 1\n");
		v1 = DynamicCOLCount_B;
	}

WHAT_FUCK:
	LandTable_CollisionMeshCount = 0;
	COL* v6 = ColList2;
	if (LandTableLoadedB == 1)
	{
		if (CurrentLandTable)
		{
			COL* v7 = CurrentLandTable->Col;
			for (int i = CurrentLandTable->COLCount; i > 0; --i)
			{
				if (v7->Flags & (ColFlags_WaterNoAlpha | ColFlags_Water | ColFlags_Solid))
				{
					memcpy(v6, v7, sizeof(COL));
					v2->Flags = v7->Flags;
					v2->Entity = nullptr;
					++v6;
					v2->Model = v7->Model;
					++v2;
					++v1;
					DynamicCOLCount_B = v1;
					if (static_cast<size_t>(++LandTable_CollisionMeshCount) >= dynacol_b.size() || v1 >= DynamicCOLArray_LandTable_Length)
					{
						return;
					}
				}
				++v7;
			}
		}
	}
}

void __cdecl sub_43ACD0_r(float x, float y, float z, float radius)
{
	float v8; // st5
	NJS_VECTOR a3a; // [esp+10h] [ebp-Ch]

	DynamicCOL* v5 = DynamicCOLArray_LandTable;
	DynamicCOLCount_B = 0;
	njPushMatrix(nullptr);

	//DynamicCOL* v4 = (DynamicCOL*)((char*)& DynamicCOLArray_LandTable[DynamicCOLCount_B_Again + 1023] + 8);
	if (!DynamicCOLCount_B_Again)
	{

	LABEL_14:
		njPopMatrix(1u);
		if (LandTableLoadedB == 1)
		{
			if (CurrentLandTable)
			{
				COL* col = ColList2;
				for (int i = LandTable_CollisionMeshCount; i > 0; --i)
				{
					if (col->Flags & (ColFlags_WaterNoAlpha | ColFlags_Water | ColFlags_Solid))
					{
						float v25 = col->Center.y - y;
						float v26 = col->Center.z - z;
						float v27 = v26 * v26 + v25 * v25 + (col->Center.x - x) * (col->Center.x - x);
						if (squareroot(v27) - radius < col->Radius)
						{
							NJS_OBJECT* v28 = col->Model;
							v5->Flags = col->Flags;
							v5->Entity = nullptr;
							v5->Model = v28;
							++v5;
							if (++DynamicCOLCount_B >= DynamicCOLArray_LandTable_Length)
							{
								break;
							}
						}
					}
					++col;
				}
			}
		}

		return;
	}

	DynamicCOL* v4 = &dynacol_b.back();
	size_t i = 0;

	float v14 = 0.0f;
	float v15 = 0.0f;

	while (true)
	{
		NJS_OBJECT* v6 = v4->Model;

		if (v4->Flags & ColFlags_UseRotation)
		{
			njUnitMatrix(nullptr);
			Angle v11 = v6->ang[2];
			if (v11)
			{
				njRotateZ(nullptr, (unsigned __int16)v11);
			}
			Angle v12 = v6->ang[0];
			if (v12)
			{
				njRotateX(nullptr, (unsigned __int16)v12);
			}
			Angle v13 = v6->ang[1];
			if (v13)
			{
				njRotateY(nullptr, (unsigned __int16)v13);
			}
			njCalcPoint(nullptr, &v6->basicdxmodel->center, &a3a);
			v14 = x - (a3a.x + v6->pos[0]);
			v15 = y - (a3a.y + v6->pos[1]);
			v8 = a3a.z;
		}
		else
		{
			auto v7   = (float*)v6->basicdxmodel;
			float v9  = x - (v7[6] + v6->pos[0]);
			float v10 = y - (v7[7] + v6->pos[1]);
			v8 = v7[8];
		}

		// TODO: oh fuck
		//if (!(v19 | v20))

		float v16 = z - (v8 + v6->pos[2]);
		float v17 = radius + v6->basicdxmodel->r + 30.0f;

		// ghidra
		v8 = (x - v14) * (x - v14) + (y - v15) * (y - v15) + v16 * v16;

		// ghidra
		if ((unsigned short)((unsigned short)(v17 * v17 < v8) << 8 | (unsigned short)(v17 * v17 == v8) << 0xe) == 0)
		{
			ObjectMaster* v21 = v4->Entity;

			v5->Flags  = v4->Flags;
			v5->Model  = v6;
			v5->Entity = v21;

			++v5;

			if (++DynamicCOLCount_B >= DynamicCOLArray_LandTable_Length)
			{
				break;
			}
		}

		if (++i < DynamicCOLArray_LandTable_Length)
		{
			goto LABEL_14;
		}

		--v4;
	}

	njPopMatrix(1u);
}

void __cdecl DynamicCOL_Add_r(uint32_t flags, ObjectMaster* entity, NJS_OBJECT* model)
{
	char v6 = 0; // c0

	if (!v6)
	{
		if ((model->ang[0] | model->ang[1] | model->ang[2]) & 0xFFF0)
		{
			flags |= ColFlags_UseRotation;
		}
	}

	DynamicCOL dynacol {};

	dynacol.Flags  = flags;
	dynacol.Model  = model;
	dynacol.Entity = entity;

	entity->Data1->Status |= Status_Ground;

	dynacol_a.push_back(dynacol);

	++DynamicCOLCount;
}

void __cdecl DynamicCOL_Remove_r(ObjectMaster* entity, NJS_OBJECT* model)
{
	auto pred = [entity, model](auto a) -> bool
	{
		return a.Entity == entity && a.Model == model;
	};

	const auto it = std::remove_if(dynacol_a.begin(), dynacol_a.end(), pred);

	if (it != dynacol_a.end())
	{
		sub_456B40(model);
		dynacol_a.erase(it);
		DynamicCOLCount -= DynamicCOLCount - static_cast<Uint16>(dynacol_a.size());
	}
}

void dynacol_init()
{
	WriteJump(ObjectArray_GetFreeObject, ObjectArray_GetFreeObject_r);
	WriteJump(ObjectArray_Remove, ObjectArray_Remove_r);

	WriteJump(DynamicCOL_Add, DynamicCOL_Add_r);
	WriteJump(DynamicCOL_Remove, DynamicCOL_Remove_r);

	WriteJump(reinterpret_cast<void*>(0x0043AEF0), DynamicCOL_RunA);
	WriteJump(reinterpret_cast<void*>(0x0043B580), DynamicCOL_RunB);
	WriteJump(reinterpret_cast<void*>(0x43ACD0), sub_43ACD0_r);
}
