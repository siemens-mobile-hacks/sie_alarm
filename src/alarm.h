#pragma once

typedef struct {
    int active;
    TTime time;
    uint32_t days;
    int auto_snooze;
    int snooze_time;
} ALARM;

ALARM *ReadPDFile();
int SavePDFile(const ALARM *alarm);
#define IsAlarmEnabled() (*RamAlarmClockState() || GetAlarmClockState(4))
