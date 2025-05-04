#pragma once

#include "../alarm.h"

typedef struct {
    ALARM *alarm;
    int timer_update;
} UI_DATA;

void UpdateTView(GUI *gui);
int CreateUI(ALARM *alarm);
