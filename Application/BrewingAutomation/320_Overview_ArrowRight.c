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
File        : 320_Overview_ArrowRight.c
Content     : Bitmap 24 * 6
---------------------------END-OF-HEADER------------------------------
*/

#include <stdlib.h>

#include "GUI.h"

#ifndef GUI_CONST_STORAGE
  #define GUI_CONST_STORAGE const
#endif


static GUI_CONST_STORAGE unsigned short ac320_Overview_ArrowRight[] = {
  0x071F, 0x071F, 0x071F, 0x071F, 0x071F, 0x071F, 0x071F, 0x071F, 0x071F, 0x071F, 0x071F, 0x071F, 0x071F, 0x071F, 0x071F, 0x071F, 0x071F, 0x071F, 0x1DAF, 0x15F2, 0x0B1F, 0x071F, 0x071F, 0x071F,
  0x071F, 0x071F, 0x071F, 0x071F, 0x071F, 0x071F, 0x071F, 0x071F, 0x071F, 0x071F, 0x071F, 0x071F, 0x0DF2, 0x018F, 0x018F, 0x018F, 0x018F, 0x018F, 0x08A6, 0x0000, 0x0CA6, 0x1678, 0x071F, 0x071F,
  0x071F, 0x071F, 0x071F, 0x071F, 0x071F, 0x071F, 0x071F, 0x071F, 0x071F, 0x071F, 0x071F, 0x071F, 0x10A6, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0421, 0x0000, 0x110A, 0x12DB,
  0x071F, 0x071F, 0x071F, 0x071F, 0x071F, 0x071F, 0x071F, 0x071F, 0x071F, 0x071F, 0x071F, 0x071F, 0x10A6, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0421, 0x0000, 0x114C, 0x0EDC,
  0x071F, 0x071F, 0x071F, 0x071F, 0x071F, 0x071F, 0x071F, 0x071F, 0x071F, 0x071F, 0x071F, 0x071F, 0x1677, 0x1656, 0x1656, 0x1656, 0x1656, 0x1656, 0x10E8, 0x0000, 0x10E8, 0x1699, 0x071F, 0x071F,
  0x071F, 0x071F, 0x071F, 0x071F, 0x071F, 0x071F, 0x071F, 0x071F, 0x071F, 0x071F, 0x071F, 0x071F, 0x071F, 0x071F, 0x071F, 0x071F, 0x071F, 0x071F, 0x1DB0, 0x1634, 0x071F, 0x071F, 0x071F, 0x071F
};

extern GUI_CONST_STORAGE GUI_BITMAP bm320_Overview_ArrowRight;

GUI_CONST_STORAGE GUI_BITMAP bm320_Overview_ArrowRight = {
  24, /* XSize */
  6, /* YSize */
  48, /* BytesPerLine */
  16, /* BitsPerPixel */
  (unsigned char *)ac320_Overview_ArrowRight,  /* Pointer to picture data */
  NULL  /* Pointer to palette */
 ,GUI_DRAW_BMP555
};

/*************************** End of file ****************************/
