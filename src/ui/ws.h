#pragma once

#include <swilib.h>

void GetAlarmState(WSHDR *ws, int active);
void GetTime(WSHDR *ws);
void GetAlarmTime(WSHDR *ws, const TTime *time);
void GetDays(WSHDR *ws);
void GetDaysInUse(WSHDR *ws, uint32_t days, int cursor);
