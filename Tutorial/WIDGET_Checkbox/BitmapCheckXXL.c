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
File        : BitmapBar.c
Purpose     : Sample bitmap for checkbox 16x16
----------------------------------------------------------------------
*/

#include "GUI.h"
#include "Main.h"

/* Colors */
static const GUI_COLOR _aColorDisabled[] = {0x808080, 0x101010};
static const GUI_COLOR _aColorEnabled[]  = {GUI_WHITE, GUI_BLACK};

/* Palettes */
static const GUI_LOGPALETTE _PalCheckDisabled = {
  2,	/* number of entries */
  1, 	/* transparency flag */
  _aColorDisabled
};

static const GUI_LOGPALETTE _PalCheckEnabled = {
  2,	/* number of entries */
  1, 	/* transparency flag */
  _aColorEnabled
};

/* Pixel data */
static const unsigned char _acXXLChecked[] = {
  ________, ________, ________,
  ________, ________, ________,
  ________, ________, ________,
  ________, ________, ________,
  ________, ________, __XX____,
  ________, ________, _XXX____,
  ________, ________, XXXX____,
  ________, _______X, XXXX____,
  ________, ______XX, XXX_____,
  ________, _____XXX, XX______,
  ________, ____XXXX, X_______,
  __XX____, ___XXXXX, ________,
  __XXX___, __XXXXX_, ________,
  __XXXX__, _XXXXX__, ________,
  __XXXXX_, XXXXX___, ________,
  ___XXXXX, XXXX____, ________,
  ____XXXX, XXX_____, ________,
  _____XXX, XX______, ________,
  ______XX, X_______, ________,
  ________, ________, ________,
  ________, ________, ________,
};

/* Bitmaps */
const GUI_BITMAP _abmXXL[2] = {
  { 22, 22, 3, 1, _acXXLChecked, &_PalCheckDisabled },
  { 22, 22, 3, 1, _acXXLChecked, &_PalCheckEnabled },
};

/*************************** End of file ****************************/
