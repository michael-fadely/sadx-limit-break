#include "stdafx.h"
#include "objdef.h"

void Ring_Main_r(ObjectMaster *a1) {
	if (a1->Data1->Scale.x == -1)
		a1->Data1->Scale.x = 4;

	Ring_Main(a1);
}

float __stdcall squareroot_r(float a1) {
	return 0;
}

void objdef_init() {
	float range = 3600000;

	//Bridge in EC01
	WriteData((float*)0x7EC9F8, range);
	WriteData((float*)0x7EC9F4, range);
	
	//Hovering helicopters
	WriteCall((void**)0x6137E8, squareroot_r);
	
	//SkyDeck ring hack
	WriteData((ObjectFuncPtr*)0x2230F2C, Ring_Main_r);
}