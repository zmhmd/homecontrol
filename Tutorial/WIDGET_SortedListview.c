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
File        : WIDGET_SortedListview.c
Purpose     : Example demonstrating the use of a sorted LISTVIEW
Requirements: WindowManager - (x)
              MemoryDevices - ( )
              AntiAliasing  - ( )
              VNC-Server    - ( )
              PNG-Library   - ( )
              TrueTypeFonts - ( )
----------------------------------------------------------------------
*/

#include <string.h>
#include <stdlib.h>
#include "DIALOG.h"
#include "LISTVIEW.h"

/*********************************************************************
*
*       Types
*
**********************************************************************
*/
/*********************************************************************
*
*       Type for column properties
*/
typedef struct {
  char  *pText;
  int    Width;
  int    Align;
  int    (*fpCompare)(const void *p0, const void *p1);
} COL_PROP;

/*********************************************************************
*
*       Static Data
*
**********************************************************************
*/
/*********************************************************************
*
*       Dialog ressource
*/
static const GUI_WIDGET_CREATE_INFO _aDialogCreate[] = {
  { FRAMEWIN_CreateIndirect, "Sorted listview sample", 0,     10,  55, 300, 175 },
  { LISTVIEW_CreateIndirect, NULL,         GUI_ID_LISTVIEW0,  10,  10, 200, 140 },
  { BUTTON_CreateIndirect,   "Add row",    GUI_ID_BUTTON0,   220,  10,  65,  20 },
  { BUTTON_CreateIndirect,   "Insert row", GUI_ID_BUTTON1,   220,  35,  65,  20 },
  { BUTTON_CreateIndirect,   "Delete row", GUI_ID_BUTTON2,   220,  60,  65,  20 },
  { BUTTON_CreateIndirect,   "OK",         GUI_ID_OK,        220,  85,  65,  20 },
  { RADIO_CreateIndirect,    0,            GUI_ID_RADIO0,    220, 115,  70,  40, 0, 0x1002 },
};

/*********************************************************************
*
*       Array of column propperties
*/
static const COL_PROP _aColProps[] = {
  { "Name",    70, GUI_TA_LEFT,    LISTVIEW_CompareText},
  { "Code",    50, GUI_TA_HCENTER, LISTVIEW_CompareText},
  { "Balance", 60, GUI_TA_RIGHT,   LISTVIEW_CompareDec }
};

/*********************************************************************
*
*       Static routines
*
**********************************************************************
*/
/*********************************************************************
*
*       _IntToStr
*
* Function description
*   Converts an integer value into a string.
*/
static void _IntToStr(char * pBuffer, int Num) {
  char * p;
  char   c;
  
  if (Num < 0) {
    *pBuffer++ = '-';
    Num = -Num;
  }
  p = pBuffer;
  if (Num) {
    while (Num) {
      *pBuffer++ = '0' + (Num % 10);
      Num /= 10;
    }
  } else {
    *pBuffer++ = '0';
  }
  *pBuffer-- = 0;
  while (p < pBuffer) {
    c          = *p;
    *p++       = *pBuffer;
    *pBuffer-- = c;
  }
}

/*********************************************************************
*
*       _AddRow
*
* Function description
*   Inserts or adds a new row of data to the given LISTVIEW
*
* Parameter:
*   hItem  - Handle of LISTVIEW widget
*   Insert - 1 for inserting a row,0 for adding a row
*/
static int _AddRow(WM_HWIN hItem, int Insert) {
  int    i;
  int    r;
  int    Len;
  int    NumRows;
  char * apText[3];
  char   acText[3][10] = {{0}};
  char   acBuffer[6];
  
  //
  // Create contents of name field
  //
  strcpy(acText[0], "Name ");
  NumRows = LISTVIEW_GetNumRows(hItem);
  _IntToStr(acBuffer, NumRows);
  Len = strlen(acBuffer);
  for (i = Len; i < 4; i++) {
    strcat(acText[0], " ");
  }
  strcat(acText[0], acBuffer);
  //
  // Create contents of code field
  //
  for (i = 0; i < 5; i++) {
    acText[1][i] = rand() % 26 + 'A';
  }
  //
  // Create contents of balance field
  //
  _IntToStr(acText[2], ((rand() % 10000) - 5000));
  //
  // Fill pointer array
  //
  for (i = 0; i < 3; i++) {
    apText[i] = acText[i];
  }
  //
  // Add/Insert row
  //
  if (Insert) {
    r = LISTVIEW_InsertRow(hItem, 0, (const GUI_ConstString *)apText);
  } else {
    r = LISTVIEW_AddRow(hItem, (const GUI_ConstString *)apText);
  }
  return r;
}

/*********************************************************************
*
*       _cbDialog
*
* Function description
*   Callback routine of dialog
*/
static void _cbDialog(WM_MESSAGE * pMsg) {
  WM_HWIN hDlg;
  WM_HWIN hItem; 
  int     Id;
  int     NCode;
  int     i;

  hDlg = pMsg->hWin;
  switch (pMsg->MsgId) {
  case WM_INIT_DIALOG:
    //
    // Set listview properties
    //
    hItem = WM_GetDialogItem(hDlg, GUI_ID_LISTVIEW0);
    HEADER_SetDragLimit(LISTVIEW_GetHeader(hItem), 1);
    LISTVIEW_SetAutoScrollV(hItem, 1);
    LISTVIEW_SetGridVis(hItem, 1);
    LISTVIEW_SetRBorder(hItem, 5);
    LISTVIEW_SetLBorder(hItem, 5);
    for (i = 0; i < 3; i++) {
      LISTVIEW_AddColumn(hItem, _aColProps[i].Width, _aColProps[i].pText, _aColProps[i].Align);
      LISTVIEW_SetCompareFunc(hItem, i, _aColProps[i].fpCompare);
    }
    LISTVIEW_EnableSort(hItem);
    for (i = 0; i < 6; i++) {
      if (_AddRow(hItem, 1)) {
        break;
      }
    }
    //
    // Set radio button text
    //
    hItem = WM_GetDialogItem(hDlg, GUI_ID_RADIO0);
    RADIO_SetText(hItem, "Sorted",   0);
    RADIO_SetText(hItem, "Unsorted", 1);
    break;
  case WM_NOTIFY_PARENT:
    Id    = WM_GetId(pMsg->hWinSrc);
    NCode = pMsg->Data.v;
    switch (NCode) {
    case WM_NOTIFICATION_VALUE_CHANGED:
      switch (Id) {
      case GUI_ID_RADIO0:
        //
        // Enable/Disable sorting
        //
        hItem = WM_GetDialogItem(hDlg, GUI_ID_LISTVIEW0);
        switch (RADIO_GetValue(WM_GetDialogItem(hDlg, GUI_ID_RADIO0))) {
        case 0:
          LISTVIEW_EnableSort(hItem);
          break;
        default:
          LISTVIEW_DisableSort(hItem);
          break;
        }
        break;
      }
      break;
    case WM_NOTIFICATION_RELEASED:
      switch (Id) {
      case GUI_ID_BUTTON0:
        //
        // Add new row
        //
        hItem = WM_GetDialogItem(hDlg, GUI_ID_LISTVIEW0);
        _AddRow(hItem, 0);
        break;
      case GUI_ID_BUTTON1:
        //
        // Insert new row
        //
        hItem = WM_GetDialogItem(hDlg, GUI_ID_LISTVIEW0);
        i = LISTVIEW_GetSelUnsorted(hItem);
        _AddRow(hItem, 1);
        if (i >= 0) {
          LISTVIEW_SetSelUnsorted(hItem, i + 1);
        }
        break;
      case GUI_ID_BUTTON2:
        //
        // Delete row
        //
        hItem = WM_GetDialogItem(hDlg, GUI_ID_LISTVIEW0);
        i = LISTVIEW_GetSelUnsorted(hItem);
        LISTVIEW_DeleteRow(hItem, (i >= 0) ? i : 0);
        break;
      case GUI_ID_OK:
        //
        // End dialog
        //
        GUI_EndDialog(hDlg, 0);
        break;
      }
      break;
    }
    break;
  default:
    WM_DefaultProc(pMsg);
  }
}

/*********************************************************************
*
*       _cbBkWin
*
* Function description
*   Callback routine of desktop window
*/
static void _cbBkWin(WM_MESSAGE * pMsg) {
  switch (pMsg->MsgId) {
  case WM_PAINT:
    GUI_Clear();
    GUI_SetFont(&GUI_Font24_ASCII);
    GUI_DispStringHCenterAt("WIDGET_SortedListview - Sample", 160, 5);
    GUI_SetFont(&GUI_Font10_ASCII);
    GUI_DispStringHCenterAt("Please touch the header of the LISTVIEW for sorting...", 160, 35);
    break;
  default:
    WM_DefaultProc(pMsg);
  }
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
  GUI_Init();
  #if GUI_SUPPORT_MEMDEV
    WM_SetCreateFlags(WM_CF_MEMDEV);
  #endif
  GUI_CURSOR_Show();
  WM_SetCallback(WM_HBKWIN, _cbBkWin);
  while (1) {
    GUI_ExecDialogBox(_aDialogCreate, GUI_COUNTOF(_aDialogCreate), _cbDialog, 0, 0, 0);
    GUI_Delay(1000);
  }
}

/*************************** End of file ****************************/


