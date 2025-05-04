#include <swilib.h>
#include "../alarm.h"

#define CBOX_CHECKED   0xE116
#define CBOX_UNCHECKED 0xE117

int GetCBox(int flag) {
    return (flag) ? CBOX_CHECKED : CBOX_UNCHECKED;
}

void GetAlarmState(WSHDR *ws, int active) {
    const int icon = (active) ? 20001 : 20002;
    wstrcatprintf(ws, "%c%c%c%c%c", UTF16_ALIGN_CENTER, UTF16_FONT_SMALL,
                  GetUnicodeSymbolByDynIcon(icon), UTF16_FONT_RESET, UTF16_ALIGN_NONE);
}

void GetTime(WSHDR *ws) {
    TTime time;
    WSHDR ws_time;
    uint16_t wsbody_time[16];
    CreateLocalWS(&ws_time, wsbody_time, 16);
    GetDateTime(NULL, &time);
    GetTime_ws(&ws_time, &time, 0x227);
    wstrcatprintf(ws, "%c%c%c%c%w%c%c", UTF16_ALIGN_RIGHT, UTF16_FONT_MEDIUM,
                  UTF16_TEXT_COLOR, PC_FOREGROUND, &ws_time,
                  UTF16_FONT_RESET, UTF16_ALIGN_NONE);
}

void GetAlarmTime(WSHDR *ws, const TTime *time) {
    WSHDR ws_time;
    uint16_t wsbody_time[16];
    CreateLocalWS(&ws_time, wsbody_time, 16);
    GetTime_ws(&ws_time, time, 0x223);
    wstrcatprintf(ws, "%c%c%w%c%c", UTF16_ALIGN_CENTER, UTF16_FONT_MEDIUM, &ws_time, UTF16_FONT_RESET, UTF16_ALIGN_NONE);
}

void GetDays(WSHDR *ws) {
    const char days[] = {'M', 'T', 'W', 'T', 'F', 'S', 'S'};
    wsAppendChar(ws, UTF16_ALIGN_LEFT);
    wsAppendChar(ws, UTF16_FONT_SMALL);
    for (int i = 0 ; i < 7; i++) {
        const char day = days[i];
        wsAppendChar(ws, 0xE300 + 29 - GetSymbolWidth(day, FONT_SMALL) / 2 + i * 28);
        wsAppendChar(ws, day);
    }
    wsAppendChar(ws, UTF16_FONT_RESET);
    wsAppendChar(ws, UTF16_ALIGN_NONE);
}

void GetDaysInUse(WSHDR *ws, uint32_t days, int cursor) {
    wsAppendChar(ws, UTF16_ALIGN_CENTER);
    for (int i = 0, f = 1; i < 7; i++, f *= 2) {
        if (cursor == i) {
            wsAppendChar(ws, UTF16_BG_INVERTION2);
            wsAppendChar(ws, GetCBox((int)days & f ));
            wsAppendChar(ws, UTF16_NO_INVERTION);
        } else {
            wsAppendChar(ws, GetCBox((int)days & f ));
        }
        wsAppendChar(ws, 0xE300 + 28 * (i + 1));
    }
    wsAppendChar(ws, UTF16_ALIGN_NONE);
}
