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
File        : WIDGET_SimpleProgbar.c
Purpose     : Demonstrates the use of the PROGBAR widget
Requirements: WindowManager - (x)
              MemoryDevices - ( )
              AntiAliasing  - ( )
              VNC-Server    - ( )
              PNG-Library   - ( )
              TrueTypeFonts - ( )
----------------------------------------------------------------------
*/

#include "GUI.h"
#include "PROGBAR.h"

/*******************************************************************
*
*       static code
*
********************************************************************
*/
/*******************************************************************
*
*        _DemoProgBar
*/
static void _DemoProgBar(void) {
  PROGBAR_Handle ahProgBar;
  int i;

  GUI_SetFont(&GUI_Font8x16);
  GUI_DispStringAt("Progress bar", 100,80);
  /* Create progress bar */  
  ahProgBar = PROGBAR_Create(100, 100, 100, 20, WM_CF_SHOW);
  GUI_Delay (500);
  /* Modify progress bar */
  for (i = 0; i <= 100; i++) {
    PROGBAR_SetValue(ahProgBar, i);
    GUI_Delay(10);
  }
  GUI_Delay (400);
  /* Delete progress bar */  
  PROGBAR_Delete(ahProgBar);
  GUI_ClearRect(0, 50, 319, 239);
  GUI_Delay(750);
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
  GUI_SetBkColor(GUI_BLACK);
  GUI_Clear();
  GUI_SetColor(GUI_WHITE);
  GUI_SetFont(&GUI_Font24_ASCII);
  GUI_DispStringHCenterAt("WIDGET_SimpleProgbar - Sample", 160, 5);
  while(1) {
    _DemoProgBar();
  }
}

/*************************** End of file ****************************/

