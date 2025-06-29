#include <swilib.h>
#include <stdlib.h>
#include <string.h>
#include "ui.h"
#include "ws.h"

typedef struct {
    ALARM alarm;
    GUI *main_gui;
    int cursor_days;
    int timer_update;
} EDIT_DATA;

extern HEADER_DESC HEADER_D;

static SOFTKEY_DESC SOFTKEY_D[] = {
    {0x0018, 0x0000, (int)"Save"},
    {0x0029, 0x0000, (int)LGP_CHANGE_PIC},
};

static const SOFTKEYSTAB SOFTKEYS_TAB = {
    SOFTKEY_D, 0,
};

void UpdateTime(void *gui) {
    WSHDR ws;
    uint16_t wsbody[32];
    CreateLocalWS(&ws, wsbody, 32);
    GetTime(&ws);
    EDIT_SetTextToEditControl(gui, 1, &ws);
}

void UpdateAlarmState(void *gui) {
    EDIT_DATA *data = EDIT_GetUserPointer(gui);

    WSHDR ws;
    uint16_t wsbody[8];
    CreateLocalWS(&ws, wsbody, 8);
    GetAlarmState(&ws, data->alarm.active);
    EDIT_SetTextToEditControl(gui, 2, &ws);
}

void UpdateDaysInUse(void *gui) {
    EDIT_DATA *data = EDIT_GetUserPointer(gui);

    WSHDR ws;
    uint16_t wsbody[32];
    CreateLocalWS(&ws, wsbody, 32);
    GetDaysInUse(&ws, data->alarm.days, data->cursor_days);
    EDIT_SetTextToEditControl(gui, 6, &ws);
}

static void AutoUpdate(void *gui) {
    EDIT_DATA *data = EDIT_GetUserPointer(gui);
    UpdateTime(gui);
    RefreshGUI();
    GUI_StartTimerProc(gui, data->timer_update, 1000, AutoUpdate);
}

static int OnKey(GUI *gui, GUI_MSG *msg) {
    EDIT_DATA *data = EDIT_GetUserPointer(gui);
    UI_DATA *ui_data = TViewGetUserPointer(data->main_gui);

    int focus = EDIT_GetFocus(gui);
    if (msg->gbsmsg->msg == KEY_DOWN || msg->gbsmsg->msg == LONG_PRESS) {
        if (msg->gbsmsg->submess == LEFT_BUTTON) {
            if (focus == 5) {
                data->cursor_days--;
                if (data->cursor_days < 0) {
                    data->cursor_days = 7 - 1;
                }
                UpdateDaysInUse(gui);
                return -1;
            }
        } else if (msg->gbsmsg->submess == RIGHT_BUTTON) {
            if (focus == 5) {
                data->cursor_days++;
                if (data->cursor_days >= 7) {
                    data->cursor_days = 0;
                }
                UpdateDaysInUse(gui);
                return -1;
            }
        }
    }
    if (msg->keys == 0x26) { // up
        if (focus == 5) {
            data->cursor_days = -1;
            UpdateDaysInUse(gui);
        }
    } else if (msg->keys == 0x25) { // down
        if (focus == 3) {
            data->cursor_days = 0;
            UpdateDaysInUse(gui);
        }
    } else if (msg->keys == 0x29) { // enter
        if (focus == 3) {
            if (data->alarm.days == 0) {
                data->alarm.active = 0;
            } else {
                data->alarm.active = !data->alarm.active;
            }
            UpdateAlarmState(gui);
            DirectRedrawGUI();
        } else if (focus == 5) {
            uint32_t prev_days = data->alarm.days;
            switch (data->cursor_days) {
                case 0:
                    data->alarm.days += (data->alarm.days & 1) ? -1 : 1;
                break;
                case 1:
                    data->alarm.days += (data->alarm.days & 2) ? -2 : 2;
                break;
                case 2:
                    data->alarm.days += (data->alarm.days & 4) ? -4 : 4;
                break;
                case 3:
                    data->alarm.days += (data->alarm.days & 8) ? -8 : 8;
                break;
                case 4:
                    data->alarm.days += (data->alarm.days & 16) ? -16 : 16;
                break;
                case 5:
                    data->alarm.days += (data->alarm.days & 32) ? -32 : 32;
                break;
                case 6:
                    data->alarm.days += (data->alarm.days & 64) ? -64 : 64;
                break;
            }
            if (!data->alarm.days) {
                data->alarm.active = 0;
            } else if (prev_days == 0) {
                data->alarm.active = 1;
            }
            UpdateAlarmState(gui);
            UpdateDaysInUse(gui);
            DirectRedrawGUI();
        }
    } else if (msg->keys == 0x18) { // save
        TTime time;
        EDIT_GetTime(gui, 3, &time);
        memcpy(&(data->alarm.time), &time, sizeof(TTime));
        memcpy(ui_data->alarm, &data->alarm, sizeof(data->alarm));
        UpdateTView(data->main_gui);
        SavePDFile(ui_data->alarm);
        RefreshAlarmClock();
        return 1;
    }
    return 0;
}

static void GHook(GUI *gui, int cmd) {
    EDIT_DATA *data = EDIT_GetUserPointer(gui);

    if (cmd == TI_CMD_REDRAW) {
        SetSoftKey(gui, &SOFTKEY_D[0], SET_LEFT_SOFTKEY);
        SetSoftKey(gui, &SOFTKEY_D[1], SET_MIDDLE_SOFTKEY);
    } else if (cmd == TI_CMD_CREATE) {
        EDIT_SetFocus(gui, 3);
    } else if (cmd == TI_CMD_FOCUS) {
        UpdateTime(gui);
        data->timer_update = GUI_NewTimer(gui);
        GUI_StartTimerProc(gui, data->timer_update, 1000, AutoUpdate);
    } else if (cmd == TI_CMD_UNFOCUS) {
        GUI_DeleteTimer(gui, data->timer_update);
    } else if (cmd == TI_CMD_DESTROY) {
        GUI_DeleteTimer(gui, data->timer_update);
        mfree(data);
    }
}

static INPUTDIA_DESC INPUTDIA_D = {
    8,
    OnKey,
    GHook,
    NULL,
    0,
    &SOFTKEYS_TAB,
    {0, 0, 0, 0},
    FONT_SMALL,
    0x64,
    0x65,
    0,
    TEXT_ALIGNMIDDLE,
    INPUTDIA_FLAGS_SWAP_SOFTKEYS,
};

int CreateInputTextDialog_Edit(GUI *main_gui) {
    UI_DATA *ui_data = TViewGetUserPointer(main_gui);
    EDIT_DATA *data = malloc(sizeof(EDIT_DATA));
    zeromem(data, sizeof(EDIT_DATA));

    memcpy(&(HEADER_D.rc), GetHeaderRECT(), sizeof(RECT));
    memcpy(&(INPUTDIA_D.rc), GetMainAreaRECT(), sizeof(RECT));

    void *ma = malloc_adr();
    void *eq = AllocEQueue(ma, mfree_adr());

    WSHDR ws;
    uint16_t wsbody[128];
    EDITCONTROL ec;
    PrepareEditControl(&ec);
    CreateLocalWS(&ws, wsbody, 128);
    memcpy(&data->alarm, ui_data->alarm, sizeof(ALARM));
    data->main_gui = main_gui;

    GetTime(&ws);
    ConstructEditControl(&ec, ECT_HEADER, ECF_NORMAL_STR, &ws, wstrlen(&ws));
    AddEditControlToEditQend(eq, &ec, ma);

    CutWSTR(&ws, 0);
    GetAlarmState(&ws, data->alarm.active);
    ConstructEditControl(&ec, ECT_HEADER, ECF_NORMAL_STR, &ws, wstrlen(&ws));
    AddEditControlToEditQend(eq, &ec, ma);

    CutWSTR(&ws, 0);
    ConstructEditControl(&ec, ECT_TIME, ECF_NORMAL_STR, 0, 16);
    ConstructEditTime(&ec, &(data->alarm.time));
    AddEditControlToEditQend(eq, &ec, ma);

    CutWSTR(&ws, 0);
    wsprintf(&ws, "\n\n");
    ConstructEditControl(&ec, ECT_HEADER, ECF_NORMAL_STR, &ws, wstrlen(&ws));
    AddEditControlToEditQend(eq, &ec, ma);

    CutWSTR(&ws, 0);
    GetDays_ws(&ws);
    ConstructEditControl(&ec, ECT_READ_ONLY, ECF_NORMAL_STR, &ws, wstrlen(&ws));
    AddEditControlToEditQend(eq, &ec, ma);

    CutWSTR(&ws, 0);
    GetDaysInUse(&ws, data->alarm.days, -1);
    ConstructEditControl(&ec, ECT_HEADER, ECF_NORMAL_STR, &ws, wstrlen(&ws));
    AddEditControlToEditQend(eq, &ec, ma);

    return CreateInputTextDialog(&INPUTDIA_D, &HEADER_D, eq, 1, data);
}