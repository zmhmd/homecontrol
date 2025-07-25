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
File        : MOTION_MovingWindow.c
Purpose     : Shows how to use simple motion support
Requirements: WindowManager - (x)
              MemoryDevices - (x)
              AntiAliasing  - ( )
              VNC-Server    - ( )
              PNG-Library   - ( )
              TrueTypeFonts - ( )
----------------------------------------------------------------------
*/

/*********************************************************************
*
*       Includes
*
**********************************************************************
*/
#include "WM.h"

/*********************************************************************
*
*       Types
*
**********************************************************************
*/
typedef struct {
  int Axis;
  int Speed;
} PARA;

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
static const PARA _aPara[] = {
  { GUI_COORD_X, +100 },
  { GUI_COORD_Y, +100 },
  { GUI_COORD_X, -100 },
  { GUI_COORD_Y, -100 },
};

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/
/*********************************************************************
*
*       _cbWin0
*/
static void _cbWin0(WM_MESSAGE * pMsg) {
  int xSize, ySize;

  switch (pMsg->MsgId) {
  case WM_PAINT:
    xSize = LCD_GetXSize();
    ySize = LCD_GetYSize();
    GUI_DrawGradientV(0, 0, xSize, ySize, GUI_BLUE, GUI_BLACK);
    GUI_SetColor(GUI_WHITE);
    GUI_SetFont(&GUI_Font24_ASCII);
    GUI_SetTextMode(GUI_TM_TRANS);
    GUI_DispStringHCenterAt("WM_Motion - Sample", 160, 5);
    break;
  default:
    WM_DefaultProc(pMsg);
    break;
  }
}

/*********************************************************************
*
*       _cbWin1
*/
static void _cbWin1(WM_MESSAGE * pMsg) {
  int xSize, ySize;

  switch (pMsg->MsgId) {
  case WM_PAINT:
    xSize = WM_GetWindowSizeX(pMsg->hWin);
    ySize = WM_GetWindowSizeY(pMsg->hWin);
    GUI_DrawGradientV(0, 0, xSize, ySize, GUI_DARKGREEN, GUI_DARKRED);
    GUI_SetTextMode(GUI_TM_TRANS);
    GUI_DispStringHCenterAt("I am a window", xSize >> 1, 5);
    GUI_DispStringHCenterAt("Try to catch me", xSize >> 1, 15);
    GUI_DispStringAt("xPos: ", 20, 30);
    GUI_DispDec(WM_GetWindowOrgX(pMsg->hWin), 3);
    GUI_DispStringAt("yPos: ", 20, 40);
    GUI_DispDec(WM_GetWindowOrgY(pMsg->hWin), 3);
    GUI_SetColor(GUI_GRAY);
    GUI_DrawRect(0, 0, xSize - 1, ySize - 1);
    break;
  default:
    WM_DefaultProc(pMsg);
    break;
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
  WM_HWIN  hWin0;
  WM_HWIN  hWin1;
  int      xSize, ySize;
  unsigned i;

  WM_SetCreateFlags(WM_CF_MEMDEV);
  GUI_Init();
  WM_MOTION_Enable(1);
  xSize = LCD_GetXSize();
  ySize = LCD_GetYSize();
  hWin0 = WM_CreateWindowAsChild(0, 0, xSize, ySize, WM_HBKWIN, WM_CF_SHOW, _cbWin0, 0);
  hWin1 = WM_CreateWindowAsChild(10, 30, 120, 60, hWin0, WM_CF_SHOW | WM_CF_MOTION_X | WM_CF_MOTION_Y, _cbWin1, 0);
  while (1) {
    for (i = 0; i < GUI_COUNTOF(_aPara); i++) {
      WM_MOTION_SetSpeed(hWin1, _aPara[i].Axis, _aPara[i].Speed);
      GUI_Delay(1000);
      WM_MOTION_SetDeceleration(hWin1, _aPara[i].Axis, 100);
      GUI_Delay(1000);
    }
  }
}

/*************************** End of file ****************************/
