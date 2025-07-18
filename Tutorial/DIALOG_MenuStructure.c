/*********************************************************************
*                SEGGER Microcontroller GmbH & Co. KG                *
*        Solutions for real time microcontroller applications        *
**********************************************************************
*                                                                    *
*        (c) 1996 - 2014  SEGGER Microcontroller GmbH & Co. KG       *
*                                                                    *
*        Internet: www.segger.com    Support:  support@segger.com    *
*                                                                    *
**********************************************************************

** emWin V5.24 - Graphical user interface for embedded applications **
All  Intellectual Property rights  in the Software belongs to  SEGGER.
emWin is protected by  international copyright laws.  Knowledge of the
source code may not be used to write a similar product.  This file may
only be used in accordance with the following terms:

The software has been licensed to  ARM LIMITED whose registered office
is situated at  110 Fulbourn Road,  Cambridge CB1 9NJ,  England solely
for  the  purposes  of  creating  libraries  for  ARM7, ARM9, Cortex-M
series,  and   Cortex-R4   processor-based  devices,  sublicensed  and
distributed as part of the  MDK-ARM  Professional  under the terms and
conditions  of  the   End  User  License  supplied  with  the  MDK-ARM
Professional. 
Full source code is available at: www.segger.com

We appreciate your understanding and fairness.
----------------------------------------------------------------------
File        : DIALOG_MenuStructure.c
Purpose     : Shows how to achieve a menu structure with dialogs
Requirements: WindowManager - (X)
              MemoryDevices - ( )
              AntiAliasing  - ( )
              VNC-Server    - ( )
              PNG-Library   - ( )
              TrueTypeFonts - ( )
---------------------------END-OF-HEADER------------------------------
*/

#include "Dialog.h"

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/
#define ID_FRAMEWIN_0  (GUI_ID_USER + 0x00)
#define ID_BUTTON_0    (GUI_ID_USER + 0x01)
#define ID_BUTTON_1    (GUI_ID_USER + 0x02)
#define ID_TEXT_0      (GUI_ID_USER + 0x03)
#define ID_RADIO_0     (GUI_ID_USER + 0x04)
#define ID_CHECKBOX_0  (GUI_ID_USER + 0x05)

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
/*********************************************************************
*
*       _aDialogCreate1
*/
static const GUI_WIDGET_CREATE_INFO _aDialogCreate1[] = {
  { FRAMEWIN_CreateIndirect, "Framewin1", ID_FRAMEWIN_0,   0,  0, 320, 240, 0, 0x64, 0 },
  { BUTTON_CreateIndirect, "Button",      ID_BUTTON_0,     5,  5,  80,  20, 0, 0x0,  0 },
  { BUTTON_CreateIndirect, "Button",      ID_BUTTON_1,     5, 30,  80,  20, 0, 0x0,  0 },
  { TEXT_CreateIndirect,   "Text",        ID_TEXT_0,     110, 60, 160,  32, 0, 0x64, 0 },
};

/*********************************************************************
*
*       _aDialogCreate2
*/
static const GUI_WIDGET_CREATE_INFO _aDialogCreate2[] = {
  { FRAMEWIN_CreateIndirect, "Framewin2", ID_FRAMEWIN_0,   0,  0, 320, 240, 0, 0x64,   0 },
  { BUTTON_CreateIndirect, "Button",      ID_BUTTON_0,     5,  5,  80,  20, 0, 0x0,    0 },
  { BUTTON_CreateIndirect, "Button",      ID_BUTTON_1,     5, 30,  80,  20, 0, 0x0,    0 },
  { RADIO_CreateIndirect, "Radio",        ID_RADIO_0,    110, 60,  80,  60, 0, 0x1403, 0 },
};

/*********************************************************************
*
*       _aDialogCreate3
*/
static const GUI_WIDGET_CREATE_INFO _aDialogCreate3[] = {
  { FRAMEWIN_CreateIndirect, "Framewin3", ID_FRAMEWIN_0,   0,  0, 320, 240, 0, 0x64, 0 },
  { BUTTON_CreateIndirect, "Button",      ID_BUTTON_0,     5,  5,  80,  20, 0, 0x0,  0 },
  { BUTTON_CreateIndirect, "Button",      ID_BUTTON_1,     5, 30,  80,  20, 0, 0x0,  0 },
  { CHECKBOX_CreateIndirect, "Checkbox",  ID_CHECKBOX_0, 110, 60, 100,  20, 0, 0x0,  0 },
};

/*********************************************************************
*
*       Prototypes
*
**********************************************************************
*/
static void _cbDialog1(WM_MESSAGE * pMsg);
static void _cbDialog2(WM_MESSAGE * pMsg);
static void _cbDialog3(WM_MESSAGE * pMsg);

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/
/*********************************************************************
*
*       _cbDialog1
*/
static void _cbDialog1(WM_MESSAGE * pMsg) {
  WM_HWIN hItem;
  int     NCode;
  int     Id;

  switch (pMsg->MsgId) {
  case WM_INIT_DIALOG:
    //
    // Initialization of 'Framewin1'
    //
    hItem = pMsg->hWin;
    FRAMEWIN_SetText(hItem, "Screen 1");
    FRAMEWIN_SetFont(hItem, GUI_FONT_32_ASCII);
    //
    // Initialization of 'Button'
    //
    hItem = WM_GetDialogItem(pMsg->hWin, ID_BUTTON_0);
    BUTTON_SetText(hItem, "Screen 2");
    //
    // Initialization of 'Button'
    //
    hItem = WM_GetDialogItem(pMsg->hWin, ID_BUTTON_1);
    BUTTON_SetText(hItem, "Screen 3");
    //
    // Initialization of 'Text'
    //
    hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_0);
    TEXT_SetFont(hItem, GUI_FONT_32_ASCII);
    TEXT_SetText(hItem, "Some text");
    break;
  case WM_NOTIFY_PARENT:
    Id    = WM_GetId(pMsg->hWinSrc);
    NCode = pMsg->Data.v;
    switch(Id) {
    case ID_BUTTON_0: // Notifications sent by 'Button'
      switch(NCode) {
      case WM_NOTIFICATION_CLICKED:
        // USER START (Optionally insert code for reacting on notification message)
        // USER END
        break;
      case WM_NOTIFICATION_RELEASED:
        GUI_EndDialog(pMsg->hWin, 0);
        GUI_CreateDialogBox(_aDialogCreate2, GUI_COUNTOF(_aDialogCreate2), _cbDialog2, WM_HBKWIN, 0, 0);
        break;
      }
      break;
    case ID_BUTTON_1: // Notifications sent by 'Button'
      switch(NCode) {
      case WM_NOTIFICATION_CLICKED:
        // USER START (Optionally insert code for reacting on notification message)
        // USER END
        break;
      case WM_NOTIFICATION_RELEASED:
        GUI_EndDialog(pMsg->hWin, 0);
        GUI_CreateDialogBox(_aDialogCreate3, GUI_COUNTOF(_aDialogCreate3), _cbDialog3, WM_HBKWIN, 0, 0);
        break;
      }
      break;
    }
    break;
  default:
    WM_DefaultProc(pMsg);
    break;
  }
}

/*********************************************************************
*
*       _cbDialog2
*/
static void _cbDialog2(WM_MESSAGE * pMsg) {
  WM_HWIN hItem;
  int     NCode;
  int     Id;

  switch (pMsg->MsgId) {
  case WM_INIT_DIALOG:
    //
    // Initialization of 'Framewin2'
    //
    hItem = pMsg->hWin;
    FRAMEWIN_SetText(hItem, "Screen 2");
    FRAMEWIN_SetFont(hItem, GUI_FONT_32_ASCII);
    //
    // Initialization of 'Button'
    //
    hItem = WM_GetDialogItem(pMsg->hWin, ID_BUTTON_0);
    BUTTON_SetText(hItem, "Screen 1");
    //
    // Initialization of 'Button'
    //
    hItem = WM_GetDialogItem(pMsg->hWin, ID_BUTTON_1);
    BUTTON_SetText(hItem, "Screen 3");
    //
    // Initialization of 'Radio'
    //
    hItem = WM_GetDialogItem(pMsg->hWin, ID_RADIO_0);
    RADIO_SetText(hItem, "Item 1", 0);
    RADIO_SetText(hItem, "Item 2", 1);
    RADIO_SetText(hItem, "Item 3", 2);
    break;
  case WM_NOTIFY_PARENT:
    Id    = WM_GetId(pMsg->hWinSrc);
    NCode = pMsg->Data.v;
    switch(Id) {
    case ID_BUTTON_0: // Notifications sent by 'Button'
      switch(NCode) {
      case WM_NOTIFICATION_CLICKED:
        // USER START (Optionally insert code for reacting on notification message)
        // USER END
        break;
      case WM_NOTIFICATION_RELEASED:
        GUI_EndDialog(pMsg->hWin, 0);
        GUI_CreateDialogBox(_aDialogCreate1, GUI_COUNTOF(_aDialogCreate1), _cbDialog1, WM_HBKWIN, 0, 0);
        break;
      }
      break;
    case ID_BUTTON_1: // Notifications sent by 'Button'
      switch(NCode) {
      case WM_NOTIFICATION_CLICKED:
        // USER START (Optionally insert code for reacting on notification message)
        // USER END
        break;
      case WM_NOTIFICATION_RELEASED:
        GUI_EndDialog(pMsg->hWin, 0);
        GUI_CreateDialogBox(_aDialogCreate3, GUI_COUNTOF(_aDialogCreate3), _cbDialog3, WM_HBKWIN, 0, 0);
        break;
      }
      break;
    case ID_RADIO_0: // Notifications sent by 'Radio'
      switch(NCode) {
      case WM_NOTIFICATION_CLICKED:
        // USER START (Optionally insert code for reacting on notification message)
        // USER END
        break;
      case WM_NOTIFICATION_RELEASED:
        // USER START (Optionally insert code for reacting on notification message)
        // USER END
        break;
      case WM_NOTIFICATION_VALUE_CHANGED:
        // USER START (Optionally insert code for reacting on notification message)
        // USER END
        break;
      }
      break;
    }
    break;
  default:
    WM_DefaultProc(pMsg);
    break;
  }
}

/*********************************************************************
*
*       _cbDialog3
*/
static void _cbDialog3(WM_MESSAGE * pMsg) {
  WM_HWIN hItem;
  int     NCode;
  int     Id;

  switch (pMsg->MsgId) {
  case WM_INIT_DIALOG:
    //
    // Initialization of 'Framewin3'
    //
    hItem = pMsg->hWin;
    FRAMEWIN_SetText(hItem, "Screen 3");
    FRAMEWIN_SetFont(hItem, GUI_FONT_32_ASCII);
    //
    // Initialization of 'Button'
    //
    hItem = WM_GetDialogItem(pMsg->hWin, ID_BUTTON_0);
    BUTTON_SetText(hItem, "Screen 1");
    //
    // Initialization of 'Button'
    //
    hItem = WM_GetDialogItem(pMsg->hWin, ID_BUTTON_1);
    BUTTON_SetText(hItem, "Screen 2");
    //
    // Initialization of 'Checkbox'
    //
    hItem = WM_GetDialogItem(pMsg->hWin, ID_CHECKBOX_0);
    CHECKBOX_SetText(hItem, "Check me...");
    break;
  case WM_NOTIFY_PARENT:
    Id    = WM_GetId(pMsg->hWinSrc);
    NCode = pMsg->Data.v;
    switch(Id) {
    case ID_BUTTON_0: // Notifications sent by 'Button'
      switch(NCode) {
      case WM_NOTIFICATION_CLICKED:
        // USER START (Optionally insert code for reacting on notification message)
        // USER END
        break;
      case WM_NOTIFICATION_RELEASED:
        GUI_EndDialog(pMsg->hWin, 0);
        GUI_CreateDialogBox(_aDialogCreate1, GUI_COUNTOF(_aDialogCreate1), _cbDialog1, WM_HBKWIN, 0, 0);
        break;
      }
      break;
    case ID_BUTTON_1: // Notifications sent by 'Button'
      switch(NCode) {
      case WM_NOTIFICATION_CLICKED:
        // USER START (Optionally insert code for reacting on notification message)
        // USER END
        break;
      case WM_NOTIFICATION_RELEASED:
        GUI_EndDialog(pMsg->hWin, 0);
        GUI_CreateDialogBox(_aDialogCreate2, GUI_COUNTOF(_aDialogCreate2), _cbDialog2, WM_HBKWIN, 0, 0);
        break;
      }
      break;
    case ID_CHECKBOX_0: // Notifications sent by 'Checkbox'
      switch(NCode) {
      case WM_NOTIFICATION_CLICKED:
        // USER START (Optionally insert code for reacting on notification message)
        // USER END
        break;
      case WM_NOTIFICATION_RELEASED:
        // USER START (Optionally insert code for reacting on notification message)
        // USER END
        break;
      case WM_NOTIFICATION_VALUE_CHANGED:
        // USER START (Optionally insert code for reacting on notification message)
        // USER END
        break;
      }
      break;
    }
    break;
  default:
    WM_DefaultProc(pMsg);
    break;
  }
}

/*********************************************************************
*
*       CreateFramewin1
*/
static WM_HWIN CreateFramewin1(void) {
  WM_HWIN hWin;

  hWin = GUI_CreateDialogBox(_aDialogCreate1, GUI_COUNTOF(_aDialogCreate1), _cbDialog1, WM_HBKWIN, 0, 0);
  return hWin;
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       MainTask
*/
void MainTask(void) {
  #if GUI_SUPPORT_MEMDEV
    WM_SetCreateFlags(WM_CF_MEMDEV);
  #endif
  GUI_Init();
  //
  // Enable new skin
  //
  FRAMEWIN_SetDefaultSkin(FRAMEWIN_SKIN_FLEX);
  BUTTON_SetDefaultSkin(BUTTON_SKIN_FLEX);
  CHECKBOX_SetDefaultSkin(CHECKBOX_SKIN_FLEX);
  RADIO_SetDefaultSkin(RADIO_SKIN_FLEX);
  //
  // Create main menu
  //
  CreateFramewin1();
  while (1) {
    GUI_Delay(100);
  }
}

/*************************** End of file ****************************/
