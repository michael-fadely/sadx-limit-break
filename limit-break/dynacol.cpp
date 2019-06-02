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
	const auto pred = [](const auto& e) -> bool
	{
		return e.used == false;
	};

	const auto it = std::find_if(object_array.begin(), object_array.end(), pred);

	if (it != object_array.end())
	{
		it->used = true;
		*it->object = {};
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
			return result;
		}

		++result;
	}

	return -1;
}

#pragma endregion

FunctionPointer(/*NJS_OBJECT_LIST*/ void*, sub_456B40, (NJS_OBJECT* a1), 0x456B40);
DataPointer(int, LandTable_CollisionMeshCount, 0x03B36D3C);
//DataPointer(int, LandTableLoadedA, 0x0091545C);
DataPointer(int, LandTableLoadedB, 0x00915460);

constexpr auto ColFlags_WaterNoAlpha = 0x400000;

static std::vector<DynamicCOL> dynacol_a;
static std::vector<DynamicCOL> dynacol_b;

void __cdecl DynamicCOL_RunA()
{
	DynamicCOL* dynacol_landtable = DynamicCOLArray_LandTable;

	if (!Camera_Data1)
	{
		return;
	}

	NJS_VECTOR v[2] {};

	v[0] = Camera_Data1->Position;

	int player_count = 1;

	if (EntityData1Ptrs[0])
	{
		v[0] = EntityData1Ptrs[0]->Position;

		if (EntityData1Ptrs[1])
		{
			++player_count;
			v[1] = EntityData1Ptrs[1]->Position;
		}
	}
	else if (EntityData1Ptrs[1])
	{
		v[0] = EntityData1Ptrs[1]->Position;
	}

	DynamicCOLCount_B = 0;
	DynamicCOLCount_B_Again = 0;
	//dynacol_b.clear();

	njPushMatrix(nullptr);

	if (!DynamicCOLCount || dynacol_a.empty())
	{
		goto WHAT_FUCK;  // NOLINT
	}

	DynamicCOL* v5 = &dynacol_a.back();

	size_t dynacol_a_i = 0;

	while (true)
	{
		NJS_OBJECT* model = v5->Model;
		if (!(v5->Entity->Data1->Status & Status_Ball))
		{
			goto CONTINUE;  // NOLINT
		}

		float px;
		float py;
		float pz;

		if (v5->Flags & ColFlags_UseRotation)
		{
			njUnitMatrix(nullptr);

			Angle rz = model->ang[2];
			if (rz)
			{
				njRotateZ(nullptr, (unsigned __int16)rz);
			}

			Angle rx = model->ang[0];
			if (rx)
			{
				njRotateX(nullptr, (unsigned __int16)rx);
			}

			Angle ry = model->ang[1];
			if (ry)
			{
				njRotateY(nullptr, (unsigned __int16)ry);
			}

			NJS_VECTOR point {}; // [esp+2Ch] [ebp-24h]
			njCalcPoint(nullptr, &model->basicdxmodel->center, &point);

			px = point.x + model->pos[0];
			py = point.y + model->pos[1];
			pz = point.z + model->pos[2];
		}
		else
		{
			NJS_MODEL_SADX* v7 = model->basicdxmodel;
			const float v8 = v7->center.x;
			const auto center = &v7->center;
			px = v8 + model->pos[0];
			py = center->y + model->pos[1];
			pz = center->z + model->pos[2];
		}

		int v15 = player_count;
		float v16 = LandTable_MinimumRadius;

		if (player_count <= 0)
		{
			goto CONTINUE;  // NOLINT
		}

		NJS_VECTOR* v17 = &v[0];

		while (true)
		{
			const float dx = v17->x - px;
			const float dy = v17->y - py;
			const float dz = v17->z - pz;

			auto dxmodel = model->getbasicdxmodel();

			if ((v16 + dxmodel->r) * (v16 + dxmodel->r) > dz * dz + dy * dy + dx * dx)
			{
				break;
			}

			v16 = 40.0f;
			--v15;
			++v17;

			if (v15 <= 0)
			{
				goto CONTINUE;  // NOLINT
			}
		}

		*dynacol_landtable = *v5;

		//dynacol_b.push_back(*v5);

		dynacol_b.resize(DynamicCOLCount_B + 1);
		dynacol_b[DynamicCOLCount_B] = *v5;

		++dynacol_landtable;
		++DynamicCOLCount_B;
		++DynamicCOLCount_B_Again;


		/*if (static_cast<size_t>(DynamicCOLCount_B_Again) >= dynacol_b.size())
		{
			break;
		}*/

		if (DynamicCOLCount_B >= DynamicCOLArray_LandTable_Length)
		{
			njPopMatrix(1u);
			return;
		}

	CONTINUE:
		--v5;
		if (++dynacol_a_i >= dynacol_a.size())
		{
			goto WHAT_FUCK;  // NOLINT
		}
	}

	/*
	PrintDebug("overflow tears 0\n");
	*/

WHAT_FUCK:
	njPopMatrix(1u);
	LandTable_CollisionMeshCount = 0;
	COL* v35 = ColList2;

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

		int v23 = player_count;
		float v36 = LandTable_MinimumRadius;

		if (player_count <= 0)
		{
			continue;
		}

		auto v24 = &v[0];

		while (true)
		{
			const float dx       = v21->Center.x - v24->x;
			const float dy       = v21->Center.y - v24->y;
			const float dz       = v21->Center.z - v24->z;
			const float distance = dz * dz + dy * dy + dx * dx;

			if (squareroot(distance) - v36 < v21->Radius)
			{
				break;
			}

			--v23;
			++v24;
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
		++LandTable_CollisionMeshCount;

		if (/*static_cast<size_tLandTable_CollisionMeshCount) >= dynacol_b.size() ||
		    */DynamicCOLCount_B >= DynamicCOLArray_LandTable_Length)
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
	dynacol_b.clear();

	size_t landtable_i = 0;

	if (DynamicCOLCount && !dynacol_a.empty())
	{
		DynamicCOL* v0 = &dynacol_a.back();

		while (true)
		{
			//DynamicCOL* v3 = (DynamicCOL*)((char*)DynamicCOLArray_B + (char*)v2 - (char*)DynamicCOLArray_LandTable);

			dynacol_b.push_back(*v0);

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

			++DynamicCOLCount_B_Again;

			/*if (static_cast<size_t>(DynamicCOLCount_B_Again) >= dynacol_b.size())
			{
				break;
			}*/

			if (v1 < DynamicCOLArray_LandTable_Length)
			{
				--v0;
				if (landtable_i < dynacol_a.size())
				{
					continue;
				}
			}
			DynamicCOLCount_B = v1;
			goto WHAT_FUCK;  // NOLINT
		}

		/*
		DynamicCOLCount_B = v1;
		PrintDebug("overflow tears 1\n");
		v1 = DynamicCOLCount_B;
		*/
	}

WHAT_FUCK:
	LandTable_CollisionMeshCount = 0;
	COL* v6 = ColList2;
	if (LandTableLoadedB == 1 && CurrentLandTable)
	{
		COL* v7 = CurrentLandTable->Col;
		for (int i = CurrentLandTable->COLCount; i > 0; --i)
		{
			if (v7->Flags & (ColFlags_WaterNoAlpha | ColFlags_Water | ColFlags_Solid))
			{
				memcpy(v6, v7, sizeof(COL));
				v2->Flags  = v7->Flags;
				v2->Entity = nullptr;
				++v6;
				v2->Model = v7->Model;
				++v2;
				++v1;
				DynamicCOLCount_B = v1;
				++LandTable_CollisionMeshCount;

				if (static_cast<size_t>(LandTable_CollisionMeshCount) >= dynacol_b.size() || v1 >= DynamicCOLArray_LandTable_Length)
				{
					return;
				}
			}
			++v7;
		}
	}
}

void __cdecl sub_43ACD0_r(float x, float y, float z, float radius)
{
	DynamicCOL* v5 = DynamicCOLArray_LandTable;
	DynamicCOLCount_B = 0;
	njPushMatrix(nullptr);

	//DynamicCOL* v4 = (DynamicCOL*)((char*)& DynamicCOLArray_LandTable[DynamicCOLCount_B_Again + 1023] + 8);
	if (DynamicCOLCount_B_Again < 1)
	{
	LABEL_14:
		njPopMatrix(1u);
		if (LandTableLoadedB == 1 && CurrentLandTable)
		{
			for (int i = 0; i < LandTable_CollisionMeshCount; ++i)
			{
				COL* col = &ColList2[i];

				if (!(col->Flags & (ColFlags_WaterNoAlpha | ColFlags_Water | ColFlags_Solid)))
				{
					continue;
				}

				const float dy = col->Center.y - y;
				const float dz = col->Center.z - z;
				const float distance = dz * dz + dy * dy + (col->Center.x - x) * (col->Center.x - x);

				if (squareroot(distance) - radius < col->Radius)
				{
					NJS_OBJECT* v28 = col->Model;

					v5->Flags  = col->Flags;
					v5->Entity = nullptr;
					v5->Model  = v28;

					++v5;
					++DynamicCOLCount_B;

					if (DynamicCOLCount_B >= DynamicCOLArray_LandTable_Length)
					{
						break;
					}
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
		float v8; // st5

		if (v4->Flags & ColFlags_UseRotation)
		{
			njUnitMatrix(nullptr);

			Angle rz = v6->ang[2];
			if (rz)
			{
				njRotateZ(nullptr, (unsigned __int16)rz);
			}

			Angle rx = v6->ang[0];
			if (rx)
			{
				njRotateX(nullptr, (unsigned __int16)rx);
			}

			Angle ry = v6->ang[1];
			if (ry)
			{
				njRotateY(nullptr, (unsigned __int16)ry);
			}

			NJS_VECTOR point; // [esp+10h] [ebp-Ch]
			njCalcPoint(nullptr, &v6->basicdxmodel->center, &point);

			v14 = x - (point.x + v6->pos[0]);
			v15 = y - (point.y + v6->pos[1]);
			v8 = point.z;
		}
		else
		{
			//auto v7 = (float*)v6->basicdxmodel;
			//float v9  = x - (v7[6] + v6->pos[0]);
			//float v10 = y - (v7[7] + v6->pos[1]);
			const auto& center = v6->basicdxmodel->center;
			v14 = center.x; // HACK
			v15 = center.y; // HACK
			v8 = center.z;
		}

		// lol ida
		//if (!(v19 | v20))

		const float v16 = z - (v8 + v6->pos[2]);
		const float v17 = radius + v6->basicdxmodel->r + 30.0f;

		// ghidra
		v8 = (x - v14) * (x - v14) + (y - v15) * (y - v15) + v16 * v16;
		const auto length = v17 * v17;

		// ghidra
		if (length <= v8)
		{
			ObjectMaster* v21 = v4->Entity;

			v5->Flags  = v4->Flags;
			v5->Model  = v6;
			v5->Entity = v21;

			++v5;
			++DynamicCOLCount_B;

			if (DynamicCOLCount_B >= DynamicCOLArray_LandTable_Length)
			{
				break;
			}
		}

		if (++i >= dynacol_b.size())
		{
			goto LABEL_14;  // NOLINT
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
	auto pred = [entity, model](const auto& a) -> bool
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

void __cdecl sub_43A210_r();
static Trampoline sub_43A210_t(0x0043A210, 0x0043A217, sub_43A210_r);
void __cdecl sub_43A210_r()
{
	auto original = reinterpret_cast<decltype(sub_43A210_r)*>(sub_43A210_t.Target());
	original();

	dynacol_a.clear();
	dynacol_b.clear();
}

void __cdecl LandTableObj_Delete_r(ObjectMaster* obj);
static Trampoline LandTableObj_Delete_t(0x0043A500, 0x0043A507, LandTableObj_Delete_r);
void __cdecl LandTableObj_Delete_r(ObjectMaster* obj)
{
	auto original = reinterpret_cast<decltype(LandTableObj_Delete_r)*>(LandTableObj_Delete_t.Target());
	original(obj);

	dynacol_a.clear();
	dynacol_b.clear();
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
