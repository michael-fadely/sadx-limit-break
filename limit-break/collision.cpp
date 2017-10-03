#include "stdafx.h"
#include "collision.h"
#include "misc.h"

// TODO:
namespace CollisionList
{
	enum _enum
	{
		Players,
		Unknown_1,
		Targetable,
		Hazard,
		Unknown_4,
		Unknown_5,
		Minimal,
		Rings,
		Unknown_8,
		Chao,
		COUNT
	};
}

static std::vector<EntityData1*> entities[CollisionList::COUNT] = {};
static std::vector<CollisionInfo*> big_dummies[CollisionList::COUNT] = {};
// TODO: actually update this. It's likely used for Gamma's targeting system.

static void __cdecl CheckSelfCollision(Uint32 num)
{
	auto& list = entities[num];

	for (Uint32 i = 0; i < list.size(); i++)
	{
		for (Uint32 x = 0; x < list.size(); x++)
		{
			if (x != i)
			{
				CheckCollision(list[i], list[x]);
			}
		}
	}
}

static void __cdecl Collision_Statistics()
{
#ifdef _DEBUG
	auto rows = (Uint32)(480.0f * VerticalStretch / DebugFontSize) - 1;
	DisplayDebugStringFormatted(NJM_LOCATION(1, rows - 10), "ENTITIES[0]: %3u [player]",       entities[0].size());
	DisplayDebugStringFormatted(NJM_LOCATION(1, rows - 9),  "ENTITIES[1]: %3u",                entities[1].size());
	DisplayDebugStringFormatted(NJM_LOCATION(1, rows - 8),  "ENTITIES[2]: %3u [targetable]",   entities[2].size());
	DisplayDebugStringFormatted(NJM_LOCATION(1, rows - 7),  "ENTITIES[3]: %3u [enemy/hazard]", entities[3].size());
	DisplayDebugStringFormatted(NJM_LOCATION(1, rows - 6),  "ENTITIES[4]: %3u",                entities[4].size());
	DisplayDebugStringFormatted(NJM_LOCATION(1, rows - 5),  "ENTITIES[5]: %3u",                entities[5].size());
	DisplayDebugStringFormatted(NJM_LOCATION(1, rows - 4),  "ENTITIES[6]: %3u [minimal]",      entities[6].size());
	DisplayDebugStringFormatted(NJM_LOCATION(1, rows - 3),  "ENTITIES[7]: %3u [rings]",        entities[7].size());
	DisplayDebugStringFormatted(NJM_LOCATION(1, rows - 2),  "ENTITIES[8]: %3u",                entities[8].size());
	DisplayDebugStringFormatted(NJM_LOCATION(1, rows - 1),  "ENTITIES[9]: %3u [chao]",         entities[9].size());
#endif
}

static void __cdecl ClearLists()
{
	Collision_Statistics();

	for (Uint32 i = 0; i < CollisionList::COUNT; i++)
	{
		entities[i].clear();
		big_dummies[i].clear();
	}
}

static void __cdecl ClearLists_hook()
{
	njPopMatrixEx();
	ClearLists();
}

static void __cdecl AddToCollisionList_r(EntityData1* entity)
{
	const auto collision = entity->CollisionInfo;

	if (collision && collision->Object->MainSub != DeleteObjectMaster)
	{
		int v2 = CurrentAct | (CurrentLevel << 8);
		bool is_chao_stage = BYTE1(v2) >= (signed int)LevelIDs_SSGarden && BYTE1(v2) <= (signed int)LevelIDs_ChaoRace;
		bool is_player = collision->List == 0;
		IsChaoStage = is_chao_stage;

		if (!is_player || !is_chao_stage)
		{
			DoSomeCollisionThing(entity);
		}

		if (collision->List < 0 || collision->List > 9)
		{
			return;
		}

		const auto i = collision->List;

		if (GameMode != GameModes_Menu)
		{
			if (std::find(entities[i].begin(), entities[i].end(), entity) == entities[i].end())
			{
				entities[i].push_back(entity);
			}

			if (std::find(big_dummies[i].begin(), big_dummies[i].end(), collision) == big_dummies[i].end())
			{
				big_dummies[i].push_back(collision);
			}
		}
	}
}

static void __cdecl RunPlayerCollision_r()
{
	if (IsChaoStage)
	{
		for (auto& i : entities[0])
		{
			DoSomeCollisionThing(i);
		}
	}

	CheckSelfCollision(0);

	for (auto& i : entities[0])
	{
		for (auto& x : entities[2])
		{
			CheckCollision(i, x);
		}

		for (auto& x : entities[3])
		{
			CheckCollision(i, x);
		}

		for (auto& x : entities[4])
		{
			CheckCollision__(i, x);
		}

		for (auto& x : entities[5])
		{
			CheckCollision__(i, x);
		}

		for (auto& x : entities[6])
		{
			CheckCollision(i, x);
		}

		for (auto& x : entities[7])
		{
			CheckCollision(i, x);
		}

		for (auto& x : entities[8])
		{
			CheckCollision_(i, x);
		}

		for (auto& x : entities[9])
		{
			CheckCollision(i, x);
		}
	}
}

static void __cdecl RunCollision_1_r()
{
	for (auto& i : entities[1])
	{
		for (auto& x : entities[0])
		{
			CheckCollision(i, x);
		}

		for (auto& x : entities[2])
		{
			CheckCollision(i, x);
		}

		for (auto& x : entities[3])
		{
			CheckCollision(i, x);
		}

		for (auto& x : entities[4])
		{
			CheckCollision(i, x);
		}

		for (auto& x : entities[5])
		{
			CheckCollision(i, x);
		}

		for (auto& x : entities[6])
		{
			CheckCollision(i, x);
		}

		for (auto& x : entities[7])
		{
			CheckCollision(i, x);
		}

		for (auto& x : entities[9])
		{
			CheckCollision(i, x);
		}
	}
}

static void __cdecl RunCollision_9_r()
{
	CheckSelfCollision(9);

	for (auto& i : entities[9])
	{
		for (auto& x : entities[2])
		{
			CheckCollision(i, x);
		}

		for (auto& x : entities[3])
		{
			CheckCollision(i, x);
		}

		for (auto& x : entities[4])
		{
			CheckCollision(i, x);
		}

		for (auto& x : entities[5])
		{
			CheckCollision(i, x);
		}

		for (auto& x : entities[6])
		{
			CheckCollision(i, x);
		}

		for (auto& x : entities[7])
		{
			CheckCollision(i, x);
		}
	}
}

static void __cdecl RunCollision_3_r()
{
	CheckSelfCollision(3);

	for (auto& i : entities[3])
	{
		for (auto& x : entities[4])
		{
			CheckCollision(i, x);
		}

		for (auto& x : entities[5])
		{
			CheckCollision(i, x);
		}
	}
}

static void __cdecl RunCollision_4_r()
{
	for (auto& i : entities[4])
	{
		for (auto& j : entities[5])
		{
			CheckCollision(i, j);
		}
	}
}

static void __cdecl RunCollision_5_r()
{
	CheckSelfCollision(5);
}

void Collision_Init()
{
	WriteJump((void*)0x0041B970, ClearLists);
	WriteCall((void*)0x004207A7, ClearLists_hook);
	WriteJump((void*)0x0041C280, AddToCollisionList_r);
	WriteJump((void*)0x00420010, RunPlayerCollision_r);
	WriteJump((void*)0x00420210, RunCollision_1_r);
	WriteJump((void*)0x004203C0, RunCollision_9_r);
	WriteJump((void*)0x00420560, RunCollision_3_r);
	WriteJump((void*)0x00420640, RunCollision_4_r);
	WriteJump((void*)0x004206A0, RunCollision_5_r);
}
