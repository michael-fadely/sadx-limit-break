#include "stdafx.h"
#include "objdef.h"

void Ring_Main_r(ObjectMaster *a1) {
	if (a1->Data1->Scale.x == -1)
		a1->Data1->Scale.x = 4;

	Ring_Main(a1);
}

void objdef_init() {
	float range = 3600000;

	//Bridge in EC01
	WriteData((float*)0x7EC9F8, range);
	WriteData((float*)0x7EC9F4, range);

	//SkyDeck ring hack
	WriteData((ObjectFuncPtr*)0x2230F2C, Ring_Main_r);
}