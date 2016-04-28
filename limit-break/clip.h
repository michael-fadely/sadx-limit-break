#pragma once

#define GET_SET(a) (static_cast<SETObjData*>(a->field_1C))

extern float clip_current;
extern float clip_max;
extern const float clip_default;

void Clip_Init();
