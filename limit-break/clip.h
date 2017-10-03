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

void clip_init();
// Reset current clip distance and limit to defaults
void clip_reset(float limit = 0.0f);
bool clip_increase(float inc);
bool clip_decrease(float dec);
