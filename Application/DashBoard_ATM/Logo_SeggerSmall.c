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
File        : Logo_SeggerSmall.c
Purpose     : Bitmap file
---------------------------END-OF-HEADER------------------------------
*/

#include <stdlib.h>

#include "GUI.h"

#ifndef GUI_CONST_STORAGE
  #define GUI_CONST_STORAGE const
#endif

/*   Palette
The following are the entries of the palette table.
Every entry is a 32-bit value (of which 24 bits are actually used)
the lower   8 bits represent the Red component,
the middle  8 bits represent the Green component,
the highest 8 bits (of the 24 bits used) represent the Blue component
as follows:   0xBBGGRR
*/

static GUI_CONST_STORAGE GUI_COLOR ColorsSeggerLogoSmall[] = {
     0x0000FF,0xCD3941,0x941010,0x8B0000
    ,0xA43131,0xD59C9C,0xD5A4A4,0xFFFFFF
    ,0xFFF6F6,0xFF9C9C,0xFFE6E6,0xFFACAC
    ,0xFFD5D5,0xD5ACAC,0xFF0000,0xFF5A5A
    ,0xFFD5CD,0xFF0808,0xFF1820,0xFFDEDE
    ,0xFF2929,0xFF6A6A,0xFF7373,0xFFEEEE
    ,0xFFB4B4,0xFF2020,0xFF4141,0xFF9494
    ,0xF68B8B,0xFF3131,0xFFC5C5,0xFF6262
    ,0xFF8B8B,0xFF1010,0xFF524A,0xFF3939
    ,0xFF4A4A,0xF67B7B,0xFFE6DE,0xFFC5CD
    ,0xFF1818,0xFFBDBD,0xFF6A73,0xFF7B7B
    ,0xFF414A,0xFF949C,0xFF8383,0xFF5252
    ,0xFF3941,0xF66A6A,0xF6C5C5,0xFFA4A4
    ,0xFF1008,0xFF838B,0xFF2931,0xFFCDCD
    ,0xF63941,0xF6F6F6,0xEEEEEE,0xEEEEF6
    ,0xD5D5D5,0x838383,0x6A6A6A,0x737373
    ,0x7B7B7B,0xB4B4B4,0x949494,0x9C949C
    ,0xF6F6FF,0xBDBDBD,0x736A73,0x7B8383
    ,0x8B8B8B,0x9C9C9C,0xA4A4A4,0xACACAC
    ,0x313131,0x4A4A4A,0x737B73,0x414141
    ,0x181818,0xDEDEDE,0x9CA49C,0x202020
    ,0x101010,0xACA4AC,0x5A5A5A,0xCDCDCD
    ,0x292929,0xE6E6EE,0xC5CDCD,0xC5C5C5
    ,0x080808,0xE6E6E6,0xD5D5DE,0xFF0008
    ,0x393939,0x525252,0x000000,0x4A414A
    ,0x394141,0x313139,0x94949C,0xA4A4AC
    ,0x626262,0x202929,0x5A5A62,0x293131
    ,0x292931,0xCDD5CD,0x737B7B,0x83838B
    ,0xDEE6DE,0x8B9494,0xC5C5CD,0x7B7B83
    ,0xACACB4,0x940808,0xB41820,0xB45A62
    ,0xB46262,0xB42020
};

static GUI_CONST_STORAGE GUI_LOGPALETTE PalSeggerLogoSmall = {
  122,	/* number of entries */
  0, 	/* No transparency */
  &ColorsSeggerLogoSmall[0]
};

static GUI_CONST_STORAGE unsigned char acSeggerLogoSmall[] = {
  0x00, 0x01, 0x02, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 
        0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x02, 0x01, 0x00,
  0x01, 0x04, 0x05, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 
        0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x04, 0x01,
  0x02, 0x05, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 
        0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x06, 0x02,
  0x03, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x07, 0x07, 0x07, 0x07, 0x0B, 0x0C, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 
        0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x0D, 0x03,
  0x03, 0x06, 0x07, 0x09, 0x0E, 0x0F, 0x08, 0x07, 0x07, 0x10, 0x11, 0x12, 0x13, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 
        0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x0D, 0x03,
  0x03, 0x06, 0x07, 0x0A, 0x14, 0x11, 0x09, 0x07, 0x07, 0x08, 0x15, 0x0E, 0x16, 0x17, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 
        0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x0D, 0x03,
  0x03, 0x06, 0x07, 0x07, 0x18, 0x19, 0x19, 0x18, 0x07, 0x07, 0x13, 0x1A, 0x11, 0x1B, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 
        0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x0D, 0x03,
  0x03, 0x06, 0x07, 0x07, 0x08, 0x1C, 0x0E, 0x1D, 0x08, 0x07, 0x07, 0x1E, 0x0E, 0x19, 0x0C, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 
        0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x0D, 0x03,
  0x03, 0x06, 0x07, 0x07, 0x07, 0x07, 0x1F, 0x0E, 0x15, 0x17, 0x07, 0x07, 0x20, 0x21, 0x22, 0x0A, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 
        0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x0D, 0x03,
  0x03, 0x06, 0x08, 0x17, 0x07, 0x07, 0x0C, 0x23, 0x21, 0x1B, 0x07, 0x07, 0x0A, 0x24, 0x11, 0x25, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 
        0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x0D, 0x03,
  0x03, 0x06, 0x26, 0x1F, 0x0A, 0x07, 0x07, 0x27, 0x21, 0x28, 0x29, 0x07, 0x07, 0x0C, 0x19, 0x11, 0x18, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 
        0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x0D, 0x03,
  0x03, 0x06, 0x13, 0x23, 0x2A, 0x17, 0x07, 0x07, 0x2B, 0x11, 0x2C, 0x13, 0x07, 0x07, 0x2D, 0x21, 0x1A, 0x13, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 
        0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x0D, 0x03,
  0x03, 0x06, 0x13, 0x23, 0x11, 0x2E, 0x07, 0x07, 0x08, 0x15, 0x0E, 0x2F, 0x08, 0x07, 0x08, 0x2B, 0x0E, 0x30, 0x31, 0x31, 0x31, 0x31, 0x31, 0x31, 0x31, 0x31, 0x31, 0x31, 0x31, 0x31, 0x31, 0x31, 0x31, 0x31, 0x31, 0x31, 0x31, 0x31, 0x31, 0x31, 0x31, 0x31, 
        0x31, 0x31, 0x31, 0x31, 0x31, 0x31, 0x31, 0x31, 0x31, 0x31, 0x31, 0x31, 0x31, 0x31, 0x31, 0x15, 0x29, 0x07, 0x0D, 0x03,
  0x03, 0x06, 0x13, 0x23, 0x0E, 0x28, 0x32, 0x07, 0x07, 0x13, 0x1D, 0x11, 0x09, 0x08, 0x07, 0x0A, 0x14, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 
        0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x15, 0x07, 0x0D, 0x03,
  0x03, 0x06, 0x13, 0x23, 0x0E, 0x0E, 0x2F, 0x17, 0x07, 0x07, 0x33, 0x34, 0x23, 0x13, 0x07, 0x07, 0x1E, 0x20, 0x35, 0x35, 0x35, 0x35, 0x35, 0x35, 0x35, 0x35, 0x35, 0x35, 0x35, 0x35, 0x35, 0x35, 0x35, 0x35, 0x35, 0x35, 0x35, 0x35, 0x35, 0x35, 0x35, 0x35, 
        0x35, 0x35, 0x35, 0x35, 0x35, 0x35, 0x35, 0x35, 0x35, 0x35, 0x35, 0x35, 0x35, 0x35, 0x35, 0x35, 0x0C, 0x07, 0x0D, 0x03,
  0x03, 0x06, 0x13, 0x23, 0x0E, 0x0E, 0x1C, 0x08, 0x07, 0x07, 0x1F, 0x0E, 0x1F, 0x17, 0x07, 0x07, 0x16, 0x1D, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 
        0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x20, 0x07, 0x0D, 0x03,
  0x03, 0x06, 0x13, 0x23, 0x0E, 0x1F, 0x07, 0x07, 0x08, 0x1C, 0x0E, 0x1D, 0x08, 0x07, 0x08, 0x1B, 0x0E, 0x19, 0x23, 0x23, 0x23, 0x23, 0x23, 0x23, 0x23, 0x23, 0x23, 0x23, 0x23, 0x23, 0x23, 0x23, 0x23, 0x23, 0x23, 0x23, 0x23, 0x23, 0x23, 0x23, 0x23, 0x23, 
        0x23, 0x23, 0x23, 0x23, 0x23, 0x23, 0x23, 0x23, 0x23, 0x23, 0x23, 0x23, 0x23, 0x23, 0x23, 0x23, 0x2D, 0x07, 0x0D, 0x03,
  0x03, 0x06, 0x13, 0x23, 0x23, 0x0C, 0x07, 0x07, 0x18, 0x19, 0x19, 0x18, 0x07, 0x07, 0x37, 0x1D, 0x19, 0x0B, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 
        0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x08, 0x07, 0x0D, 0x03,
  0x03, 0x06, 0x26, 0x24, 0x27, 0x07, 0x07, 0x0A, 0x14, 0x11, 0x09, 0x07, 0x07, 0x17, 0x38, 0x0E, 0x1B, 0x08, 0x07, 0x39, 0x3A, 0x39, 0x07, 0x07, 0x07, 0x39, 0x39, 0x39, 0x39, 0x07, 0x07, 0x07, 0x39, 0x39, 0x39, 0x07, 0x07, 0x07, 0x07, 0x07, 0x39, 0x3B, 
        0x39, 0x07, 0x07, 0x07, 0x07, 0x39, 0x39, 0x39, 0x07, 0x07, 0x07, 0x39, 0x39, 0x39, 0x07, 0x07, 0x07, 0x07, 0x0D, 0x03,
  0x03, 0x06, 0x17, 0x1E, 0x07, 0x07, 0x17, 0x2A, 0x0E, 0x0F, 0x08, 0x07, 0x08, 0x2E, 0x0E, 0x38, 0x08, 0x07, 0x3C, 0x3D, 0x3E, 0x3F, 0x3C, 0x07, 0x3C, 0x3D, 0x40, 0x40, 0x40, 0x41, 0x07, 0x39, 0x42, 0x3E, 0x3E, 0x43, 0x07, 0x07, 0x44, 0x45, 0x46, 0x3E, 
        0x47, 0x3C, 0x07, 0x3A, 0x48, 0x40, 0x40, 0x40, 0x49, 0x39, 0x4A, 0x40, 0x40, 0x40, 0x4B, 0x3A, 0x07, 0x07, 0x0D, 0x03,
  0x03, 0x06, 0x07, 0x07, 0x07, 0x07, 0x2E, 0x11, 0x24, 0x13, 0x07, 0x07, 0x33, 0x28, 0x23, 0x0C, 0x07, 0x3A, 0x4C, 0x4D, 0x41, 0x4E, 0x4F, 0x3C, 0x42, 0x50, 0x46, 0x40, 0x40, 0x41, 0x51, 0x4F, 0x4F, 0x52, 0x3D, 0x53, 0x49, 0x39, 0x42, 0x54, 0x40, 0x55, 
        0x56, 0x53, 0x39, 0x57, 0x50, 0x56, 0x40, 0x40, 0x4A, 0x39, 0x58, 0x4F, 0x40, 0x3F, 0x54, 0x40, 0x07, 0x07, 0x0D, 0x03,
  0x03, 0x06, 0x07, 0x07, 0x07, 0x32, 0x28, 0x21, 0x1E, 0x07, 0x07, 0x17, 0x14, 0x11, 0x33, 0x07, 0x07, 0x59, 0x4C, 0x4F, 0x4B, 0x57, 0x5A, 0x39, 0x42, 0x53, 0x4B, 0x45, 0x5B, 0x3A, 0x4B, 0x5C, 0x5B, 0x07, 0x39, 0x5B, 0x51, 0x3A, 0x4F, 0x4D, 0x39, 0x07, 
        0x5D, 0x45, 0x07, 0x57, 0x50, 0x47, 0x45, 0x45, 0x5D, 0x07, 0x58, 0x3F, 0x5D, 0x5E, 0x53, 0x3E, 0x07, 0x07, 0x0D, 0x03,
  0x03, 0x06, 0x07, 0x07, 0x0C, 0x23, 0x21, 0x1B, 0x07, 0x07, 0x0A, 0x1F, 0x5F, 0x15, 0x07, 0x07, 0x07, 0x07, 0x41, 0x56, 0x60, 0x58, 0x61, 0x3C, 0x42, 0x54, 0x4F, 0x4F, 0x61, 0x5B, 0x48, 0x62, 0x3A, 0x39, 0x3E, 0x63, 0x3E, 0x3C, 0x58, 0x40, 0x07, 0x45, 
        0x4D, 0x60, 0x41, 0x57, 0x54, 0x4C, 0x4D, 0x4D, 0x4B, 0x07, 0x58, 0x53, 0x64, 0x65, 0x50, 0x45, 0x07, 0x07, 0x0D, 0x03,
  0x03, 0x06, 0x07, 0x07, 0x0F, 0x5F, 0x1F, 0x17, 0x07, 0x07, 0x33, 0x21, 0x30, 0x0C, 0x07, 0x07, 0x07, 0x39, 0x42, 0x57, 0x51, 0x49, 0x50, 0x42, 0x42, 0x58, 0x51, 0x3A, 0x39, 0x07, 0x4B, 0x5C, 0x57, 0x44, 0x45, 0x3E, 0x4C, 0x57, 0x4F, 0x4D, 0x07, 0x5D, 
        0x66, 0x53, 0x48, 0x57, 0x53, 0x67, 0x3A, 0x3A, 0x07, 0x07, 0x58, 0x68, 0x5B, 0x41, 0x5C, 0x42, 0x07, 0x07, 0x0D, 0x03,
  0x03, 0x06, 0x07, 0x0B, 0x0E, 0x23, 0x17, 0x07, 0x07, 0x0A, 0x19, 0x21, 0x32, 0x07, 0x07, 0x07, 0x07, 0x39, 0x56, 0x69, 0x40, 0x56, 0x4C, 0x41, 0x49, 0x50, 0x4F, 0x63, 0x63, 0x4E, 0x5D, 0x3F, 0x58, 0x6A, 0x56, 0x6B, 0x4C, 0x3C, 0x45, 0x6C, 0x4D, 0x3E, 
        0x4F, 0x50, 0x48, 0x6D, 0x50, 0x60, 0x63, 0x63, 0x3E, 0x5D, 0x53, 0x3D, 0x07, 0x07, 0x53, 0x6E, 0x07, 0x07, 0x0D, 0x03,
  0x03, 0x06, 0x07, 0x37, 0x1A, 0x18, 0x07, 0x07, 0x07, 0x17, 0x15, 0x2E, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x51, 0x49, 0x3D, 0x6F, 0x5B, 0x39, 0x70, 0x4A, 0x42, 0x42, 0x42, 0x41, 0x39, 0x3A, 0x4A, 0x3D, 0x71, 0x72, 0x4A, 0x3A, 0x39, 0x57, 0x48, 0x73, 
        0x4B, 0x4B, 0x57, 0x3A, 0x4B, 0x42, 0x42, 0x42, 0x4B, 0x3A, 0x4A, 0x3C, 0x07, 0x07, 0x74, 0x5A, 0x07, 0x07, 0x0D, 0x03,
  0x75, 0x05, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 
        0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x06, 0x75,
  0x76, 0x77, 0x17, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 
        0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x78, 0x76,
  0x00, 0x79, 0x75, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 
        0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x75, 0x79, 0x00
};

extern GUI_CONST_STORAGE GUI_BITMAP bmSeggerLogoSmall;

GUI_CONST_STORAGE GUI_BITMAP bmSeggerLogoSmall = {
  62, /* XSize */
  29, /* YSize */
  62, /* BytesPerLine */
  8, /* BitsPerPixel */
  acSeggerLogoSmall,  /* Pointer to picture data (indices) */
  &PalSeggerLogoSmall  /* Pointer to palette */
};

/* *** End of file *** */
