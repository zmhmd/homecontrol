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
File        : MEMDEV_Banding.c
Purpose     : Example demonstrating the use of banding memory devices
Requirements: WindowManager - ( )
              MemoryDevices - (x)
              AntiAliasing  - ( )
              VNC-Server    - ( )
              PNG-Library   - ( )
              TrueTypeFonts - ( )
----------------------------------------------------------------------
*/

#include "GUI.h"

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/
#define USE_BANDING_MEMDEV (1) /* Set to 0 for drawing without banding memory device */
#define SIZE_OF_ARRAY(Array) (sizeof(Array) / sizeof(Array[0]))

/*******************************************************************
*
*       static variables
*
********************************************************************
*/

static const GUI_POINT aPoints[] = {
  {-50,  0},
  {-10, 10},
  {  0, 50},
  { 10, 10},
  { 50,  0},
  { 10,-10},
  {  0,-50},
  {-10,-10}
};

typedef struct {
  int XPos_Poly;
  int YPos_Poly;
  int XPos_Text;
  int YPos_Text;
  GUI_POINT aPointsDest[8];
} tDrawItContext;

/*******************************************************************
*
*       static code
*
********************************************************************
*/

/*******************************************************************
*
*       _DrawIt
*/
static void _DrawIt(void * pData) {
  tDrawItContext * pDrawItContext = (tDrawItContext *)pData;
  GUI_Clear();
  GUI_SetFont(&GUI_Font8x8);
  GUI_SetTextMode(GUI_TM_TRANS);
  //
  // draw background
  //
  GUI_SetColor(GUI_GREEN);
  GUI_FillRect(pDrawItContext->XPos_Text, 
               pDrawItContext->YPos_Text - 25,
               pDrawItContext->XPos_Text + 100,
               pDrawItContext->YPos_Text - 5);
  //
  // draw polygon
  //
  GUI_SetColor(GUI_BLUE);
  GUI_FillPolygon(pDrawItContext->aPointsDest, SIZE_OF_ARRAY(aPoints), 160, 120);
  //
  // draw foreground
  //
  GUI_SetColor(GUI_RED);
  GUI_FillRect(220 - pDrawItContext->XPos_Text, 
               pDrawItContext->YPos_Text + 5,
               220 - pDrawItContext->XPos_Text + 100,
               pDrawItContext->YPos_Text + 25);
}

/*******************************************************************
*
*       _DemoBandingMemdev
*/
static void _DemoBandingMemdev(void) {
  tDrawItContext DrawItContext;
  int i, swap=0;
  GUI_SetBkColor(GUI_BLACK);
  GUI_Clear();
  GUI_SetColor(GUI_WHITE);
  GUI_SetFont(&GUI_Font24_ASCII);
  GUI_DispStringHCenterAt("MEMDEV_Banding - Sample", 160, 5);
  GUI_SetFont(&GUI_Font8x9);
  GUI_DispStringHCenterAt("Banding memory device\nwithout flickering", 160, 40);
  DrawItContext.XPos_Poly = 160;
  DrawItContext.YPos_Poly = 120;
  DrawItContext.YPos_Text = 116;
  while (1) {
    swap = ~swap;
    for (i = 0; i < 220; i++) {
      float angle = i * 3.1415926 / 55;
      DrawItContext.XPos_Text = (swap) ? i : 220 - i;
      /* Rotate the polygon */
      GUI_RotatePolygon(DrawItContext.aPointsDest, aPoints, 
                        SIZE_OF_ARRAY(aPoints), angle);
      #if USE_BANDING_MEMDEV
      {
        GUI_RECT Rect = {0, 70, 320,170};
        /* Use banding memory device for drawing */
        GUI_MEMDEV_Draw(&Rect, &_DrawIt, &DrawItContext, 0, 0);
      }
      #else
        /* Simple drawing without using memory devices */
        _DrawIt((void *)&DrawItContext);
      #endif
      #ifdef WIN32
        GUI_Delay(20); /* Use a short delay only in the simulation */
      #endif
    }
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
  _DemoBandingMemdev();
}

/*************************** End of file ****************************/

