#include <swilib.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "alarm.h"

#define ALARMCLOCK_PD_ID 0x210C

ALARM *ReadPDFile() {
    ALARM *alarm = malloc(sizeof(ALARM));

    char key[32];
    char value[32];
    uint32_t len;

    len = 32;
    sprintf(key, "%s", "alarm_active");
    if (ReadValueFromPDFile(ALARMCLOCK_PD_ID, key, value, &len) != 0) {
        mfree(alarm);
        return NULL;
    }
    value[len] = '\0';
    alarm->active = atoi(value);

    len = 32;
    sprintf(key, "%s", "alarm_time");
    if (ReadValueFromPDFile(ALARMCLOCK_PD_ID, key, value, &len) != 0) {
        mfree(alarm);
        return NULL;
    }
    value[len] = '\0';
    uint32_t seconds = strtoul(value, NULL, 10);
    GetTimeFromSeconds(&(alarm->time), (int)seconds);

    len = 32;
    sprintf(key, "%s", "days_in_use");
    if (ReadValueFromPDFile(ALARMCLOCK_PD_ID, key, value, &len) != 0) {
        mfree(alarm);
        return NULL;
    }
    value[len] = '\0';
    alarm->days = 0;
    for (int i = 0, f = 1; i < 7; i++, f *= 2) {
        const int day_flag = value[i] - '0';
        if (day_flag) {
            alarm->days |= f;
        }
    }

    len = 32;
    sprintf(key, "%s", "auto_snooze");
    if (ReadValueFromPDFile(ALARMCLOCK_PD_ID, key, value, &len) != 0) {
        mfree(alarm);
        return NULL;
    }
    value[len] = '\0';
    alarm->auto_snooze = atoi(value);

    len = 32;
    sprintf(key, "%s", "snooze_time");
    if (ReadValueFromPDFile(ALARMCLOCK_PD_ID, key, value, &len) != 0) {
        mfree(alarm);
        return NULL;
    }
    value[len] = '\0';
    alarm->snooze_time = atoi(value);

    return alarm;
}

int SavePDFile(const ALARM *alarm) {
    uint32_t err;
    const char *path = "1:\\system\\alarmclock.pd";

    int fp = sys_open(path, A_WriteOnly + A_Truncate + A_Create + A_TXT, P_WRITE, &err);
    if (fp != -1) {
        size_t len;
        char s[32];
        char line[128];

        sprintf(s, "%d", alarm->snooze_time);
        len = 21 + strlen(s);
        sprintf(line, "%06d:T:snooze_time=%s\r\n", len, s);
        sys_write(fp, line, len + 2, &err);

        for (int i = 0, f = 1; i < 7; i++, f *= 2) {
            s[i] = (alarm->days & f) ? '1' : '0';
        }
        s[7] = '\0';
        len = 21 + 7;
        sprintf(line, "%06d:T:days_in_use=%s\r\n", len, s);
        sys_write(fp, line, len + 2, &err);

        sprintf(s, "%d", GetSecondsFromTime(&(alarm->time)));
        len = 20 + strlen(s);
        sprintf(line, "%06d:T:alarm_time=%s\r\n", len, s);
        sys_write(fp, line, len + 2, &err);

        s[0] = (alarm->active) ? '1' : '0';
        s[1] = '\0';
        len = 22 + 1;
        sprintf(line, "%06d:T:alarm_active=%s\r\n", len, s);
        sys_write(fp, line, len + 2, &err);

        s[0] = (alarm->auto_snooze) ? '1' : '0';
        s[1] = '\0';
        len = 21 + 1;
        sprintf(line, "%06d:T:auto_snooze=%s\r\n", len, s);
        sys_write(fp, line, len + 2, &err);

        sys_close(fp, &err);
        return 1;
    }
    return 0;
}
