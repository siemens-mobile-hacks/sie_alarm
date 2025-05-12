#include <stdlib.h>
#include <swilib.h>
#include <string.h>
#include "ws.h"
#include "ui.h"
#include "edit.h"

int HEADER_ICON[] = {4, 0};

HEADER_DESC HEADER_D = {{0, 0, 0, 0}, HEADER_ICON, (int)"Alarm clock", LGP_NULL};

const int SOFTKEYS[] = {SET_LEFT_SOFTKEY, SET_MIDDLE_SOFTKEY, SET_RIGHT_SOFTKEY};

static SOFTKEY_DESC SOFTKEY_D[] = {
    {0x0018, 0x0000, (int)"Edit"},
    {0x0018, 0x0000, (int)LGP_EDIT_PIC},
    {0x0001, 0x0000, (int)"Exit"},
};

static SOFTKEYSTAB SOFTKEYS_TAB = {
    SOFTKEY_D, 3
};

void UpdateTView(GUI *gui) {
    UI_DATA *data = TViewGetUserPointer(gui);

    WSHDR *ws = AllocWS(128);
    GetTime(ws);
    GetAlarmState(ws, data->alarm->active);
    GetAlarmTime(ws, &(data->alarm->time));
    wsAppendChar(ws, '\n');
    GetDays(ws);
    GetDaysInUse(ws, data->alarm->days, -1);
    TViewSetText(gui, ws, malloc_adr(), mfree_adr());
}

static void AutoUpdate(void *gui) {
    UI_DATA *data = TViewGetUserPointer(gui);

    UpdateTView(gui);
    DirectRedrawGUI();
    GUI_StartTimerProc(gui, data->timer_update, 1000, AutoUpdate);
}

static int OnKey(GUI *gui, GUI_MSG *msg) {
    if (msg->keys == 0x01) {
        return 1;
    } else if (msg->keys == 0x18) {
        CreateInputTextDialog_Edit(gui);
    }
    return -1;
}

static void GHook(GUI *gui, int cmd) {
    UI_DATA *data = TViewGetUserPointer(gui);

    if (cmd == TI_CMD_FOCUS) {
        UpdateTView(gui);
        data->timer_update = GUI_NewTimer(gui);
        GUI_StartTimerProc(gui, data->timer_update, 1000, AutoUpdate);
    } else if (cmd == TI_CMD_UNFOCUS) {
        GUI_DeleteTimer(gui, data->timer_update);
    } else if (cmd == TI_CMD_DESTROY) {
        GUI_DeleteTimer(gui, data->timer_update);
        mfree(data);
    }
}

void Locret() {}

static TVIEW_DESC TVIEW_D = {
    8,
    OnKey,
    GHook,
    Locret,
    SOFTKEYS,
    &SOFTKEYS_TAB,
    {0, 0, 0, 0},
    FONT_SMALL,
    0x64,
    0x65,
    0,
    0,
};

int CreateUI(ALARM *alarm) {
    memcpy(&(HEADER_D.rc), GetHeaderRECT(), sizeof(RECT));
    memcpy(&(TVIEW_D.rc), GetMainAreaRECT(), sizeof(RECT));
    TVIEW_D.rc.x += 6;
    TVIEW_D.rc.y += 10;
    TVIEW_D.rc.x2 -= 6;

    UI_DATA *data = malloc(sizeof(UI_DATA));
    void *gui = TViewGetGUI(malloc_adr(), mfree_adr());
    TViewSetDefinition(gui, &TVIEW_D);
    zeromem(data, sizeof(UI_DATA));

    data->alarm = alarm;
    TViewSetUserPointer(gui, data);
    SetHeaderToMenu(gui, &HEADER_D, malloc_adr());

    IMGHDR *img_alarm_on = GetPITaddr(3);
    IMGHDR *img_alarm_off = GetPITaddr(2);
    FreeDynIcon(20001);
    FreeDynIcon(20002);
    SetDynIcon(20001, img_alarm_on, (char*)img_alarm_on->bitmap);
    SetDynIcon(20002, img_alarm_off, (char*)img_alarm_off->bitmap);
    UpdateTView(gui);

    return CreateGUI(gui);
}
