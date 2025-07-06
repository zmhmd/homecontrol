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
File        : 320_Overview_ArrowDown_2.c
Content     : Bitmap 4 * 12
---------------------------END-OF-HEADER------------------------------
*/

#include <stdlib.h>

#include "GUI.h"

#ifndef GUI_CONST_STORAGE
  #define GUI_CONST_STORAGE const
#endif


static GUI_CONST_STORAGE unsigned short ac320_Overview_ArrowDown_2[] = {
  0x071F, 0x071F, 0x071F, 0x071F,
  0x071F, 0x071F, 0x071F, 0x071F,
  0x071F, 0x071F, 0x071F, 0x071F,
  0x071F, 0x071F, 0x071F, 0x071F,
  0x071F, 0x071F, 0x071F, 0x071F,
  0x16DC, 0x0000, 0x10A6, 0x071F,
  0x16DC, 0x0000, 0x10A6, 0x071F,
  0x16DC, 0x0000, 0x10A6, 0x071F,
  0x1D4B, 0x0000, 0x0421, 0x1DD1,
  0x1678, 0x0000, 0x0843, 0x0EFD,
  0x071F, 0x110A, 0x15D2, 0x071F,
  0x071F, 0x1678, 0x1678, 0x071F
};

extern GUI_CONST_STORAGE GUI_BITMAP bm320_Overview_ArrowDown_2;

GUI_CONST_STORAGE GUI_BITMAP bm320_Overview_ArrowDown_2 = {
  4, /* XSize */
  12, /* YSize */
  8, /* BytesPerLine */
  16, /* BitsPerPixel */
  (unsigned char *)ac320_Overview_ArrowDown_2,  /* Pointer to picture data */
  NULL  /* Pointer to palette */
 ,GUI_DRAW_BMP555
};

/*************************** End of file ****************************/
