#include "stdafx.h"
#include "objdef.h"

void objdef_init() {
	float range = 3600000;

	//Bridge in EC01
	WriteData((float*)0x7EC9F8, range);
	WriteData((float*)0x7EC9F4, range);
}