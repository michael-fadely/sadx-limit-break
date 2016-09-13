#pragma once

// Current clip distance - cannot exceed clip_current
extern float clip_current;
// Clip distance upper limit
extern float clip_limit;
// Minimum recorded clip distance above clip_default
extern float clip_min;
// Maximum recorded clip distance
extern float clip_max;

// Default SADX object clip distance
extern const float clip_default;

void Clip_Init();
// Reset current clip distance and limit to defaults
void Clip_Reset();
bool Clip_Increase(float inc);
bool Clip_Decrease(float dec);
