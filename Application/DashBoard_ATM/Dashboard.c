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
File        : Dashboard.c
Purpose     : dashboard demo
---------------------------END-OF-HEADER------------------------------
*/

#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "GUI.h"
#include "LCDConf.h"
#include "MULTIPAGE.h"
#include "FRAMEWIN.h"
#include "DROPDOWN.h"

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/

#define SHOW_RECTS  0

#define AA_FACTOR   3
#define HIRES       1

#if HIRES
  #define FACTOR        AA_FACTOR
  #define HIRES_ON();   GUI_AA_EnableHiRes();
  #define HIRES_OFF();  GUI_AA_DisableHiRes();
#else
  #define FACTOR        1
  #define HIRES_ON();
  #define HIRES_OFF();
#endif

#define FLAG_SHOW_MARK      0
#define FLAG_SHOW_PITCH     1
#define FLAG_SHOW_GRAD      2
#define FLAG_SHOW_ARC       3   // needs five bits (3 - 7)
#define FLAG_SHOW_TEXT      8
#define FLAG_SHOW_SCALE     9
#define FLAG_NEEDLE_FRAME  10
#define FLAG_NEEDLE_LINE   11

#define PI  3.1415926536

#define NEEDLE_GRAD 720

#define ARRAY(aItems) aItems, GUI_COUNTOF(aItems)

/*********************************************************************
*
*       Types
*
**********************************************************************
*/

typedef struct {
  int x;
  int y;
  int Pressed;
  int Duration;
} PID_EVENT;

typedef struct {
  int x;
  int y;
  int xHere;
  int yHere;
  int DirX;
  int DirY;
  int PPM;
  int Dif;
  int PrevTime;
  const GUI_BITMAP* pBitmap;
} NAVIMAP;

typedef struct {
  U8  Sep[4];
} COLOR;

typedef struct {
  int x;
  int y;
  int x0;
  int y0;
  int ArcStart;
  int ArcEnd;
  int ArcRadius;
  int ArcArea1;
  int ArcArea2;
  int NumMarkLines;
  int NumPitchLines;
  int LineLen1;
  int LineLen2;
  int ArcWidth;
  int GradDist;
  int PenSize1;
  int PenSize2;
  int PenSize3;
  int NeedleType;
  int NeedleRadius;
  int NumStep;
  int NumStart;
  int NumExp;
  int LinePos1;
  int LinePos2;
  int ArcPos;
  int AxisRadius;
  int TextDist;
  U16 Flags;
  WM_HWIN hWin;
  GUI_MEMDEV_Handle hMemDev;
  char acText[33];
  const GUI_BITMAP* pBitmap;
  int BitmapY;
  COLOR Color[7];
} SCALE;

typedef struct {
  int NeedlePos;
  int NeedleUPM;
  int NeedleDir;
  int NeedlePrevTime;
} NEEDLE;


static void _DrawScale(SCALE* pObj);


/*********************************************************************
*
*       static data
*
**********************************************************************
*/

static int      _InitDialog;
static unsigned _Break;

static WM_HWIN _hDialogColor;
static WM_HWIN _hDialogMark;
static WM_HWIN _hDialogPitch;
static WM_HWIN _hDialogArc;
static WM_HWIN _hDialogGrad;
static WM_HWIN _hDialogScale;
static WM_HWIN _hDialogMisc;

static DROPDOWN_Handle _hDropDownScale;
static DROPDOWN_Handle _hDropDownColor;

static SCALE   _Scale[4];
static SCALE   _ScalePrev[4];
static int     _tDiff;
static int     _AutoMode;

static GUI_MEMDEV_Handle _hBkMemDev;

static NEEDLE  _Needle[4] = {
  {0, 5, 1, 0},
  {0, 6, 1, 0},
  {0, 4, 1, 0},
  {0, 2, 1, 0}
};

static PID_EVENT _aPID_Events[] = {
  { 320, 395, 1,  1000 },
  { 320, 395, 0, 10000 },
  { 320, 420, 1,  1000 },
  { 320, 420, 0, 10000 },
  { 320, 445, 1,  1000 },
  { 320, 445, 0, 10000 },
  { 320, 445, 0,     0 },
};

static const int _Pow10[] = {1, 10, 100, 1000};

/*********************************************************************
*
*       static data, presets
*
**********************************************************************
*/

static const SCALE _Presets[4][4] = {
  {
    {
       87, 250, 0, 0,  89, 225,  62,  41, 122,  4, 2, 15,  7, 15, 24,
        2, 2, 2, 1, 100, 1, 0, 0, 0, 0, 0, 16, 25, 0x37B, 0, 0, "Fuel", 0, 0,
      {{{0xFF, 0xFF, 0xFF}}, {{0x00, 0xFF, 0x00}}, {{0xFF, 0x64, 0x00, 0xFF}},
       {{0x98, 0x00, 0x00}}, {{0x90, 0x90, 0x00}}, {{0x00, 0x50, 0x00}}, {{0xE8, 0xE8, 0xFF}}}
    }, {
      144, 136, 0, 0,  72, 225,  70,  93, 184,  3, 1, 15,  7, 15, 24,
        2, 2, 2, 1, 100, 1, 0, 0, 0, 0, 0, 16, 25, 0x37B, 0, 0, "Oil", 0, 0,
      {{{0xFF, 0xFF, 0xFF}}, {{0x00, 0xFF, 0x00}}, {{0xFF, 0x64, 0x00, 0xFF}},
       {{0x98, 0x00, 0x00}}, {{0x90, 0x90, 0x00}}, {{0x00, 0x50, 0x00}}, {{0xE8, 0xE8, 0xFF}}}
    }, {
      335, 185, 0, 0,  58, 234, 149, 258, 335, 15, 4, 25, 10, 25, 29,
        2, 2, 2, 4, 100, 1, 1, 1, 0, 5, 0, 16, 25, 0x77F, 0, 0, "Speed", 0, 0,
      {{{0xFF, 0xFF, 0xFF}}, {{0x00, 0xFF, 0x00}}, {{0x00, 0x8A, 0xFF, 0x88}},
       {{0x00, 0x70, 0x00}}, {{0x90, 0x90, 0x00}}, {{0xAA, 0x00, 0x00}}, {{0xE8, 0xE8, 0xFF}}}
    }, {
      522, 208, 0, 0,  59, 225, 108, 257, 311,  8, 4, 25, 10, 25, 34,
        2, 2, 2, 3, 100, 1, 0, 0, 0, 0, 0, 16, 25, 0x77F, 0, 0, "RPM", 0, 0,
      {{{0xFF, 0xFF, 0xFF}}, {{0x00, 0xFF, 0x00}}, {{0xFF, 0xAA, 0x00, 0xB5}},
       {{0x00, 0x70, 0x00}}, {{0x90, 0x90, 0x00}}, {{0xAA, 0x00, 0x00}}, {{0xE8, 0xE8, 0xFF}}}
    }
  }, {
    {
       96, 208, 0, 0, 40, 160,  73,  41, 122,  4, 2, 15,  7, 15, 24,
        2, 2, 2, 3, 100, 1, 0, 0, 0, 0, 0, 16, 22, 0x74B, 0, 0, "Fuel", 0, 0,
      {{{0xFF, 0xFF, 0xAC}}, {{0x00, 0xFF, 0x70}}, {{0xFF, 0x64, 0x64, 0x80}},
       {{0x98, 0x00, 0x00}}, {{0x90, 0x90, 0x00}}, {{0x00, 0x50, 0x00}}, {{0xE8, 0xE8, 0xFF}}}
    }, {
      395, 247, 0, 0, 90, 270,  75,  93, 184,  3, 1, 15,  7, 15, 24,
        2, 2, 2, 1, 100, 1, 0, 0, 0, 0, 0, 16, 22, 0x77B, 0, 0, "Oil", 0, 0,
      {{{0xFF, 0xFF, 0xAC}}, {{0x00, 0xFF, 0x70}}, {{0xFF, 0x64, 0x64, 0x80}},
       {{0x98, 0x00, 0x00}}, {{0x90, 0x90, 0x00}}, {{0x00, 0x50, 0x00}}, {{0xE8, 0xE8, 0xFF}}}
    }, {
      272, 184, 0, 0,  58, 254, 156, 261, 327, 12, 4, 19, 10, 17, 42,
        2, 2, 2, 0, 100, 2, 1, 1, 17, 22, 0, 16, 22, 0x7C7, 0, 0, "Speed", 0, 0,
      {{{0xFF, 0xFF, 0xAC}}, {{0x00, 0xFF, 0x70}}, {{0x00, 0xCA, 0xFF, 0x68}},
       {{0x00, 0x8C, 0x00}}, {{0xC4, 0xC4, 0x00}}, {{0xD8, 0x00, 0x00}}, {{0x75, 0xFF, 0xFF}}}
    }, {
      511, 208, 0, 0, 128, 270, 107, 257, 311,  8, 4, 25, 10, 25, 34,
        2, 2, 2, 3, 100, 1, 0, 0, 0, 0, 0, 16, 22, 0x777, 0, 0, "RPM", 0, 0,
      {{{0xFF, 0xFF, 0xAC}}, {{0x00, 0xFF, 0x70}}, {{0xFF, 0xAA, 0x00, 0xB5}},
       {{0x00, 0x70, 0x00}}, {{0x90, 0x90, 0x00}}, {{0xAA, 0x00, 0x00}}, {{0xE8, 0xE8, 0xFF}}}
    }
  }, {
    {
      101, 212, 0, 0,  30, 270,  66,  60, 120,  5, 1, 15,  7, 13, 34,
        2, 2, 2, 0, 100, 1, 0, 0, 0, 0, 0, 16, 25, 0xF43, 0, 0, "Fuel", 0, 0,
      {{{0xFF, 0xFF, 0xFF}}, {{0x00, 0xFF, 0x00}}, {{0xA0, 0x7C, 0xFF, 0xA0}},
       {{0x90, 0x00, 0x00}}, {{0x80, 0x80, 0x00}}, {{0x00, 0x70, 0x00}}, {{0xFF, 0xFF, 0xFF}}}
    }, {
      543, 215, 0, 0,  30, 270,  66, 240, 320,  5, 1, 15,  7, 15, 34,
        2, 2, 2, 0, 100, 1, 0, 0, 0, 0, 0, 16, 25, 0xF43, 0, 0, "Oil", 0, 0,
      {{{0xFF, 0xFF, 0xFF}}, {{0x00, 0xFF, 0x00}}, {{0xA0, 0x7C, 0xFF, 0xA0}},
       {{0x00, 0x70, 0x00}}, {{0x90, 0x90, 0x00}}, {{0xAA, 0x00, 0x00}}, {{0xFF, 0xFF, 0xFF}}}
    }, {
      325, 155, 0, 0,  30, 270,  95, 269, 317,  5, 1, 16,  8,  9, 34,
        2, 2, 2, 1, 100, 1, 0, 0, 0, 0, 0, 16, 40, 0x773, 0, 0, "Speed", 0, 0,
      {{{0xFF, 0xFF, 0xFF}}, {{0x00, 0xFF, 0x00}}, {{0xFF, 0x4C, 0x38, 0xA0}},
       {{0x00, 0x70, 0x00}}, {{0x90, 0x90, 0x00}}, {{0xAA, 0x00, 0x00}}, {{0xFF, 0x64, 0x64}}}
    }, {
      325, 155, 0, 0,  30, 270, 137, 228, 329, 10, 1, 16,  9, 25, 21,
        2, 2, 2, 0, 100, 2, 1, 1, 0, 0, 0, 16, 16, 0x747, 0, 0, "RPM", 0, 0,
      {{{0xFF, 0xFF, 0xFF}}, {{0x00, 0xFF, 0x00}}, {{0x00, 0x8C, 0xFF, 0xA4}},
       {{0x00, 0x70, 0x00}}, {{0x90, 0x90, 0x00}}, {{0xAA, 0x00, 0x00}}, {{0xFF, 0x64, 0x64}}}
    }
  }, {
    {
      102, 251, 0, 0,  90, 270,  62,  60, 120,  0, 5, 15,  7, 13, 24,
        2, 2, 2, 4, 100, 1, 0, 0, 0, 0, 0, 20, 25, 0x77F, 0, 0, "Fuel", 0, 0,
      {{{0xFF, 0xFF, 0xC0}}, {{0x00, 0xFF, 0xFF}}, {{0x80, 0xFF, 0x00, 0x80}},
       {{0xAA, 0x00, 0x00}}, {{0x90, 0x90, 0x00}}, {{0x00, 0x70, 0x00}}, {{0xFF, 0xFF, 0xFF}}}
    }, {
      292, 251, 0, 0,  90, 270,  62,  90, 270,  0, 3, 15,  7, 15, 24,
        2, 2, 2, 4, 100, 1, 0, 0, 0, 0, 0, 20, 25, 0x757, 0, 0, "Oil", 0, 0,
      {{{0xFF, 0xFF, 0xC0}}, {{0x00, 0xFF, 0xFF}}, {{0x80, 0xFF, 0x00, 0x80}},
       {{0x90, 0x90, 0x00}}, {{0x00, 0x70, 0x00}}, {{0x90, 0x90, 0x00}}, {{0xFF, 0xFF, 0xFF}}}
    }, {
      447, 192, 0, 0, 123, 291, 160, 270, 331, 13, 4, 13,  5,  7, 13,
        2, 2, 2, 2,  80, 2, 1, 1, 35, 44, 50, 16, 25, 0x73F, 0, 0, "Speed", 0, 0,
      {{{0xFF, 0xFF, 0xC0}}, {{0x00, 0xFF, 0xFF}}, {{0x95, 0x64, 0xFF, 0xB0}},
       {{0x00, 0x70, 0x00}}, {{0x90, 0x90, 0x00}}, {{0xAA, 0x00, 0x00}}, {{0xFF, 0xFF, 0xFF}}}
    }, {
      197, 144, 0, 0,  81, 255, 124, 257, 311,  8, 1, 13,  5,  7, 10,
        2, 2, 2, 2,  80, 1, 0, 0, 25, 34, 40, 16, 25, 0x73F, 0, 0, "RPM", 0, 0,
      {{{0xFF, 0xFF, 0xC0}}, {{0x00, 0xFF, 0xFF}}, {{0x95, 0x64, 0xFF, 0xB0}},
       {{0x00, 0x70, 0x00}}, {{0x90, 0x90, 0x00}}, {{0xAA, 0x00, 0x00}}, {{0xFF, 0xFF, 0xFF}}}
    }
  }
};

/*********************************************************************
*
*       static data, dialog resource
*
**********************************************************************
*/

static const GUI_WIDGET_CREATE_INFO _aDialogColor[] = {
  { WINDOW_CreateIndirect, "",         0,                0,   0, 280, 134 },
  { TEXT_CreateIndirect,     "Red:",   0,                8,  36,  70,  20, TEXT_CF_LEFT },
  { TEXT_CreateIndirect,     "Green:", 0,                8,  60,  70,  20, TEXT_CF_LEFT },
  { TEXT_CreateIndirect,     "Blue:",  0,                8,  84,  70,  20, TEXT_CF_LEFT },
  { TEXT_CreateIndirect,     "Alpha",  0,                8, 108,  70,  20, TEXT_CF_LEFT },
  { SLIDER_CreateIndirect,   NULL,    GUI_ID_SLIDER0,   80,  33, 130,  20 },
  { SLIDER_CreateIndirect,   NULL,    GUI_ID_SLIDER1,   80,  57, 130,  20 },
  { SLIDER_CreateIndirect,   NULL,    GUI_ID_SLIDER2,   80,  81, 130,  20 },
  { SLIDER_CreateIndirect,   NULL,    GUI_ID_SLIDER3,   80, 105, 130,  20 },
  { DROPDOWN_CreateIndirect, NULL,    GUI_ID_USER,     115,   5, 100,  98 }
};

static const GUI_WIDGET_CREATE_INFO _aDialogMark[] = {
  { WINDOW_CreateIndirect,   "",               0,    0,   0, 280, 134 },
  { TEXT_CreateIndirect,     "NumMarkLines:",  0,    8,  12,  70,  20, TEXT_CF_LEFT },
  { TEXT_CreateIndirect,     "LenMarkLines:",  0,    8,  36,  70,  20, TEXT_CF_LEFT },
  { TEXT_CreateIndirect,     "PosMarkLines:",  0,    8,  60,  70,  20, TEXT_CF_LEFT },
  { TEXT_CreateIndirect,     "PenSize:",       0,    8,  84,  70,  20, TEXT_CF_LEFT },
  { SLIDER_CreateIndirect,   NULL, GUI_ID_SLIDER0,  80,   9, 130,  20 },
  { SLIDER_CreateIndirect,   NULL, GUI_ID_SLIDER1,  80,  33, 130,  20 },
  { SLIDER_CreateIndirect,   NULL, GUI_ID_SLIDER2,  80,  57, 130,  20 },
  { SLIDER_CreateIndirect,   NULL, GUI_ID_SLIDER3,  80,  81, 130,  20 },
  { TEXT_CreateIndirect,     "Active", 0,          240,  12,  30,  20, TEXT_CF_LEFT },
  { CHECKBOX_CreateIndirect, NULL, GUI_ID_USER,    222,  11 }
};

static const GUI_WIDGET_CREATE_INFO _aDialogPitch[] = {
  { WINDOW_CreateIndirect,   "",               0,    0,   0, 280, 134 },
  { TEXT_CreateIndirect,     "NumPitchLines:", 0,    8,  12,  70,  20, TEXT_CF_LEFT },
  { TEXT_CreateIndirect,     "LenPitchLines:", 0,    8,  36,  70,  20, TEXT_CF_LEFT },
  { TEXT_CreateIndirect,     "PosPitchLines:", 0,    8,  60,  70,  20, TEXT_CF_LEFT },
  { TEXT_CreateIndirect,     "PenSize:",       0,    8,  84,  70,  20, TEXT_CF_LEFT },
  { SLIDER_CreateIndirect,   NULL, GUI_ID_SLIDER0,  80,   9, 130,  20 },
  { SLIDER_CreateIndirect,   NULL, GUI_ID_SLIDER1,  80,  33, 130,  20 },
  { SLIDER_CreateIndirect,   NULL, GUI_ID_SLIDER2,  80,  57, 130,  20 },
  { SLIDER_CreateIndirect,   NULL, GUI_ID_SLIDER3,  80,  81, 130,  20 },
  { TEXT_CreateIndirect,     "Active", 0,          240,  12,  30,  20, TEXT_CF_LEFT },
  { CHECKBOX_CreateIndirect, NULL, GUI_ID_USER,    222,  11 }
};

static const GUI_WIDGET_CREATE_INFO _aDialogArc[] = {
  { WINDOW_CreateIndirect,   "",               0,    0,   0, 280, 134 },
  { TEXT_CreateIndirect,     "ArcArea1:", 0,         8,  12,  70,  20, TEXT_CF_LEFT },
  { TEXT_CreateIndirect,     "ArcArea2:", 0,         8,  36,  70,  20, TEXT_CF_LEFT },
  { TEXT_CreateIndirect,     "ArcWidth:", 0,         8,  60,  70,  20, TEXT_CF_LEFT },
  { TEXT_CreateIndirect,     "ArcPos:",   0,         8,  84,  70,  20, TEXT_CF_LEFT },
  { TEXT_CreateIndirect,     "PenSize:",  0,         8, 108,  70,  20, TEXT_CF_LEFT },
  { SLIDER_CreateIndirect,   NULL, GUI_ID_SLIDER0,  80,   9, 130,  20 },
  { SLIDER_CreateIndirect,   NULL, GUI_ID_SLIDER1,  80,  33, 130,  20 },
  { SLIDER_CreateIndirect,   NULL, GUI_ID_SLIDER2,  80,  57, 130,  20 },
  { SLIDER_CreateIndirect,   NULL, GUI_ID_SLIDER3,  80,  81, 130,  20 },
  { SLIDER_CreateIndirect,   NULL, GUI_ID_SLIDER4,  80, 105, 130,  20 },
  { TEXT_CreateIndirect,     "Area1", 0,           240,  12,  30,  20, TEXT_CF_LEFT },
  { TEXT_CreateIndirect,     "Area2", 0,           240,  36,  30,  20, TEXT_CF_LEFT },
  { TEXT_CreateIndirect,     "Area3", 0,           240,  60,  30,  20, TEXT_CF_LEFT },
  { TEXT_CreateIndirect,     "Arc1",  0,           240,  84,  30,  20, TEXT_CF_LEFT },
  { TEXT_CreateIndirect,     "Arc2",  0,           240, 108,  30,  20, TEXT_CF_LEFT },
  { CHECKBOX_CreateIndirect, NULL, GUI_ID_USER+0,  222,  11 },
  { CHECKBOX_CreateIndirect, NULL, GUI_ID_USER+1,  222,  35 },
  { CHECKBOX_CreateIndirect, NULL, GUI_ID_USER+2,  222,  59 },
  { CHECKBOX_CreateIndirect, NULL, GUI_ID_USER+3,  222,  83 },
  { CHECKBOX_CreateIndirect, NULL, GUI_ID_USER+4,  222, 107 }
};

static const GUI_WIDGET_CREATE_INFO _aDialogGrad[] = {
  { WINDOW_CreateIndirect,   "",             0,      0,   0, 280, 134 },
  { TEXT_CreateIndirect,     "GradDistance:",0,      8,  12,  70,  20, TEXT_CF_LEFT },
  { TEXT_CreateIndirect,     "StepWidth:",   0,      8,  36,  70,  20, TEXT_CF_LEFT },
  { TEXT_CreateIndirect,     "StartNumber:", 0,      8,  60,  70,  20, TEXT_CF_LEFT },
  { TEXT_CreateIndirect,     "Exponent:",    0,      8,  84,  70,  20, TEXT_CF_LEFT },
  { TEXT_CreateIndirect,     "TextDistance:",0,      8, 108,  70,  20, TEXT_CF_LEFT },
  { SLIDER_CreateIndirect,   NULL, GUI_ID_SLIDER0,  80,   9, 130,  20 },
  { SLIDER_CreateIndirect,   NULL, GUI_ID_SLIDER1,  80,  33, 130,  20 },
  { SLIDER_CreateIndirect,   NULL, GUI_ID_SLIDER2,  80,  57, 130,  20 },
  { SLIDER_CreateIndirect,   NULL, GUI_ID_SLIDER3,  80,  81, 130,  20 },
  { SLIDER_CreateIndirect,   NULL, GUI_ID_SLIDER4,  80, 105, 130,  20 },
  { TEXT_CreateIndirect,     "Active", 0,          240,  12,  30,  20, TEXT_CF_LEFT },
  { TEXT_CreateIndirect,     "Text", 0,            240, 108,  30,  20, TEXT_CF_LEFT },
  { CHECKBOX_CreateIndirect, NULL, GUI_ID_USER+0,  222,  11 },
  { CHECKBOX_CreateIndirect, NULL, GUI_ID_USER+1,  222, 107 }
};

static const GUI_WIDGET_CREATE_INFO _aDialogScale[] = {
  { WINDOW_CreateIndirect,   "",            0,       0,   0, 280, 134 },
  { TEXT_CreateIndirect,     "ArcStart:",   0,       8,  12,  70,  20, TEXT_CF_LEFT },
  { TEXT_CreateIndirect,     "ArcEnd:",     0,       8,  36,  70,  20, TEXT_CF_LEFT },
  { TEXT_CreateIndirect,     "ArcRadius:",  0,       8,  60,  70,  20, TEXT_CF_LEFT },
  { TEXT_CreateIndirect,     "X-Position:", 0,       8,  84,  70,  20, TEXT_CF_LEFT },
  { TEXT_CreateIndirect,     "Y-Position:", 0,       8, 108,  70,  20, TEXT_CF_LEFT },
  { SLIDER_CreateIndirect,   NULL, GUI_ID_SLIDER0,  80,   9, 130,  20 },
  { SLIDER_CreateIndirect,   NULL, GUI_ID_SLIDER1,  80,  33, 130,  20 },
  { SLIDER_CreateIndirect,   NULL, GUI_ID_SLIDER2,  80,  57, 130,  20 },
  { SLIDER_CreateIndirect,   NULL, GUI_ID_SLIDER3,  80,  81, 130,  20 },
  { SLIDER_CreateIndirect,   NULL, GUI_ID_SLIDER4,  80, 105, 130,  20 },
  { TEXT_CreateIndirect,     "Active", 0,          240,  12,  30,  20, TEXT_CF_LEFT },
  { CHECKBOX_CreateIndirect, NULL, GUI_ID_USER,    222,  11 }
};

static const GUI_WIDGET_CREATE_INFO _aDialogMisc[] = {
  { WINDOW_CreateIndirect,   "",              0,     0,   0, 280, 134 },
  { TEXT_CreateIndirect,     "NeedleShape:",  0,     8,  12,  70,  20, TEXT_CF_LEFT },
  { TEXT_CreateIndirect,     "NeedleRadius:", 0,     8,  36,  70,  20, TEXT_CF_LEFT },
  { TEXT_CreateIndirect,     "AxisRadius:",   0,     8,  60,  70,  20, TEXT_CF_LEFT },
  { SLIDER_CreateIndirect,   NULL, GUI_ID_SLIDER0,  80,   9, 130,  20 },
  { SLIDER_CreateIndirect,   NULL, GUI_ID_SLIDER1,  80,  33, 130,  20 },
  { SLIDER_CreateIndirect,   NULL, GUI_ID_SLIDER2,  80,  57, 130,  20 },
  { TEXT_CreateIndirect,     "Frame", 0,           240,  12,  30,  20, TEXT_CF_LEFT },
  { TEXT_CreateIndirect,     "Line",  0,           240,  36,  30,  20, TEXT_CF_LEFT },
  { CHECKBOX_CreateIndirect, NULL, GUI_ID_USER+0,  222,  11 },
  { CHECKBOX_CreateIndirect, NULL, GUI_ID_USER+1,  222,  35 }
};

/*********************************************************************
*
*       static data, bitmaps
*
**********************************************************************
*/
/*********************************************************************
*
*       NavigationMap
*/
static const GUI_COLOR ColorsMap[] = {
     0xEEEEEE,0x99CCFF,0xCCFFCC,0xFFFFFF
    ,0xCCCCCC,0x0000FF,0x888888,0x000000
    ,0x33FFFF,0x444444,0xDDDDDD,0xBBBBBB
    ,0x99CC99,0x777777,0xAAAAAA,0x555555
    ,0x00FFFF,0x666666,0x999999,0x660000
    ,0x669966,0xCCCCFF,0xFF0066,0xCC0033
    ,0xCCFFFF,0x9999FF,0x6666FF,0x99FFFF
};

static const GUI_LOGPALETTE PalMap = {
  28,	/* number of entries */
  0, 	/* No transparency */
  &ColorsMap[0]
};

static const unsigned char acMap[] = {
  /* RLE: 011 Pixels @ 000,000*/ 11, 0x0C,
  /* RLE: 001 Pixels @ 011,000*/ 1, 0x14,
  /* RLE: 026 Pixels @ 012,000*/ 26, 0x0C,
  /* RLE: 001 Pixels @ 038,000*/ 1, 0x02,
  /* RLE: 033 Pixels @ 039,000*/ 33, 0x01,
  /* RLE: 001 Pixels @ 072,000*/ 1, 0x04,
  /* RLE: 007 Pixels @ 073,000*/ 7, 0x03,
  /* ABS: 002 Pixels @ 080,000*/ 0, 2, 0x04, 0x04,
  /* RLE: 091 Pixels @ 082,000*/ 91, 0x00,
  /* ABS: 004 Pixels @ 173,000*/ 0, 4, 0x0D, 0x03, 0x06, 0x06,
  /* RLE: 047 Pixels @ 177,000*/ 47, 0x01,
  /* RLE: 001 Pixels @ 224,000*/ 1, 0x04,
  /* RLE: 019 Pixels @ 225,000*/ 19, 0x03,
  /* RLE: 003 Pixels @ 244,000*/ 3, 0x04,
  /* RLE: 141 Pixels @ 247,000*/ 141, 0x01,
  /* RLE: 006 Pixels @ 000,001*/ 6, 0x0C,
  /* RLE: 001 Pixels @ 006,001*/ 1, 0x14,
  /* RLE: 005 Pixels @ 007,001*/ 5, 0x0C,
  /* RLE: 001 Pixels @ 012,001*/ 1, 0x14,
  /* RLE: 007 Pixels @ 013,001*/ 7, 0x0C,
  /* RLE: 003 Pixels @ 020,001*/ 3, 0x14,
  /* RLE: 014 Pixels @ 023,001*/ 14, 0x0C,
  /* RLE: 001 Pixels @ 037,001*/ 1, 0x02,
  /* RLE: 034 Pixels @ 038,001*/ 34, 0x01,
  /* RLE: 001 Pixels @ 072,001*/ 1, 0x04,
  /* RLE: 008 Pixels @ 073,001*/ 8, 0x03,
  /* ABS: 002 Pixels @ 081,001*/ 0, 2, 0x04, 0x04,
  /* RLE: 091 Pixels @ 083,001*/ 91, 0x00,
  /* ABS: 004 Pixels @ 174,001*/ 0, 4, 0x0D, 0x06, 0x06, 0x06,
  /* RLE: 046 Pixels @ 178,001*/ 46, 0x01,
  /* RLE: 001 Pixels @ 224,001*/ 1, 0x04,
  /* RLE: 015 Pixels @ 225,001*/ 15, 0x03,
  /* RLE: 003 Pixels @ 240,001*/ 3, 0x04,
  /* RLE: 145 Pixels @ 243,001*/ 145, 0x01,
  /* RLE: 019 Pixels @ 000,002*/ 19, 0x0C,
  /* ABS: 005 Pixels @ 019,002*/ 0, 5, 0x14, 0x0C, 0x0C, 0x14, 0x14,
  /* RLE: 012 Pixels @ 024,002*/ 12, 0x0C,
  /* RLE: 001 Pixels @ 036,002*/ 1, 0x02,
  /* RLE: 036 Pixels @ 037,002*/ 36, 0x01,
  /* RLE: 001 Pixels @ 073,002*/ 1, 0x04,
  /* RLE: 008 Pixels @ 074,002*/ 8, 0x03,
  /* ABS: 002 Pixels @ 082,002*/ 0, 2, 0x04, 0x04,
  /* RLE: 091 Pixels @ 084,002*/ 91, 0x00,
  /* RLE: 001 Pixels @ 175,002*/ 1, 0x0D,
  /* RLE: 004 Pixels @ 176,002*/ 4, 0x06,
  /* RLE: 044 Pixels @ 180,002*/ 44, 0x01,
  /* RLE: 001 Pixels @ 224,002*/ 1, 0x04,
  /* RLE: 011 Pixels @ 225,002*/ 11, 0x03,
  /* RLE: 003 Pixels @ 236,002*/ 3, 0x04,
  /* RLE: 149 Pixels @ 239,002*/ 149, 0x01,
  /* RLE: 019 Pixels @ 000,003*/ 19, 0x0C,
  /* ABS: 006 Pixels @ 019,003*/ 0, 6, 0x14, 0x0C, 0x0C, 0x0C, 0x14, 0x14,
  /* RLE: 006 Pixels @ 025,003*/ 6, 0x0C,
  /* ABS: 005 Pixels @ 031,003*/ 0, 5, 0x14, 0x0C, 0x0C, 0x0C, 0x02,
  /* RLE: 037 Pixels @ 036,003*/ 37, 0x01,
  /* ABS: 002 Pixels @ 073,003*/ 0, 2, 0x04, 0x04,
  /* RLE: 008 Pixels @ 075,003*/ 8, 0x03,
  /* ABS: 002 Pixels @ 083,003*/ 0, 2, 0x04, 0x04,
  /* RLE: 091 Pixels @ 085,003*/ 91, 0x00,
  /* ABS: 005 Pixels @ 176,003*/ 0, 5, 0x0D, 0x06, 0x03, 0x06, 0x06,
  /* RLE: 043 Pixels @ 181,003*/ 43, 0x01,
  /* ABS: 002 Pixels @ 224,003*/ 0, 2, 0x04, 0x04,
  /* RLE: 006 Pixels @ 226,003*/ 6, 0x03,
  /* RLE: 003 Pixels @ 232,003*/ 3, 0x04,
  /* RLE: 153 Pixels @ 235,003*/ 153, 0x01,
  /* RLE: 001 Pixels @ 000,004*/ 1, 0x14,
  /* RLE: 017 Pixels @ 001,004*/ 17, 0x0C,
  /* RLE: 001 Pixels @ 018,004*/ 1, 0x14,
  /* RLE: 004 Pixels @ 019,004*/ 4, 0x0C,
  /* ABS: 002 Pixels @ 023,004*/ 0, 2, 0x14, 0x14,
  /* RLE: 007 Pixels @ 025,004*/ 7, 0x0C,
  /* ABS: 003 Pixels @ 032,004*/ 0, 3, 0x14, 0x0C, 0x02,
  /* RLE: 038 Pixels @ 035,004*/ 38, 0x01,
  /* RLE: 003 Pixels @ 073,004*/ 3, 0x04,
  /* RLE: 008 Pixels @ 076,004*/ 8, 0x03,
  /* ABS: 002 Pixels @ 084,004*/ 0, 2, 0x04, 0x04,
  /* RLE: 091 Pixels @ 086,004*/ 91, 0x00,
  /* ABS: 005 Pixels @ 177,004*/ 0, 5, 0x0D, 0x0D, 0x03, 0x06, 0x0D,
  /* RLE: 043 Pixels @ 182,004*/ 43, 0x01,
  /* RLE: 006 Pixels @ 225,004*/ 6, 0x04,
  /* RLE: 157 Pixels @ 231,004*/ 157, 0x01,
  /* RLE: 007 Pixels @ 000,005*/ 7, 0x0C,
  /* RLE: 001 Pixels @ 007,005*/ 1, 0x14,
  /* RLE: 011 Pixels @ 008,005*/ 11, 0x0C,
  /* ABS: 005 Pixels @ 019,005*/ 0, 5, 0x14, 0x0C, 0x0C, 0x0C, 0x14,
  /* RLE: 009 Pixels @ 024,005*/ 9, 0x0C,
  /* RLE: 042 Pixels @ 033,005*/ 42, 0x01,
  /* ABS: 002 Pixels @ 075,005*/ 0, 2, 0x04, 0x04,
  /* RLE: 008 Pixels @ 077,005*/ 8, 0x03,
  /* ABS: 002 Pixels @ 085,005*/ 0, 2, 0x04, 0x04,
  /* RLE: 092 Pixels @ 087,005*/ 92, 0x00,
  /* ABS: 004 Pixels @ 179,005*/ 0, 4, 0x0D, 0x03, 0x06, 0x06,
  /* RLE: 205 Pixels @ 183,005*/ 205, 0x01,
  /* RLE: 006 Pixels @ 000,006*/ 6, 0x0C,
  /* ABS: 003 Pixels @ 006,006*/ 0, 3, 0x14, 0x0C, 0x14,
  /* RLE: 005 Pixels @ 009,006*/ 5, 0x0C,
  /* RLE: 001 Pixels @ 014,006*/ 1, 0x14,
  /* RLE: 005 Pixels @ 015,006*/ 5, 0x0C,
  /* ABS: 004 Pixels @ 020,006*/ 0, 4, 0x14, 0x0C, 0x14, 0x14,
  /* RLE: 008 Pixels @ 024,006*/ 8, 0x0C,
  /* RLE: 044 Pixels @ 032,006*/ 44, 0x01,
  /* ABS: 002 Pixels @ 076,006*/ 0, 2, 0x04, 0x04,
  /* RLE: 008 Pixels @ 078,006*/ 8, 0x03,
  /* ABS: 002 Pixels @ 086,006*/ 0, 2, 0x04, 0x04,
  /* RLE: 092 Pixels @ 088,006*/ 92, 0x00,
  /* ABS: 004 Pixels @ 180,006*/ 0, 4, 0x0D, 0x06, 0x06, 0x06,
  /* RLE: 204 Pixels @ 184,006*/ 204, 0x01,
  /* RLE: 004 Pixels @ 000,007*/ 4, 0x0C,
  /* ABS: 006 Pixels @ 004,007*/ 0, 6, 0x14, 0x14, 0x0C, 0x0C, 0x14, 0x14,
  /* RLE: 012 Pixels @ 010,007*/ 12, 0x0C,
  /* RLE: 001 Pixels @ 022,007*/ 1, 0x14,
  /* RLE: 008 Pixels @ 023,007*/ 8, 0x0C,
  /* RLE: 046 Pixels @ 031,007*/ 46, 0x01,
  /* ABS: 002 Pixels @ 077,007*/ 0, 2, 0x04, 0x04,
  /* RLE: 008 Pixels @ 079,007*/ 8, 0x03,
  /* ABS: 002 Pixels @ 087,007*/ 0, 2, 0x04, 0x04,
  /* RLE: 092 Pixels @ 089,007*/ 92, 0x00,
  /* ABS: 004 Pixels @ 181,007*/ 0, 4, 0x0D, 0x06, 0x06, 0x06,
  /* RLE: 203 Pixels @ 185,007*/ 203, 0x01,
  /* RLE: 003 Pixels @ 000,008*/ 3, 0x0C,
  /* RLE: 001 Pixels @ 003,008*/ 1, 0x14,
  /* RLE: 005 Pixels @ 004,008*/ 5, 0x0C,
  /* ABS: 002 Pixels @ 009,008*/ 0, 2, 0x14, 0x14,
  /* RLE: 011 Pixels @ 011,008*/ 11, 0x0C,
  /* RLE: 001 Pixels @ 022,008*/ 1, 0x14,
  /* RLE: 007 Pixels @ 023,008*/ 7, 0x0C,
  /* RLE: 048 Pixels @ 030,008*/ 48, 0x01,
  /* ABS: 002 Pixels @ 078,008*/ 0, 2, 0x04, 0x04,
  /* RLE: 008 Pixels @ 080,008*/ 8, 0x03,
  /* ABS: 002 Pixels @ 088,008*/ 0, 2, 0x04, 0x04,
  /* RLE: 092 Pixels @ 090,008*/ 92, 0x00,
  /* ABS: 005 Pixels @ 182,008*/ 0, 5, 0x0D, 0x06, 0x03, 0x06, 0x0D,
  /* RLE: 201 Pixels @ 187,008*/ 201, 0x01,
  /* RLE: 004 Pixels @ 000,009*/ 4, 0x0C,
  /* RLE: 001 Pixels @ 004,009*/ 1, 0x14,
  /* RLE: 004 Pixels @ 005,009*/ 4, 0x0C,
  /* ABS: 002 Pixels @ 009,009*/ 0, 2, 0x14, 0x14,
  /* RLE: 011 Pixels @ 011,009*/ 11, 0x0C,
  /* RLE: 001 Pixels @ 022,009*/ 1, 0x14,
  /* RLE: 006 Pixels @ 023,009*/ 6, 0x0C,
  /* RLE: 050 Pixels @ 029,009*/ 50, 0x01,
  /* ABS: 002 Pixels @ 079,009*/ 0, 2, 0x04, 0x04,
  /* RLE: 008 Pixels @ 081,009*/ 8, 0x03,
  /* ABS: 002 Pixels @ 089,009*/ 0, 2, 0x04, 0x04,
  /* RLE: 092 Pixels @ 091,009*/ 92, 0x00,
  /* ABS: 005 Pixels @ 183,009*/ 0, 5, 0x0D, 0x06, 0x03, 0x06, 0x06,
  /* RLE: 200 Pixels @ 188,009*/ 200, 0x01,
  /* RLE: 003 Pixels @ 000,010*/ 3, 0x0C,
  /* RLE: 001 Pixels @ 003,010*/ 1, 0x14,
  /* RLE: 006 Pixels @ 004,010*/ 6, 0x0C,
  /* ABS: 002 Pixels @ 010,010*/ 0, 2, 0x14, 0x14,
  /* RLE: 016 Pixels @ 012,010*/ 16, 0x0C,
  /* RLE: 052 Pixels @ 028,010*/ 52, 0x01,
  /* ABS: 002 Pixels @ 080,010*/ 0, 2, 0x04, 0x04,
  /* RLE: 008 Pixels @ 082,010*/ 8, 0x03,
  /* ABS: 002 Pixels @ 090,010*/ 0, 2, 0x04, 0x04,
  /* RLE: 092 Pixels @ 092,010*/ 92, 0x00,
  /* ABS: 005 Pixels @ 184,010*/ 0, 5, 0x0D, 0x0D, 0x03, 0x06, 0x06,
  /* RLE: 199 Pixels @ 189,010*/ 199, 0x01,
  /* RLE: 003 Pixels @ 000,011*/ 3, 0x0C,
  /* RLE: 001 Pixels @ 003,011*/ 1, 0x14,
  /* RLE: 005 Pixels @ 004,011*/ 5, 0x0C,
  /* ABS: 002 Pixels @ 009,011*/ 0, 2, 0x14, 0x14,
  /* RLE: 015 Pixels @ 011,011*/ 15, 0x0C,
  /* RLE: 001 Pixels @ 026,011*/ 1, 0x14,
  /* RLE: 054 Pixels @ 027,011*/ 54, 0x01,
  /* ABS: 002 Pixels @ 081,011*/ 0, 2, 0x04, 0x04,
  /* RLE: 008 Pixels @ 083,011*/ 8, 0x03,
  /* ABS: 002 Pixels @ 091,011*/ 0, 2, 0x04, 0x04,
  /* RLE: 093 Pixels @ 093,011*/ 93, 0x00,
  /* ABS: 004 Pixels @ 186,011*/ 0, 4, 0x0D, 0x06, 0x06, 0x06,
  /* RLE: 198 Pixels @ 190,011*/ 198, 0x01,
  /* RLE: 004 Pixels @ 000,012*/ 4, 0x0C,
  /* ABS: 006 Pixels @ 004,012*/ 0, 6, 0x14, 0x0C, 0x0C, 0x0C, 0x14, 0x14,
  /* RLE: 016 Pixels @ 010,012*/ 16, 0x0C,
  /* RLE: 056 Pixels @ 026,012*/ 56, 0x01,
  /* ABS: 002 Pixels @ 082,012*/ 0, 2, 0x04, 0x04,
  /* RLE: 008 Pixels @ 084,012*/ 8, 0x03,
  /* ABS: 002 Pixels @ 092,012*/ 0, 2, 0x04, 0x04,
  /* RLE: 093 Pixels @ 094,012*/ 93, 0x00,
  /* ABS: 004 Pixels @ 187,012*/ 0, 4, 0x0D, 0x06, 0x06, 0x0D,
  /* RLE: 197 Pixels @ 191,012*/ 197, 0x01,
  /* RLE: 005 Pixels @ 000,013*/ 5, 0x0C,
  /* ABS: 004 Pixels @ 005,013*/ 0, 4, 0x14, 0x0C, 0x0C, 0x14,
  /* RLE: 008 Pixels @ 009,013*/ 8, 0x0C,
  /* RLE: 001 Pixels @ 017,013*/ 1, 0x14,
  /* RLE: 006 Pixels @ 018,013*/ 6, 0x0C,
  /* RLE: 059 Pixels @ 024,013*/ 59, 0x01,
  /* ABS: 002 Pixels @ 083,013*/ 0, 2, 0x04, 0x04,
  /* RLE: 008 Pixels @ 085,013*/ 8, 0x03,
  /* ABS: 002 Pixels @ 093,013*/ 0, 2, 0x04, 0x04,
  /* RLE: 093 Pixels @ 095,013*/ 93, 0x00,
  /* ABS: 004 Pixels @ 188,013*/ 0, 4, 0x0D, 0x06, 0x03, 0x06,
  /* RLE: 196 Pixels @ 192,013*/ 196, 0x01,
  /* RLE: 008 Pixels @ 000,014*/ 8, 0x0C,
  /* RLE: 001 Pixels @ 008,014*/ 1, 0x14,
  /* RLE: 014 Pixels @ 009,014*/ 14, 0x0C,
  /* RLE: 061 Pixels @ 023,014*/ 61, 0x01,
  /* ABS: 002 Pixels @ 084,014*/ 0, 2, 0x04, 0x04,
  /* RLE: 008 Pixels @ 086,014*/ 8, 0x03,
  /* ABS: 002 Pixels @ 094,014*/ 0, 2, 0x04, 0x04,
  /* RLE: 093 Pixels @ 096,014*/ 93, 0x00,
  /* ABS: 004 Pixels @ 189,014*/ 0, 4, 0x0D, 0x06, 0x03, 0x06,
  /* RLE: 195 Pixels @ 193,014*/ 195, 0x01,
  /* RLE: 008 Pixels @ 000,015*/ 8, 0x0C,
  /* ABS: 002 Pixels @ 008,015*/ 0, 2, 0x14, 0x14,
  /* RLE: 012 Pixels @ 010,015*/ 12, 0x0C,
  /* RLE: 063 Pixels @ 022,015*/ 63, 0x01,
  /* ABS: 002 Pixels @ 085,015*/ 0, 2, 0x04, 0x04,
  /* RLE: 008 Pixels @ 087,015*/ 8, 0x03,
  /* ABS: 002 Pixels @ 095,015*/ 0, 2, 0x04, 0x04,
  /* RLE: 093 Pixels @ 097,015*/ 93, 0x00,
  /* ABS: 005 Pixels @ 190,015*/ 0, 5, 0x0D, 0x06, 0x03, 0x06, 0x06,
  /* RLE: 193 Pixels @ 195,015*/ 193, 0x01,
  /* RLE: 013 Pixels @ 000,016*/ 13, 0x0C,
  /* RLE: 001 Pixels @ 013,016*/ 1, 0x14,
  /* RLE: 007 Pixels @ 014,016*/ 7, 0x0C,
  /* RLE: 065 Pixels @ 021,016*/ 65, 0x01,
  /* ABS: 002 Pixels @ 086,016*/ 0, 2, 0x04, 0x04,
  /* RLE: 008 Pixels @ 088,016*/ 8, 0x03,
  /* ABS: 002 Pixels @ 096,016*/ 0, 2, 0x04, 0x04,
  /* RLE: 093 Pixels @ 098,016*/ 93, 0x00,
  /* RLE: 001 Pixels @ 191,016*/ 1, 0x0D,
  /* RLE: 004 Pixels @ 192,016*/ 4, 0x06,
  /* RLE: 192 Pixels @ 196,016*/ 192, 0x01,
  /* RLE: 019 Pixels @ 000,017*/ 19, 0x0C,
  /* ABS: 002 Pixels @ 019,017*/ 0, 2, 0x07, 0x07,
  /* RLE: 066 Pixels @ 021,017*/ 66, 0x01,
  /* ABS: 002 Pixels @ 087,017*/ 0, 2, 0x04, 0x04,
  /* RLE: 008 Pixels @ 089,017*/ 8, 0x03,
  /* ABS: 002 Pixels @ 097,017*/ 0, 2, 0x04, 0x04,
  /* RLE: 093 Pixels @ 099,017*/ 93, 0x00,
  /* ABS: 005 Pixels @ 192,017*/ 0, 5, 0x0D, 0x0D, 0x06, 0x06, 0x06,
  /* RLE: 191 Pixels @ 197,017*/ 191, 0x01,
  /* RLE: 007 Pixels @ 000,018*/ 7, 0x0C,
  /* RLE: 001 Pixels @ 007,018*/ 1, 0x14,
  /* RLE: 010 Pixels @ 008,018*/ 10, 0x0C,
  /* ABS: 002 Pixels @ 018,018*/ 0, 2, 0x07, 0x02,
  /* RLE: 068 Pixels @ 020,018*/ 68, 0x01,
  /* ABS: 002 Pixels @ 088,018*/ 0, 2, 0x04, 0x04,
  /* RLE: 009 Pixels @ 090,018*/ 9, 0x03,
  /* ABS: 002 Pixels @ 099,018*/ 0, 2, 0x04, 0x04,
  /* RLE: 093 Pixels @ 101,018*/ 93, 0x00,
  /* ABS: 004 Pixels @ 194,018*/ 0, 4, 0x0D, 0x06, 0x06, 0x0D,
  /* RLE: 190 Pixels @ 198,018*/ 190, 0x01,
  /* RLE: 003 Pixels @ 000,019*/ 3, 0x14,
  /* RLE: 013 Pixels @ 003,019*/ 13, 0x0C,
  /* RLE: 004 Pixels @ 016,019*/ 4, 0x02,
  /* RLE: 069 Pixels @ 020,019*/ 69, 0x01,
  /* ABS: 002 Pixels @ 089,019*/ 0, 2, 0x04, 0x04,
  /* RLE: 009 Pixels @ 091,019*/ 9, 0x03,
  /* ABS: 002 Pixels @ 100,019*/ 0, 2, 0x04, 0x04,
  /* RLE: 093 Pixels @ 102,019*/ 93, 0x00,
  /* ABS: 004 Pixels @ 195,019*/ 0, 4, 0x0D, 0x03, 0x03, 0x06,
  /* RLE: 189 Pixels @ 199,019*/ 189, 0x01,
  /* ABS: 004 Pixels @ 000,020*/ 0, 4, 0x0C, 0x0C, 0x14, 0x14,
  /* RLE: 008 Pixels @ 004,020*/ 8, 0x0C,
  /* ABS: 003 Pixels @ 012,020*/ 0, 3, 0x14, 0x07, 0x07,
  /* RLE: 006 Pixels @ 015,020*/ 6, 0x02,
  /* RLE: 069 Pixels @ 021,020*/ 69, 0x01,
  /* ABS: 002 Pixels @ 090,020*/ 0, 2, 0x04, 0x04,
  /* RLE: 009 Pixels @ 092,020*/ 9, 0x03,
  /* ABS: 002 Pixels @ 101,020*/ 0, 2, 0x04, 0x04,
  /* RLE: 093 Pixels @ 103,020*/ 93, 0x00,
  /* ABS: 004 Pixels @ 196,020*/ 0, 4, 0x0D, 0x06, 0x03, 0x06,
  /* RLE: 188 Pixels @ 200,020*/ 188, 0x01,
  /* RLE: 003 Pixels @ 000,021*/ 3, 0x0C,
  /* ABS: 002 Pixels @ 003,021*/ 0, 2, 0x14, 0x14,
  /* RLE: 007 Pixels @ 005,021*/ 7, 0x0C,
  /* RLE: 001 Pixels @ 012,021*/ 1, 0x07,
  /* RLE: 008 Pixels @ 013,021*/ 8, 0x02,
  /* RLE: 001 Pixels @ 021,021*/ 1, 0x07,
  /* RLE: 069 Pixels @ 022,021*/ 69, 0x01,
  /* ABS: 002 Pixels @ 091,021*/ 0, 2, 0x04, 0x04,
  /* RLE: 009 Pixels @ 093,021*/ 9, 0x03,
  /* ABS: 002 Pixels @ 102,021*/ 0, 2, 0x04, 0x04,
  /* RLE: 093 Pixels @ 104,021*/ 93, 0x00,
  /* RLE: 001 Pixels @ 197,021*/ 1, 0x0D,
  /* RLE: 004 Pixels @ 198,021*/ 4, 0x06,
  /* RLE: 186 Pixels @ 202,021*/ 186, 0x01,
  /* RLE: 003 Pixels @ 000,022*/ 3, 0x0C,
  /* ABS: 002 Pixels @ 003,022*/ 0, 2, 0x14, 0x14,
  /* RLE: 005 Pixels @ 005,022*/ 5, 0x0C,
  /* RLE: 011 Pixels @ 010,022*/ 11, 0x02,
  /* RLE: 001 Pixels @ 021,022*/ 1, 0x07,
  /* RLE: 070 Pixels @ 022,022*/ 70, 0x01,
  /* ABS: 002 Pixels @ 092,022*/ 0, 2, 0x04, 0x04,
  /* RLE: 009 Pixels @ 094,022*/ 9, 0x03,
  /* ABS: 002 Pixels @ 103,022*/ 0, 2, 0x04, 0x04,
  /* RLE: 093 Pixels @ 105,022*/ 93, 0x00,
  /* RLE: 001 Pixels @ 198,022*/ 1, 0x0D,
  /* RLE: 004 Pixels @ 199,022*/ 4, 0x06,
  /* RLE: 185 Pixels @ 203,022*/ 185, 0x01,
  /* RLE: 003 Pixels @ 000,023*/ 3, 0x0C,
  /* ABS: 006 Pixels @ 003,023*/ 0, 6, 0x14, 0x14, 0x0C, 0x0C, 0x07, 0x07,
  /* RLE: 012 Pixels @ 009,023*/ 12, 0x02,
  /* RLE: 001 Pixels @ 021,023*/ 1, 0x07,
  /* RLE: 071 Pixels @ 022,023*/ 71, 0x01,
  /* RLE: 003 Pixels @ 093,023*/ 3, 0x04,
  /* RLE: 008 Pixels @ 096,023*/ 8, 0x03,
  /* ABS: 002 Pixels @ 104,023*/ 0, 2, 0x04, 0x04,
  /* RLE: 093 Pixels @ 106,023*/ 93, 0x00,
  /* ABS: 005 Pixels @ 199,023*/ 0, 5, 0x0D, 0x0D, 0x06, 0x06, 0x06,
  /* RLE: 184 Pixels @ 204,023*/ 184, 0x01,
  /* ABS: 007 Pixels @ 000,024*/ 0, 7, 0x0C, 0x0C, 0x14, 0x14, 0x0C, 0x0C, 0x07,
  /* RLE: 014 Pixels @ 007,024*/ 14, 0x02,
  /* RLE: 074 Pixels @ 021,024*/ 74, 0x01,
  /* ABS: 002 Pixels @ 095,024*/ 0, 2, 0x04, 0x04,
  /* RLE: 008 Pixels @ 097,024*/ 8, 0x03,
  /* ABS: 002 Pixels @ 105,024*/ 0, 2, 0x04, 0x04,
  /* RLE: 094 Pixels @ 107,024*/ 94, 0x00,
  /* ABS: 004 Pixels @ 201,024*/ 0, 4, 0x0D, 0x03, 0x06, 0x0D,
  /* RLE: 183 Pixels @ 205,024*/ 183, 0x01,
  /* ABS: 004 Pixels @ 000,025*/ 0, 4, 0x0C, 0x0C, 0x14, 0x0C,
  /* RLE: 017 Pixels @ 004,025*/ 17, 0x02,
  /* RLE: 075 Pixels @ 021,025*/ 75, 0x01,
  /* ABS: 002 Pixels @ 096,025*/ 0, 2, 0x04, 0x04,
  /* RLE: 008 Pixels @ 098,025*/ 8, 0x03,
  /* ABS: 002 Pixels @ 106,025*/ 0, 2, 0x04, 0x04,
  /* RLE: 094 Pixels @ 108,025*/ 94, 0x00,
  /* ABS: 004 Pixels @ 202,025*/ 0, 4, 0x0D, 0x03, 0x03, 0x06,
  /* RLE: 182 Pixels @ 206,025*/ 182, 0x01,
  /* ABS: 003 Pixels @ 000,026*/ 0, 3, 0x0C, 0x07, 0x07,
  /* RLE: 019 Pixels @ 003,026*/ 19, 0x02,
  /* RLE: 075 Pixels @ 022,026*/ 75, 0x01,
  /* ABS: 002 Pixels @ 097,026*/ 0, 2, 0x04, 0x04,
  /* RLE: 008 Pixels @ 099,026*/ 8, 0x03,
  /* ABS: 002 Pixels @ 107,026*/ 0, 2, 0x04, 0x04,
  /* RLE: 094 Pixels @ 109,026*/ 94, 0x00,
  /* ABS: 004 Pixels @ 203,026*/ 0, 4, 0x0D, 0x06, 0x06, 0x06,
  /* RLE: 181 Pixels @ 207,026*/ 181, 0x01,
  /* RLE: 001 Pixels @ 000,027*/ 1, 0x07,
  /* RLE: 021 Pixels @ 001,027*/ 21, 0x02,
  /* RLE: 001 Pixels @ 022,027*/ 1, 0x07,
  /* RLE: 075 Pixels @ 023,027*/ 75, 0x01,
  /* ABS: 002 Pixels @ 098,027*/ 0, 2, 0x04, 0x04,
  /* RLE: 008 Pixels @ 100,027*/ 8, 0x03,
  /* ABS: 002 Pixels @ 108,027*/ 0, 2, 0x04, 0x04,
  /* RLE: 094 Pixels @ 110,027*/ 94, 0x00,
  /* ABS: 004 Pixels @ 204,027*/ 0, 4, 0x0D, 0x06, 0x06, 0x06,
  /* RLE: 180 Pixels @ 208,027*/ 180, 0x01,
  /* RLE: 022 Pixels @ 000,028*/ 22, 0x02,
  /* RLE: 001 Pixels @ 022,028*/ 1, 0x07,
  /* RLE: 076 Pixels @ 023,028*/ 76, 0x01,
  /* ABS: 002 Pixels @ 099,028*/ 0, 2, 0x04, 0x04,
  /* RLE: 008 Pixels @ 101,028*/ 8, 0x03,
  /* ABS: 002 Pixels @ 109,028*/ 0, 2, 0x04, 0x04,
  /* RLE: 094 Pixels @ 111,028*/ 94, 0x00,
  /* RLE: 001 Pixels @ 205,028*/ 1, 0x0D,
  /* RLE: 004 Pixels @ 206,028*/ 4, 0x06,
  /* RLE: 176 Pixels @ 210,028*/ 176, 0x01,
  /* ABS: 002 Pixels @ 386,028*/ 0, 2, 0x15, 0x00,
  /* RLE: 021 Pixels @ 000,029*/ 21, 0x02,
  /* RLE: 005 Pixels @ 021,029*/ 5, 0x04,
  /* RLE: 074 Pixels @ 026,029*/ 74, 0x01,
  /* ABS: 002 Pixels @ 100,029*/ 0, 2, 0x04, 0x04,
  /* RLE: 008 Pixels @ 102,029*/ 8, 0x03,
  /* ABS: 002 Pixels @ 110,029*/ 0, 2, 0x04, 0x04,
  /* RLE: 094 Pixels @ 112,029*/ 94, 0x00,
  /* ABS: 005 Pixels @ 206,029*/ 0, 5, 0x0D, 0x06, 0x03, 0x06, 0x06,
  /* RLE: 175 Pixels @ 211,029*/ 175, 0x01,
  /* ABS: 002 Pixels @ 386,029*/ 0, 2, 0x03, 0x00,
  /* RLE: 020 Pixels @ 000,030*/ 20, 0x02,
  /* ABS: 007 Pixels @ 020,030*/ 0, 7, 0x04, 0x04, 0x03, 0x03, 0x03, 0x04, 0x04,
  /* RLE: 074 Pixels @ 027,030*/ 74, 0x01,
  /* ABS: 002 Pixels @ 101,030*/ 0, 2, 0x04, 0x04,
  /* RLE: 008 Pixels @ 103,030*/ 8, 0x03,
  /* ABS: 002 Pixels @ 111,030*/ 0, 2, 0x04, 0x04,
  /* RLE: 094 Pixels @ 113,030*/ 94, 0x00,
  /* ABS: 005 Pixels @ 207,030*/ 0, 5, 0x0D, 0x0D, 0x03, 0x06, 0x0D,
  /* RLE: 174 Pixels @ 212,030*/ 174, 0x01,
  /* ABS: 002 Pixels @ 386,030*/ 0, 2, 0x03, 0x0E,
  /* RLE: 020 Pixels @ 000,031*/ 20, 0x02,
  /* RLE: 001 Pixels @ 020,031*/ 1, 0x04,
  /* RLE: 005 Pixels @ 021,031*/ 5, 0x03,
  /* RLE: 001 Pixels @ 026,031*/ 1, 0x04,
  /* RLE: 075 Pixels @ 027,031*/ 75, 0x01,
  /* ABS: 002 Pixels @ 102,031*/ 0, 2, 0x04, 0x04,
  /* RLE: 008 Pixels @ 104,031*/ 8, 0x03,
  /* ABS: 002 Pixels @ 112,031*/ 0, 2, 0x04, 0x04,
  /* RLE: 095 Pixels @ 114,031*/ 95, 0x00,
  /* ABS: 004 Pixels @ 209,031*/ 0, 4, 0x0D, 0x03, 0x06, 0x06,
  /* RLE: 172 Pixels @ 213,031*/ 172, 0x01,
  /* ABS: 003 Pixels @ 385,031*/ 0, 3, 0x00, 0x03, 0x03,
  /* RLE: 019 Pixels @ 000,032*/ 19, 0x02,
  /* ABS: 002 Pixels @ 019,032*/ 0, 2, 0x04, 0x04,
  /* RLE: 006 Pixels @ 021,032*/ 6, 0x03,
  /* RLE: 076 Pixels @ 027,032*/ 76, 0x01,
  /* ABS: 002 Pixels @ 103,032*/ 0, 2, 0x04, 0x04,
  /* RLE: 008 Pixels @ 105,032*/ 8, 0x03,
  /* ABS: 002 Pixels @ 113,032*/ 0, 2, 0x04, 0x04,
  /* RLE: 095 Pixels @ 115,032*/ 95, 0x00,
  /* ABS: 004 Pixels @ 210,032*/ 0, 4, 0x0D, 0x06, 0x06, 0x06,
  /* RLE: 166 Pixels @ 214,032*/ 166, 0x01,
  /* RLE: 003 Pixels @ 380,032*/ 3, 0x18,
  /* ABS: 005 Pixels @ 383,032*/ 0, 5, 0x15, 0x03, 0x03, 0x06, 0x0E,
  /* RLE: 020 Pixels @ 000,033*/ 20, 0x02,
  /* RLE: 001 Pixels @ 020,033*/ 1, 0x04,
  /* RLE: 006 Pixels @ 021,033*/ 6, 0x03,
  /* RLE: 001 Pixels @ 027,033*/ 1, 0x04,
  /* RLE: 076 Pixels @ 028,033*/ 76, 0x01,
  /* ABS: 002 Pixels @ 104,033*/ 0, 2, 0x04, 0x04,
  /* RLE: 008 Pixels @ 106,033*/ 8, 0x03,
  /* ABS: 002 Pixels @ 114,033*/ 0, 2, 0x04, 0x04,
  /* RLE: 095 Pixels @ 116,033*/ 95, 0x00,
  /* ABS: 004 Pixels @ 211,033*/ 0, 4, 0x0D, 0x06, 0x06, 0x06,
  /* RLE: 165 Pixels @ 215,033*/ 165, 0x01,
  /* ABS: 008 Pixels @ 380,033*/ 0, 8, 0x03, 0x04, 0x03, 0x03, 0x0A, 0x12, 0x0F, 0x0A,
  /* RLE: 020 Pixels @ 000,034*/ 20, 0x02,
  /* RLE: 001 Pixels @ 020,034*/ 1, 0x04,
  /* RLE: 006 Pixels @ 021,034*/ 6, 0x03,
  /* RLE: 001 Pixels @ 027,034*/ 1, 0x04,
  /* RLE: 077 Pixels @ 028,034*/ 77, 0x01,
  /* ABS: 002 Pixels @ 105,034*/ 0, 2, 0x04, 0x04,
  /* RLE: 008 Pixels @ 107,034*/ 8, 0x03,
  /* ABS: 002 Pixels @ 115,034*/ 0, 2, 0x04, 0x04,
  /* RLE: 062 Pixels @ 117,034*/ 62, 0x00,
  /* ABS: 002 Pixels @ 179,034*/ 0, 2, 0x03, 0x03,
  /* RLE: 031 Pixels @ 181,034*/ 31, 0x00,
  /* ABS: 005 Pixels @ 212,034*/ 0, 5, 0x0D, 0x06, 0x03, 0x06, 0x0D,
  /* RLE: 163 Pixels @ 217,034*/ 163, 0x01,
  /* ABS: 008 Pixels @ 380,034*/ 0, 8, 0x03, 0x0F, 0x04, 0x04, 0x00, 0x07, 0x0D, 0x03,
  /* RLE: 020 Pixels @ 000,035*/ 20, 0x02,
  /* RLE: 001 Pixels @ 020,035*/ 1, 0x04,
  /* RLE: 006 Pixels @ 021,035*/ 6, 0x03,
  /* RLE: 001 Pixels @ 027,035*/ 1, 0x04,
  /* RLE: 078 Pixels @ 028,035*/ 78, 0x01,
  /* ABS: 002 Pixels @ 106,035*/ 0, 2, 0x04, 0x04,
  /* RLE: 008 Pixels @ 108,035*/ 8, 0x03,
  /* ABS: 002 Pixels @ 116,035*/ 0, 2, 0x04, 0x04,
  /* RLE: 060 Pixels @ 118,035*/ 60, 0x00,
  /* ABS: 004 Pixels @ 178,035*/ 0, 4, 0x03, 0x0A, 0x0E, 0x03,
  /* RLE: 031 Pixels @ 182,035*/ 31, 0x00,
  /* ABS: 005 Pixels @ 213,035*/ 0, 5, 0x0D, 0x06, 0x03, 0x06, 0x06,
  /* RLE: 162 Pixels @ 218,035*/ 162, 0x01,
  /* ABS: 008 Pixels @ 380,035*/ 0, 8, 0x03, 0x11, 0x07, 0x0D, 0x03, 0x0E, 0x07, 0x00,
  /* RLE: 020 Pixels @ 000,036*/ 20, 0x02,
  /* RLE: 001 Pixels @ 020,036*/ 1, 0x04,
  /* RLE: 006 Pixels @ 021,036*/ 6, 0x03,
  /* RLE: 001 Pixels @ 027,036*/ 1, 0x04,
  /* RLE: 079 Pixels @ 028,036*/ 79, 0x01,
  /* ABS: 002 Pixels @ 107,036*/ 0, 2, 0x04, 0x04,
  /* RLE: 008 Pixels @ 109,036*/ 8, 0x03,
  /* ABS: 002 Pixels @ 117,036*/ 0, 2, 0x04, 0x04,
  /* RLE: 054 Pixels @ 119,036*/ 54, 0x00,
  /* ABS: 009 Pixels @ 173,036*/ 0, 9, 0x03, 0x00, 0x03, 0x00, 0x03, 0x0B, 0x0D, 0x0E, 0x03,
  /* RLE: 004 Pixels @ 182,036*/ 4, 0x00,
  /* RLE: 001 Pixels @ 186,036*/ 1, 0x0A,
  /* RLE: 027 Pixels @ 187,036*/ 27, 0x00,
  /* ABS: 005 Pixels @ 214,036*/ 0, 5, 0x0D, 0x0D, 0x03, 0x06, 0x06,
  /* RLE: 122 Pixels @ 219,036*/ 122, 0x01,
  /* RLE: 005 Pixels @ 341,036*/ 5, 0x04,
  /* RLE: 034 Pixels @ 346,036*/ 34, 0x01,
  /* ABS: 008 Pixels @ 380,036*/ 0, 8, 0x03, 0x11, 0x0F, 0x06, 0x03, 0x03, 0x0D, 0x0F,
  /* RLE: 020 Pixels @ 000,037*/ 20, 0x02,
  /* RLE: 001 Pixels @ 020,037*/ 1, 0x04,
  /* RLE: 006 Pixels @ 021,037*/ 6, 0x03,
  /* RLE: 001 Pixels @ 027,037*/ 1, 0x04,
  /* RLE: 080 Pixels @ 028,037*/ 80, 0x01,
  /* ABS: 002 Pixels @ 108,037*/ 0, 2, 0x04, 0x04,
  /* RLE: 008 Pixels @ 110,037*/ 8, 0x03,
  /* ABS: 002 Pixels @ 118,037*/ 0, 2, 0x04, 0x04,
  /* RLE: 053 Pixels @ 120,037*/ 53, 0x00,
  /* ABS: 014 Pixels @ 173,037*/ 0, 14, 0x03, 0x0E, 0x0E, 0x00, 0x03, 0x0F, 0x07, 0x03, 0x03, 0x00, 0x03, 0x00, 0x03, 0x03,
  /* RLE: 029 Pixels @ 187,037*/ 29, 0x00,
  /* ABS: 004 Pixels @ 216,037*/ 0, 4, 0x0D, 0x06, 0x06, 0x06,
  /* RLE: 120 Pixels @ 220,037*/ 120, 0x01,
  /* ABS: 002 Pixels @ 340,037*/ 0, 2, 0x04, 0x04,
  /* RLE: 005 Pixels @ 342,037*/ 5, 0x03,
  /* ABS: 002 Pixels @ 347,037*/ 0, 2, 0x04, 0x04,
  /* RLE: 028 Pixels @ 349,037*/ 28, 0x01,
  /* RLE: 005 Pixels @ 377,037*/ 5, 0x03,
  /* ABS: 006 Pixels @ 382,037*/ 0, 6, 0x0B, 0x07, 0x0B, 0x03, 0x00, 0x07,
  /* RLE: 021 Pixels @ 000,038*/ 21, 0x02,
  /* RLE: 007 Pixels @ 021,038*/ 7, 0x03,
  /* RLE: 001 Pixels @ 028,038*/ 1, 0x04,
  /* RLE: 080 Pixels @ 029,038*/ 80, 0x01,
  /* ABS: 002 Pixels @ 109,038*/ 0, 2, 0x04, 0x04,
  /* RLE: 008 Pixels @ 111,038*/ 8, 0x03,
  /* ABS: 002 Pixels @ 119,038*/ 0, 2, 0x04, 0x04,
  /* RLE: 052 Pixels @ 121,038*/ 52, 0x00,
  /* ABS: 014 Pixels @ 173,038*/ 0, 14, 0x03, 0x0B, 0x07, 0x07, 0x00, 0x04, 0x07, 0x04, 0x03, 0x00, 0x03, 0x11, 0x12, 0x03,
  /* RLE: 030 Pixels @ 187,038*/ 30, 0x00,
  /* ABS: 004 Pixels @ 217,038*/ 0, 4, 0x0D, 0x06, 0x06, 0x06,
  /* RLE: 119 Pixels @ 221,038*/ 119, 0x01,
  /* RLE: 001 Pixels @ 340,038*/ 1, 0x04,
  /* RLE: 009 Pixels @ 341,038*/ 9, 0x03,
  /* ABS: 002 Pixels @ 350,038*/ 0, 2, 0x04, 0x04,
  /* RLE: 024 Pixels @ 352,038*/ 24, 0x01,
  /* ABS: 012 Pixels @ 376,038*/ 0, 12, 0x03, 0x03, 0x06, 0x07, 0x12, 0x03, 0x03, 0x12, 0x07, 0x0E, 0x0B, 0x0A,
  /* RLE: 021 Pixels @ 000,039*/ 21, 0x02,
  /* RLE: 001 Pixels @ 021,039*/ 1, 0x04,
  /* RLE: 006 Pixels @ 022,039*/ 6, 0x03,
  /* RLE: 001 Pixels @ 028,039*/ 1, 0x04,
  /* RLE: 081 Pixels @ 029,039*/ 81, 0x01,
  /* ABS: 002 Pixels @ 110,039*/ 0, 2, 0x04, 0x04,
  /* RLE: 008 Pixels @ 112,039*/ 8, 0x03,
  /* ABS: 002 Pixels @ 120,039*/ 0, 2, 0x04, 0x04,
  /* RLE: 050 Pixels @ 122,039*/ 50, 0x00,
  /* ABS: 015 Pixels @ 172,039*/ 0, 15, 0x04, 0x03, 0x0E, 0x07, 0x11, 0x03, 0x03, 0x06, 0x0F, 0x03, 0x03, 0x03, 0x0A, 0x00, 0x03,
  /* RLE: 031 Pixels @ 187,039*/ 31, 0x00,
  /* ABS: 004 Pixels @ 218,039*/ 0, 4, 0x0D, 0x06, 0x03, 0x06,
  /* RLE: 118 Pixels @ 222,039*/ 118, 0x01,
  /* RLE: 001 Pixels @ 340,039*/ 1, 0x04,
  /* RLE: 012 Pixels @ 341,039*/ 12, 0x03,
  /* ABS: 002 Pixels @ 353,039*/ 0, 2, 0x04, 0x04,
  /* RLE: 020 Pixels @ 355,039*/ 20, 0x01,
  /* ABS: 013 Pixels @ 375,039*/ 0, 13, 0x15, 0x03, 0x11, 0x0E, 0x0A, 0x00, 0x00, 0x03, 0x03, 0x0D, 0x0F, 0x04, 0x03,
  /* RLE: 021 Pixels @ 000,040*/ 21, 0x02,
  /* RLE: 001 Pixels @ 021,040*/ 1, 0x04,
  /* RLE: 006 Pixels @ 022,040*/ 6, 0x03,
  /* RLE: 001 Pixels @ 028,040*/ 1, 0x04,
  /* RLE: 082 Pixels @ 029,040*/ 82, 0x01,
  /* ABS: 002 Pixels @ 111,040*/ 0, 2, 0x04, 0x04,
  /* RLE: 008 Pixels @ 113,040*/ 8, 0x03,
  /* ABS: 002 Pixels @ 121,040*/ 0, 2, 0x04, 0x04,
  /* RLE: 048 Pixels @ 123,040*/ 48, 0x00,
  /* RLE: 004 Pixels @ 171,040*/ 4, 0x03,
  /* ABS: 011 Pixels @ 175,040*/ 0, 11, 0x0A, 0x07, 0x0B, 0x03, 0x00, 0x07, 0x0E, 0x03, 0x00, 0x03, 0x03,
  /* RLE: 033 Pixels @ 186,040*/ 33, 0x00,
  /* ABS: 005 Pixels @ 219,040*/ 0, 5, 0x0D, 0x06, 0x03, 0x06, 0x0D,
  /* RLE: 116 Pixels @ 224,040*/ 116, 0x01,
  /* RLE: 001 Pixels @ 340,040*/ 1, 0x04,
  /* RLE: 015 Pixels @ 341,040*/ 15, 0x03,
  /* ABS: 002 Pixels @ 356,040*/ 0, 2, 0x04, 0x04,
  /* RLE: 010 Pixels @ 358,040*/ 10, 0x01,
  /* RLE: 003 Pixels @ 368,040*/ 3, 0x15,
  /* RLE: 004 Pixels @ 371,040*/ 4, 0x01,
  /* ABS: 008 Pixels @ 375,040*/ 0, 8, 0x00, 0x00, 0x07, 0x0B, 0x0D, 0x07, 0x07, 0x0E,
  /* RLE: 004 Pixels @ 383,040*/ 4, 0x03,
  /* RLE: 001 Pixels @ 387,040*/ 1, 0x00,
  /* RLE: 021 Pixels @ 000,041*/ 21, 0x02,
  /* RLE: 001 Pixels @ 021,041*/ 1, 0x04,
  /* RLE: 006 Pixels @ 022,041*/ 6, 0x03,
  /* RLE: 001 Pixels @ 028,041*/ 1, 0x04,
  /* RLE: 083 Pixels @ 029,041*/ 83, 0x01,
  /* ABS: 002 Pixels @ 112,041*/ 0, 2, 0x04, 0x04,
  /* RLE: 008 Pixels @ 114,041*/ 8, 0x03,
  /* ABS: 002 Pixels @ 122,041*/ 0, 2, 0x04, 0x04,
  /* RLE: 046 Pixels @ 124,041*/ 46, 0x00,
  /* ABS: 015 Pixels @ 170,041*/ 0, 15, 0x03, 0x0A, 0x06, 0x12, 0x03, 0x03, 0x0E, 0x07, 0x04, 0x04, 0x04, 0x00, 0x03, 0x04, 0x04,
  /* RLE: 035 Pixels @ 185,041*/ 35, 0x00,
  /* ABS: 005 Pixels @ 220,041*/ 0, 5, 0x0D, 0x06, 0x03, 0x06, 0x06,
  /* RLE: 115 Pixels @ 225,041*/ 115, 0x01,
  /* ABS: 002 Pixels @ 340,041*/ 0, 2, 0x04, 0x04,
  /* RLE: 017 Pixels @ 342,041*/ 17, 0x03,
  /* ABS: 002 Pixels @ 359,041*/ 0, 2, 0x04, 0x04,
  /* RLE: 006 Pixels @ 361,041*/ 6, 0x01,
  /* ABS: 005 Pixels @ 367,041*/ 0, 5, 0x15, 0x03, 0x00, 0x03, 0x18,
  /* RLE: 005 Pixels @ 372,041*/ 5, 0x03,
  /* ABS: 011 Pixels @ 377,041*/ 0, 11, 0x0D, 0x07, 0x0D, 0x04, 0x0E, 0x06, 0x03, 0x03, 0x04, 0x04, 0x04,
  /* RLE: 021 Pixels @ 000,042*/ 21, 0x02,
  /* RLE: 001 Pixels @ 021,042*/ 1, 0x04,
  /* RLE: 006 Pixels @ 022,042*/ 6, 0x03,
  /* RLE: 001 Pixels @ 028,042*/ 1, 0x04,
  /* RLE: 084 Pixels @ 029,042*/ 84, 0x01,
  /* ABS: 002 Pixels @ 113,042*/ 0, 2, 0x04, 0x04,
  /* RLE: 008 Pixels @ 115,042*/ 8, 0x03,
  /* ABS: 002 Pixels @ 123,042*/ 0, 2, 0x04, 0x04,
  /* RLE: 044 Pixels @ 125,042*/ 44, 0x00,
  /* ABS: 011 Pixels @ 169,042*/ 0, 11, 0x03, 0x0E, 0x11, 0x0B, 0x0A, 0x03, 0x03, 0x03, 0x11, 0x07, 0x04,
  /* RLE: 004 Pixels @ 180,042*/ 4, 0x03,
  /* RLE: 001 Pixels @ 184,042*/ 1, 0x04,
  /* RLE: 036 Pixels @ 185,042*/ 36, 0x00,
  /* ABS: 005 Pixels @ 221,042*/ 0, 5, 0x06, 0x0D, 0x06, 0x06, 0x06,
  /* RLE: 115 Pixels @ 226,042*/ 115, 0x01,
  /* ABS: 002 Pixels @ 341,042*/ 0, 2, 0x04, 0x04,
  /* RLE: 019 Pixels @ 343,042*/ 19, 0x03,
  /* ABS: 026 Pixels @ 362,042*/ 0, 26, 0x04, 0x04, 0x01, 0x01, 0x01, 0x15, 0x00, 0x0F, 0x04, 0x03, 0x03, 0x06, 0x07, 0x11, 0x00, 0x03, 0x03, 0x0A, 0x0A, 0x0F, 0x0B, 0x03, 0x04, 0x04, 0x01, 0x01,
  /* RLE: 021 Pixels @ 000,043*/ 21, 0x02,
  /* RLE: 001 Pixels @ 021,043*/ 1, 0x04,
  /* RLE: 007 Pixels @ 022,043*/ 7, 0x03,
  /* RLE: 085 Pixels @ 029,043*/ 85, 0x01,
  /* ABS: 002 Pixels @ 114,043*/ 0, 2, 0x04, 0x04,
  /* RLE: 008 Pixels @ 116,043*/ 8, 0x03,
  /* ABS: 002 Pixels @ 124,043*/ 0, 2, 0x04, 0x04,
  /* RLE: 039 Pixels @ 126,043*/ 39, 0x00,
  /* ABS: 014 Pixels @ 165,043*/ 0, 14, 0x03, 0x03, 0x00, 0x03, 0x03, 0x07, 0x0B, 0x0E, 0x11, 0x07, 0x0E, 0x03, 0x03, 0x00,
  /* RLE: 006 Pixels @ 179,043*/ 6, 0x03,
  /* RLE: 038 Pixels @ 185,043*/ 38, 0x00,
  /* ABS: 004 Pixels @ 223,043*/ 0, 4, 0x0D, 0x06, 0x06, 0x06,
  /* RLE: 116 Pixels @ 227,043*/ 116, 0x01,
  /* RLE: 003 Pixels @ 343,043*/ 3, 0x04,
  /* RLE: 023 Pixels @ 346,043*/ 23, 0x03,
  /* ABS: 019 Pixels @ 369,043*/ 0, 19, 0x0F, 0x12, 0x03, 0x06, 0x06, 0x00, 0x0D, 0x0F, 0x03, 0x0B, 0x07, 0x0F, 0x0B, 0x03, 0x00, 0x04, 0x01, 0x01, 0x01,
  /* RLE: 022 Pixels @ 000,044*/ 22, 0x02,
  /* RLE: 001 Pixels @ 022,044*/ 1, 0x04,
  /* RLE: 006 Pixels @ 023,044*/ 6, 0x03,
  /* RLE: 001 Pixels @ 029,044*/ 1, 0x04,
  /* RLE: 085 Pixels @ 030,044*/ 85, 0x01,
  /* ABS: 002 Pixels @ 115,044*/ 0, 2, 0x04, 0x04,
  /* RLE: 008 Pixels @ 117,044*/ 8, 0x03,
  /* ABS: 002 Pixels @ 125,044*/ 0, 2, 0x04, 0x04,
  /* RLE: 037 Pixels @ 127,044*/ 37, 0x00,
  /* ABS: 012 Pixels @ 164,044*/ 0, 12, 0x03, 0x00, 0x0F, 0x07, 0x0E, 0x03, 0x0E, 0x07, 0x0F, 0x0E, 0x06, 0x11,
  /* RLE: 009 Pixels @ 176,044*/ 9, 0x03,
  /* RLE: 001 Pixels @ 185,044*/ 1, 0x04,
  /* RLE: 038 Pixels @ 186,044*/ 38, 0x00,
  /* ABS: 004 Pixels @ 224,044*/ 0, 4, 0x0D, 0x06, 0x03, 0x0D,
  /* RLE: 118 Pixels @ 228,044*/ 118, 0x01,
  /* RLE: 003 Pixels @ 346,044*/ 3, 0x04,
  /* RLE: 016 Pixels @ 349,044*/ 16, 0x03,
  /* ABS: 013 Pixels @ 365,044*/ 0, 13, 0x00, 0x0D, 0x00, 0x03, 0x0B, 0x07, 0x00, 0x07, 0x04, 0x0B, 0x0F, 0x04, 0x00,
  /* RLE: 004 Pixels @ 378,044*/ 4, 0x03,
  /* RLE: 001 Pixels @ 382,044*/ 1, 0x00,
  /* RLE: 005 Pixels @ 383,044*/ 5, 0x01,
  /* RLE: 022 Pixels @ 000,045*/ 22, 0x02,
  /* RLE: 001 Pixels @ 022,045*/ 1, 0x04,
  /* RLE: 006 Pixels @ 023,045*/ 6, 0x03,
  /* RLE: 001 Pixels @ 029,045*/ 1, 0x04,
  /* RLE: 086 Pixels @ 030,045*/ 86, 0x01,
  /* ABS: 002 Pixels @ 116,045*/ 0, 2, 0x04, 0x04,
  /* RLE: 008 Pixels @ 118,045*/ 8, 0x03,
  /* ABS: 002 Pixels @ 126,045*/ 0, 2, 0x04, 0x04,
  /* RLE: 034 Pixels @ 128,045*/ 34, 0x00,
  /* ABS: 016 Pixels @ 162,045*/ 0, 16, 0x04, 0x03, 0x0E, 0x06, 0x0B, 0x0B, 0x07, 0x04, 0x03, 0x03, 0x00, 0x00, 0x0D, 0x0E, 0x03, 0x04,
  /* RLE: 007 Pixels @ 178,045*/ 7, 0x03,
  /* RLE: 001 Pixels @ 185,045*/ 1, 0x04,
  /* RLE: 039 Pixels @ 186,045*/ 39, 0x00,
  /* ABS: 004 Pixels @ 225,045*/ 0, 4, 0x0D, 0x06, 0x03, 0x06,
  /* RLE: 120 Pixels @ 229,045*/ 120, 0x01,
  /* RLE: 003 Pixels @ 349,045*/ 3, 0x04,
  /* RLE: 013 Pixels @ 352,045*/ 13, 0x03,
  /* ABS: 017 Pixels @ 365,045*/ 0, 17, 0x11, 0x07, 0x0F, 0x0A, 0x00, 0x07, 0x0B, 0x11, 0x07, 0x12, 0x03, 0x0E, 0x06, 0x03, 0x03, 0x04, 0x04,
  /* RLE: 006 Pixels @ 382,045*/ 6, 0x01,
  /* RLE: 022 Pixels @ 000,046*/ 22, 0x02,
  /* RLE: 001 Pixels @ 022,046*/ 1, 0x04,
  /* RLE: 007 Pixels @ 023,046*/ 7, 0x03,
  /* RLE: 001 Pixels @ 030,046*/ 1, 0x00,
  /* RLE: 086 Pixels @ 031,046*/ 86, 0x01,
  /* ABS: 002 Pixels @ 117,046*/ 0, 2, 0x04, 0x04,
  /* RLE: 008 Pixels @ 119,046*/ 8, 0x03,
  /* ABS: 002 Pixels @ 127,046*/ 0, 2, 0x04, 0x04,
  /* RLE: 031 Pixels @ 129,046*/ 31, 0x00,
  /* RLE: 004 Pixels @ 160,046*/ 4, 0x03,
  /* ABS: 015 Pixels @ 164,046*/ 0, 15, 0x11, 0x07, 0x03, 0x03, 0x06, 0x0F, 0x03, 0x0B, 0x07, 0x07, 0x0E, 0x03, 0x03, 0x00, 0x04,
  /* RLE: 007 Pixels @ 179,046*/ 7, 0x03,
  /* RLE: 040 Pixels @ 186,046*/ 40, 0x00,
  /* ABS: 004 Pixels @ 226,046*/ 0, 4, 0x0D, 0x06, 0x03, 0x06,
  /* RLE: 122 Pixels @ 230,046*/ 122, 0x01,
  /* RLE: 003 Pixels @ 352,046*/ 3, 0x04,
  /* RLE: 007 Pixels @ 355,046*/ 7, 0x03,
  /* ABS: 019 Pixels @ 362,046*/ 0, 19, 0x00, 0x03, 0x03, 0x0B, 0x07, 0x06, 0x07, 0x04, 0x12, 0x11, 0x0A, 0x07, 0x12, 0x0B, 0x0F, 0x0B, 0x03, 0x04, 0x04,
  /* RLE: 007 Pixels @ 381,046*/ 7, 0x01,
  /* RLE: 021 Pixels @ 000,047*/ 21, 0x02,
  /* RLE: 001 Pixels @ 021,047*/ 1, 0x18,
  /* RLE: 005 Pixels @ 022,047*/ 5, 0x03,
  /* ABS: 005 Pixels @ 027,047*/ 0, 5, 0x00, 0x04, 0x0E, 0x03, 0x18,
  /* RLE: 086 Pixels @ 032,047*/ 86, 0x01,
  /* ABS: 002 Pixels @ 118,047*/ 0, 2, 0x04, 0x04,
  /* RLE: 009 Pixels @ 120,047*/ 9, 0x03,
  /* ABS: 002 Pixels @ 129,047*/ 0, 2, 0x04, 0x04,
  /* RLE: 028 Pixels @ 131,047*/ 28, 0x00,
  /* ABS: 012 Pixels @ 159,047*/ 0, 12, 0x03, 0x0A, 0x06, 0x0E, 0x00, 0x0A, 0x07, 0x0B, 0x03, 0x00, 0x0F, 0x0E,
  /* RLE: 005 Pixels @ 171,047*/ 5, 0x03,
  /* RLE: 003 Pixels @ 176,047*/ 3, 0x00,
  /* RLE: 001 Pixels @ 179,047*/ 1, 0x04,
  /* RLE: 006 Pixels @ 180,047*/ 6, 0x03,
  /* RLE: 001 Pixels @ 186,047*/ 1, 0x04,
  /* RLE: 040 Pixels @ 187,047*/ 40, 0x00,
  /* RLE: 001 Pixels @ 227,047*/ 1, 0x0D,
  /* RLE: 004 Pixels @ 228,047*/ 4, 0x06,
  /* RLE: 123 Pixels @ 232,047*/ 123, 0x01,
  /* RLE: 003 Pixels @ 355,047*/ 3, 0x04,
  /* RLE: 003 Pixels @ 358,047*/ 3, 0x03,
  /* ABS: 018 Pixels @ 361,047*/ 0, 18, 0x00, 0x07, 0x0D, 0x03, 0x00, 0x07, 0x04, 0x0E, 0x07, 0x11, 0x07, 0x0A, 0x0A, 0x0D, 0x0D, 0x04, 0x03, 0x00,
  /* RLE: 009 Pixels @ 379,047*/ 9, 0x01,
  /* RLE: 021 Pixels @ 000,048*/ 21, 0x02,
  /* ABS: 005 Pixels @ 021,048*/ 0, 5, 0x03, 0x00, 0x0B, 0x12, 0x0D,
  /* RLE: 004 Pixels @ 026,048*/ 4, 0x07,
  /* ABS: 002 Pixels @ 030,048*/ 0, 2, 0x0A, 0x15,
  /* RLE: 083 Pixels @ 032,048*/ 83, 0x01,
  /* ABS: 005 Pixels @ 115,048*/ 0, 5, 0x15, 0x03, 0x00, 0x18, 0x00,
  /* RLE: 010 Pixels @ 120,048*/ 10, 0x03,
  /* ABS: 002 Pixels @ 130,048*/ 0, 2, 0x04, 0x04,
  /* RLE: 026 Pixels @ 132,048*/ 26, 0x00,
  /* ABS: 015 Pixels @ 158,048*/ 0, 15, 0x03, 0x04, 0x0F, 0x0B, 0x0D, 0x0F, 0x03, 0x12, 0x0F, 0x00, 0x03, 0x04, 0x00, 0x03, 0x04,
  /* RLE: 005 Pixels @ 173,048*/ 5, 0x00,
  /* ABS: 002 Pixels @ 178,048*/ 0, 2, 0x0A, 0x04,
  /* RLE: 006 Pixels @ 180,048*/ 6, 0x03,
  /* RLE: 001 Pixels @ 186,048*/ 1, 0x04,
  /* RLE: 041 Pixels @ 187,048*/ 41, 0x00,
  /* RLE: 001 Pixels @ 228,048*/ 1, 0x0D,
  /* RLE: 004 Pixels @ 229,048*/ 4, 0x06,
  /* RLE: 125 Pixels @ 233,048*/ 125, 0x01,
  /* ABS: 015 Pixels @ 358,048*/ 0, 15, 0x04, 0x04, 0x00, 0x03, 0x04, 0x07, 0x0D, 0x03, 0x06, 0x12, 0x03, 0x04, 0x07, 0x07, 0x0E,
  /* RLE: 004 Pixels @ 373,048*/ 4, 0x03,
  /* RLE: 001 Pixels @ 377,048*/ 1, 0x00,
  /* RLE: 010 Pixels @ 378,048*/ 10, 0x01,
  /* RLE: 021 Pixels @ 000,049*/ 21, 0x02,
  /* ABS: 011 Pixels @ 021,049*/ 0, 11, 0x03, 0x06, 0x07, 0x07, 0x06, 0x0E, 0x04, 0x0A, 0x03, 0x03, 0x15,
  /* RLE: 083 Pixels @ 032,049*/ 83, 0x01,
  /* ABS: 007 Pixels @ 115,049*/ 0, 7, 0x03, 0x0A, 0x04, 0x03, 0x03, 0x0B, 0x0A,
  /* RLE: 009 Pixels @ 122,049*/ 9, 0x03,
  /* ABS: 002 Pixels @ 131,049*/ 0, 2, 0x04, 0x04,
  /* RLE: 023 Pixels @ 133,049*/ 23, 0x00,
  /* ABS: 014 Pixels @ 156,049*/ 0, 14, 0x04, 0x04, 0x03, 0x06, 0x0E, 0x00, 0x06, 0x12, 0x00, 0x00, 0x0F, 0x12, 0x03, 0x03,
  /* RLE: 009 Pixels @ 170,049*/ 9, 0x00,
  /* RLE: 001 Pixels @ 179,049*/ 1, 0x04,
  /* RLE: 007 Pixels @ 180,049*/ 7, 0x03,
  /* RLE: 042 Pixels @ 187,049*/ 42, 0x00,
  /* RLE: 001 Pixels @ 229,049*/ 1, 0x0D,
  /* RLE: 004 Pixels @ 230,049*/ 4, 0x06,
  /* RLE: 127 Pixels @ 234,049*/ 127, 0x01,
  /* ABS: 015 Pixels @ 361,049*/ 0, 15, 0x00, 0x03, 0x04, 0x0F, 0x0D, 0x0E, 0x07, 0x00, 0x03, 0x0A, 0x0F, 0x0B, 0x03, 0x04, 0x04,
  /* RLE: 012 Pixels @ 376,049*/ 12, 0x01,
  /* RLE: 021 Pixels @ 000,050*/ 21, 0x02,
  /* ABS: 010 Pixels @ 021,050*/ 0, 10, 0x03, 0x00, 0x00, 0x03, 0x03, 0x03, 0x00, 0x00, 0x03, 0x04,
  /* RLE: 084 Pixels @ 031,050*/ 84, 0x01,
  /* ABS: 007 Pixels @ 115,050*/ 0, 7, 0x03, 0x06, 0x0F, 0x00, 0x00, 0x07, 0x0A,
  /* RLE: 010 Pixels @ 122,050*/ 10, 0x03,
  /* ABS: 002 Pixels @ 132,050*/ 0, 2, 0x04, 0x04,
  /* RLE: 016 Pixels @ 134,050*/ 16, 0x00,
  /* RLE: 001 Pixels @ 150,050*/ 1, 0x03,
  /* RLE: 004 Pixels @ 151,050*/ 4, 0x00,
  /* RLE: 004 Pixels @ 155,050*/ 4, 0x03,
  /* ABS: 011 Pixels @ 159,050*/ 0, 11, 0x0E, 0x0F, 0x11, 0x0A, 0x04, 0x06, 0x03, 0x0A, 0x00, 0x03, 0x04,
  /* RLE: 010 Pixels @ 170,050*/ 10, 0x00,
  /* RLE: 001 Pixels @ 180,050*/ 1, 0x04,
  /* RLE: 006 Pixels @ 181,050*/ 6, 0x03,
  /* RLE: 001 Pixels @ 187,050*/ 1, 0x04,
  /* RLE: 043 Pixels @ 188,050*/ 43, 0x00,
  /* ABS: 004 Pixels @ 231,050*/ 0, 4, 0x0D, 0x03, 0x06, 0x0D,
  /* RLE: 127 Pixels @ 235,050*/ 127, 0x01,
  /* ABS: 007 Pixels @ 362,050*/ 0, 7, 0x18, 0x03, 0x00, 0x0F, 0x07, 0x07, 0x0B,
  /* RLE: 004 Pixels @ 369,050*/ 4, 0x03,
  /* ABS: 002 Pixels @ 373,050*/ 0, 2, 0x00, 0x04,
  /* RLE: 013 Pixels @ 375,050*/ 13, 0x01,
  /* RLE: 021 Pixels @ 000,051*/ 21, 0x02,
  /* ABS: 010 Pixels @ 021,051*/ 0, 10, 0x18, 0x03, 0x04, 0x0E, 0x06, 0x0F, 0x07, 0x07, 0x03, 0x04,
  /* RLE: 084 Pixels @ 031,051*/ 84, 0x01,
  /* ABS: 007 Pixels @ 115,051*/ 0, 7, 0x03, 0x00, 0x0F, 0x0D, 0x00, 0x07, 0x00,
  /* RLE: 011 Pixels @ 122,051*/ 11, 0x03,
  /* ABS: 002 Pixels @ 133,051*/ 0, 2, 0x04, 0x04,
  /* RLE: 014 Pixels @ 135,051*/ 14, 0x00,
  /* ABS: 017 Pixels @ 149,051*/ 0, 17, 0x03, 0x0B, 0x03, 0x00, 0x00, 0x03, 0x0A, 0x06, 0x12, 0x03, 0x00, 0x07, 0x12, 0x0A, 0x06, 0x0E, 0x03,
  /* RLE: 014 Pixels @ 166,051*/ 14, 0x00,
  /* RLE: 001 Pixels @ 180,051*/ 1, 0x04,
  /* RLE: 006 Pixels @ 181,051*/ 6, 0x03,
  /* RLE: 001 Pixels @ 187,051*/ 1, 0x04,
  /* RLE: 044 Pixels @ 188,051*/ 44, 0x00,
  /* ABS: 004 Pixels @ 232,051*/ 0, 4, 0x0D, 0x03, 0x03, 0x06,
  /* RLE: 127 Pixels @ 236,051*/ 127, 0x01,
  /* ABS: 010 Pixels @ 363,051*/ 0, 10, 0x15, 0x03, 0x00, 0x11, 0x07, 0x11, 0x03, 0x04, 0x04, 0x04,
  /* RLE: 015 Pixels @ 373,051*/ 15, 0x01,
  /* RLE: 022 Pixels @ 000,052*/ 22, 0x02,
  /* ABS: 009 Pixels @ 022,052*/ 0, 9, 0x03, 0x06, 0x07, 0x06, 0x0E, 0x0E, 0x0D, 0x03, 0x00,
  /* RLE: 084 Pixels @ 031,052*/ 84, 0x01,
  /* ABS: 009 Pixels @ 115,052*/ 0, 9, 0x00, 0x03, 0x0A, 0x07, 0x0D, 0x07, 0x07, 0x0E, 0x00,
  /* RLE: 010 Pixels @ 124,052*/ 10, 0x03,
  /* ABS: 002 Pixels @ 134,052*/ 0, 2, 0x04, 0x04,
  /* RLE: 013 Pixels @ 136,052*/ 13, 0x00,
  /* ABS: 018 Pixels @ 149,052*/ 0, 18, 0x0A, 0x07, 0x0B, 0x03, 0x03, 0x0E, 0x11, 0x0B, 0x0A, 0x03, 0x03, 0x0A, 0x11, 0x0F, 0x0E, 0x03, 0x00, 0x04,
  /* RLE: 012 Pixels @ 167,052*/ 12, 0x00,
  /* ABS: 002 Pixels @ 179,052*/ 0, 2, 0x0A, 0x04,
  /* RLE: 007 Pixels @ 181,052*/ 7, 0x03,
  /* RLE: 045 Pixels @ 188,052*/ 45, 0x00,
  /* ABS: 004 Pixels @ 233,052*/ 0, 4, 0x0D, 0x06, 0x06, 0x06,
  /* RLE: 127 Pixels @ 237,052*/ 127, 0x01,
  /* ABS: 006 Pixels @ 364,052*/ 0, 6, 0x15, 0x03, 0x03, 0x06, 0x0A, 0x03,
  /* RLE: 018 Pixels @ 370,052*/ 18, 0x01,
  /* RLE: 022 Pixels @ 000,053*/ 22, 0x02,
  /* RLE: 006 Pixels @ 022,053*/ 6, 0x03,
  /* ABS: 003 Pixels @ 028,053*/ 0, 3, 0x06, 0x0B, 0x03,
  /* RLE: 082 Pixels @ 031,053*/ 82, 0x01,
  /* ABS: 014 Pixels @ 113,053*/ 0, 14, 0x15, 0x03, 0x03, 0x0A, 0x00, 0x0E, 0x07, 0x0D, 0x0B, 0x11, 0x07, 0x0E, 0x03, 0x04,
  /* RLE: 008 Pixels @ 127,053*/ 8, 0x03,
  /* ABS: 002 Pixels @ 135,053*/ 0, 2, 0x04, 0x04,
  /* RLE: 008 Pixels @ 137,053*/ 8, 0x00,
  /* RLE: 005 Pixels @ 145,053*/ 5, 0x03,
  /* ABS: 010 Pixels @ 150,053*/ 0, 10, 0x0E, 0x07, 0x00, 0x03, 0x07, 0x0B, 0x0E, 0x11, 0x07, 0x0E,
  /* RLE: 005 Pixels @ 160,053*/ 5, 0x03,
  /* RLE: 016 Pixels @ 165,053*/ 16, 0x00,
  /* RLE: 001 Pixels @ 181,053*/ 1, 0x04,
  /* RLE: 006 Pixels @ 182,053*/ 6, 0x03,
  /* RLE: 001 Pixels @ 188,053*/ 1, 0x04,
  /* RLE: 045 Pixels @ 189,053*/ 45, 0x00,
  /* RLE: 001 Pixels @ 234,053*/ 1, 0x0D,
  /* RLE: 004 Pixels @ 235,053*/ 4, 0x06,
  /* RLE: 127 Pixels @ 239,053*/ 127, 0x01,
  /* ABS: 004 Pixels @ 366,053*/ 0, 4, 0x00, 0x03, 0x03, 0x15,
  /* RLE: 018 Pixels @ 370,053*/ 18, 0x01,
  /* RLE: 022 Pixels @ 000,054*/ 22, 0x02,
  /* ABS: 009 Pixels @ 022,054*/ 0, 9, 0x00, 0x03, 0x03, 0x0A, 0x04, 0x0E, 0x0F, 0x0B, 0x03,
  /* RLE: 081 Pixels @ 031,054*/ 81, 0x01,
  /* ABS: 016 Pixels @ 112,054*/ 0, 16, 0x04, 0x03, 0x00, 0x0D, 0x07, 0x0F, 0x03, 0x0D, 0x0F, 0x03, 0x03, 0x0A, 0x03, 0x00, 0x04, 0x04,
  /* RLE: 008 Pixels @ 128,054*/ 8, 0x03,
  /* ABS: 002 Pixels @ 136,054*/ 0, 2, 0x04, 0x04,
  /* RLE: 006 Pixels @ 138,054*/ 6, 0x00,
  /* ABS: 020 Pixels @ 144,054*/ 0, 20, 0x03, 0x03, 0x0D, 0x00, 0x03, 0x03, 0x03, 0x11, 0x06, 0x03, 0x0E, 0x07, 0x0F, 0x0E, 0x06, 0x11, 0x03, 0x04, 0x04, 0x04,
  /* RLE: 017 Pixels @ 164,054*/ 17, 0x00,
  /* RLE: 001 Pixels @ 181,054*/ 1, 0x04,
  /* RLE: 006 Pixels @ 182,054*/ 6, 0x03,
  /* RLE: 001 Pixels @ 188,054*/ 1, 0x04,
  /* RLE: 046 Pixels @ 189,054*/ 46, 0x00,
  /* RLE: 001 Pixels @ 235,054*/ 1, 0x0D,
  /* RLE: 004 Pixels @ 236,054*/ 4, 0x06,
  /* RLE: 148 Pixels @ 240,054*/ 148, 0x01,
  /* RLE: 022 Pixels @ 000,055*/ 22, 0x02,
  /* ABS: 002 Pixels @ 022,055*/ 0, 2, 0x00, 0x0A,
  /* RLE: 005 Pixels @ 024,055*/ 5, 0x07,
  /* ABS: 003 Pixels @ 029,055*/ 0, 3, 0x00, 0x03, 0x04,
  /* RLE: 077 Pixels @ 032,055*/ 77, 0x01,
  /* ABS: 014 Pixels @ 109,055*/ 0, 14, 0x15, 0x00, 0x00, 0x00, 0x03, 0x06, 0x12, 0x03, 0x00, 0x03, 0x00, 0x07, 0x0E, 0x03,
  /* RLE: 004 Pixels @ 123,055*/ 4, 0x00,
  /* ABS: 002 Pixels @ 127,055*/ 0, 2, 0x04, 0x04,
  /* RLE: 008 Pixels @ 129,055*/ 8, 0x03,
  /* ABS: 002 Pixels @ 137,055*/ 0, 2, 0x04, 0x04,
  /* RLE: 004 Pixels @ 139,055*/ 4, 0x00,
  /* ABS: 020 Pixels @ 143,055*/ 0, 20, 0x03, 0x0E, 0x00, 0x0B, 0x03, 0x0B, 0x00, 0x03, 0x0A, 0x07, 0x0B, 0x03, 0x03, 0x00, 0x00, 0x0D, 0x0E, 0x03, 0x04, 0x0A,
  /* RLE: 018 Pixels @ 163,055*/ 18, 0x00,
  /* RLE: 001 Pixels @ 181,055*/ 1, 0x04,
  /* RLE: 007 Pixels @ 182,055*/ 7, 0x03,
  /* RLE: 047 Pixels @ 189,055*/ 47, 0x00,
  /* ABS: 005 Pixels @ 236,055*/ 0, 5, 0x0D, 0x06, 0x03, 0x06, 0x06,
  /* RLE: 147 Pixels @ 241,055*/ 147, 0x01,
  /* RLE: 022 Pixels @ 000,056*/ 22, 0x02,
  /* ABS: 010 Pixels @ 022,056*/ 0, 10, 0x18, 0x03, 0x0B, 0x04, 0x0A, 0x03, 0x0B, 0x0D, 0x03, 0x00,
  /* RLE: 075 Pixels @ 032,056*/ 75, 0x01,
  /* ABS: 017 Pixels @ 107,056*/ 0, 17, 0x15, 0x03, 0x03, 0x0A, 0x00, 0x03, 0x03, 0x07, 0x0B, 0x03, 0x03, 0x0B, 0x06, 0x0A, 0x03, 0x03, 0x04,
  /* RLE: 004 Pixels @ 124,056*/ 4, 0x00,
  /* ABS: 002 Pixels @ 128,056*/ 0, 2, 0x04, 0x04,
  /* RLE: 008 Pixels @ 130,056*/ 8, 0x03,
  /* ABS: 022 Pixels @ 138,056*/ 0, 22, 0x04, 0x04, 0x00, 0x03, 0x03, 0x03, 0x0E, 0x00, 0x03, 0x03, 0x0F, 0x12, 0x03, 0x03, 0x12, 0x0F, 0x03, 0x0B, 0x07, 0x07, 0x0E, 0x03,
  /* RLE: 021 Pixels @ 160,056*/ 21, 0x00,
  /* ABS: 002 Pixels @ 181,056*/ 0, 2, 0x0A, 0x04,
  /* RLE: 006 Pixels @ 183,056*/ 6, 0x03,
  /* RLE: 001 Pixels @ 189,056*/ 1, 0x04,
  /* RLE: 048 Pixels @ 190,056*/ 48, 0x00,
  /* ABS: 004 Pixels @ 238,056*/ 0, 4, 0x0D, 0x03, 0x06, 0x0D,
  /* RLE: 146 Pixels @ 242,056*/ 146, 0x01,
  /* RLE: 023 Pixels @ 000,057*/ 23, 0x02,
  /* RLE: 005 Pixels @ 023,057*/ 5, 0x03,
  /* ABS: 004 Pixels @ 028,057*/ 0, 4, 0x04, 0x07, 0x00, 0x00,
  /* RLE: 071 Pixels @ 032,057*/ 71, 0x01,
  /* ABS: 020 Pixels @ 103,057*/ 0, 20, 0x15, 0x00, 0x00, 0x15, 0x03, 0x00, 0x0D, 0x07, 0x07, 0x0F, 0x00, 0x0D, 0x0F, 0x03, 0x03, 0x0E, 0x11, 0x03, 0x00, 0x04,
  /* RLE: 006 Pixels @ 123,057*/ 6, 0x00,
  /* ABS: 002 Pixels @ 129,057*/ 0, 2, 0x04, 0x04,
  /* RLE: 010 Pixels @ 131,057*/ 10, 0x03,
  /* ABS: 014 Pixels @ 141,057*/ 0, 14, 0x00, 0x0F, 0x00, 0x03, 0x0E, 0x00, 0x03, 0x0B, 0x07, 0x0A, 0x03, 0x00, 0x0F, 0x12,
  /* RLE: 005 Pixels @ 155,057*/ 5, 0x03,
  /* RLE: 022 Pixels @ 160,057*/ 22, 0x00,
  /* RLE: 001 Pixels @ 182,057*/ 1, 0x04,
  /* RLE: 006 Pixels @ 183,057*/ 6, 0x03,
  /* RLE: 001 Pixels @ 189,057*/ 1, 0x04,
  /* RLE: 049 Pixels @ 190,057*/ 49, 0x00,
  /* ABS: 004 Pixels @ 239,057*/ 0, 4, 0x0D, 0x03, 0x06, 0x06,
  /* RLE: 145 Pixels @ 243,057*/ 145, 0x01,
  /* RLE: 023 Pixels @ 000,058*/ 23, 0x02,
  /* ABS: 009 Pixels @ 023,058*/ 0, 9, 0x03, 0x04, 0x0E, 0x0D, 0x07, 0x07, 0x0F, 0x03, 0x00,
  /* RLE: 071 Pixels @ 032,058*/ 71, 0x01,
  /* ABS: 018 Pixels @ 103,058*/ 0, 18, 0x03, 0x00, 0x0A, 0x03, 0x03, 0x12, 0x0D, 0x03, 0x00, 0x0F, 0x0D, 0x0A, 0x07, 0x11, 0x0E, 0x07, 0x04, 0x03,
  /* RLE: 009 Pixels @ 121,058*/ 9, 0x00,
  /* ABS: 002 Pixels @ 130,058*/ 0, 2, 0x04, 0x04,
  /* RLE: 009 Pixels @ 132,058*/ 9, 0x03,
  /* ABS: 016 Pixels @ 141,058*/ 0, 16, 0x00, 0x0F, 0x06, 0x03, 0x0F, 0x06, 0x03, 0x03, 0x06, 0x0F, 0x03, 0x03, 0x0A, 0x00, 0x03, 0x04,
  /* RLE: 024 Pixels @ 157,058*/ 24, 0x00,
  /* ABS: 002 Pixels @ 181,058*/ 0, 2, 0x0A, 0x04,
  /* RLE: 006 Pixels @ 183,058*/ 6, 0x03,
  /* RLE: 001 Pixels @ 189,058*/ 1, 0x04,
  /* RLE: 050 Pixels @ 190,058*/ 50, 0x00,
  /* ABS: 004 Pixels @ 240,058*/ 0, 4, 0x0D, 0x06, 0x06, 0x06,
  /* RLE: 144 Pixels @ 244,058*/ 144, 0x01,
  /* RLE: 023 Pixels @ 000,059*/ 23, 0x02,
  /* ABS: 009 Pixels @ 023,059*/ 0, 9, 0x03, 0x0B, 0x0F, 0x06, 0x0B, 0x04, 0x03, 0x03, 0x04,
  /* RLE: 071 Pixels @ 032,059*/ 71, 0x01,
  /* ABS: 017 Pixels @ 103,059*/ 0, 17, 0x03, 0x0F, 0x0F, 0x03, 0x03, 0x07, 0x0B, 0x03, 0x03, 0x0B, 0x07, 0x03, 0x0A, 0x12, 0x06, 0x0A, 0x03,
  /* RLE: 011 Pixels @ 120,059*/ 11, 0x00,
  /* ABS: 002 Pixels @ 131,059*/ 0, 2, 0x04, 0x04,
  /* RLE: 005 Pixels @ 133,059*/ 5, 0x03,
  /* ABS: 018 Pixels @ 138,059*/ 0, 18, 0x0B, 0x03, 0x03, 0x03, 0x04, 0x07, 0x04, 0x0A, 0x07, 0x0B, 0x03, 0x04, 0x07, 0x0E, 0x03, 0x03, 0x00, 0x0A,
  /* RLE: 027 Pixels @ 156,059*/ 27, 0x00,
  /* RLE: 007 Pixels @ 183,059*/ 7, 0x03,
  /* RLE: 001 Pixels @ 190,059*/ 1, 0x04,
  /* RLE: 050 Pixels @ 191,059*/ 50, 0x00,
  /* RLE: 001 Pixels @ 241,059*/ 1, 0x0D,
  /* RLE: 004 Pixels @ 242,059*/ 4, 0x06,
  /* RLE: 142 Pixels @ 246,059*/ 142, 0x01,
  /* RLE: 023 Pixels @ 000,060*/ 23, 0x02,
  /* RLE: 001 Pixels @ 023,060*/ 1, 0x00,
  /* RLE: 008 Pixels @ 024,060*/ 8, 0x03,
  /* RLE: 001 Pixels @ 032,060*/ 1, 0x04,
  /* RLE: 066 Pixels @ 033,060*/ 66, 0x01,
  /* ABS: 015 Pixels @ 099,060*/ 0, 15, 0x15, 0x03, 0x03, 0x00, 0x03, 0x0A, 0x07, 0x12, 0x03, 0x0D, 0x11, 0x03, 0x03, 0x0B, 0x0F,
  /* RLE: 005 Pixels @ 114,060*/ 5, 0x03,
  /* RLE: 013 Pixels @ 119,060*/ 13, 0x00,
  /* ABS: 022 Pixels @ 132,060*/ 0, 22, 0x04, 0x04, 0x03, 0x03, 0x03, 0x00, 0x07, 0x0E, 0x03, 0x03, 0x0B, 0x07, 0x0F, 0x03, 0x06, 0x0F, 0x04, 0x0D, 0x0B, 0x00, 0x03, 0x04,
  /* RLE: 029 Pixels @ 154,060*/ 29, 0x00,
  /* RLE: 001 Pixels @ 183,060*/ 1, 0x04,
  /* RLE: 006 Pixels @ 184,060*/ 6, 0x03,
  /* RLE: 001 Pixels @ 190,060*/ 1, 0x04,
  /* RLE: 051 Pixels @ 191,060*/ 51, 0x00,
  /* ABS: 005 Pixels @ 242,060*/ 0, 5, 0x0D, 0x06, 0x03, 0x06, 0x0D,
  /* RLE: 141 Pixels @ 247,060*/ 141, 0x01,
  /* RLE: 025 Pixels @ 000,061*/ 25, 0x02,
  /* RLE: 001 Pixels @ 025,061*/ 1, 0x04,
  /* RLE: 006 Pixels @ 026,061*/ 6, 0x03,
  /* RLE: 001 Pixels @ 032,061*/ 1, 0x04,
  /* RLE: 066 Pixels @ 033,061*/ 66, 0x01,
  /* ABS: 018 Pixels @ 099,061*/ 0, 18, 0x03, 0x0A, 0x0B, 0x03, 0x03, 0x03, 0x0E, 0x07, 0x0B, 0x00, 0x07, 0x11, 0x06, 0x07, 0x0B, 0x03, 0x04, 0x04,
  /* RLE: 016 Pixels @ 117,061*/ 16, 0x00,
  /* ABS: 019 Pixels @ 133,061*/ 0, 19, 0x04, 0x04, 0x03, 0x03, 0x03, 0x0E, 0x07, 0x0B, 0x06, 0x0F, 0x0E, 0x07, 0x12, 0x03, 0x11, 0x07, 0x0B, 0x03, 0x03,
  /* RLE: 031 Pixels @ 152,061*/ 31, 0x00,
  /* RLE: 001 Pixels @ 183,061*/ 1, 0x04,
  /* RLE: 006 Pixels @ 184,061*/ 6, 0x03,
  /* RLE: 001 Pixels @ 190,061*/ 1, 0x04,
  /* RLE: 052 Pixels @ 191,061*/ 52, 0x00,
  /* ABS: 005 Pixels @ 243,061*/ 0, 5, 0x0D, 0x06, 0x03, 0x06, 0x06,
  /* RLE: 140 Pixels @ 248,061*/ 140, 0x01,
  /* RLE: 025 Pixels @ 000,062*/ 25, 0x02,
  /* RLE: 001 Pixels @ 025,062*/ 1, 0x04,
  /* RLE: 007 Pixels @ 026,062*/ 7, 0x03,
  /* RLE: 001 Pixels @ 033,062*/ 1, 0x00,
  /* RLE: 065 Pixels @ 034,062*/ 65, 0x01,
  /* ABS: 017 Pixels @ 099,062*/ 0, 17, 0x03, 0x12, 0x07, 0x0A, 0x03, 0x0A, 0x0F, 0x07, 0x07, 0x00, 0x0A, 0x12, 0x12, 0x0A, 0x03, 0x00, 0x04,
  /* RLE: 018 Pixels @ 116,062*/ 18, 0x00,
  /* ABS: 016 Pixels @ 134,062*/ 0, 16, 0x04, 0x04, 0x03, 0x03, 0x03, 0x11, 0x07, 0x06, 0x00, 0x03, 0x0B, 0x07, 0x00, 0x03, 0x03, 0x03,
  /* RLE: 033 Pixels @ 150,062*/ 33, 0x00,
  /* RLE: 001 Pixels @ 183,062*/ 1, 0x0A,
  /* RLE: 007 Pixels @ 184,062*/ 7, 0x03,
  /* RLE: 001 Pixels @ 191,062*/ 1, 0x04,
  /* RLE: 053 Pixels @ 192,062*/ 53, 0x00,
  /* ABS: 004 Pixels @ 245,062*/ 0, 4, 0x0D, 0x03, 0x06, 0x06,
  /* RLE: 139 Pixels @ 249,062*/ 139, 0x01,
  /* RLE: 024 Pixels @ 000,063*/ 24, 0x02,
  /* ABS: 002 Pixels @ 024,063*/ 0, 2, 0x18, 0x00,
  /* RLE: 004 Pixels @ 026,063*/ 4, 0x03,
  /* ABS: 005 Pixels @ 030,063*/ 0, 5, 0x00, 0x0A, 0x0B, 0x03, 0x15,
  /* RLE: 064 Pixels @ 035,063*/ 64, 0x01,
  /* ABS: 010 Pixels @ 099,063*/ 0, 10, 0x00, 0x03, 0x0F, 0x0F, 0x0E, 0x07, 0x0E, 0x00, 0x0F, 0x0D,
  /* RLE: 004 Pixels @ 109,063*/ 4, 0x03,
  /* RLE: 022 Pixels @ 113,063*/ 22, 0x00,
  /* RLE: 003 Pixels @ 135,063*/ 3, 0x04,
  /* ABS: 011 Pixels @ 138,063*/ 0, 11, 0x03, 0x00, 0x07, 0x0B, 0x03, 0x03, 0x03, 0x11, 0x0B, 0x03, 0x04,
  /* RLE: 035 Pixels @ 149,063*/ 35, 0x00,
  /* RLE: 001 Pixels @ 184,063*/ 1, 0x04,
  /* RLE: 006 Pixels @ 185,063*/ 6, 0x03,
  /* RLE: 001 Pixels @ 191,063*/ 1, 0x04,
  /* RLE: 054 Pixels @ 192,063*/ 54, 0x00,
  /* ABS: 004 Pixels @ 246,063*/ 0, 4, 0x0D, 0x06, 0x06, 0x06,
  /* RLE: 138 Pixels @ 250,063*/ 138, 0x01,
  /* RLE: 024 Pixels @ 000,064*/ 24, 0x02,
  /* ABS: 005 Pixels @ 024,064*/ 0, 5, 0x03, 0x00, 0x0B, 0x0E, 0x06,
  /* RLE: 004 Pixels @ 029,064*/ 4, 0x07,
  /* ABS: 002 Pixels @ 033,064*/ 0, 2, 0x0A, 0x15,
  /* RLE: 064 Pixels @ 035,064*/ 64, 0x01,
  /* ABS: 014 Pixels @ 099,064*/ 0, 14, 0x04, 0x03, 0x0A, 0x07, 0x07, 0x04, 0x03, 0x03, 0x04, 0x07, 0x0E, 0x03, 0x04, 0x04,
  /* RLE: 026 Pixels @ 113,064*/ 26, 0x00,
  /* ABS: 004 Pixels @ 139,064*/ 0, 4, 0x03, 0x12, 0x07, 0x00,
  /* RLE: 004 Pixels @ 143,064*/ 4, 0x03,
  /* RLE: 036 Pixels @ 147,064*/ 36, 0x00,
  /* ABS: 002 Pixels @ 183,064*/ 0, 2, 0x0A, 0x04,
  /* RLE: 006 Pixels @ 185,064*/ 6, 0x03,
  /* RLE: 001 Pixels @ 191,064*/ 1, 0x04,
  /* RLE: 055 Pixels @ 192,064*/ 55, 0x00,
  /* ABS: 004 Pixels @ 247,064*/ 0, 4, 0x0D, 0x06, 0x06, 0x06,
  /* RLE: 137 Pixels @ 251,064*/ 137, 0x01,
  /* RLE: 024 Pixels @ 000,065*/ 24, 0x02,
  /* ABS: 011 Pixels @ 024,065*/ 0, 11, 0x03, 0x06, 0x07, 0x07, 0x0D, 0x07, 0x0E, 0x0A, 0x00, 0x03, 0x15,
  /* RLE: 062 Pixels @ 035,065*/ 62, 0x01,
  /* ABS: 015 Pixels @ 097,065*/ 0, 15, 0x04, 0x04, 0x03, 0x03, 0x03, 0x0E, 0x07, 0x0B, 0x03, 0x03, 0x03, 0x0B, 0x03, 0x00, 0x04,
  /* RLE: 027 Pixels @ 112,065*/ 27, 0x00,
  /* ABS: 006 Pixels @ 139,065*/ 0, 6, 0x03, 0x03, 0x0F, 0x06, 0x03, 0x0A,
  /* RLE: 040 Pixels @ 145,065*/ 40, 0x00,
  /* RLE: 007 Pixels @ 185,065*/ 7, 0x03,
  /* RLE: 001 Pixels @ 192,065*/ 1, 0x04,
  /* RLE: 055 Pixels @ 193,065*/ 55, 0x00,
  /* ABS: 004 Pixels @ 248,065*/ 0, 4, 0x0D, 0x06, 0x03, 0x06,
  /* RLE: 136 Pixels @ 252,065*/ 136, 0x01,
  /* RLE: 024 Pixels @ 000,066*/ 24, 0x02,
  /* ABS: 010 Pixels @ 024,066*/ 0, 10, 0x03, 0x0A, 0x00, 0x03, 0x03, 0x07, 0x04, 0x03, 0x03, 0x00,
  /* RLE: 060 Pixels @ 034,066*/ 60, 0x01,
  /* ABS: 004 Pixels @ 094,066*/ 0, 4, 0x15, 0x00, 0x03, 0x00,
  /* RLE: 005 Pixels @ 098,066*/ 5, 0x03,
  /* ABS: 005 Pixels @ 103,066*/ 0, 5, 0x0D, 0x07, 0x00, 0x03, 0x03,
  /* RLE: 032 Pixels @ 108,066*/ 32, 0x00,
  /* ABS: 004 Pixels @ 140,066*/ 0, 4, 0x03, 0x0A, 0x00, 0x03,
  /* RLE: 041 Pixels @ 144,066*/ 41, 0x00,
  /* RLE: 001 Pixels @ 185,066*/ 1, 0x04,
  /* RLE: 006 Pixels @ 186,066*/ 6, 0x03,
  /* RLE: 001 Pixels @ 192,066*/ 1, 0x04,
  /* RLE: 056 Pixels @ 193,066*/ 56, 0x00,
  /* ABS: 005 Pixels @ 249,066*/ 0, 5, 0x0D, 0x06, 0x03, 0x06, 0x0D,
  /* RLE: 134 Pixels @ 254,066*/ 134, 0x01,
  /* RLE: 024 Pixels @ 000,067*/ 24, 0x02,
  /* ABS: 007 Pixels @ 024,067*/ 0, 7, 0x18, 0x03, 0x00, 0x03, 0x03, 0x0D, 0x0E,
  /* RLE: 004 Pixels @ 031,067*/ 4, 0x03,
  /* RLE: 058 Pixels @ 035,067*/ 58, 0x01,
  /* ABS: 005 Pixels @ 093,067*/ 0, 5, 0x15, 0x03, 0x00, 0x04, 0x00,
  /* RLE: 005 Pixels @ 098,067*/ 5, 0x03,
  /* ABS: 006 Pixels @ 103,067*/ 0, 6, 0x00, 0x0F, 0x12, 0x03, 0x04, 0x04,
  /* RLE: 032 Pixels @ 109,067*/ 32, 0x00,
  /* ABS: 002 Pixels @ 141,067*/ 0, 2, 0x03, 0x03,
  /* RLE: 006 Pixels @ 143,067*/ 6, 0x00,
  /* RLE: 001 Pixels @ 149,067*/ 1, 0x07,
  /* RLE: 035 Pixels @ 150,067*/ 35, 0x00,
  /* RLE: 001 Pixels @ 185,067*/ 1, 0x04,
  /* RLE: 006 Pixels @ 186,067*/ 6, 0x03,
  /* RLE: 001 Pixels @ 192,067*/ 1, 0x04,
  /* RLE: 057 Pixels @ 193,067*/ 57, 0x00,
  /* ABS: 005 Pixels @ 250,067*/ 0, 5, 0x0D, 0x06, 0x03, 0x06, 0x06,
  /* RLE: 133 Pixels @ 255,067*/ 133, 0x01,
  /* RLE: 025 Pixels @ 000,068*/ 25, 0x02,
  /* ABS: 010 Pixels @ 025,068*/ 0, 10, 0x00, 0x03, 0x03, 0x03, 0x12, 0x0D, 0x0B, 0x12, 0x0D, 0x03,
  /* RLE: 057 Pixels @ 035,068*/ 57, 0x01,
  /* ABS: 007 Pixels @ 092,068*/ 0, 7, 0x18, 0x03, 0x00, 0x0F, 0x07, 0x0F, 0x00,
  /* RLE: 005 Pixels @ 099,068*/ 5, 0x03,
  /* ABS: 003 Pixels @ 104,068*/ 0, 3, 0x0A, 0x03, 0x03,
  /* RLE: 042 Pixels @ 107,068*/ 42, 0x00,
  /* RLE: 001 Pixels @ 149,068*/ 1, 0x07,
  /* RLE: 035 Pixels @ 150,068*/ 35, 0x00,
  /* RLE: 001 Pixels @ 185,068*/ 1, 0x0A,
  /* RLE: 007 Pixels @ 186,068*/ 7, 0x03,
  /* RLE: 001 Pixels @ 193,068*/ 1, 0x04,
  /* RLE: 057 Pixels @ 194,068*/ 57, 0x00,
  /* ABS: 005 Pixels @ 251,068*/ 0, 5, 0x06, 0x0D, 0x06, 0x06, 0x06,
  /* RLE: 132 Pixels @ 256,068*/ 132, 0x01,
  /* RLE: 025 Pixels @ 000,069*/ 25, 0x02,
  /* ABS: 004 Pixels @ 025,069*/ 0, 4, 0x03, 0x0E, 0x06, 0x0F,
  /* RLE: 004 Pixels @ 029,069*/ 4, 0x07,
  /* ABS: 002 Pixels @ 033,069*/ 0, 2, 0x06, 0x03,
  /* RLE: 051 Pixels @ 035,069*/ 51, 0x01,
  /* ABS: 013 Pixels @ 086,069*/ 0, 13, 0x15, 0x00, 0x03, 0x18, 0x01, 0x03, 0x03, 0x04, 0x0E, 0x0B, 0x00, 0x0F, 0x0D,
  /* RLE: 005 Pixels @ 099,069*/ 5, 0x03,
  /* RLE: 046 Pixels @ 104,069*/ 46, 0x00,
  /* RLE: 001 Pixels @ 150,069*/ 1, 0x07,
  /* RLE: 035 Pixels @ 151,069*/ 35, 0x00,
  /* RLE: 001 Pixels @ 186,069*/ 1, 0x04,
  /* RLE: 006 Pixels @ 187,069*/ 6, 0x03,
  /* RLE: 001 Pixels @ 193,069*/ 1, 0x04,
  /* RLE: 059 Pixels @ 194,069*/ 59, 0x00,
  /* ABS: 004 Pixels @ 253,069*/ 0, 4, 0x0D, 0x06, 0x06, 0x06,
  /* RLE: 131 Pixels @ 257,069*/ 131, 0x01,
  /* RLE: 025 Pixels @ 000,070*/ 25, 0x02,
  /* ABS: 010 Pixels @ 025,070*/ 0, 10, 0x03, 0x0D, 0x11, 0x06, 0x0B, 0x0A, 0x00, 0x03, 0x03, 0x00,
  /* RLE: 051 Pixels @ 035,070*/ 51, 0x01,
  /* ABS: 020 Pixels @ 086,070*/ 0, 20, 0x03, 0x00, 0x0B, 0x03, 0x03, 0x03, 0x0D, 0x07, 0x07, 0x0B, 0x03, 0x0A, 0x07, 0x0E, 0x03, 0x03, 0x04, 0x04, 0x04, 0x0A,
  /* RLE: 080 Pixels @ 106,070*/ 80, 0x00,
  /* RLE: 001 Pixels @ 186,070*/ 1, 0x04,
  /* RLE: 006 Pixels @ 187,070*/ 6, 0x03,
  /* RLE: 001 Pixels @ 193,070*/ 1, 0x04,
  /* RLE: 060 Pixels @ 194,070*/ 60, 0x00,
  /* ABS: 004 Pixels @ 254,070*/ 0, 4, 0x0D, 0x06, 0x03, 0x0D,
  /* RLE: 130 Pixels @ 258,070*/ 130, 0x01,
  /* RLE: 025 Pixels @ 000,071*/ 25, 0x02,
  /* RLE: 004 Pixels @ 025,071*/ 4, 0x03,
  /* RLE: 001 Pixels @ 029,071*/ 1, 0x00,
  /* RLE: 004 Pixels @ 030,071*/ 4, 0x03,
  /* RLE: 001 Pixels @ 034,071*/ 1, 0x04,
  /* RLE: 051 Pixels @ 035,071*/ 51, 0x01,
  /* ABS: 017 Pixels @ 086,071*/ 0, 17, 0x03, 0x06, 0x07, 0x00, 0x0A, 0x06, 0x12, 0x03, 0x0D, 0x0F, 0x00, 0x03, 0x0E, 0x07, 0x00, 0x00, 0x04,
  /* RLE: 083 Pixels @ 103,071*/ 83, 0x00,
  /* RLE: 001 Pixels @ 186,071*/ 1, 0x04,
  /* RLE: 007 Pixels @ 187,071*/ 7, 0x03,
  /* RLE: 061 Pixels @ 194,071*/ 61, 0x00,
  /* ABS: 004 Pixels @ 255,071*/ 0, 4, 0x0D, 0x06, 0x03, 0x06,
  /* RLE: 129 Pixels @ 259,071*/ 129, 0x01,
  /* RLE: 025 Pixels @ 000,072*/ 25, 0x02,
  /* ABS: 010 Pixels @ 025,072*/ 0, 10, 0x00, 0x03, 0x0B, 0x0F, 0x07, 0x0F, 0x04, 0x03, 0x03, 0x04,
  /* RLE: 051 Pixels @ 035,072*/ 51, 0x01,
  /* ABS: 015 Pixels @ 086,072*/ 0, 15, 0x03, 0x00, 0x0F, 0x11, 0x0B, 0x07, 0x0E, 0x03, 0x00, 0x0F, 0x06, 0x03, 0x03, 0x0A, 0x03,
  /* RLE: 085 Pixels @ 101,072*/ 85, 0x00,
  /* ABS: 002 Pixels @ 186,072*/ 0, 2, 0x0A, 0x04,
  /* RLE: 006 Pixels @ 188,072*/ 6, 0x03,
  /* RLE: 001 Pixels @ 194,072*/ 1, 0x04,
  /* RLE: 061 Pixels @ 195,072*/ 61, 0x00,
  /* ABS: 005 Pixels @ 256,072*/ 0, 5, 0x0D, 0x06, 0x03, 0x06, 0x06,
  /* RLE: 127 Pixels @ 261,072*/ 127, 0x01,
  /* RLE: 025 Pixels @ 000,073*/ 25, 0x02,
  /* ABS: 010 Pixels @ 025,073*/ 0, 10, 0x03, 0x0A, 0x07, 0x06, 0x0B, 0x12, 0x07, 0x00, 0x03, 0x04,
  /* RLE: 051 Pixels @ 035,073*/ 51, 0x01,
  /* ABS: 013 Pixels @ 086,073*/ 0, 13, 0x0A, 0x03, 0x0A, 0x07, 0x12, 0x12, 0x07, 0x00, 0x03, 0x04, 0x07, 0x04, 0x03,
  /* RLE: 053 Pixels @ 099,073*/ 53, 0x00,
  /* RLE: 001 Pixels @ 152,073*/ 1, 0x07,
  /* RLE: 034 Pixels @ 153,073*/ 34, 0x00,
  /* RLE: 001 Pixels @ 187,073*/ 1, 0x04,
  /* RLE: 006 Pixels @ 188,073*/ 6, 0x03,
  /* RLE: 001 Pixels @ 194,073*/ 1, 0x04,
  /* RLE: 062 Pixels @ 195,073*/ 62, 0x00,
  /* RLE: 001 Pixels @ 257,073*/ 1, 0x0D,
  /* RLE: 004 Pixels @ 258,073*/ 4, 0x06,
  /* RLE: 126 Pixels @ 262,073*/ 126, 0x01,
  /* RLE: 025 Pixels @ 000,074*/ 25, 0x02,
  /* ABS: 010 Pixels @ 025,074*/ 0, 10, 0x03, 0x0B, 0x0D, 0x03, 0x03, 0x03, 0x0D, 0x0B, 0x03, 0x04,
  /* RLE: 050 Pixels @ 035,074*/ 50, 0x01,
  /* ABS: 013 Pixels @ 085,074*/ 0, 13, 0x04, 0x04, 0x03, 0x03, 0x0E, 0x07, 0x04, 0x0F, 0x11, 0x03, 0x03, 0x04, 0x03,
  /* RLE: 008 Pixels @ 098,074*/ 8, 0x00,
  /* RLE: 001 Pixels @ 106,074*/ 1, 0x0A,
  /* RLE: 045 Pixels @ 107,074*/ 45, 0x00,
  /* RLE: 001 Pixels @ 152,074*/ 1, 0x07,
  /* RLE: 034 Pixels @ 153,074*/ 34, 0x00,
  /* RLE: 001 Pixels @ 187,074*/ 1, 0x04,
  /* RLE: 007 Pixels @ 188,074*/ 7, 0x03,
  /* RLE: 063 Pixels @ 195,074*/ 63, 0x00,
  /* ABS: 005 Pixels @ 258,074*/ 0, 5, 0x06, 0x0D, 0x06, 0x06, 0x06,
  /* RLE: 125 Pixels @ 263,074*/ 125, 0x01,
  /* RLE: 025 Pixels @ 000,075*/ 25, 0x02,
  /* ABS: 010 Pixels @ 025,075*/ 0, 10, 0x03, 0x0B, 0x0D, 0x03, 0x03, 0x03, 0x0F, 0x0B, 0x03, 0x04,
  /* RLE: 048 Pixels @ 035,075*/ 48, 0x01,
  /* RLE: 003 Pixels @ 083,075*/ 3, 0x04,
  /* RLE: 004 Pixels @ 086,075*/ 4, 0x03,
  /* ABS: 006 Pixels @ 090,075*/ 0, 6, 0x0D, 0x0F, 0x00, 0x07, 0x0B, 0x03,
  /* RLE: 013 Pixels @ 096,075*/ 13, 0x00,
  /* RLE: 001 Pixels @ 109,075*/ 1, 0x0A,
  /* RLE: 042 Pixels @ 110,075*/ 42, 0x00,
  /* RLE: 001 Pixels @ 152,075*/ 1, 0x07,
  /* RLE: 035 Pixels @ 153,075*/ 35, 0x00,
  /* RLE: 001 Pixels @ 188,075*/ 1, 0x04,
  /* RLE: 006 Pixels @ 189,075*/ 6, 0x03,
  /* RLE: 001 Pixels @ 195,075*/ 1, 0x04,
  /* RLE: 064 Pixels @ 196,075*/ 64, 0x00,
  /* ABS: 004 Pixels @ 260,075*/ 0, 4, 0x0D, 0x06, 0x06, 0x06,
  /* RLE: 124 Pixels @ 264,075*/ 124, 0x01,
  /* RLE: 025 Pixels @ 000,076*/ 25, 0x02,
  /* ABS: 010 Pixels @ 025,076*/ 0, 10, 0x03, 0x00, 0x0F, 0x11, 0x06, 0x0F, 0x0F, 0x00, 0x03, 0x03,
  /* RLE: 047 Pixels @ 035,076*/ 47, 0x01,
  /* ABS: 002 Pixels @ 082,076*/ 0, 2, 0x04, 0x04,
  /* RLE: 006 Pixels @ 084,076*/ 6, 0x03,
  /* ABS: 005 Pixels @ 090,076*/ 0, 5, 0x00, 0x07, 0x12, 0x0A, 0x03,
  /* RLE: 013 Pixels @ 095,076*/ 13, 0x00,
  /* RLE: 001 Pixels @ 108,076*/ 1, 0x0A,
  /* RLE: 076 Pixels @ 109,076*/ 76, 0x00,
  /* ABS: 004 Pixels @ 185,076*/ 0, 4, 0x0A, 0x00, 0x0A, 0x04,
  /* RLE: 006 Pixels @ 189,076*/ 6, 0x03,
  /* RLE: 001 Pixels @ 195,076*/ 1, 0x04,
  /* RLE: 065 Pixels @ 196,076*/ 65, 0x00,
  /* ABS: 004 Pixels @ 261,076*/ 0, 4, 0x0D, 0x03, 0x03, 0x0D,
  /* RLE: 123 Pixels @ 265,076*/ 123, 0x01,
  /* RLE: 025 Pixels @ 000,077*/ 25, 0x02,
  /* ABS: 011 Pixels @ 025,077*/ 0, 11, 0x0A, 0x03, 0x00, 0x12, 0x06, 0x12, 0x00, 0x03, 0x03, 0x03, 0x04,
  /* RLE: 044 Pixels @ 036,077*/ 44, 0x01,
  /* RLE: 003 Pixels @ 080,077*/ 3, 0x04,
  /* RLE: 008 Pixels @ 083,077*/ 8, 0x03,
  /* ABS: 003 Pixels @ 091,077*/ 0, 3, 0x0A, 0x03, 0x03,
  /* RLE: 015 Pixels @ 094,077*/ 15, 0x00,
  /* RLE: 001 Pixels @ 109,077*/ 1, 0x0A,
  /* RLE: 078 Pixels @ 110,077*/ 78, 0x00,
  /* RLE: 001 Pixels @ 188,077*/ 1, 0x04,
  /* RLE: 007 Pixels @ 189,077*/ 7, 0x03,
  /* RLE: 066 Pixels @ 196,077*/ 66, 0x00,
  /* ABS: 004 Pixels @ 262,077*/ 0, 4, 0x0D, 0x06, 0x03, 0x06,
  /* RLE: 122 Pixels @ 266,077*/ 122, 0x01,
  /* RLE: 026 Pixels @ 000,078*/ 26, 0x02,
  /* ABS: 010 Pixels @ 026,078*/ 0, 10, 0x18, 0x03, 0x03, 0x0B, 0x0F, 0x07, 0x07, 0x0B, 0x03, 0x00,
  /* RLE: 043 Pixels @ 036,078*/ 43, 0x01,
  /* ABS: 002 Pixels @ 079,078*/ 0, 2, 0x04, 0x04,
  /* RLE: 010 Pixels @ 081,078*/ 10, 0x03,
  /* ABS: 003 Pixels @ 091,078*/ 0, 3, 0x00, 0x00, 0x04,
  /* RLE: 095 Pixels @ 094,078*/ 95, 0x00,
  /* RLE: 001 Pixels @ 189,078*/ 1, 0x04,
  /* RLE: 006 Pixels @ 190,078*/ 6, 0x03,
  /* RLE: 001 Pixels @ 196,078*/ 1, 0x04,
  /* RLE: 066 Pixels @ 197,078*/ 66, 0x00,
  /* RLE: 001 Pixels @ 263,078*/ 1, 0x0D,
  /* RLE: 004 Pixels @ 264,078*/ 4, 0x06,
  /* RLE: 120 Pixels @ 268,078*/ 120, 0x01,
  /* RLE: 027 Pixels @ 000,079*/ 27, 0x02,
  /* ABS: 009 Pixels @ 027,079*/ 0, 9, 0x03, 0x04, 0x07, 0x06, 0x0B, 0x0E, 0x07, 0x00, 0x03,
  /* RLE: 041 Pixels @ 036,079*/ 41, 0x01,
  /* ABS: 002 Pixels @ 077,079*/ 0, 2, 0x04, 0x04,
  /* RLE: 011 Pixels @ 079,079*/ 11, 0x03,
  /* ABS: 002 Pixels @ 090,079*/ 0, 2, 0x04, 0x04,
  /* RLE: 062 Pixels @ 092,079*/ 62, 0x00,
  /* RLE: 001 Pixels @ 154,079*/ 1, 0x07,
  /* RLE: 034 Pixels @ 155,079*/ 34, 0x00,
  /* RLE: 001 Pixels @ 189,079*/ 1, 0x04,
  /* RLE: 006 Pixels @ 190,079*/ 6, 0x03,
  /* RLE: 001 Pixels @ 196,079*/ 1, 0x04,
  /* RLE: 067 Pixels @ 197,079*/ 67, 0x00,
  /* RLE: 001 Pixels @ 264,079*/ 1, 0x0D,
  /* RLE: 004 Pixels @ 265,079*/ 4, 0x06,
  /* RLE: 119 Pixels @ 269,079*/ 119, 0x01,
  /* RLE: 027 Pixels @ 000,080*/ 27, 0x02,
  /* ABS: 009 Pixels @ 027,080*/ 0, 9, 0x03, 0x0B, 0x0D, 0x03, 0x03, 0x03, 0x06, 0x0B, 0x00,
  /* RLE: 039 Pixels @ 036,080*/ 39, 0x01,
  /* RLE: 003 Pixels @ 075,080*/ 3, 0x04,
  /* RLE: 011 Pixels @ 078,080*/ 11, 0x03,
  /* ABS: 002 Pixels @ 089,080*/ 0, 2, 0x04, 0x04,
  /* RLE: 017 Pixels @ 091,080*/ 17, 0x00,
  /* RLE: 001 Pixels @ 108,080*/ 1, 0x0A,
  /* RLE: 046 Pixels @ 109,080*/ 46, 0x00,
  /* RLE: 001 Pixels @ 155,080*/ 1, 0x07,
  /* RLE: 032 Pixels @ 156,080*/ 32, 0x00,
  /* ABS: 002 Pixels @ 188,080*/ 0, 2, 0x0A, 0x04,
  /* RLE: 007 Pixels @ 190,080*/ 7, 0x03,
  /* RLE: 068 Pixels @ 197,080*/ 68, 0x00,
  /* ABS: 005 Pixels @ 265,080*/ 0, 5, 0x06, 0x0D, 0x06, 0x06, 0x0D,
  /* RLE: 118 Pixels @ 270,080*/ 118, 0x01,
  /* RLE: 027 Pixels @ 000,081*/ 27, 0x02,
  /* ABS: 009 Pixels @ 027,081*/ 0, 9, 0x03, 0x0B, 0x11, 0x03, 0x03, 0x00, 0x0F, 0x04, 0x03,
  /* RLE: 038 Pixels @ 036,081*/ 38, 0x01,
  /* ABS: 002 Pixels @ 074,081*/ 0, 2, 0x04, 0x04,
  /* RLE: 011 Pixels @ 076,081*/ 11, 0x03,
  /* ABS: 002 Pixels @ 087,081*/ 0, 2, 0x04, 0x04,
  /* RLE: 066 Pixels @ 089,081*/ 66, 0x00,
  /* RLE: 001 Pixels @ 155,081*/ 1, 0x07,
  /* RLE: 034 Pixels @ 156,081*/ 34, 0x00,
  /* RLE: 001 Pixels @ 190,081*/ 1, 0x04,
  /* RLE: 006 Pixels @ 191,081*/ 6, 0x03,
  /* RLE: 001 Pixels @ 197,081*/ 1, 0x04,
  /* RLE: 069 Pixels @ 198,081*/ 69, 0x00,
  /* ABS: 004 Pixels @ 267,081*/ 0, 4, 0x0D, 0x03, 0x06, 0x06,
  /* RLE: 117 Pixels @ 271,081*/ 117, 0x01,
  /* RLE: 027 Pixels @ 000,082*/ 27, 0x02,
  /* ABS: 011 Pixels @ 027,082*/ 0, 11, 0x03, 0x00, 0x0F, 0x07, 0x03, 0x00, 0x06, 0x03, 0x03, 0x03, 0x00,
  /* RLE: 034 Pixels @ 038,082*/ 34, 0x01,
  /* RLE: 003 Pixels @ 072,082*/ 3, 0x04,
  /* RLE: 011 Pixels @ 075,082*/ 11, 0x03,
  /* ABS: 002 Pixels @ 086,082*/ 0, 2, 0x04, 0x04,
  /* RLE: 102 Pixels @ 088,082*/ 102, 0x00,
  /* RLE: 001 Pixels @ 190,082*/ 1, 0x04,
  /* RLE: 006 Pixels @ 191,082*/ 6, 0x03,
  /* RLE: 001 Pixels @ 197,082*/ 1, 0x04,
  /* RLE: 070 Pixels @ 198,082*/ 70, 0x00,
  /* ABS: 004 Pixels @ 268,082*/ 0, 4, 0x0D, 0x03, 0x03, 0x06,
  /* RLE: 116 Pixels @ 272,082*/ 116, 0x01,
  /* RLE: 027 Pixels @ 000,083*/ 27, 0x02,
  /* ABS: 012 Pixels @ 027,083*/ 0, 12, 0x0A, 0x03, 0x00, 0x0B, 0x03, 0x03, 0x03, 0x00, 0x0B, 0x0E, 0x03, 0x15,
  /* RLE: 032 Pixels @ 039,083*/ 32, 0x01,
  /* ABS: 002 Pixels @ 071,083*/ 0, 2, 0x04, 0x04,
  /* RLE: 011 Pixels @ 073,083*/ 11, 0x03,
  /* ABS: 002 Pixels @ 084,083*/ 0, 2, 0x04, 0x04,
  /* RLE: 023 Pixels @ 086,083*/ 23, 0x00,
  /* RLE: 001 Pixels @ 109,083*/ 1, 0x0A,
  /* RLE: 080 Pixels @ 110,083*/ 80, 0x00,
  /* RLE: 001 Pixels @ 190,083*/ 1, 0x04,
  /* RLE: 007 Pixels @ 191,083*/ 7, 0x03,
  /* RLE: 071 Pixels @ 198,083*/ 71, 0x00,
  /* ABS: 004 Pixels @ 269,083*/ 0, 4, 0x0D, 0x06, 0x06, 0x06,
  /* RLE: 115 Pixels @ 273,083*/ 115, 0x01,
  /* RLE: 028 Pixels @ 000,084*/ 28, 0x02,
  /* ABS: 005 Pixels @ 028,084*/ 0, 5, 0x03, 0x0A, 0x0B, 0x12, 0x11,
  /* RLE: 004 Pixels @ 033,084*/ 4, 0x07,
  /* ABS: 002 Pixels @ 037,084*/ 0, 2, 0x00, 0x15,
  /* RLE: 030 Pixels @ 039,084*/ 30, 0x01,
  /* ABS: 002 Pixels @ 069,084*/ 0, 2, 0x04, 0x04,
  /* RLE: 011 Pixels @ 071,084*/ 11, 0x03,
  /* RLE: 003 Pixels @ 082,084*/ 3, 0x04,
  /* RLE: 105 Pixels @ 085,084*/ 105, 0x00,
  /* ABS: 002 Pixels @ 190,084*/ 0, 2, 0x0A, 0x04,
  /* RLE: 006 Pixels @ 192,084*/ 6, 0x03,
  /* RLE: 001 Pixels @ 198,084*/ 1, 0x04,
  /* RLE: 071 Pixels @ 199,084*/ 71, 0x00,
  /* ABS: 004 Pixels @ 270,084*/ 0, 4, 0x0D, 0x06, 0x06, 0x06,
  /* RLE: 114 Pixels @ 274,084*/ 114, 0x01,
  /* RLE: 028 Pixels @ 000,085*/ 28, 0x02,
  /* ABS: 011 Pixels @ 028,085*/ 0, 11, 0x03, 0x06, 0x07, 0x07, 0x07, 0x12, 0x0A, 0x00, 0x03, 0x03, 0x15,
  /* RLE: 028 Pixels @ 039,085*/ 28, 0x01,
  /* RLE: 003 Pixels @ 067,085*/ 3, 0x04,
  /* RLE: 011 Pixels @ 070,085*/ 11, 0x03,
  /* ABS: 002 Pixels @ 081,085*/ 0, 2, 0x04, 0x04,
  /* RLE: 074 Pixels @ 083,085*/ 74, 0x00,
  /* RLE: 001 Pixels @ 157,085*/ 1, 0x07,
  /* RLE: 033 Pixels @ 158,085*/ 33, 0x00,
  /* RLE: 001 Pixels @ 191,085*/ 1, 0x04,
  /* RLE: 006 Pixels @ 192,085*/ 6, 0x03,
  /* RLE: 001 Pixels @ 198,085*/ 1, 0x04,
  /* RLE: 072 Pixels @ 199,085*/ 72, 0x00,
  /* RLE: 001 Pixels @ 271,085*/ 1, 0x0D,
  /* RLE: 004 Pixels @ 272,085*/ 4, 0x06,
  /* RLE: 112 Pixels @ 276,085*/ 112, 0x01,
  /* RLE: 028 Pixels @ 000,086*/ 28, 0x02,
  /* ABS: 009 Pixels @ 028,086*/ 0, 9, 0x03, 0x00, 0x03, 0x00, 0x07, 0x07, 0x0E, 0x03, 0x00,
  /* RLE: 030 Pixels @ 037,086*/ 30, 0x01,
  /* RLE: 001 Pixels @ 067,086*/ 1, 0x04,
  /* RLE: 011 Pixels @ 068,086*/ 11, 0x03,
  /* ABS: 002 Pixels @ 079,086*/ 0, 2, 0x04, 0x04,
  /* RLE: 027 Pixels @ 081,086*/ 27, 0x00,
  /* RLE: 001 Pixels @ 108,086*/ 1, 0x0A,
  /* RLE: 048 Pixels @ 109,086*/ 48, 0x00,
  /* RLE: 001 Pixels @ 157,086*/ 1, 0x07,
  /* RLE: 032 Pixels @ 158,086*/ 32, 0x00,
  /* ABS: 002 Pixels @ 190,086*/ 0, 2, 0x0A, 0x04,
  /* RLE: 006 Pixels @ 192,086*/ 6, 0x03,
  /* RLE: 001 Pixels @ 198,086*/ 1, 0x04,
  /* RLE: 073 Pixels @ 199,086*/ 73, 0x00,
  /* ABS: 005 Pixels @ 272,086*/ 0, 5, 0x0D, 0x06, 0x03, 0x06, 0x0D,
  /* RLE: 111 Pixels @ 277,086*/ 111, 0x01,
  /* RLE: 028 Pixels @ 000,087*/ 28, 0x02,
  /* ABS: 009 Pixels @ 028,087*/ 0, 9, 0x00, 0x03, 0x00, 0x0F, 0x11, 0x0B, 0x0F, 0x0B, 0x03,
  /* RLE: 029 Pixels @ 037,087*/ 29, 0x01,
  /* ABS: 002 Pixels @ 066,087*/ 0, 2, 0x04, 0x04,
  /* RLE: 010 Pixels @ 068,087*/ 10, 0x03,
  /* ABS: 002 Pixels @ 078,087*/ 0, 2, 0x04, 0x04,
  /* RLE: 078 Pixels @ 080,087*/ 78, 0x00,
  /* RLE: 001 Pixels @ 158,087*/ 1, 0x07,
  /* RLE: 033 Pixels @ 159,087*/ 33, 0x00,
  /* RLE: 007 Pixels @ 192,087*/ 7, 0x03,
  /* RLE: 001 Pixels @ 199,087*/ 1, 0x04,
  /* RLE: 073 Pixels @ 200,087*/ 73, 0x00,
  /* ABS: 005 Pixels @ 273,087*/ 0, 5, 0x0D, 0x06, 0x03, 0x06, 0x06,
  /* RLE: 110 Pixels @ 278,087*/ 110, 0x01,
  /* RLE: 028 Pixels @ 000,088*/ 28, 0x02,
  /* ABS: 010 Pixels @ 028,088*/ 0, 10, 0x18, 0x00, 0x07, 0x0D, 0x03, 0x03, 0x03, 0x04, 0x03, 0x04,
  /* RLE: 027 Pixels @ 038,088*/ 27, 0x01,
  /* ABS: 002 Pixels @ 065,088*/ 0, 2, 0x04, 0x04,
  /* RLE: 009 Pixels @ 067,088*/ 9, 0x03,
  /* ABS: 002 Pixels @ 076,088*/ 0, 2, 0x04, 0x04,
  /* RLE: 114 Pixels @ 078,088*/ 114, 0x00,
  /* RLE: 001 Pixels @ 192,088*/ 1, 0x04,
  /* RLE: 006 Pixels @ 193,088*/ 6, 0x03,
  /* RLE: 001 Pixels @ 199,088*/ 1, 0x04,
  /* RLE: 075 Pixels @ 200,088*/ 75, 0x00,
  /* ABS: 004 Pixels @ 275,088*/ 0, 4, 0x0D, 0x03, 0x06, 0x06,
  /* RLE: 109 Pixels @ 279,088*/ 109, 0x01,
  /* RLE: 028 Pixels @ 000,089*/ 28, 0x02,
  /* ABS: 003 Pixels @ 028,089*/ 0, 3, 0x18, 0x03, 0x0E,
  /* RLE: 006 Pixels @ 031,089*/ 6, 0x03,
  /* RLE: 001 Pixels @ 037,089*/ 1, 0x04,
  /* RLE: 026 Pixels @ 038,089*/ 26, 0x01,
  /* ABS: 002 Pixels @ 064,089*/ 0, 2, 0x04, 0x04,
  /* RLE: 008 Pixels @ 066,089*/ 8, 0x03,
  /* RLE: 003 Pixels @ 074,089*/ 3, 0x04,
  /* RLE: 033 Pixels @ 077,089*/ 33, 0x00,
  /* RLE: 001 Pixels @ 110,089*/ 1, 0x0A,
  /* RLE: 081 Pixels @ 111,089*/ 81, 0x00,
  /* RLE: 001 Pixels @ 192,089*/ 1, 0x04,
  /* RLE: 006 Pixels @ 193,089*/ 6, 0x03,
  /* RLE: 001 Pixels @ 199,089*/ 1, 0x04,
  /* RLE: 076 Pixels @ 200,089*/ 76, 0x00,
  /* ABS: 004 Pixels @ 276,089*/ 0, 4, 0x0D, 0x06, 0x06, 0x06,
  /* RLE: 108 Pixels @ 280,089*/ 108, 0x01,
  /* RLE: 029 Pixels @ 000,090*/ 29, 0x02,
  /* ABS: 002 Pixels @ 029,090*/ 0, 2, 0x03, 0x00,
  /* RLE: 006 Pixels @ 031,090*/ 6, 0x03,
  /* RLE: 001 Pixels @ 037,090*/ 1, 0x04,
  /* RLE: 025 Pixels @ 038,090*/ 25, 0x01,
  /* ABS: 002 Pixels @ 063,090*/ 0, 2, 0x04, 0x04,
  /* RLE: 008 Pixels @ 065,090*/ 8, 0x03,
  /* ABS: 002 Pixels @ 073,090*/ 0, 2, 0x04, 0x04,
  /* RLE: 117 Pixels @ 075,090*/ 117, 0x00,
  /* RLE: 001 Pixels @ 192,090*/ 1, 0x0A,
  /* RLE: 007 Pixels @ 193,090*/ 7, 0x03,
  /* RLE: 001 Pixels @ 200,090*/ 1, 0x04,
  /* RLE: 076 Pixels @ 201,090*/ 76, 0x00,
  /* ABS: 004 Pixels @ 277,090*/ 0, 4, 0x0D, 0x06, 0x06, 0x06,
  /* RLE: 107 Pixels @ 281,090*/ 107, 0x01,
  /* RLE: 030 Pixels @ 000,091*/ 30, 0x02,
  /* RLE: 001 Pixels @ 030,091*/ 1, 0x04,
  /* RLE: 006 Pixels @ 031,091*/ 6, 0x03,
  /* RLE: 001 Pixels @ 037,091*/ 1, 0x04,
  /* RLE: 024 Pixels @ 038,091*/ 24, 0x01,
  /* ABS: 002 Pixels @ 062,091*/ 0, 2, 0x04, 0x04,
  /* RLE: 008 Pixels @ 064,091*/ 8, 0x03,
  /* RLE: 001 Pixels @ 072,091*/ 1, 0x04,
  /* RLE: 028 Pixels @ 073,091*/ 28, 0x00,
  /* ABS: 006 Pixels @ 101,091*/ 0, 6, 0x03, 0x03, 0x00, 0x03, 0x03, 0x03,
  /* RLE: 052 Pixels @ 107,091*/ 52, 0x00,
  /* RLE: 001 Pixels @ 159,091*/ 1, 0x07,
  /* RLE: 033 Pixels @ 160,091*/ 33, 0x00,
  /* RLE: 001 Pixels @ 193,091*/ 1, 0x04,
  /* RLE: 006 Pixels @ 194,091*/ 6, 0x03,
  /* RLE: 001 Pixels @ 200,091*/ 1, 0x04,
  /* RLE: 077 Pixels @ 201,091*/ 77, 0x00,
  /* ABS: 005 Pixels @ 278,091*/ 0, 5, 0x0D, 0x06, 0x03, 0x06, 0x06,
  /* RLE: 105 Pixels @ 283,091*/ 105, 0x01,
  /* RLE: 030 Pixels @ 000,092*/ 30, 0x02,
  /* RLE: 001 Pixels @ 030,092*/ 1, 0x04,
  /* RLE: 006 Pixels @ 031,092*/ 6, 0x03,
  /* RLE: 001 Pixels @ 037,092*/ 1, 0x04,
  /* RLE: 023 Pixels @ 038,092*/ 23, 0x01,
  /* ABS: 002 Pixels @ 061,092*/ 0, 2, 0x04, 0x04,
  /* RLE: 008 Pixels @ 063,092*/ 8, 0x03,
  /* ABS: 002 Pixels @ 071,092*/ 0, 2, 0x04, 0x04,
  /* RLE: 027 Pixels @ 073,092*/ 27, 0x00,
  /* ABS: 007 Pixels @ 100,092*/ 0, 7, 0x03, 0x04, 0x0A, 0x03, 0x03, 0x12, 0x0A,
  /* RLE: 053 Pixels @ 107,092*/ 53, 0x00,
  /* RLE: 001 Pixels @ 160,092*/ 1, 0x07,
  /* RLE: 032 Pixels @ 161,092*/ 32, 0x00,
  /* RLE: 001 Pixels @ 193,092*/ 1, 0x04,
  /* RLE: 006 Pixels @ 194,092*/ 6, 0x03,
  /* RLE: 001 Pixels @ 200,092*/ 1, 0x04,
  /* RLE: 078 Pixels @ 201,092*/ 78, 0x00,
  /* ABS: 005 Pixels @ 279,092*/ 0, 5, 0x0D, 0x06, 0x03, 0x06, 0x0D,
  /* RLE: 104 Pixels @ 284,092*/ 104, 0x01,
  /* RLE: 031 Pixels @ 000,093*/ 31, 0x02,
  /* RLE: 007 Pixels @ 031,093*/ 7, 0x03,
  /* RLE: 001 Pixels @ 038,093*/ 1, 0x04,
  /* RLE: 021 Pixels @ 039,093*/ 21, 0x01,
  /* ABS: 002 Pixels @ 060,093*/ 0, 2, 0x04, 0x04,
  /* RLE: 008 Pixels @ 062,093*/ 8, 0x03,
  /* ABS: 002 Pixels @ 070,093*/ 0, 2, 0x04, 0x04,
  /* RLE: 028 Pixels @ 072,093*/ 28, 0x00,
  /* ABS: 008 Pixels @ 100,093*/ 0, 8, 0x03, 0x11, 0x0F, 0x03, 0x0A, 0x07, 0x03, 0x03,
  /* RLE: 052 Pixels @ 108,093*/ 52, 0x00,
  /* RLE: 001 Pixels @ 160,093*/ 1, 0x07,
  /* RLE: 033 Pixels @ 161,093*/ 33, 0x00,
  /* RLE: 007 Pixels @ 194,093*/ 7, 0x03,
  /* RLE: 001 Pixels @ 201,093*/ 1, 0x04,
  /* RLE: 078 Pixels @ 202,093*/ 78, 0x00,
  /* ABS: 005 Pixels @ 280,093*/ 0, 5, 0x0D, 0x06, 0x03, 0x06, 0x06,
  /* RLE: 103 Pixels @ 285,093*/ 103, 0x01,
  /* RLE: 031 Pixels @ 000,094*/ 31, 0x02,
  /* RLE: 001 Pixels @ 031,094*/ 1, 0x04,
  /* RLE: 006 Pixels @ 032,094*/ 6, 0x03,
  /* RLE: 001 Pixels @ 038,094*/ 1, 0x04,
  /* RLE: 020 Pixels @ 039,094*/ 20, 0x01,
  /* ABS: 002 Pixels @ 059,094*/ 0, 2, 0x04, 0x04,
  /* RLE: 008 Pixels @ 061,094*/ 8, 0x03,
  /* ABS: 002 Pixels @ 069,094*/ 0, 2, 0x04, 0x04,
  /* RLE: 029 Pixels @ 071,094*/ 29, 0x00,
  /* ABS: 010 Pixels @ 100,094*/ 0, 10, 0x03, 0x00, 0x07, 0x06, 0x00, 0x07, 0x00, 0x03, 0x03, 0x03,
  /* RLE: 083 Pixels @ 110,094*/ 83, 0x00,
  /* ABS: 002 Pixels @ 193,094*/ 0, 2, 0x0A, 0x04,
  /* RLE: 006 Pixels @ 195,094*/ 6, 0x03,
  /* RLE: 001 Pixels @ 201,094*/ 1, 0x04,
  /* RLE: 080 Pixels @ 202,094*/ 80, 0x00,
  /* ABS: 004 Pixels @ 282,094*/ 0, 4, 0x0D, 0x06, 0x06, 0x06,
  /* RLE: 102 Pixels @ 286,094*/ 102, 0x01,
  /* RLE: 031 Pixels @ 000,095*/ 31, 0x02,
  /* RLE: 001 Pixels @ 031,095*/ 1, 0x04,
  /* RLE: 006 Pixels @ 032,095*/ 6, 0x03,
  /* RLE: 001 Pixels @ 038,095*/ 1, 0x04,
  /* RLE: 019 Pixels @ 039,095*/ 19, 0x01,
  /* ABS: 002 Pixels @ 058,095*/ 0, 2, 0x04, 0x04,
  /* RLE: 008 Pixels @ 060,095*/ 8, 0x03,
  /* ABS: 002 Pixels @ 068,095*/ 0, 2, 0x04, 0x04,
  /* RLE: 030 Pixels @ 070,095*/ 30, 0x00,
  /* ABS: 011 Pixels @ 100,095*/ 0, 11, 0x03, 0x03, 0x0B, 0x07, 0x0F, 0x07, 0x07, 0x06, 0x0A, 0x03, 0x03,
  /* RLE: 083 Pixels @ 111,095*/ 83, 0x00,
  /* RLE: 001 Pixels @ 194,095*/ 1, 0x04,
  /* RLE: 006 Pixels @ 195,095*/ 6, 0x03,
  /* RLE: 001 Pixels @ 201,095*/ 1, 0x04,
  /* RLE: 081 Pixels @ 202,095*/ 81, 0x00,
  /* ABS: 004 Pixels @ 283,095*/ 0, 4, 0x0D, 0x06, 0x06, 0x06,
  /* RLE: 101 Pixels @ 287,095*/ 101, 0x01,
  /* RLE: 031 Pixels @ 000,096*/ 31, 0x02,
  /* RLE: 001 Pixels @ 031,096*/ 1, 0x04,
  /* RLE: 006 Pixels @ 032,096*/ 6, 0x03,
  /* RLE: 001 Pixels @ 038,096*/ 1, 0x04,
  /* RLE: 018 Pixels @ 039,096*/ 18, 0x01,
  /* ABS: 002 Pixels @ 057,096*/ 0, 2, 0x04, 0x04,
  /* RLE: 008 Pixels @ 059,096*/ 8, 0x03,
  /* ABS: 002 Pixels @ 067,096*/ 0, 2, 0x04, 0x04,
  /* RLE: 029 Pixels @ 069,096*/ 29, 0x00,
  /* ABS: 013 Pixels @ 098,096*/ 0, 13, 0x03, 0x03, 0x04, 0x00, 0x03, 0x12, 0x07, 0x06, 0x04, 0x11, 0x07, 0x0E, 0x03,
  /* RLE: 084 Pixels @ 111,096*/ 84, 0x00,
  /* RLE: 007 Pixels @ 195,096*/ 7, 0x03,
  /* RLE: 001 Pixels @ 202,096*/ 1, 0x04,
  /* RLE: 081 Pixels @ 203,096*/ 81, 0x00,
  /* ABS: 004 Pixels @ 284,096*/ 0, 4, 0x0D, 0x06, 0x03, 0x06,
  /* RLE: 100 Pixels @ 288,096*/ 100, 0x01,
  /* RLE: 031 Pixels @ 000,097*/ 31, 0x02,
  /* RLE: 001 Pixels @ 031,097*/ 1, 0x04,
  /* RLE: 006 Pixels @ 032,097*/ 6, 0x03,
  /* RLE: 001 Pixels @ 038,097*/ 1, 0x04,
  /* RLE: 017 Pixels @ 039,097*/ 17, 0x01,
  /* ABS: 002 Pixels @ 056,097*/ 0, 2, 0x04, 0x04,
  /* RLE: 008 Pixels @ 058,097*/ 8, 0x03,
  /* ABS: 002 Pixels @ 066,097*/ 0, 2, 0x04, 0x04,
  /* RLE: 028 Pixels @ 068,097*/ 28, 0x00,
  /* ABS: 015 Pixels @ 096,097*/ 0, 15, 0x04, 0x03, 0x00, 0x0F, 0x07, 0x0D, 0x03, 0x03, 0x11, 0x0F, 0x03, 0x03, 0x00, 0x03, 0x03,
  /* RLE: 051 Pixels @ 111,097*/ 51, 0x00,
  /* ABS: 004 Pixels @ 162,097*/ 0, 4, 0x07, 0x00, 0x00, 0x0A,
  /* RLE: 029 Pixels @ 166,097*/ 29, 0x00,
  /* RLE: 001 Pixels @ 195,097*/ 1, 0x04,
  /* RLE: 006 Pixels @ 196,097*/ 6, 0x03,
  /* RLE: 001 Pixels @ 202,097*/ 1, 0x04,
  /* RLE: 082 Pixels @ 203,097*/ 82, 0x00,
  /* ABS: 005 Pixels @ 285,097*/ 0, 5, 0x0D, 0x06, 0x03, 0x06, 0x06,
  /* RLE: 098 Pixels @ 290,097*/ 98, 0x01,
  /* RLE: 031 Pixels @ 000,098*/ 31, 0x02,
  /* RLE: 001 Pixels @ 031,098*/ 1, 0x04,
  /* RLE: 007 Pixels @ 032,098*/ 7, 0x03,
  /* RLE: 016 Pixels @ 039,098*/ 16, 0x01,
  /* ABS: 002 Pixels @ 055,098*/ 0, 2, 0x04, 0x04,
  /* RLE: 008 Pixels @ 057,098*/ 8, 0x03,
  /* ABS: 002 Pixels @ 065,098*/ 0, 2, 0x04, 0x04,
  /* RLE: 028 Pixels @ 067,098*/ 28, 0x00,
  /* ABS: 013 Pixels @ 095,098*/ 0, 13, 0x04, 0x04, 0x03, 0x0F, 0x0E, 0x03, 0x00, 0x03, 0x03, 0x00, 0x07, 0x0E, 0x03,
  /* RLE: 054 Pixels @ 108,098*/ 54, 0x00,
  /* ABS: 003 Pixels @ 162,098*/ 0, 3, 0x07, 0x00, 0x0A,
  /* RLE: 029 Pixels @ 165,098*/ 29, 0x00,
  /* ABS: 002 Pixels @ 194,098*/ 0, 2, 0x0A, 0x04,
  /* RLE: 006 Pixels @ 196,098*/ 6, 0x03,
  /* RLE: 001 Pixels @ 202,098*/ 1, 0x04,
  /* RLE: 083 Pixels @ 203,098*/ 83, 0x00,
  /* ABS: 005 Pixels @ 286,098*/ 0, 5, 0x0D, 0x06, 0x03, 0x06, 0x0D,
  /* RLE: 097 Pixels @ 291,098*/ 97, 0x01,
  /* RLE: 032 Pixels @ 000,099*/ 32, 0x02,
  /* RLE: 001 Pixels @ 032,099*/ 1, 0x04,
  /* RLE: 006 Pixels @ 033,099*/ 6, 0x03,
  /* RLE: 001 Pixels @ 039,099*/ 1, 0x04,
  /* RLE: 014 Pixels @ 040,099*/ 14, 0x01,
  /* ABS: 002 Pixels @ 054,099*/ 0, 2, 0x04, 0x04,
  /* RLE: 008 Pixels @ 056,099*/ 8, 0x03,
  /* ABS: 002 Pixels @ 064,099*/ 0, 2, 0x04, 0x04,
  /* RLE: 029 Pixels @ 066,099*/ 29, 0x00,
  /* RLE: 003 Pixels @ 095,099*/ 3, 0x03,
  /* ABS: 010 Pixels @ 098,099*/ 0, 10, 0x07, 0x04, 0x03, 0x03, 0x0B, 0x06, 0x03, 0x0A, 0x03, 0x03,
  /* RLE: 055 Pixels @ 108,099*/ 55, 0x00,
  /* ABS: 004 Pixels @ 163,099*/ 0, 4, 0x07, 0x00, 0x00, 0x0A,
  /* RLE: 028 Pixels @ 167,099*/ 28, 0x00,
  /* RLE: 001 Pixels @ 195,099*/ 1, 0x0A,
  /* RLE: 007 Pixels @ 196,099*/ 7, 0x03,
  /* RLE: 084 Pixels @ 203,099*/ 84, 0x00,
  /* RLE: 001 Pixels @ 287,099*/ 1, 0x0D,
  /* RLE: 004 Pixels @ 288,099*/ 4, 0x06,
  /* RLE: 096 Pixels @ 292,099*/ 96, 0x01,
  /* RLE: 032 Pixels @ 000,100*/ 32, 0x02,
  /* RLE: 001 Pixels @ 032,100*/ 1, 0x04,
  /* RLE: 006 Pixels @ 033,100*/ 6, 0x03,
  /* RLE: 001 Pixels @ 039,100*/ 1, 0x04,
  /* RLE: 013 Pixels @ 040,100*/ 13, 0x01,
  /* ABS: 002 Pixels @ 053,100*/ 0, 2, 0x04, 0x04,
  /* RLE: 008 Pixels @ 055,100*/ 8, 0x03,
  /* ABS: 002 Pixels @ 063,100*/ 0, 2, 0x04, 0x04,
  /* RLE: 024 Pixels @ 065,100*/ 24, 0x00,
  /* ABS: 016 Pixels @ 089,100*/ 0, 16, 0x03, 0x03, 0x00, 0x00, 0x03, 0x03, 0x04, 0x0B, 0x00, 0x11, 0x11, 0x03, 0x03, 0x0E, 0x0D, 0x03,
  /* RLE: 058 Pixels @ 105,100*/ 58, 0x00,
  /* ABS: 003 Pixels @ 163,100*/ 0, 3, 0x0A, 0x00, 0x0A,
  /* RLE: 027 Pixels @ 166,100*/ 27, 0x00,
  /* ABS: 004 Pixels @ 193,100*/ 0, 4, 0x0A, 0x00, 0x0A, 0x04,
  /* RLE: 006 Pixels @ 197,100*/ 6, 0x03,
  /* RLE: 001 Pixels @ 203,100*/ 1, 0x04,
  /* RLE: 085 Pixels @ 204,100*/ 85, 0x00,
  /* RLE: 004 Pixels @ 289,100*/ 4, 0x06,
  /* RLE: 095 Pixels @ 293,100*/ 95, 0x01,
  /* RLE: 032 Pixels @ 000,101*/ 32, 0x02,
  /* RLE: 001 Pixels @ 032,101*/ 1, 0x04,
  /* RLE: 006 Pixels @ 033,101*/ 6, 0x03,
  /* RLE: 001 Pixels @ 039,101*/ 1, 0x04,
  /* RLE: 012 Pixels @ 040,101*/ 12, 0x01,
  /* ABS: 002 Pixels @ 052,101*/ 0, 2, 0x04, 0x04,
  /* RLE: 008 Pixels @ 054,101*/ 8, 0x03,
  /* ABS: 002 Pixels @ 062,101*/ 0, 2, 0x04, 0x04,
  /* RLE: 024 Pixels @ 064,101*/ 24, 0x00,
  /* ABS: 017 Pixels @ 088,101*/ 0, 17, 0x03, 0x0B, 0x00, 0x03, 0x03, 0x00, 0x0F, 0x07, 0x07, 0x0F, 0x00, 0x07, 0x11, 0x06, 0x07, 0x0A, 0x03,
  /* RLE: 061 Pixels @ 105,101*/ 61, 0x00,
  /* RLE: 001 Pixels @ 166,101*/ 1, 0x0A,
  /* RLE: 029 Pixels @ 167,101*/ 29, 0x00,
  /* RLE: 001 Pixels @ 196,101*/ 1, 0x04,
  /* RLE: 006 Pixels @ 197,101*/ 6, 0x03,
  /* RLE: 001 Pixels @ 203,101*/ 1, 0x04,
  /* RLE: 086 Pixels @ 204,101*/ 86, 0x00,
  /* ABS: 004 Pixels @ 290,101*/ 0, 4, 0x0D, 0x06, 0x03, 0x06,
  /* RLE: 094 Pixels @ 294,101*/ 94, 0x01,
  /* RLE: 032 Pixels @ 000,102*/ 32, 0x02,
  /* RLE: 001 Pixels @ 032,102*/ 1, 0x04,
  /* RLE: 006 Pixels @ 033,102*/ 6, 0x03,
  /* RLE: 001 Pixels @ 039,102*/ 1, 0x04,
  /* RLE: 011 Pixels @ 040,102*/ 11, 0x01,
  /* ABS: 002 Pixels @ 051,102*/ 0, 2, 0x04, 0x04,
  /* RLE: 008 Pixels @ 053,102*/ 8, 0x03,
  /* ABS: 002 Pixels @ 061,102*/ 0, 2, 0x04, 0x04,
  /* RLE: 026 Pixels @ 063,102*/ 26, 0x00,
  /* ABS: 015 Pixels @ 089,102*/ 0, 15, 0x0F, 0x11, 0x03, 0x03, 0x11, 0x12, 0x03, 0x00, 0x0F, 0x11, 0x0A, 0x12, 0x12, 0x0A, 0x03,
  /* RLE: 058 Pixels @ 104,102*/ 58, 0x00,
  /* RLE: 001 Pixels @ 162,102*/ 1, 0x0A,
  /* RLE: 033 Pixels @ 163,102*/ 33, 0x00,
  /* RLE: 001 Pixels @ 196,102*/ 1, 0x04,
  /* RLE: 007 Pixels @ 197,102*/ 7, 0x03,
  /* RLE: 087 Pixels @ 204,102*/ 87, 0x00,
  /* ABS: 004 Pixels @ 291,102*/ 0, 4, 0x06, 0x06, 0x03, 0x0D,
  /* RLE: 093 Pixels @ 295,102*/ 93, 0x01,
  /* RLE: 032 Pixels @ 000,103*/ 32, 0x02,
  /* RLE: 001 Pixels @ 032,103*/ 1, 0x04,
  /* RLE: 006 Pixels @ 033,103*/ 6, 0x03,
  /* RLE: 001 Pixels @ 039,103*/ 1, 0x04,
  /* RLE: 010 Pixels @ 040,103*/ 10, 0x01,
  /* ABS: 002 Pixels @ 050,103*/ 0, 2, 0x04, 0x04,
  /* RLE: 008 Pixels @ 052,103*/ 8, 0x03,
  /* ABS: 002 Pixels @ 060,103*/ 0, 2, 0x04, 0x04,
  /* RLE: 022 Pixels @ 062,103*/ 22, 0x00,
  /* RLE: 005 Pixels @ 084,103*/ 5, 0x03,
  /* ABS: 010 Pixels @ 089,103*/ 0, 10, 0x0A, 0x07, 0x12, 0x03, 0x07, 0x0B, 0x03, 0x03, 0x0B, 0x07,
  /* RLE: 004 Pixels @ 099,103*/ 4, 0x03,
  /* RLE: 061 Pixels @ 103,103*/ 61, 0x00,
  /* ABS: 004 Pixels @ 164,103*/ 0, 4, 0x07, 0x00, 0x00, 0x0A,
  /* RLE: 029 Pixels @ 168,103*/ 29, 0x00,
  /* RLE: 001 Pixels @ 197,103*/ 1, 0x04,
  /* RLE: 006 Pixels @ 198,103*/ 6, 0x03,
  /* RLE: 001 Pixels @ 204,103*/ 1, 0x04,
  /* RLE: 087 Pixels @ 205,103*/ 87, 0x00,
  /* ABS: 004 Pixels @ 292,103*/ 0, 4, 0x0D, 0x06, 0x03, 0x06,
  /* RLE: 092 Pixels @ 296,103*/ 92, 0x01,
  /* RLE: 032 Pixels @ 000,104*/ 32, 0x02,
  /* RLE: 001 Pixels @ 032,104*/ 1, 0x04,
  /* RLE: 007 Pixels @ 033,104*/ 7, 0x03,
  /* RLE: 001 Pixels @ 040,104*/ 1, 0x04,
  /* RLE: 007 Pixels @ 041,104*/ 7, 0x01,
  /* ABS: 002 Pixels @ 048,104*/ 0, 2, 0x04, 0x04,
  /* RLE: 010 Pixels @ 050,104*/ 10, 0x03,
  /* ABS: 002 Pixels @ 060,104*/ 0, 2, 0x04, 0x04,
  /* RLE: 022 Pixels @ 062,104*/ 22, 0x00,
  /* ABS: 017 Pixels @ 084,104*/ 0, 17, 0x03, 0x04, 0x0B, 0x03, 0x03, 0x03, 0x0E, 0x07, 0x0B, 0x11, 0x11, 0x03, 0x03, 0x0E, 0x0F, 0x03, 0x04,
  /* RLE: 062 Pixels @ 101,104*/ 62, 0x00,
  /* ABS: 004 Pixels @ 163,104*/ 0, 4, 0x0A, 0x00, 0x07, 0x0A,
  /* RLE: 029 Pixels @ 167,104*/ 29, 0x00,
  /* ABS: 002 Pixels @ 196,104*/ 0, 2, 0x0A, 0x04,
  /* RLE: 006 Pixels @ 198,104*/ 6, 0x03,
  /* RLE: 001 Pixels @ 204,104*/ 1, 0x04,
  /* RLE: 088 Pixels @ 205,104*/ 88, 0x00,
  /* RLE: 005 Pixels @ 293,104*/ 5, 0x06,
  /* RLE: 046 Pixels @ 298,104*/ 46, 0x01,
  /* RLE: 010 Pixels @ 344,104*/ 10, 0x09,
  /* RLE: 034 Pixels @ 354,104*/ 34, 0x01,
  /* RLE: 033 Pixels @ 000,105*/ 33, 0x02,
  /* RLE: 001 Pixels @ 033,105*/ 1, 0x04,
  /* RLE: 006 Pixels @ 034,105*/ 6, 0x03,
  /* RLE: 001 Pixels @ 040,105*/ 1, 0x04,
  /* RLE: 005 Pixels @ 041,105*/ 5, 0x01,
  /* RLE: 003 Pixels @ 046,105*/ 3, 0x04,
  /* RLE: 013 Pixels @ 049,105*/ 13, 0x03,
  /* ABS: 002 Pixels @ 062,105*/ 0, 2, 0x04, 0x04,
  /* RLE: 020 Pixels @ 064,105*/ 20, 0x00,
  /* ABS: 017 Pixels @ 084,105*/ 0, 17, 0x03, 0x0D, 0x07, 0x00, 0x03, 0x0B, 0x07, 0x0F, 0x07, 0x04, 0x07, 0x11, 0x06, 0x07, 0x04, 0x03, 0x0A,
  /* RLE: 064 Pixels @ 101,105*/ 64, 0x00,
  /* ABS: 003 Pixels @ 165,105*/ 0, 3, 0x07, 0x00, 0x0A,
  /* RLE: 029 Pixels @ 168,105*/ 29, 0x00,
  /* RLE: 001 Pixels @ 197,105*/ 1, 0x0A,
  /* RLE: 007 Pixels @ 198,105*/ 7, 0x03,
  /* RLE: 089 Pixels @ 205,105*/ 89, 0x00,
  /* RLE: 001 Pixels @ 294,105*/ 1, 0x0D,
  /* RLE: 004 Pixels @ 295,105*/ 4, 0x06,
  /* RLE: 029 Pixels @ 299,105*/ 29, 0x01,
  /* RLE: 016 Pixels @ 328,105*/ 16, 0x09,
  /* RLE: 010 Pixels @ 344,105*/ 10, 0x05,
  /* RLE: 006 Pixels @ 354,105*/ 6, 0x09,
  /* RLE: 028 Pixels @ 360,105*/ 28, 0x01,
  /* RLE: 033 Pixels @ 000,106*/ 33, 0x02,
  /* RLE: 001 Pixels @ 033,106*/ 1, 0x04,
  /* RLE: 007 Pixels @ 034,106*/ 7, 0x03,
  /* ABS: 006 Pixels @ 041,106*/ 0, 6, 0x04, 0x01, 0x01, 0x04, 0x04, 0x04,
  /* RLE: 018 Pixels @ 047,106*/ 18, 0x03,
  /* ABS: 002 Pixels @ 065,106*/ 0, 2, 0x04, 0x04,
  /* RLE: 017 Pixels @ 067,106*/ 17, 0x00,
  /* ABS: 015 Pixels @ 084,106*/ 0, 15, 0x03, 0x00, 0x0F, 0x11, 0x06, 0x07, 0x0B, 0x00, 0x0F, 0x11, 0x0A, 0x12, 0x0E, 0x0A, 0x03,
  /* RLE: 098 Pixels @ 099,106*/ 98, 0x00,
  /* ABS: 002 Pixels @ 197,106*/ 0, 2, 0x0A, 0x04,
  /* RLE: 006 Pixels @ 199,106*/ 6, 0x03,
  /* RLE: 001 Pixels @ 205,106*/ 1, 0x04,
  /* RLE: 089 Pixels @ 206,106*/ 89, 0x00,
  /* ABS: 005 Pixels @ 295,106*/ 0, 5, 0x06, 0x0D, 0x06, 0x06, 0x06,
  /* RLE: 011 Pixels @ 300,106*/ 11, 0x01,
  /* ABS: 003 Pixels @ 311,106*/ 0, 3, 0x00, 0x00, 0x0A,
  /* RLE: 014 Pixels @ 314,106*/ 14, 0x09,
  /* ABS: 003 Pixels @ 328,106*/ 0, 3, 0x05, 0x05, 0x1A,
  /* RLE: 005 Pixels @ 331,106*/ 5, 0x03,
  /* ABS: 002 Pixels @ 336,106*/ 0, 2, 0x15, 0x15,
  /* RLE: 004 Pixels @ 338,106*/ 4, 0x03,
  /* RLE: 001 Pixels @ 342,106*/ 1, 0x19,
  /* RLE: 017 Pixels @ 343,106*/ 17, 0x05,
  /* RLE: 006 Pixels @ 360,106*/ 6, 0x09,
  /* RLE: 022 Pixels @ 366,106*/ 22, 0x01,
  /* RLE: 033 Pixels @ 000,107*/ 33, 0x02,
  /* ABS: 002 Pixels @ 033,107*/ 0, 2, 0x04, 0x04,
  /* RLE: 006 Pixels @ 035,107*/ 6, 0x03,
  /* ABS: 004 Pixels @ 041,107*/ 0, 4, 0x04, 0x01, 0x04, 0x04,
  /* RLE: 022 Pixels @ 045,107*/ 22, 0x03,
  /* ABS: 002 Pixels @ 067,107*/ 0, 2, 0x04, 0x04,
  /* RLE: 014 Pixels @ 069,107*/ 14, 0x00,
  /* ABS: 015 Pixels @ 083,107*/ 0, 15, 0x04, 0x0A, 0x03, 0x04, 0x07, 0x07, 0x00, 0x03, 0x03, 0x0A, 0x07, 0x0E, 0x03, 0x03, 0x03,
  /* RLE: 068 Pixels @ 098,107*/ 68, 0x00,
  /* ABS: 004 Pixels @ 166,107*/ 0, 4, 0x0A, 0x00, 0x00, 0x0A,
  /* RLE: 028 Pixels @ 170,107*/ 28, 0x00,
  /* RLE: 001 Pixels @ 198,107*/ 1, 0x04,
  /* RLE: 006 Pixels @ 199,107*/ 6, 0x03,
  /* RLE: 001 Pixels @ 205,107*/ 1, 0x04,
  /* RLE: 068 Pixels @ 206,107*/ 68, 0x00,
  /* ABS: 003 Pixels @ 274,107*/ 0, 3, 0x03, 0x00, 0x00,
  /* RLE: 004 Pixels @ 277,107*/ 4, 0x03,
  /* RLE: 016 Pixels @ 281,107*/ 16, 0x00,
  /* RLE: 001 Pixels @ 297,107*/ 1, 0x0D,
  /* RLE: 012 Pixels @ 298,107*/ 12, 0x09,
  /* ABS: 005 Pixels @ 310,107*/ 0, 5, 0x04, 0x03, 0x12, 0x03, 0x15,
  /* RLE: 009 Pixels @ 315,107*/ 9, 0x03,
  /* RLE: 001 Pixels @ 324,107*/ 1, 0x00,
  /* RLE: 006 Pixels @ 325,107*/ 6, 0x03,
  /* ABS: 013 Pixels @ 331,107*/ 0, 13, 0x00, 0x11, 0x07, 0x07, 0x0E, 0x03, 0x03, 0x06, 0x07, 0x0F, 0x04, 0x03, 0x1A,
  /* RLE: 022 Pixels @ 344,107*/ 22, 0x05,
  /* RLE: 006 Pixels @ 366,107*/ 6, 0x09,
  /* RLE: 016 Pixels @ 372,107*/ 16, 0x01,
  /* RLE: 034 Pixels @ 000,108*/ 34, 0x02,
  /* RLE: 001 Pixels @ 034,108*/ 1, 0x04,
  /* RLE: 006 Pixels @ 035,108*/ 6, 0x03,
  /* ABS: 002 Pixels @ 041,108*/ 0, 2, 0x04, 0x04,
  /* RLE: 027 Pixels @ 043,108*/ 27, 0x03,
  /* ABS: 002 Pixels @ 070,108*/ 0, 2, 0x04, 0x04,
  /* RLE: 010 Pixels @ 072,108*/ 10, 0x00,
  /* ABS: 013 Pixels @ 082,108*/ 0, 13, 0x04, 0x04, 0x03, 0x03, 0x03, 0x0E, 0x07, 0x0B, 0x03, 0x03, 0x03, 0x04, 0x03,
  /* RLE: 073 Pixels @ 095,108*/ 73, 0x00,
  /* RLE: 001 Pixels @ 168,108*/ 1, 0x0A,
  /* RLE: 029 Pixels @ 169,108*/ 29, 0x00,
  /* RLE: 001 Pixels @ 198,108*/ 1, 0x04,
  /* RLE: 007 Pixels @ 199,108*/ 7, 0x03,
  /* RLE: 061 Pixels @ 206,108*/ 61, 0x00,
  /* RLE: 004 Pixels @ 267,108*/ 4, 0x03,
  /* ABS: 015 Pixels @ 271,108*/ 0, 15, 0x00, 0x03, 0x00, 0x00, 0x03, 0x03, 0x00, 0x00, 0x03, 0x0A, 0x03, 0x00, 0x03, 0x03, 0x00,
  /* RLE: 012 Pixels @ 286,108*/ 12, 0x09,
  /* RLE: 006 Pixels @ 298,108*/ 6, 0x05,
  /* RLE: 001 Pixels @ 304,108*/ 1, 0x1A,
  /* RLE: 006 Pixels @ 305,108*/ 6, 0x03,
  /* ABS: 033 Pixels @ 311,108*/ 0, 33, 0x0B, 0x07, 0x03, 0x03, 0x0B, 0x0E, 0x0D, 0x0F, 0x00, 0x06, 0x07, 0x07, 0x12, 0x03, 0x00, 0x11, 0x07, 0x07, 0x0E, 0x03, 0x06, 0x06, 0x03, 0x04, 0x0B, 0x03, 0x0E, 0x0F, 0x00, 0x0B, 0x07, 0x00, 0x15,
  /* RLE: 028 Pixels @ 344,108*/ 28, 0x05,
  /* RLE: 006 Pixels @ 372,108*/ 6, 0x09,
  /* RLE: 010 Pixels @ 378,108*/ 10, 0x01,
  /* RLE: 034 Pixels @ 000,109*/ 34, 0x02,
  /* RLE: 001 Pixels @ 034,109*/ 1, 0x04,
  /* RLE: 018 Pixels @ 035,109*/ 18, 0x03,
  /* RLE: 003 Pixels @ 053,109*/ 3, 0x04,
  /* RLE: 016 Pixels @ 056,109*/ 16, 0x03,
  /* ABS: 002 Pixels @ 072,109*/ 0, 2, 0x04, 0x04,
  /* RLE: 006 Pixels @ 074,109*/ 6, 0x00,
  /* RLE: 008 Pixels @ 080,109*/ 8, 0x03,
  /* ABS: 007 Pixels @ 088,109*/ 0, 7, 0x0D, 0x07, 0x00, 0x03, 0x00, 0x00, 0x03,
  /* RLE: 071 Pixels @ 095,109*/ 71, 0x00,
  /* ABS: 005 Pixels @ 166,109*/ 0, 5, 0x0A, 0x07, 0x00, 0x00, 0x0A,
  /* RLE: 028 Pixels @ 171,109*/ 28, 0x00,
  /* RLE: 001 Pixels @ 199,109*/ 1, 0x04,
  /* RLE: 006 Pixels @ 200,109*/ 6, 0x03,
  /* RLE: 001 Pixels @ 206,109*/ 1, 0x04,
  /* RLE: 061 Pixels @ 207,109*/ 61, 0x00,
  /* ABS: 017 Pixels @ 268,109*/ 0, 17, 0x06, 0x07, 0x03, 0x09, 0x03, 0x07, 0x12, 0x03, 0x0A, 0x0B, 0x0D, 0x00, 0x07, 0x0A, 0x03, 0x06, 0x06,
  /* RLE: 006 Pixels @ 285,109*/ 6, 0x03,
  /* ABS: 002 Pixels @ 291,109*/ 0, 2, 0x15, 0x15,
  /* RLE: 004 Pixels @ 293,109*/ 4, 0x03,
  /* ABS: 002 Pixels @ 297,109*/ 0, 2, 0x15, 0x00,
  /* RLE: 006 Pixels @ 299,109*/ 6, 0x03,
  /* ABS: 039 Pixels @ 305,109*/ 0, 39, 0x00, 0x11, 0x07, 0x07, 0x0E, 0x00, 0x11, 0x07, 0x0F, 0x03, 0x0B, 0x07, 0x0E, 0x0A, 0x0B, 0x06, 0x00, 0x0B, 0x07, 0x00, 0x06, 0x06, 0x03, 0x04, 0x04, 0x03, 0x0E, 0x07, 0x0F, 0x06, 0x04, 0x03, 0x06, 0x11, 0x0B, 0x0B, 0x07, 0x0B, 0x00,
  /* RLE: 034 Pixels @ 344,109*/ 34, 0x05,
  /* RLE: 006 Pixels @ 378,109*/ 6, 0x09,
  /* RLE: 004 Pixels @ 384,109*/ 4, 0x01,
  /* RLE: 035 Pixels @ 000,110*/ 35, 0x02,
  /* RLE: 001 Pixels @ 035,110*/ 1, 0x04,
  /* RLE: 016 Pixels @ 036,110*/ 16, 0x03,
  /* ABS: 007 Pixels @ 052,110*/ 0, 7, 0x04, 0x04, 0x00, 0x00, 0x00, 0x04, 0x04,
  /* RLE: 016 Pixels @ 059,110*/ 16, 0x03,
  /* ABS: 008 Pixels @ 075,110*/ 0, 8, 0x04, 0x04, 0x00, 0x00, 0x03, 0x00, 0x0E, 0x0A,
  /* RLE: 005 Pixels @ 083,110*/ 5, 0x03,
  /* ABS: 005 Pixels @ 088,110*/ 0, 5, 0x00, 0x0F, 0x06, 0x03, 0x04,
  /* RLE: 072 Pixels @ 093,110*/ 72, 0x00,
  /* ABS: 005 Pixels @ 165,110*/ 0, 5, 0x0A, 0x00, 0x07, 0x00, 0x0A,
  /* RLE: 028 Pixels @ 170,110*/ 28, 0x00,
  /* ABS: 002 Pixels @ 198,110*/ 0, 2, 0x0A, 0x04,
  /* RLE: 006 Pixels @ 200,110*/ 6, 0x03,
  /* RLE: 001 Pixels @ 206,110*/ 1, 0x04,
  /* RLE: 050 Pixels @ 207,110*/ 50, 0x00,
  /* RLE: 010 Pixels @ 257,110*/ 10, 0x09,
  /* ABS: 008 Pixels @ 267,110*/ 0, 8, 0x03, 0x0E, 0x07, 0x03, 0x05, 0x03, 0x0F, 0x06,
  /* RLE: 008 Pixels @ 275,110*/ 8, 0x03,
  /* ABS: 061 Pixels @ 283,110*/ 0, 61, 0x0E, 0x11, 0x03, 0x00, 0x11, 0x07, 0x07, 0x0E, 0x03, 0x03, 0x06, 0x07, 0x0F, 0x04, 0x03, 0x04, 0x0E, 0x0E, 0x07, 0x0F, 0x0A, 0x03, 0x06, 0x06, 0x03, 0x04, 0x0B, 0x00, 0x06, 0x07, 0x00, 0x03, 0x0B, 0x07, 0x03, 0x03, 0x03, 0x0A, 0x0E, 0x11, 0x07, 0x0A, 0x0E, 0x07, 0x0F, 0x06, 0x0B, 0x03, 0x03, 0x0A, 0x0E, 0x0D, 0x07, 0x0B, 0x06, 0x11, 0x0B, 0x00, 0x00, 0x00, 0x03,
  /* RLE: 040 Pixels @ 344,110*/ 40, 0x05,
  /* RLE: 004 Pixels @ 384,110*/ 4, 0x09,
  /* RLE: 035 Pixels @ 000,111*/ 35, 0x02,
  /* RLE: 001 Pixels @ 035,111*/ 1, 0x04,
  /* RLE: 014 Pixels @ 036,111*/ 14, 0x03,
  /* ABS: 003 Pixels @ 050,111*/ 0, 3, 0x04, 0x04, 0x0A,
  /* RLE: 006 Pixels @ 053,111*/ 6, 0x00,
  /* ABS: 002 Pixels @ 059,111*/ 0, 2, 0x04, 0x04,
  /* RLE: 018 Pixels @ 061,111*/ 18, 0x03,
  /* ABS: 005 Pixels @ 079,111*/ 0, 5, 0x0A, 0x07, 0x0F, 0x07, 0x00,
  /* RLE: 005 Pixels @ 084,111*/ 5, 0x03,
  /* ABS: 003 Pixels @ 089,111*/ 0, 3, 0x0A, 0x03, 0x03,
  /* RLE: 076 Pixels @ 092,111*/ 76, 0x00,
  /* ABS: 004 Pixels @ 168,111*/ 0, 4, 0x07, 0x00, 0x00, 0x0A,
  /* RLE: 027 Pixels @ 172,111*/ 27, 0x00,
  /* RLE: 001 Pixels @ 199,111*/ 1, 0x04,
  /* RLE: 007 Pixels @ 200,111*/ 7, 0x03,
  /* RLE: 046 Pixels @ 207,111*/ 46, 0x00,
  /* RLE: 004 Pixels @ 253,111*/ 4, 0x09,
  /* RLE: 010 Pixels @ 257,111*/ 10, 0x05,
  /* ABS: 077 Pixels @ 267,111*/ 0, 77, 0x03, 0x0B, 0x07, 0x03, 0x03, 0x03, 0x0D, 0x06, 0x03, 0x0B, 0x06, 0x03, 0x03, 0x07, 0x0E, 0x03, 0x0B, 0x07, 0x03, 0x06, 0x06, 0x03, 0x04, 0x04, 0x03, 0x0E, 0x0F, 0x00, 0x0B, 0x07, 0x00, 0x0B, 0x07, 0x0B, 0x00, 0x0F, 0x12, 0x03, 0x0E, 0x07, 0x0F, 0x06, 0x04, 0x03, 0x0A, 0x07, 0x0A, 0x03, 0x0B, 0x07, 0x03, 0x03, 0x0B, 0x0F, 0x04, 0x04, 0x07, 0x00, 0x03, 0x04, 0x0E, 0x0D, 0x07, 0x0B, 0x0B, 0x0D, 0x00, 0x04, 0x07, 0x04, 0x0B, 0x07, 0x0A, 0x0B, 0x07, 0x04, 0x15,
  /* RLE: 044 Pixels @ 344,111*/ 44, 0x05,
  /* RLE: 036 Pixels @ 000,112*/ 36, 0x02,
  /* RLE: 001 Pixels @ 036,112*/ 1, 0x04,
  /* RLE: 011 Pixels @ 037,112*/ 11, 0x03,
  /* ABS: 002 Pixels @ 048,112*/ 0, 2, 0x04, 0x04,
  /* RLE: 012 Pixels @ 050,112*/ 12, 0x00,
  /* ABS: 002 Pixels @ 062,112*/ 0, 2, 0x04, 0x04,
  /* RLE: 013 Pixels @ 064,112*/ 13, 0x03,
  /* ABS: 007 Pixels @ 077,112*/ 0, 7, 0x00, 0x0B, 0x06, 0x04, 0x03, 0x11, 0x11,
  /* RLE: 004 Pixels @ 084,112*/ 4, 0x03,
  /* RLE: 082 Pixels @ 088,112*/ 82, 0x00,
  /* RLE: 001 Pixels @ 170,112*/ 1, 0x0A,
  /* RLE: 029 Pixels @ 171,112*/ 29, 0x00,
  /* RLE: 001 Pixels @ 200,112*/ 1, 0x04,
  /* RLE: 006 Pixels @ 201,112*/ 6, 0x03,
  /* RLE: 001 Pixels @ 207,112*/ 1, 0x04,
  /* RLE: 041 Pixels @ 208,112*/ 41, 0x00,
  /* RLE: 004 Pixels @ 249,112*/ 4, 0x09,
  /* RLE: 014 Pixels @ 253,112*/ 14, 0x05,
  /* ABS: 002 Pixels @ 267,112*/ 0, 2, 0x03, 0x0B,
  /* RLE: 005 Pixels @ 269,112*/ 5, 0x07,
  /* ABS: 070 Pixels @ 274,112*/ 0, 70, 0x06, 0x03, 0x0B, 0x07, 0x03, 0x03, 0x06, 0x06, 0x03, 0x0B, 0x07, 0x03, 0x0E, 0x07, 0x0F, 0x06, 0x0B, 0x03, 0x06, 0x11, 0x0B, 0x0B, 0x07, 0x0B, 0x0B, 0x07, 0x03, 0x03, 0x06, 0x06, 0x03, 0x03, 0x0A, 0x0E, 0x0D, 0x07, 0x04, 0x0A, 0x07, 0x00, 0x03, 0x04, 0x07, 0x0A, 0x03, 0x12, 0x11, 0x00, 0x0E, 0x07, 0x0A, 0x0B, 0x0D, 0x00, 0x0A, 0x07, 0x04, 0x00, 0x12, 0x07, 0x11, 0x0B, 0x03, 0x03, 0x0E, 0x0F, 0x11, 0x0B, 0x03, 0x19,
  /* RLE: 044 Pixels @ 344,112*/ 44, 0x05,
  /* RLE: 035 Pixels @ 000,113*/ 35, 0x02,
  /* ABS: 002 Pixels @ 035,113*/ 0, 2, 0x04, 0x04,
  /* RLE: 009 Pixels @ 037,113*/ 9, 0x03,
  /* RLE: 003 Pixels @ 046,113*/ 3, 0x04,
  /* RLE: 001 Pixels @ 049,113*/ 1, 0x0A,
  /* RLE: 014 Pixels @ 050,113*/ 14, 0x00,
  /* ABS: 002 Pixels @ 064,113*/ 0, 2, 0x04, 0x04,
  /* RLE: 011 Pixels @ 066,113*/ 11, 0x03,
  /* ABS: 013 Pixels @ 077,113*/ 0, 13, 0x0F, 0x07, 0x07, 0x0B, 0x03, 0x00, 0x07, 0x12, 0x03, 0x03, 0x04, 0x04, 0x0A,
  /* RLE: 079 Pixels @ 090,113*/ 79, 0x00,
  /* RLE: 001 Pixels @ 169,113*/ 1, 0x0A,
  /* RLE: 030 Pixels @ 170,113*/ 30, 0x00,
  /* RLE: 001 Pixels @ 200,113*/ 1, 0x04,
  /* RLE: 010 Pixels @ 201,113*/ 10, 0x03,
  /* RLE: 034 Pixels @ 211,113*/ 34, 0x00,
  /* RLE: 004 Pixels @ 245,113*/ 4, 0x09,
  /* RLE: 018 Pixels @ 249,113*/ 18, 0x05,
  /* ABS: 063 Pixels @ 267,113*/ 0, 63, 0x03, 0x0B, 0x07, 0x0E, 0x0A, 0x0A, 0x0D, 0x0F, 0x03, 0x0B, 0x07, 0x03, 0x03, 0x06, 0x06, 0x03, 0x0B, 0x07, 0x03, 0x03, 0x0A, 0x0E, 0x0D, 0x07, 0x0B, 0x06, 0x11, 0x0B, 0x00, 0x0A, 0x00, 0x04, 0x07, 0x00, 0x03, 0x06, 0x06, 0x03, 0x0B, 0x0D, 0x00, 0x04, 0x07, 0x04, 0x00, 0x07, 0x0B, 0x00, 0x0A, 0x07, 0x00, 0x03, 0x00, 0x11, 0x0F, 0x0E, 0x12, 0x0B, 0x00, 0x12, 0x07, 0x11, 0x0B,
  /* RLE: 006 Pixels @ 330,113*/ 6, 0x03,
  /* ABS: 002 Pixels @ 336,113*/ 0, 2, 0x15, 0x15,
  /* RLE: 004 Pixels @ 338,113*/ 4, 0x03,
  /* RLE: 001 Pixels @ 342,113*/ 1, 0x15,
  /* RLE: 045 Pixels @ 343,113*/ 45, 0x05,
  /* RLE: 034 Pixels @ 000,114*/ 34, 0x02,
  /* ABS: 002 Pixels @ 034,114*/ 0, 2, 0x04, 0x04,
  /* RLE: 009 Pixels @ 036,114*/ 9, 0x03,
  /* ABS: 002 Pixels @ 045,114*/ 0, 2, 0x04, 0x04,
  /* RLE: 020 Pixels @ 047,114*/ 20, 0x00,
  /* ABS: 021 Pixels @ 067,114*/ 0, 21, 0x04, 0x04, 0x03, 0x03, 0x03, 0x0B, 0x04, 0x03, 0x0B, 0x06, 0x0E, 0x03, 0x0D, 0x0F, 0x00, 0x03, 0x0B, 0x0F, 0x00, 0x00, 0x04,
  /* RLE: 079 Pixels @ 088,114*/ 79, 0x00,
  /* ABS: 005 Pixels @ 167,114*/ 0, 5, 0x0A, 0x00, 0x00, 0x00, 0x0A,
  /* RLE: 027 Pixels @ 172,114*/ 27, 0x00,
  /* ABS: 002 Pixels @ 199,114*/ 0, 2, 0x0A, 0x04,
  /* RLE: 005 Pixels @ 201,114*/ 5, 0x03,
  /* ABS: 005 Pixels @ 206,114*/ 0, 5, 0x00, 0x0A, 0x0B, 0x06, 0x03,
  /* RLE: 029 Pixels @ 211,114*/ 29, 0x00,
  /* RLE: 005 Pixels @ 240,114*/ 5, 0x09,
  /* RLE: 022 Pixels @ 245,114*/ 22, 0x05,
  /* ABS: 052 Pixels @ 267,114*/ 0, 52, 0x03, 0x0A, 0x07, 0x0A, 0x03, 0x03, 0x0E, 0x07, 0x03, 0x0B, 0x07, 0x00, 0x03, 0x06, 0x06, 0x03, 0x0A, 0x07, 0x00, 0x0B, 0x0D, 0x00, 0x0A, 0x07, 0x04, 0x0B, 0x07, 0x0A, 0x0B, 0x07, 0x04, 0x00, 0x07, 0x0A, 0x03, 0x0E, 0x07, 0x03, 0x00, 0x12, 0x07, 0x11, 0x0B, 0x03, 0x03, 0x0D, 0x07, 0x04, 0x03, 0x03, 0x03, 0x15,
  /* RLE: 011 Pixels @ 319,114*/ 11, 0x03,
  /* RLE: 001 Pixels @ 330,114*/ 1, 0x15,
  /* RLE: 013 Pixels @ 331,114*/ 13, 0x05,
  /* RLE: 005 Pixels @ 344,114*/ 5, 0x09,
  /* RLE: 005 Pixels @ 349,114*/ 5, 0x03,
  /* RLE: 034 Pixels @ 354,114*/ 34, 0x05,
  /* RLE: 034 Pixels @ 000,115*/ 34, 0x02,
  /* RLE: 001 Pixels @ 034,115*/ 1, 0x04,
  /* RLE: 008 Pixels @ 035,115*/ 8, 0x03,
  /* ABS: 002 Pixels @ 043,115*/ 0, 2, 0x04, 0x04,
  /* RLE: 024 Pixels @ 045,115*/ 24, 0x00,
  /* ABS: 017 Pixels @ 069,115*/ 0, 17, 0x04, 0x04, 0x03, 0x11, 0x0F, 0x00, 0x0B, 0x07, 0x0E, 0x03, 0x00, 0x0F, 0x0D, 0x03, 0x03, 0x00, 0x03,
  /* RLE: 083 Pixels @ 086,115*/ 83, 0x00,
  /* RLE: 001 Pixels @ 169,115*/ 1, 0x07,
  /* RLE: 031 Pixels @ 170,115*/ 31, 0x00,
  /* ABS: 009 Pixels @ 201,115*/ 0, 9, 0x03, 0x00, 0x0B, 0x06, 0x0F, 0x07, 0x07, 0x07, 0x0F,
  /* RLE: 026 Pixels @ 210,115*/ 26, 0x00,
  /* RLE: 004 Pixels @ 236,115*/ 4, 0x09,
  /* RLE: 027 Pixels @ 240,115*/ 27, 0x05,
  /* ABS: 039 Pixels @ 267,115*/ 0, 39, 0x00, 0x0A, 0x07, 0x0A, 0x00, 0x03, 0x0B, 0x07, 0x03, 0x0A, 0x07, 0x0B, 0x0A, 0x0F, 0x11, 0x03, 0x0A, 0x07, 0x0A, 0x00, 0x12, 0x07, 0x11, 0x0B, 0x03, 0x03, 0x0E, 0x0F, 0x11, 0x0B, 0x03, 0x0A, 0x0F, 0x00, 0x03, 0x04, 0x12, 0x03, 0x00,
  /* RLE: 004 Pixels @ 306,115*/ 4, 0x03,
  /* ABS: 006 Pixels @ 310,115*/ 0, 6, 0x15, 0x00, 0x03, 0x03, 0x03, 0x19,
  /* RLE: 012 Pixels @ 316,115*/ 12, 0x05,
  /* RLE: 016 Pixels @ 328,115*/ 16, 0x09,
  /* RLE: 005 Pixels @ 344,115*/ 5, 0x01,
  /* RLE: 001 Pixels @ 349,115*/ 1, 0x04,
  /* RLE: 008 Pixels @ 350,115*/ 8, 0x03,
  /* ABS: 002 Pixels @ 358,115*/ 0, 2, 0x09, 0x09,
  /* RLE: 028 Pixels @ 360,115*/ 28, 0x05,
  /* RLE: 034 Pixels @ 000,116*/ 34, 0x02,
  /* RLE: 001 Pixels @ 034,116*/ 1, 0x04,
  /* RLE: 007 Pixels @ 035,116*/ 7, 0x03,
  /* RLE: 001 Pixels @ 042,116*/ 1, 0x04,
  /* RLE: 027 Pixels @ 043,116*/ 27, 0x00,
  /* ABS: 013 Pixels @ 070,116*/ 0, 13, 0x0A, 0x03, 0x00, 0x07, 0x0D, 0x03, 0x06, 0x07, 0x00, 0x03, 0x04, 0x07, 0x04,
  /* RLE: 085 Pixels @ 083,116*/ 85, 0x00,
  /* ABS: 006 Pixels @ 168,116*/ 0, 6, 0x0A, 0x00, 0x07, 0x0A, 0x00, 0x0A,
  /* RLE: 024 Pixels @ 174,116*/ 24, 0x00,
  /* ABS: 013 Pixels @ 198,116*/ 0, 13, 0x0A, 0x00, 0x0A, 0x03, 0x12, 0x07, 0x0F, 0x06, 0x0B, 0x0A, 0x00, 0x03, 0x03,
  /* RLE: 021 Pixels @ 211,116*/ 21, 0x00,
  /* RLE: 004 Pixels @ 232,116*/ 4, 0x09,
  /* RLE: 031 Pixels @ 236,116*/ 31, 0x05,
  /* ABS: 019 Pixels @ 267,116*/ 0, 19, 0x15, 0x0A, 0x07, 0x0A, 0x15, 0x00, 0x0A, 0x0B, 0x03, 0x03, 0x12, 0x07, 0x06, 0x0B, 0x12, 0x03, 0x0A, 0x0F, 0x00,
  /* RLE: 005 Pixels @ 286,116*/ 5, 0x03,
  /* ABS: 002 Pixels @ 291,116*/ 0, 2, 0x15, 0x15,
  /* RLE: 004 Pixels @ 293,116*/ 4, 0x03,
  /* ABS: 008 Pixels @ 297,116*/ 0, 8, 0x15, 0x03, 0x03, 0x03, 0x15, 0x03, 0x00, 0x15,
  /* RLE: 008 Pixels @ 305,116*/ 8, 0x05,
  /* RLE: 015 Pixels @ 313,116*/ 15, 0x09,
  /* RLE: 021 Pixels @ 328,116*/ 21, 0x01,
  /* ABS: 002 Pixels @ 349,116*/ 0, 2, 0x04, 0x04,
  /* RLE: 008 Pixels @ 351,116*/ 8, 0x03,
  /* RLE: 001 Pixels @ 359,116*/ 1, 0x04,
  /* RLE: 006 Pixels @ 360,116*/ 6, 0x09,
  /* RLE: 022 Pixels @ 366,116*/ 22, 0x05,
  /* RLE: 033 Pixels @ 000,117*/ 33, 0x02,
  /* ABS: 002 Pixels @ 033,117*/ 0, 2, 0x04, 0x04,
  /* RLE: 006 Pixels @ 035,117*/ 6, 0x03,
  /* ABS: 003 Pixels @ 041,117*/ 0, 3, 0x04, 0x04, 0x0A,
  /* RLE: 028 Pixels @ 044,117*/ 28, 0x00,
  /* ABS: 013 Pixels @ 072,117*/ 0, 13, 0x03, 0x04, 0x07, 0x0E, 0x03, 0x0F, 0x11, 0x03, 0x03, 0x0A, 0x03, 0x00, 0x0A,
  /* RLE: 085 Pixels @ 085,117*/ 85, 0x00,
  /* ABS: 003 Pixels @ 170,117*/ 0, 3, 0x07, 0x00, 0x0A,
  /* RLE: 028 Pixels @ 173,117*/ 28, 0x00,
  /* ABS: 008 Pixels @ 201,117*/ 0, 8, 0x03, 0x00, 0x00, 0x03, 0x03, 0x00, 0x04, 0x03,
  /* RLE: 019 Pixels @ 209,117*/ 19, 0x00,
  /* RLE: 004 Pixels @ 228,117*/ 4, 0x09,
  /* RLE: 035 Pixels @ 232,117*/ 35, 0x05,
  /* ABS: 010 Pixels @ 267,117*/ 0, 10, 0x19, 0x03, 0x03, 0x03, 0x19, 0x19, 0x00, 0x00, 0x15, 0x15,
  /* RLE: 005 Pixels @ 277,117*/ 5, 0x03,
  /* ABS: 005 Pixels @ 282,117*/ 0, 5, 0x00, 0x03, 0x03, 0x03, 0x1A,
  /* RLE: 011 Pixels @ 287,117*/ 11, 0x05,
  /* RLE: 015 Pixels @ 298,117*/ 15, 0x09,
  /* RLE: 036 Pixels @ 313,117*/ 36, 0x01,
  /* RLE: 003 Pixels @ 349,117*/ 3, 0x04,
  /* RLE: 008 Pixels @ 352,117*/ 8, 0x03,
  /* RLE: 003 Pixels @ 360,117*/ 3, 0x04,
  /* RLE: 003 Pixels @ 363,117*/ 3, 0x01,
  /* RLE: 006 Pixels @ 366,117*/ 6, 0x09,
  /* RLE: 016 Pixels @ 372,117*/ 16, 0x05,
  /* RLE: 034 Pixels @ 000,118*/ 34, 0x02,
  /* RLE: 001 Pixels @ 034,118*/ 1, 0x04,
  /* RLE: 006 Pixels @ 035,118*/ 6, 0x03,
  /* RLE: 001 Pixels @ 041,118*/ 1, 0x04,
  /* RLE: 030 Pixels @ 042,118*/ 30, 0x00,
  /* ABS: 012 Pixels @ 072,118*/ 0, 12, 0x03, 0x03, 0x12, 0x07, 0x04, 0x0A, 0x07, 0x0B, 0x03, 0x03, 0x03, 0x04,
  /* RLE: 116 Pixels @ 084,118*/ 116, 0x00,
  /* ABS: 010 Pixels @ 200,118*/ 0, 10, 0x0A, 0x03, 0x04, 0x0E, 0x0D, 0x07, 0x07, 0x07, 0x03, 0x0A,
  /* RLE: 013 Pixels @ 210,118*/ 13, 0x00,
  /* RLE: 005 Pixels @ 223,118*/ 5, 0x09,
  /* RLE: 054 Pixels @ 228,118*/ 54, 0x05,
  /* RLE: 016 Pixels @ 282,118*/ 16, 0x09,
  /* RLE: 005 Pixels @ 298,118*/ 5, 0x00,
  /* ABS: 011 Pixels @ 303,118*/ 0, 11, 0x0D, 0x03, 0x06, 0x00, 0x00, 0x00, 0x06, 0x06, 0x03, 0x06, 0x06,
  /* RLE: 037 Pixels @ 314,118*/ 37, 0x01,
  /* ABS: 002 Pixels @ 351,118*/ 0, 2, 0x04, 0x04,
  /* RLE: 009 Pixels @ 353,118*/ 9, 0x03,
  /* ABS: 002 Pixels @ 362,118*/ 0, 2, 0x04, 0x04,
  /* RLE: 008 Pixels @ 364,118*/ 8, 0x01,
  /* RLE: 006 Pixels @ 372,118*/ 6, 0x09,
  /* RLE: 010 Pixels @ 378,118*/ 10, 0x05,
  /* RLE: 034 Pixels @ 000,119*/ 34, 0x02,
  /* RLE: 001 Pixels @ 034,119*/ 1, 0x04,
  /* RLE: 006 Pixels @ 035,119*/ 6, 0x03,
  /* ABS: 002 Pixels @ 041,119*/ 0, 2, 0x04, 0x0A,
  /* RLE: 030 Pixels @ 043,119*/ 30, 0x00,
  /* ABS: 006 Pixels @ 073,119*/ 0, 6, 0x03, 0x03, 0x11, 0x0F, 0x00, 0x0A,
  /* RLE: 004 Pixels @ 079,119*/ 4, 0x03,
  /* ABS: 002 Pixels @ 083,119*/ 0, 2, 0x04, 0x0A,
  /* RLE: 116 Pixels @ 085,119*/ 116, 0x00,
  /* ABS: 009 Pixels @ 201,119*/ 0, 9, 0x03, 0x12, 0x07, 0x06, 0x0E, 0x0E, 0x0D, 0x00, 0x03,
  /* RLE: 009 Pixels @ 210,119*/ 9, 0x00,
  /* RLE: 004 Pixels @ 219,119*/ 4, 0x09,
  /* RLE: 044 Pixels @ 223,119*/ 44, 0x05,
  /* RLE: 015 Pixels @ 267,119*/ 15, 0x09,
  /* RLE: 021 Pixels @ 282,119*/ 21, 0x00,
  /* ABS: 004 Pixels @ 303,119*/ 0, 4, 0x0D, 0x0D, 0x06, 0x06,
  /* RLE: 004 Pixels @ 307,119*/ 4, 0x00,
  /* ABS: 004 Pixels @ 311,119*/ 0, 4, 0x06, 0x03, 0x06, 0x06,
  /* RLE: 037 Pixels @ 315,119*/ 37, 0x01,
  /* ABS: 002 Pixels @ 352,119*/ 0, 2, 0x04, 0x04,
  /* RLE: 009 Pixels @ 354,119*/ 9, 0x03,
  /* ABS: 002 Pixels @ 363,119*/ 0, 2, 0x04, 0x04,
  /* RLE: 013 Pixels @ 365,119*/ 13, 0x01,
  /* RLE: 006 Pixels @ 378,119*/ 6, 0x09,
  /* RLE: 004 Pixels @ 384,119*/ 4, 0x05,
  /* RLE: 034 Pixels @ 000,120*/ 34, 0x02,
  /* RLE: 001 Pixels @ 034,120*/ 1, 0x04,
  /* RLE: 006 Pixels @ 035,120*/ 6, 0x03,
  /* RLE: 001 Pixels @ 041,120*/ 1, 0x04,
  /* RLE: 032 Pixels @ 042,120*/ 32, 0x00,
  /* ABS: 004 Pixels @ 074,120*/ 0, 4, 0x03, 0x00, 0x07, 0x12,
  /* RLE: 005 Pixels @ 078,120*/ 5, 0x03,
  /* ABS: 003 Pixels @ 083,120*/ 0, 3, 0x04, 0x00, 0x0A,
  /* RLE: 084 Pixels @ 086,120*/ 84, 0x00,
  /* ABS: 004 Pixels @ 170,120*/ 0, 4, 0x0A, 0x00, 0x00, 0x0A,
  /* RLE: 026 Pixels @ 174,120*/ 26, 0x00,
  /* RLE: 001 Pixels @ 200,120*/ 1, 0x0A,
  /* RLE: 006 Pixels @ 201,120*/ 6, 0x03,
  /* ABS: 003 Pixels @ 207,120*/ 0, 3, 0x06, 0x0E, 0x03,
  /* RLE: 005 Pixels @ 210,120*/ 5, 0x00,
  /* RLE: 004 Pixels @ 215,120*/ 4, 0x09,
  /* RLE: 038 Pixels @ 219,120*/ 38, 0x05,
  /* RLE: 010 Pixels @ 257,120*/ 10, 0x09,
  /* RLE: 037 Pixels @ 267,120*/ 37, 0x00,
  /* RLE: 003 Pixels @ 304,120*/ 3, 0x06,
  /* RLE: 005 Pixels @ 307,120*/ 5, 0x00,
  /* RLE: 004 Pixels @ 312,120*/ 4, 0x06,
  /* RLE: 037 Pixels @ 316,120*/ 37, 0x01,
  /* ABS: 002 Pixels @ 353,120*/ 0, 2, 0x04, 0x04,
  /* RLE: 009 Pixels @ 355,120*/ 9, 0x03,
  /* ABS: 002 Pixels @ 364,120*/ 0, 2, 0x04, 0x04,
  /* RLE: 018 Pixels @ 366,120*/ 18, 0x01,
  /* RLE: 004 Pixels @ 384,120*/ 4, 0x09,
  /* RLE: 034 Pixels @ 000,121*/ 34, 0x02,
  /* RLE: 001 Pixels @ 034,121*/ 1, 0x04,
  /* RLE: 006 Pixels @ 035,121*/ 6, 0x03,
  /* ABS: 003 Pixels @ 041,121*/ 0, 3, 0x04, 0x00, 0x0A,
  /* RLE: 031 Pixels @ 044,121*/ 31, 0x00,
  /* ABS: 002 Pixels @ 075,121*/ 0, 2, 0x03, 0x0A,
  /* RLE: 006 Pixels @ 077,121*/ 6, 0x03,
  /* ABS: 002 Pixels @ 083,121*/ 0, 2, 0x04, 0x04,
  /* RLE: 086 Pixels @ 085,121*/ 86, 0x00,
  /* ABS: 004 Pixels @ 171,121*/ 0, 4, 0x0A, 0x07, 0x00, 0x0A,
  /* RLE: 026 Pixels @ 175,121*/ 26, 0x00,
  /* ABS: 010 Pixels @ 201,121*/ 0, 10, 0x0A, 0x03, 0x03, 0x00, 0x0B, 0x0E, 0x0F, 0x0E, 0x03, 0x04,
  /* RLE: 004 Pixels @ 211,121*/ 4, 0x09,
  /* RLE: 038 Pixels @ 215,121*/ 38, 0x05,
  /* RLE: 004 Pixels @ 253,121*/ 4, 0x09,
  /* RLE: 047 Pixels @ 257,121*/ 47, 0x00,
  /* ABS: 003 Pixels @ 304,121*/ 0, 3, 0x0D, 0x06, 0x06,
  /* RLE: 006 Pixels @ 307,121*/ 6, 0x00,
  /* RLE: 005 Pixels @ 313,121*/ 5, 0x06,
  /* RLE: 036 Pixels @ 318,121*/ 36, 0x01,
  /* ABS: 002 Pixels @ 354,121*/ 0, 2, 0x04, 0x04,
  /* RLE: 009 Pixels @ 356,121*/ 9, 0x03,
  /* ABS: 002 Pixels @ 365,121*/ 0, 2, 0x04, 0x04,
  /* RLE: 021 Pixels @ 367,121*/ 21, 0x01,
  /* RLE: 034 Pixels @ 000,122*/ 34, 0x02,
  /* RLE: 001 Pixels @ 034,122*/ 1, 0x04,
  /* RLE: 006 Pixels @ 035,122*/ 6, 0x03,
  /* ABS: 002 Pixels @ 041,122*/ 0, 2, 0x04, 0x0A,
  /* RLE: 034 Pixels @ 043,122*/ 34, 0x00,
  /* RLE: 007 Pixels @ 077,122*/ 7, 0x03,
  /* ABS: 003 Pixels @ 084,122*/ 0, 3, 0x04, 0x04, 0x0A,
  /* RLE: 083 Pixels @ 087,122*/ 83, 0x00,
  /* ABS: 033 Pixels @ 170,122*/ 0, 33, 0x0A, 0x00, 0x07, 0x00, 0x00, 0x0A, 0x00, 0x0A, 0x00, 0x0A, 0x00, 0x0A, 0x00, 0x0A, 0x00, 0x0A, 0x00, 0x0A, 0x00, 0x0A, 0x00, 0x0A, 0x00, 0x0A, 0x00, 0x0A, 0x00, 0x0A, 0x00, 0x0A, 0x00, 0x00, 0x03,
  /* RLE: 005 Pixels @ 203,122*/ 5, 0x07,
  /* ABS: 003 Pixels @ 208,122*/ 0, 3, 0x0A, 0x03, 0x06,
  /* RLE: 038 Pixels @ 211,122*/ 38, 0x05,
  /* RLE: 004 Pixels @ 249,122*/ 4, 0x09,
  /* RLE: 051 Pixels @ 253,122*/ 51, 0x00,
  /* ABS: 003 Pixels @ 304,122*/ 0, 3, 0x06, 0x03, 0x06,
  /* RLE: 007 Pixels @ 307,122*/ 7, 0x00,
  /* ABS: 005 Pixels @ 314,122*/ 0, 5, 0x06, 0x06, 0x03, 0x06, 0x06,
  /* RLE: 036 Pixels @ 319,122*/ 36, 0x01,
  /* ABS: 002 Pixels @ 355,122*/ 0, 2, 0x04, 0x04,
  /* RLE: 009 Pixels @ 357,122*/ 9, 0x03,
  /* ABS: 002 Pixels @ 366,122*/ 0, 2, 0x04, 0x04,
  /* RLE: 020 Pixels @ 368,122*/ 20, 0x01,
  /* RLE: 035 Pixels @ 000,123*/ 35, 0x02,
  /* RLE: 007 Pixels @ 035,123*/ 7, 0x03,
  /* RLE: 001 Pixels @ 042,123*/ 1, 0x04,
  /* RLE: 034 Pixels @ 043,123*/ 34, 0x00,
  /* RLE: 001 Pixels @ 077,123*/ 1, 0x04,
  /* RLE: 007 Pixels @ 078,123*/ 7, 0x03,
  /* ABS: 003 Pixels @ 085,123*/ 0, 3, 0x04, 0x04, 0x0A,
  /* RLE: 085 Pixels @ 088,123*/ 85, 0x00,
  /* ABS: 038 Pixels @ 173,123*/ 0, 38, 0x07, 0x00, 0x00, 0x0A, 0x00, 0x0A, 0x00, 0x0A, 0x00, 0x0A, 0x00, 0x0A, 0x00, 0x0A, 0x00, 0x0A, 0x00, 0x0A, 0x00, 0x0A, 0x00, 0x0A, 0x00, 0x0A, 0x00, 0x0A, 0x00, 0x0A, 0x00, 0x03, 0x0E, 0x04, 0x00, 0x03, 0x00, 0x07, 0x00, 0x15,
  /* RLE: 034 Pixels @ 211,123*/ 34, 0x05,
  /* RLE: 004 Pixels @ 245,123*/ 4, 0x09,
  /* RLE: 055 Pixels @ 249,123*/ 55, 0x00,
  /* ABS: 003 Pixels @ 304,123*/ 0, 3, 0x0D, 0x03, 0x06,
  /* RLE: 008 Pixels @ 307,123*/ 8, 0x00,
  /* ABS: 005 Pixels @ 315,123*/ 0, 5, 0x06, 0x06, 0x03, 0x06, 0x06,
  /* RLE: 037 Pixels @ 320,123*/ 37, 0x01,
  /* ABS: 002 Pixels @ 357,123*/ 0, 2, 0x04, 0x04,
  /* RLE: 009 Pixels @ 359,123*/ 9, 0x03,
  /* ABS: 002 Pixels @ 368,123*/ 0, 2, 0x04, 0x04,
  /* RLE: 018 Pixels @ 370,123*/ 18, 0x01,
  /* RLE: 035 Pixels @ 000,124*/ 35, 0x02,
  /* RLE: 001 Pixels @ 035,124*/ 1, 0x04,
  /* RLE: 006 Pixels @ 036,124*/ 6, 0x03,
  /* RLE: 001 Pixels @ 042,124*/ 1, 0x04,
  /* RLE: 033 Pixels @ 043,124*/ 33, 0x00,
  /* ABS: 002 Pixels @ 076,124*/ 0, 2, 0x0A, 0x04,
  /* RLE: 008 Pixels @ 078,124*/ 8, 0x03,
  /* ABS: 003 Pixels @ 086,124*/ 0, 3, 0x04, 0x00, 0x0A,
  /* RLE: 086 Pixels @ 089,124*/ 86, 0x00,
  /* RLE: 001 Pixels @ 175,124*/ 1, 0x0A,
  /* RLE: 023 Pixels @ 176,124*/ 23, 0x00,
  /* RLE: 003 Pixels @ 199,124*/ 3, 0x09,
  /* RLE: 001 Pixels @ 202,124*/ 1, 0x00,
  /* RLE: 004 Pixels @ 203,124*/ 4, 0x03,
  /* ABS: 004 Pixels @ 207,124*/ 0, 4, 0x0A, 0x07, 0x00, 0x15,
  /* RLE: 029 Pixels @ 211,124*/ 29, 0x05,
  /* RLE: 005 Pixels @ 240,124*/ 5, 0x09,
  /* RLE: 059 Pixels @ 245,124*/ 59, 0x00,
  /* ABS: 003 Pixels @ 304,124*/ 0, 3, 0x06, 0x03, 0x06,
  /* RLE: 010 Pixels @ 307,124*/ 10, 0x00,
  /* ABS: 004 Pixels @ 317,124*/ 0, 4, 0x06, 0x03, 0x06, 0x06,
  /* RLE: 037 Pixels @ 321,124*/ 37, 0x01,
  /* ABS: 002 Pixels @ 358,124*/ 0, 2, 0x04, 0x04,
  /* RLE: 009 Pixels @ 360,124*/ 9, 0x03,
  /* ABS: 002 Pixels @ 369,124*/ 0, 2, 0x04, 0x04,
  /* RLE: 017 Pixels @ 371,124*/ 17, 0x01,
  /* RLE: 035 Pixels @ 000,125*/ 35, 0x02,
  /* RLE: 001 Pixels @ 035,125*/ 1, 0x04,
  /* RLE: 006 Pixels @ 036,125*/ 6, 0x03,
  /* ABS: 003 Pixels @ 042,125*/ 0, 3, 0x04, 0x00, 0x0A,
  /* RLE: 032 Pixels @ 045,125*/ 32, 0x00,
  /* ABS: 002 Pixels @ 077,125*/ 0, 2, 0x0A, 0x04,
  /* RLE: 008 Pixels @ 079,125*/ 8, 0x03,
  /* ABS: 003 Pixels @ 087,125*/ 0, 3, 0x04, 0x00, 0x0A,
  /* RLE: 084 Pixels @ 090,125*/ 84, 0x00,
  /* RLE: 001 Pixels @ 174,125*/ 1, 0x0A,
  /* RLE: 020 Pixels @ 175,125*/ 20, 0x00,
  /* RLE: 004 Pixels @ 195,125*/ 4, 0x09,
  /* RLE: 003 Pixels @ 199,125*/ 3, 0x05,
  /* ABS: 009 Pixels @ 202,125*/ 0, 9, 0x15, 0x00, 0x0E, 0x0D, 0x07, 0x07, 0x0F, 0x00, 0x15,
  /* RLE: 025 Pixels @ 211,125*/ 25, 0x05,
  /* RLE: 004 Pixels @ 236,125*/ 4, 0x09,
  /* RLE: 064 Pixels @ 240,125*/ 64, 0x00,
  /* ABS: 004 Pixels @ 304,125*/ 0, 4, 0x0D, 0x06, 0x06, 0x06,
  /* RLE: 010 Pixels @ 308,125*/ 10, 0x00,
  /* RLE: 004 Pixels @ 318,125*/ 4, 0x06,
  /* RLE: 037 Pixels @ 322,125*/ 37, 0x01,
  /* ABS: 002 Pixels @ 359,125*/ 0, 2, 0x04, 0x04,
  /* RLE: 009 Pixels @ 361,125*/ 9, 0x03,
  /* ABS: 002 Pixels @ 370,125*/ 0, 2, 0x04, 0x04,
  /* RLE: 016 Pixels @ 372,125*/ 16, 0x01,
  /* RLE: 035 Pixels @ 000,126*/ 35, 0x02,
  /* RLE: 001 Pixels @ 035,126*/ 1, 0x04,
  /* RLE: 006 Pixels @ 036,126*/ 6, 0x03,
  /* ABS: 002 Pixels @ 042,126*/ 0, 2, 0x04, 0x0A,
  /* RLE: 033 Pixels @ 044,126*/ 33, 0x00,
  /* ABS: 003 Pixels @ 077,126*/ 0, 3, 0x0A, 0x04, 0x04,
  /* RLE: 007 Pixels @ 080,126*/ 7, 0x03,
  /* ABS: 005 Pixels @ 087,126*/ 0, 5, 0x04, 0x04, 0x00, 0x00, 0x0A,
  /* RLE: 048 Pixels @ 092,126*/ 48, 0x00,
  /* ABS: 033 Pixels @ 140,126*/ 0, 33, 0x0A, 0x00, 0x00, 0x00, 0x0A, 0x00, 0x00, 0x00, 0x0A, 0x00, 0x00, 0x00, 0x0A, 0x00, 0x00, 0x00, 0x0A, 0x00, 0x00, 0x00, 0x0A, 0x00, 0x00, 0x00, 0x0A, 0x00, 0x00, 0x00, 0x0A, 0x00, 0x00, 0x00, 0x0A,
  /* RLE: 019 Pixels @ 173,126*/ 19, 0x00,
  /* RLE: 003 Pixels @ 192,126*/ 3, 0x09,
  /* RLE: 007 Pixels @ 195,126*/ 7, 0x05,
  /* ABS: 009 Pixels @ 202,126*/ 0, 9, 0x15, 0x0A, 0x0F, 0x06, 0x0B, 0x04, 0x03, 0x03, 0x1A,
  /* RLE: 021 Pixels @ 211,126*/ 21, 0x05,
  /* RLE: 004 Pixels @ 232,126*/ 4, 0x09,
  /* RLE: 069 Pixels @ 236,126*/ 69, 0x00,
  /* RLE: 003 Pixels @ 305,126*/ 3, 0x06,
  /* RLE: 011 Pixels @ 308,126*/ 11, 0x00,
  /* RLE: 004 Pixels @ 319,126*/ 4, 0x06,
  /* RLE: 037 Pixels @ 323,126*/ 37, 0x01,
  /* ABS: 002 Pixels @ 360,126*/ 0, 2, 0x04, 0x04,
  /* RLE: 009 Pixels @ 362,126*/ 9, 0x03,
  /* ABS: 002 Pixels @ 371,126*/ 0, 2, 0x04, 0x04,
  /* RLE: 015 Pixels @ 373,126*/ 15, 0x01,
  /* RLE: 035 Pixels @ 000,127*/ 35, 0x02,
  /* RLE: 001 Pixels @ 035,127*/ 1, 0x04,
  /* RLE: 006 Pixels @ 036,127*/ 6, 0x03,
  /* ABS: 003 Pixels @ 042,127*/ 0, 3, 0x04, 0x00, 0x0A,
  /* RLE: 034 Pixels @ 045,127*/ 34, 0x00,
  /* ABS: 002 Pixels @ 079,127*/ 0, 2, 0x04, 0x04,
  /* RLE: 007 Pixels @ 081,127*/ 7, 0x03,
  /* ABS: 003 Pixels @ 088,127*/ 0, 3, 0x04, 0x04, 0x0A,
  /* RLE: 048 Pixels @ 091,127*/ 48, 0x00,
  /* ABS: 037 Pixels @ 139,127*/ 0, 37, 0x0A, 0x00, 0x0A, 0x00, 0x0A, 0x00, 0x0A, 0x00, 0x0A, 0x00, 0x0A, 0x00, 0x0A, 0x00, 0x0A, 0x00, 0x0A, 0x00, 0x0A, 0x00, 0x0A, 0x00, 0x0A, 0x00, 0x0A, 0x00, 0x0A, 0x00, 0x0A, 0x00, 0x0A, 0x00, 0x0A, 0x00, 0x0A, 0x00, 0x07,
  /* RLE: 012 Pixels @ 176,127*/ 12, 0x00,
  /* RLE: 004 Pixels @ 188,127*/ 4, 0x09,
  /* RLE: 010 Pixels @ 192,127*/ 10, 0x05,
  /* RLE: 001 Pixels @ 202,127*/ 1, 0x19,
  /* RLE: 004 Pixels @ 203,127*/ 4, 0x03,
  /* ABS: 002 Pixels @ 207,127*/ 0, 2, 0x00, 0x19,
  /* RLE: 019 Pixels @ 209,127*/ 19, 0x05,
  /* RLE: 004 Pixels @ 228,127*/ 4, 0x09,
  /* RLE: 073 Pixels @ 232,127*/ 73, 0x00,
  /* ABS: 003 Pixels @ 305,127*/ 0, 3, 0x0D, 0x06, 0x06,
  /* RLE: 012 Pixels @ 308,127*/ 12, 0x00,
  /* ABS: 005 Pixels @ 320,127*/ 0, 5, 0x06, 0x06, 0x03, 0x06, 0x06,
  /* RLE: 036 Pixels @ 325,127*/ 36, 0x01,
  /* ABS: 002 Pixels @ 361,127*/ 0, 2, 0x04, 0x04,
  /* RLE: 009 Pixels @ 363,127*/ 9, 0x03,
  /* ABS: 002 Pixels @ 372,127*/ 0, 2, 0x04, 0x04,
  /* RLE: 014 Pixels @ 374,127*/ 14, 0x01,
  /* RLE: 035 Pixels @ 000,128*/ 35, 0x02,
  /* RLE: 001 Pixels @ 035,128*/ 1, 0x04,
  /* RLE: 006 Pixels @ 036,128*/ 6, 0x03,
  /* ABS: 002 Pixels @ 042,128*/ 0, 2, 0x04, 0x0A,
  /* RLE: 036 Pixels @ 044,128*/ 36, 0x00,
  /* ABS: 002 Pixels @ 080,128*/ 0, 2, 0x04, 0x04,
  /* RLE: 007 Pixels @ 082,128*/ 7, 0x03,
  /* ABS: 002 Pixels @ 089,128*/ 0, 2, 0x04, 0x04,
  /* RLE: 043 Pixels @ 091,128*/ 43, 0x00,
  /* ABS: 005 Pixels @ 134,128*/ 0, 5, 0x0A, 0x00, 0x00, 0x00, 0x0A,
  /* RLE: 036 Pixels @ 139,128*/ 36, 0x00,
  /* RLE: 001 Pixels @ 175,128*/ 1, 0x07,
  /* RLE: 008 Pixels @ 176,128*/ 8, 0x00,
  /* RLE: 004 Pixels @ 184,128*/ 4, 0x09,
  /* RLE: 021 Pixels @ 188,128*/ 21, 0x05,
  /* ABS: 006 Pixels @ 209,128*/ 0, 6, 0x19, 0x15, 0x03, 0x03, 0x00, 0x1A,
  /* RLE: 008 Pixels @ 215,128*/ 8, 0x05,
  /* RLE: 005 Pixels @ 223,128*/ 5, 0x09,
  /* RLE: 077 Pixels @ 228,128*/ 77, 0x00,
  /* ABS: 003 Pixels @ 305,128*/ 0, 3, 0x06, 0x03, 0x06,
  /* RLE: 013 Pixels @ 308,128*/ 13, 0x00,
  /* ABS: 005 Pixels @ 321,128*/ 0, 5, 0x06, 0x06, 0x03, 0x06, 0x06,
  /* RLE: 036 Pixels @ 326,128*/ 36, 0x01,
  /* RLE: 003 Pixels @ 362,128*/ 3, 0x04,
  /* RLE: 008 Pixels @ 365,128*/ 8, 0x03,
  /* RLE: 003 Pixels @ 373,128*/ 3, 0x04,
  /* RLE: 012 Pixels @ 376,128*/ 12, 0x01,
  /* RLE: 036 Pixels @ 000,129*/ 36, 0x02,
  /* RLE: 007 Pixels @ 036,129*/ 7, 0x03,
  /* RLE: 001 Pixels @ 043,129*/ 1, 0x04,
  /* RLE: 037 Pixels @ 044,129*/ 37, 0x00,
  /* RLE: 001 Pixels @ 081,129*/ 1, 0x04,
  /* RLE: 008 Pixels @ 082,129*/ 8, 0x03,
  /* ABS: 004 Pixels @ 090,129*/ 0, 4, 0x04, 0x00, 0x00, 0x0A,
  /* RLE: 041 Pixels @ 094,129*/ 41, 0x00,
  /* ABS: 003 Pixels @ 135,129*/ 0, 3, 0x0A, 0x00, 0x0A,
  /* RLE: 037 Pixels @ 138,129*/ 37, 0x00,
  /* RLE: 001 Pixels @ 175,129*/ 1, 0x07,
  /* RLE: 004 Pixels @ 176,129*/ 4, 0x00,
  /* RLE: 004 Pixels @ 180,129*/ 4, 0x09,
  /* RLE: 020 Pixels @ 184,129*/ 20, 0x05,
  /* ABS: 002 Pixels @ 204,129*/ 0, 2, 0x1A, 0x15,
  /* RLE: 004 Pixels @ 206,129*/ 4, 0x03,
  /* ABS: 005 Pixels @ 210,129*/ 0, 5, 0x00, 0x0B, 0x12, 0x00, 0x19,
  /* RLE: 004 Pixels @ 215,129*/ 4, 0x05,
  /* RLE: 004 Pixels @ 219,129*/ 4, 0x09,
  /* RLE: 082 Pixels @ 223,129*/ 82, 0x00,
  /* ABS: 003 Pixels @ 305,129*/ 0, 3, 0x0D, 0x03, 0x06,
  /* RLE: 014 Pixels @ 308,129*/ 14, 0x00,
  /* ABS: 005 Pixels @ 322,129*/ 0, 5, 0x06, 0x06, 0x03, 0x06, 0x06,
  /* RLE: 037 Pixels @ 327,129*/ 37, 0x01,
  /* ABS: 002 Pixels @ 364,129*/ 0, 2, 0x04, 0x04,
  /* RLE: 009 Pixels @ 366,129*/ 9, 0x03,
  /* ABS: 002 Pixels @ 375,129*/ 0, 2, 0x04, 0x04,
  /* RLE: 011 Pixels @ 377,129*/ 11, 0x01,
  /* RLE: 036 Pixels @ 000,130*/ 36, 0x02,
  /* RLE: 001 Pixels @ 036,130*/ 1, 0x04,
  /* RLE: 006 Pixels @ 037,130*/ 6, 0x03,
  /* RLE: 001 Pixels @ 043,130*/ 1, 0x04,
  /* RLE: 036 Pixels @ 044,130*/ 36, 0x00,
  /* ABS: 003 Pixels @ 080,130*/ 0, 3, 0x0A, 0x04, 0x04,
  /* RLE: 007 Pixels @ 083,130*/ 7, 0x03,
  /* ABS: 003 Pixels @ 090,130*/ 0, 3, 0x04, 0x04, 0x0A,
  /* RLE: 039 Pixels @ 093,130*/ 39, 0x00,
  /* ABS: 003 Pixels @ 132,130*/ 0, 3, 0x0A, 0x00, 0x0A,
  /* RLE: 042 Pixels @ 135,130*/ 42, 0x00,
  /* RLE: 003 Pixels @ 177,130*/ 3, 0x09,
  /* RLE: 024 Pixels @ 180,130*/ 24, 0x05,
  /* ABS: 005 Pixels @ 204,130*/ 0, 5, 0x00, 0x00, 0x0B, 0x12, 0x11,
  /* RLE: 004 Pixels @ 209,130*/ 4, 0x07,
  /* ABS: 002 Pixels @ 213,130*/ 0, 2, 0x00, 0x19,
  /* RLE: 004 Pixels @ 215,130*/ 4, 0x09,
  /* RLE: 086 Pixels @ 219,130*/ 86, 0x00,
  /* ABS: 004 Pixels @ 305,130*/ 0, 4, 0x0D, 0x03, 0x06, 0x06,
  /* RLE: 015 Pixels @ 309,130*/ 15, 0x00,
  /* RLE: 004 Pixels @ 324,130*/ 4, 0x06,
  /* RLE: 037 Pixels @ 328,130*/ 37, 0x01,
  /* ABS: 002 Pixels @ 365,130*/ 0, 2, 0x04, 0x04,
  /* RLE: 009 Pixels @ 367,130*/ 9, 0x03,
  /* ABS: 002 Pixels @ 376,130*/ 0, 2, 0x04, 0x04,
  /* RLE: 010 Pixels @ 378,130*/ 10, 0x01,
  /* RLE: 001 Pixels @ 000,131*/ 1, 0x09,
  /* RLE: 035 Pixels @ 001,131*/ 35, 0x02,
  /* RLE: 001 Pixels @ 036,131*/ 1, 0x04,
  /* RLE: 006 Pixels @ 037,131*/ 6, 0x03,
  /* ABS: 003 Pixels @ 043,131*/ 0, 3, 0x04, 0x00, 0x0A,
  /* RLE: 036 Pixels @ 046,131*/ 36, 0x00,
  /* ABS: 002 Pixels @ 082,131*/ 0, 2, 0x0A, 0x04,
  /* RLE: 007 Pixels @ 084,131*/ 7, 0x03,
  /* ABS: 003 Pixels @ 091,131*/ 0, 3, 0x04, 0x04, 0x0A,
  /* RLE: 041 Pixels @ 094,131*/ 41, 0x00,
  /* RLE: 001 Pixels @ 135,131*/ 1, 0x0A,
  /* RLE: 037 Pixels @ 136,131*/ 37, 0x00,
  /* RLE: 004 Pixels @ 173,131*/ 4, 0x09,
  /* RLE: 027 Pixels @ 177,131*/ 27, 0x05,
  /* ABS: 011 Pixels @ 204,131*/ 0, 11, 0x03, 0x12, 0x07, 0x07, 0x06, 0x07, 0x0B, 0x00, 0x03, 0x03, 0x12,
  /* RLE: 091 Pixels @ 215,131*/ 91, 0x00,
  /* ABS: 003 Pixels @ 306,131*/ 0, 3, 0x0D, 0x06, 0x06,
  /* RLE: 016 Pixels @ 309,131*/ 16, 0x00,
  /* RLE: 004 Pixels @ 325,131*/ 4, 0x06,
  /* RLE: 037 Pixels @ 329,131*/ 37, 0x01,
  /* ABS: 002 Pixels @ 366,131*/ 0, 2, 0x04, 0x04,
  /* RLE: 009 Pixels @ 368,131*/ 9, 0x03,
  /* ABS: 002 Pixels @ 377,131*/ 0, 2, 0x04, 0x04,
  /* RLE: 009 Pixels @ 379,131*/ 9, 0x01,
  /* RLE: 003 Pixels @ 000,132*/ 3, 0x09,
  /* RLE: 033 Pixels @ 003,132*/ 33, 0x02,
  /* RLE: 001 Pixels @ 036,132*/ 1, 0x04,
  /* RLE: 006 Pixels @ 037,132*/ 6, 0x03,
  /* ABS: 002 Pixels @ 043,132*/ 0, 2, 0x04, 0x0A,
  /* RLE: 037 Pixels @ 045,132*/ 37, 0x00,
  /* ABS: 003 Pixels @ 082,132*/ 0, 3, 0x0A, 0x04, 0x04,
  /* RLE: 007 Pixels @ 085,132*/ 7, 0x03,
  /* ABS: 002 Pixels @ 092,132*/ 0, 2, 0x04, 0x04,
  /* RLE: 075 Pixels @ 094,132*/ 75, 0x00,
  /* RLE: 004 Pixels @ 169,132*/ 4, 0x09,
  /* RLE: 031 Pixels @ 173,132*/ 31, 0x05,
  /* RLE: 003 Pixels @ 204,132*/ 3, 0x00,
  /* ABS: 007 Pixels @ 207,132*/ 0, 7, 0x03, 0x03, 0x0F, 0x0B, 0x03, 0x03, 0x04,
  /* RLE: 092 Pixels @ 214,132*/ 92, 0x00,
  /* RLE: 003 Pixels @ 306,132*/ 3, 0x06,
  /* RLE: 017 Pixels @ 309,132*/ 17, 0x00,
  /* ABS: 005 Pixels @ 326,132*/ 0, 5, 0x06, 0x06, 0x03, 0x06, 0x06,
  /* RLE: 036 Pixels @ 331,132*/ 36, 0x01,
  /* ABS: 002 Pixels @ 367,132*/ 0, 2, 0x04, 0x04,
  /* RLE: 009 Pixels @ 369,132*/ 9, 0x03,
  /* ABS: 002 Pixels @ 378,132*/ 0, 2, 0x04, 0x04,
  /* RLE: 008 Pixels @ 380,132*/ 8, 0x01,
  /* ABS: 005 Pixels @ 000,133*/ 0, 5, 0x05, 0x05, 0x09, 0x09, 0x09,
  /* RLE: 031 Pixels @ 005,133*/ 31, 0x02,
  /* RLE: 001 Pixels @ 036,133*/ 1, 0x04,
  /* RLE: 006 Pixels @ 037,133*/ 6, 0x03,
  /* ABS: 003 Pixels @ 043,133*/ 0, 3, 0x04, 0x00, 0x0A,
  /* RLE: 038 Pixels @ 046,133*/ 38, 0x00,
  /* RLE: 001 Pixels @ 084,133*/ 1, 0x0A,
  /* RLE: 008 Pixels @ 085,133*/ 8, 0x03,
  /* ABS: 002 Pixels @ 093,133*/ 0, 2, 0x04, 0x0A,
  /* RLE: 070 Pixels @ 095,133*/ 70, 0x00,
  /* RLE: 004 Pixels @ 165,133*/ 4, 0x09,
  /* RLE: 034 Pixels @ 169,133*/ 34, 0x05,
  /* ABS: 008 Pixels @ 203,133*/ 0, 8, 0x09, 0x12, 0x04, 0x04, 0x03, 0x03, 0x06, 0x12,
  /* RLE: 004 Pixels @ 211,133*/ 4, 0x03,
  /* RLE: 091 Pixels @ 215,133*/ 91, 0x00,
  /* ABS: 004 Pixels @ 306,133*/ 0, 4, 0x0D, 0x06, 0x06, 0x06,
  /* RLE: 017 Pixels @ 310,133*/ 17, 0x00,
  /* ABS: 005 Pixels @ 327,133*/ 0, 5, 0x06, 0x06, 0x03, 0x06, 0x06,
  /* RLE: 036 Pixels @ 332,133*/ 36, 0x01,
  /* ABS: 002 Pixels @ 368,133*/ 0, 2, 0x04, 0x04,
  /* RLE: 009 Pixels @ 370,133*/ 9, 0x03,
  /* ABS: 002 Pixels @ 379,133*/ 0, 2, 0x04, 0x04,
  /* RLE: 007 Pixels @ 381,133*/ 7, 0x01,
  /* ABS: 007 Pixels @ 000,134*/ 0, 7, 0x00, 0x00, 0x1A, 0x05, 0x09, 0x09, 0x09,
  /* RLE: 029 Pixels @ 007,134*/ 29, 0x02,
  /* RLE: 001 Pixels @ 036,134*/ 1, 0x04,
  /* RLE: 006 Pixels @ 037,134*/ 6, 0x03,
  /* ABS: 002 Pixels @ 043,134*/ 0, 2, 0x04, 0x0A,
  /* RLE: 039 Pixels @ 045,134*/ 39, 0x00,
  /* ABS: 002 Pixels @ 084,134*/ 0, 2, 0x0A, 0x04,
  /* RLE: 008 Pixels @ 086,134*/ 8, 0x03,
  /* ABS: 003 Pixels @ 094,134*/ 0, 3, 0x04, 0x00, 0x0A,
  /* RLE: 037 Pixels @ 097,134*/ 37, 0x00,
  /* ABS: 004 Pixels @ 134,134*/ 0, 4, 0x0A, 0x00, 0x00, 0x00,
  /* RLE: 022 Pixels @ 138,134*/ 22, 0x07,
  /* RLE: 001 Pixels @ 160,134*/ 1, 0x00,
  /* RLE: 004 Pixels @ 161,134*/ 4, 0x09,
  /* RLE: 034 Pixels @ 165,134*/ 34, 0x05,
  /* RLE: 004 Pixels @ 199,134*/ 4, 0x09,
  /* RLE: 003 Pixels @ 203,134*/ 3, 0x00,
  /* RLE: 003 Pixels @ 206,134*/ 3, 0x03,
  /* ABS: 005 Pixels @ 209,134*/ 0, 5, 0x0E, 0x11, 0x0E, 0x06, 0x07,
  /* RLE: 093 Pixels @ 214,134*/ 93, 0x00,
  /* ABS: 004 Pixels @ 307,134*/ 0, 4, 0x0D, 0x0D, 0x06, 0x06,
  /* RLE: 017 Pixels @ 311,134*/ 17, 0x00,
  /* ABS: 005 Pixels @ 328,134*/ 0, 5, 0x06, 0x06, 0x03, 0x06, 0x06,
  /* RLE: 037 Pixels @ 333,134*/ 37, 0x01,
  /* ABS: 002 Pixels @ 370,134*/ 0, 2, 0x04, 0x04,
  /* RLE: 009 Pixels @ 372,134*/ 9, 0x03,
  /* ABS: 002 Pixels @ 381,134*/ 0, 2, 0x04, 0x04,
  /* RLE: 005 Pixels @ 383,134*/ 5, 0x01,
  /* ABS: 009 Pixels @ 000,135*/ 0, 9, 0x12, 0x00, 0x03, 0x03, 0x19, 0x05, 0x09, 0x09, 0x09,
  /* RLE: 028 Pixels @ 009,135*/ 28, 0x02,
  /* RLE: 007 Pixels @ 037,135*/ 7, 0x03,
  /* ABS: 004 Pixels @ 044,135*/ 0, 4, 0x04, 0x0A, 0x00, 0x0A,
  /* RLE: 037 Pixels @ 048,135*/ 37, 0x00,
  /* ABS: 002 Pixels @ 085,135*/ 0, 2, 0x0A, 0x04,
  /* RLE: 007 Pixels @ 087,135*/ 7, 0x03,
  /* ABS: 002 Pixels @ 094,135*/ 0, 2, 0x04, 0x04,
  /* RLE: 039 Pixels @ 096,135*/ 39, 0x00,
  /* ABS: 003 Pixels @ 135,135*/ 0, 3, 0x0A, 0x00, 0x07,
  /* RLE: 022 Pixels @ 138,135*/ 22, 0x10,
  /* RLE: 001 Pixels @ 160,135*/ 1, 0x07,
  /* RLE: 034 Pixels @ 161,135*/ 34, 0x05,
  /* RLE: 004 Pixels @ 195,135*/ 4, 0x09,
  /* RLE: 006 Pixels @ 199,135*/ 6, 0x00,
  /* ABS: 003 Pixels @ 205,135*/ 0, 3, 0x03, 0x0B, 0x06,
  /* RLE: 004 Pixels @ 208,135*/ 4, 0x07,
  /* ABS: 004 Pixels @ 212,135*/ 0, 4, 0x11, 0x12, 0x00, 0x03,
  /* RLE: 092 Pixels @ 216,135*/ 92, 0x00,
  /* ABS: 004 Pixels @ 308,135*/ 0, 4, 0x0D, 0x03, 0x06, 0x06,
  /* RLE: 018 Pixels @ 312,135*/ 18, 0x00,
  /* RLE: 004 Pixels @ 330,135*/ 4, 0x06,
  /* RLE: 037 Pixels @ 334,135*/ 37, 0x01,
  /* ABS: 002 Pixels @ 371,135*/ 0, 2, 0x04, 0x04,
  /* RLE: 009 Pixels @ 373,135*/ 9, 0x03,
  /* ABS: 002 Pixels @ 382,135*/ 0, 2, 0x04, 0x04,
  /* RLE: 004 Pixels @ 384,135*/ 4, 0x01,
  /* ABS: 011 Pixels @ 000,136*/ 0, 11, 0x07, 0x06, 0x06, 0x0A, 0x03, 0x19, 0x05, 0x05, 0x09, 0x09, 0x09,
  /* RLE: 026 Pixels @ 011,136*/ 26, 0x02,
  /* RLE: 001 Pixels @ 037,136*/ 1, 0x04,
  /* RLE: 006 Pixels @ 038,136*/ 6, 0x03,
  /* RLE: 001 Pixels @ 044,136*/ 1, 0x04,
  /* RLE: 040 Pixels @ 045,136*/ 40, 0x00,
  /* ABS: 003 Pixels @ 085,136*/ 0, 3, 0x0A, 0x04, 0x04,
  /* RLE: 007 Pixels @ 088,136*/ 7, 0x03,
  /* ABS: 003 Pixels @ 095,136*/ 0, 3, 0x04, 0x04, 0x0A,
  /* RLE: 036 Pixels @ 098,136*/ 36, 0x00,
  /* ABS: 005 Pixels @ 134,136*/ 0, 5, 0x0A, 0x00, 0x07, 0x10, 0x10,
  /* RLE: 020 Pixels @ 139,136*/ 20, 0x07,
  /* ABS: 003 Pixels @ 159,136*/ 0, 3, 0x10, 0x10, 0x07,
  /* RLE: 030 Pixels @ 162,136*/ 30, 0x05,
  /* RLE: 003 Pixels @ 192,136*/ 3, 0x09,
  /* RLE: 010 Pixels @ 195,136*/ 10, 0x00,
  /* ABS: 006 Pixels @ 205,136*/ 0, 6, 0x03, 0x12, 0x11, 0x12, 0x0B, 0x0A,
  /* RLE: 004 Pixels @ 211,136*/ 4, 0x03,
  /* RLE: 094 Pixels @ 215,136*/ 94, 0x00,
  /* ABS: 004 Pixels @ 309,136*/ 0, 4, 0x0D, 0x03, 0x03, 0x06,
  /* RLE: 018 Pixels @ 313,136*/ 18, 0x00,
  /* RLE: 004 Pixels @ 331,136*/ 4, 0x06,
  /* RLE: 037 Pixels @ 335,136*/ 37, 0x01,
  /* ABS: 002 Pixels @ 372,136*/ 0, 2, 0x04, 0x04,
  /* RLE: 009 Pixels @ 374,136*/ 9, 0x03,
  /* ABS: 009 Pixels @ 383,136*/ 0, 9, 0x04, 0x04, 0x01, 0x01, 0x01, 0x0D, 0x0A, 0x0B, 0x07,
  /* RLE: 004 Pixels @ 004,137*/ 4, 0x00,
  /* ABS: 005 Pixels @ 008,137*/ 0, 5, 0x19, 0x05, 0x09, 0x09, 0x09,
  /* RLE: 024 Pixels @ 013,137*/ 24, 0x02,
  /* RLE: 001 Pixels @ 037,137*/ 1, 0x04,
  /* RLE: 006 Pixels @ 038,137*/ 6, 0x03,
  /* ABS: 002 Pixels @ 044,137*/ 0, 2, 0x04, 0x0A,
  /* RLE: 041 Pixels @ 046,137*/ 41, 0x00,
  /* ABS: 002 Pixels @ 087,137*/ 0, 2, 0x0A, 0x04,
  /* RLE: 007 Pixels @ 089,137*/ 7, 0x03,
  /* ABS: 003 Pixels @ 096,137*/ 0, 3, 0x04, 0x04, 0x0A,
  /* RLE: 036 Pixels @ 099,137*/ 36, 0x00,
  /* ABS: 004 Pixels @ 135,137*/ 0, 4, 0x0A, 0x07, 0x10, 0x07,
  /* RLE: 020 Pixels @ 139,137*/ 20, 0x10,
  /* ABS: 003 Pixels @ 159,137*/ 0, 3, 0x07, 0x10, 0x07,
  /* RLE: 026 Pixels @ 162,137*/ 26, 0x05,
  /* RLE: 004 Pixels @ 188,137*/ 4, 0x09,
  /* RLE: 014 Pixels @ 192,137*/ 14, 0x00,
  /* RLE: 004 Pixels @ 206,137*/ 4, 0x03,
  /* ABS: 005 Pixels @ 210,137*/ 0, 5, 0x00, 0x0A, 0x03, 0x03, 0x03,
  /* RLE: 095 Pixels @ 215,137*/ 95, 0x00,
  /* ABS: 004 Pixels @ 310,137*/ 0, 4, 0x0D, 0x06, 0x06, 0x06,
  /* RLE: 018 Pixels @ 314,137*/ 18, 0x00,
  /* ABS: 004 Pixels @ 332,137*/ 0, 4, 0x06, 0x06, 0x03, 0x06,
  /* RLE: 037 Pixels @ 336,137*/ 37, 0x01,
  /* ABS: 002 Pixels @ 373,137*/ 0, 2, 0x04, 0x04,
  /* RLE: 009 Pixels @ 375,137*/ 9, 0x03,
  /* ABS: 019 Pixels @ 384,137*/ 0, 19, 0x04, 0x04, 0x01, 0x01, 0x0A, 0x03, 0x0A, 0x07, 0x0A, 0x03, 0x0B, 0x0A, 0x03, 0x03, 0x1A, 0x05, 0x09, 0x09, 0x09,
  /* RLE: 022 Pixels @ 015,138*/ 22, 0x02,
  /* RLE: 001 Pixels @ 037,138*/ 1, 0x04,
  /* RLE: 006 Pixels @ 038,138*/ 6, 0x03,
  /* RLE: 001 Pixels @ 044,138*/ 1, 0x04,
  /* RLE: 042 Pixels @ 045,138*/ 42, 0x00,
  /* ABS: 002 Pixels @ 087,138*/ 0, 2, 0x0A, 0x04,
  /* RLE: 008 Pixels @ 089,138*/ 8, 0x03,
  /* ABS: 004 Pixels @ 097,138*/ 0, 4, 0x04, 0x00, 0x00, 0x0A,
  /* RLE: 032 Pixels @ 101,138*/ 32, 0x00,
  /* ABS: 006 Pixels @ 133,138*/ 0, 6, 0x0A, 0x00, 0x00, 0x07, 0x10, 0x07,
  /* RLE: 020 Pixels @ 139,138*/ 20, 0x10,
  /* ABS: 003 Pixels @ 159,138*/ 0, 3, 0x07, 0x10, 0x07,
  /* RLE: 022 Pixels @ 162,138*/ 22, 0x05,
  /* RLE: 004 Pixels @ 184,138*/ 4, 0x09,
  /* RLE: 017 Pixels @ 188,138*/ 17, 0x00,
  /* ABS: 011 Pixels @ 205,138*/ 0, 11, 0x03, 0x03, 0x0B, 0x06, 0x0F, 0x07, 0x07, 0x03, 0x04, 0x0A, 0x03,
  /* RLE: 095 Pixels @ 216,138*/ 95, 0x00,
  /* ABS: 005 Pixels @ 311,138*/ 0, 5, 0x0D, 0x0D, 0x06, 0x06, 0x06,
  /* RLE: 017 Pixels @ 316,138*/ 17, 0x00,
  /* ABS: 005 Pixels @ 333,138*/ 0, 5, 0x06, 0x06, 0x03, 0x06, 0x06,
  /* RLE: 036 Pixels @ 338,138*/ 36, 0x01,
  /* ABS: 002 Pixels @ 374,138*/ 0, 2, 0x04, 0x04,
  /* RLE: 009 Pixels @ 376,138*/ 9, 0x03,
  /* ABS: 020 Pixels @ 385,138*/ 0, 20, 0x04, 0x04, 0x01, 0x03, 0x03, 0x06, 0x0F, 0x03, 0x06, 0x11, 0x11, 0x11, 0x00, 0x00, 0x19, 0x03, 0x03, 0x03, 0x0B, 0x09,
  /* RLE: 020 Pixels @ 017,139*/ 20, 0x02,
  /* RLE: 001 Pixels @ 037,139*/ 1, 0x04,
  /* RLE: 006 Pixels @ 038,139*/ 6, 0x03,
  /* ABS: 003 Pixels @ 044,139*/ 0, 3, 0x04, 0x00, 0x0A,
  /* RLE: 041 Pixels @ 047,139*/ 41, 0x00,
  /* ABS: 002 Pixels @ 088,139*/ 0, 2, 0x0A, 0x04,
  /* RLE: 007 Pixels @ 090,139*/ 7, 0x03,
  /* ABS: 003 Pixels @ 097,139*/ 0, 3, 0x04, 0x04, 0x0A,
  /* RLE: 034 Pixels @ 100,139*/ 34, 0x00,
  /* ABS: 008 Pixels @ 134,139*/ 0, 8, 0x0A, 0x00, 0x07, 0x10, 0x07, 0x10, 0x10, 0x07,
  /* RLE: 005 Pixels @ 142,139*/ 5, 0x10,
  /* RLE: 003 Pixels @ 147,139*/ 3, 0x07,
  /* RLE: 003 Pixels @ 150,139*/ 3, 0x10,
  /* RLE: 004 Pixels @ 153,139*/ 4, 0x07,
  /* ABS: 005 Pixels @ 157,139*/ 0, 5, 0x10, 0x10, 0x07, 0x10, 0x07,
  /* RLE: 018 Pixels @ 162,139*/ 18, 0x05,
  /* RLE: 004 Pixels @ 180,139*/ 4, 0x09,
  /* RLE: 022 Pixels @ 184,139*/ 22, 0x00,
  /* ABS: 010 Pixels @ 206,139*/ 0, 10, 0x04, 0x07, 0x11, 0x0E, 0x0B, 0x00, 0x03, 0x12, 0x0B, 0x03,
  /* RLE: 096 Pixels @ 216,139*/ 96, 0x00,
  /* ABS: 005 Pixels @ 312,139*/ 0, 5, 0x0D, 0x06, 0x06, 0x0D, 0x06,
  /* RLE: 017 Pixels @ 317,139*/ 17, 0x00,
  /* ABS: 005 Pixels @ 334,139*/ 0, 5, 0x06, 0x06, 0x03, 0x06, 0x06,
  /* RLE: 036 Pixels @ 339,139*/ 36, 0x01,
  /* RLE: 003 Pixels @ 375,139*/ 3, 0x04,
  /* RLE: 008 Pixels @ 378,139*/ 8, 0x03,
  /* ABS: 021 Pixels @ 386,139*/ 0, 21, 0x04, 0x04, 0x03, 0x00, 0x07, 0x0B, 0x03, 0x11, 0x0D, 0x03, 0x12, 0x12, 0x03, 0x03, 0x0A, 0x0F, 0x04, 0x19, 0x09, 0x09, 0x09,
  /* RLE: 018 Pixels @ 019,140*/ 18, 0x02,
  /* RLE: 001 Pixels @ 037,140*/ 1, 0x04,
  /* RLE: 007 Pixels @ 038,140*/ 7, 0x03,
  /* RLE: 001 Pixels @ 045,140*/ 1, 0x0A,
  /* RLE: 042 Pixels @ 046,140*/ 42, 0x00,
  /* ABS: 003 Pixels @ 088,140*/ 0, 3, 0x0A, 0x04, 0x04,
  /* RLE: 007 Pixels @ 091,140*/ 7, 0x03,
  /* ABS: 002 Pixels @ 098,140*/ 0, 2, 0x04, 0x04,
  /* RLE: 033 Pixels @ 100,140*/ 33, 0x00,
  /* ABS: 009 Pixels @ 133,140*/ 0, 9, 0x0A, 0x00, 0x00, 0x07, 0x10, 0x07, 0x10, 0x10, 0x07,
  /* RLE: 004 Pixels @ 142,140*/ 4, 0x10,
  /* ABS: 008 Pixels @ 146,140*/ 0, 8, 0x07, 0x10, 0x10, 0x10, 0x07, 0x10, 0x10, 0x07,
  /* RLE: 005 Pixels @ 154,140*/ 5, 0x10,
  /* ABS: 003 Pixels @ 159,140*/ 0, 3, 0x07, 0x10, 0x07,
  /* RLE: 015 Pixels @ 162,140*/ 15, 0x05,
  /* RLE: 003 Pixels @ 177,140*/ 3, 0x09,
  /* RLE: 025 Pixels @ 180,140*/ 25, 0x00,
  /* ABS: 003 Pixels @ 205,140*/ 0, 3, 0x03, 0x0E, 0x0D,
  /* RLE: 006 Pixels @ 208,140*/ 6, 0x03,
  /* ABS: 002 Pixels @ 214,140*/ 0, 2, 0x00, 0x03,
  /* RLE: 097 Pixels @ 216,140*/ 97, 0x00,
  /* ABS: 005 Pixels @ 313,140*/ 0, 5, 0x0D, 0x0D, 0x03, 0x06, 0x06,
  /* RLE: 017 Pixels @ 318,140*/ 17, 0x00,
  /* RLE: 005 Pixels @ 335,140*/ 5, 0x06,
  /* RLE: 037 Pixels @ 340,140*/ 37, 0x01,
  /* ABS: 002 Pixels @ 377,140*/ 0, 2, 0x04, 0x04,
  /* RLE: 010 Pixels @ 379,140*/ 10, 0x03,
  /* ABS: 020 Pixels @ 001,141*/ 0, 20, 0x06, 0x0F, 0x03, 0x00, 0x0A, 0x07, 0x0D, 0x0A, 0x0A, 0x03, 0x0E, 0x07, 0x11, 0x03, 0x03, 0x00, 0x1A, 0x09, 0x09, 0x09,
  /* RLE: 017 Pixels @ 021,141*/ 17, 0x02,
  /* RLE: 001 Pixels @ 038,141*/ 1, 0x04,
  /* RLE: 006 Pixels @ 039,141*/ 6, 0x03,
  /* ABS: 002 Pixels @ 045,141*/ 0, 2, 0x04, 0x0A,
  /* RLE: 043 Pixels @ 047,141*/ 43, 0x00,
  /* ABS: 002 Pixels @ 090,141*/ 0, 2, 0x04, 0x04,
  /* RLE: 007 Pixels @ 092,141*/ 7, 0x03,
  /* ABS: 004 Pixels @ 099,141*/ 0, 4, 0x04, 0x04, 0x00, 0x0A,
  /* RLE: 026 Pixels @ 103,141*/ 26, 0x00,
  /* ABS: 013 Pixels @ 129,141*/ 0, 13, 0x0A, 0x00, 0x00, 0x0A, 0x00, 0x0A, 0x09, 0x07, 0x10, 0x07, 0x10, 0x10, 0x07,
  /* RLE: 004 Pixels @ 142,141*/ 4, 0x10,
  /* ABS: 006 Pixels @ 146,141*/ 0, 6, 0x07, 0x10, 0x10, 0x10, 0x07, 0x10,
  /* RLE: 004 Pixels @ 152,141*/ 4, 0x07,
  /* RLE: 003 Pixels @ 156,141*/ 3, 0x10,
  /* ABS: 003 Pixels @ 159,141*/ 0, 3, 0x07, 0x10, 0x07,
  /* RLE: 011 Pixels @ 162,141*/ 11, 0x05,
  /* RLE: 004 Pixels @ 173,141*/ 4, 0x09,
  /* RLE: 028 Pixels @ 177,141*/ 28, 0x00,
  /* ABS: 011 Pixels @ 205,141*/ 0, 11, 0x03, 0x0A, 0x11, 0x03, 0x03, 0x00, 0x0B, 0x0B, 0x04, 0x07, 0x03,
  /* RLE: 099 Pixels @ 216,141*/ 99, 0x00,
  /* ABS: 004 Pixels @ 315,141*/ 0, 4, 0x0D, 0x03, 0x06, 0x06,
  /* RLE: 018 Pixels @ 319,141*/ 18, 0x00,
  /* RLE: 004 Pixels @ 337,141*/ 4, 0x06,
  /* RLE: 037 Pixels @ 341,141*/ 37, 0x01,
  /* ABS: 002 Pixels @ 378,141*/ 0, 2, 0x04, 0x04,
  /* RLE: 008 Pixels @ 380,141*/ 8, 0x03,
  /* ABS: 023 Pixels @ 000,142*/ 0, 23, 0x00, 0x00, 0x04, 0x03, 0x07, 0x0B, 0x00, 0x0F, 0x0D, 0x03, 0x03, 0x0B, 0x07, 0x07, 0x04, 0x06, 0x00, 0x03, 0x03, 0x1A, 0x09, 0x09, 0x09,
  /* RLE: 015 Pixels @ 023,142*/ 15, 0x02,
  /* RLE: 001 Pixels @ 038,142*/ 1, 0x04,
  /* RLE: 006 Pixels @ 039,142*/ 6, 0x03,
  /* RLE: 001 Pixels @ 045,142*/ 1, 0x04,
  /* RLE: 045 Pixels @ 046,142*/ 45, 0x00,
  /* RLE: 001 Pixels @ 091,142*/ 1, 0x04,
  /* RLE: 008 Pixels @ 092,142*/ 8, 0x03,
  /* ABS: 002 Pixels @ 100,142*/ 0, 2, 0x04, 0x0A,
  /* RLE: 018 Pixels @ 102,142*/ 18, 0x00,
  /* RLE: 001 Pixels @ 120,142*/ 1, 0x0A,
  /* RLE: 004 Pixels @ 121,142*/ 4, 0x00,
  /* ABS: 006 Pixels @ 125,142*/ 0, 6, 0x0A, 0x00, 0x00, 0x0A, 0x00, 0x0A,
  /* RLE: 004 Pixels @ 131,142*/ 4, 0x09,
  /* ABS: 007 Pixels @ 135,142*/ 0, 7, 0x05, 0x07, 0x10, 0x07, 0x10, 0x10, 0x07,
  /* RLE: 005 Pixels @ 142,142*/ 5, 0x10,
  /* RLE: 003 Pixels @ 147,142*/ 3, 0x07,
  /* ABS: 012 Pixels @ 150,142*/ 0, 12, 0x10, 0x10, 0x07, 0x10, 0x10, 0x10, 0x07, 0x10, 0x10, 0x07, 0x10, 0x07,
  /* RLE: 007 Pixels @ 162,142*/ 7, 0x05,
  /* RLE: 004 Pixels @ 169,142*/ 4, 0x09,
  /* RLE: 033 Pixels @ 173,142*/ 33, 0x00,
  /* ABS: 002 Pixels @ 206,142*/ 0, 2, 0x03, 0x06,
  /* RLE: 004 Pixels @ 208,142*/ 4, 0x07,
  /* ABS: 004 Pixels @ 212,142*/ 0, 4, 0x06, 0x03, 0x00, 0x03,
  /* RLE: 100 Pixels @ 216,142*/ 100, 0x00,
  /* ABS: 004 Pixels @ 316,142*/ 0, 4, 0x0D, 0x03, 0x06, 0x06,
  /* RLE: 018 Pixels @ 320,142*/ 18, 0x00,
  /* ABS: 004 Pixels @ 338,142*/ 0, 4, 0x06, 0x06, 0x03, 0x06,
  /* RLE: 037 Pixels @ 342,142*/ 37, 0x01,
  /* ABS: 002 Pixels @ 379,142*/ 0, 2, 0x04, 0x04,
  /* RLE: 007 Pixels @ 381,142*/ 7, 0x03,
  /* ABS: 022 Pixels @ 000,143*/ 0, 22, 0x0A, 0x0A, 0x04, 0x03, 0x0E, 0x0F, 0x0B, 0x06, 0x0F, 0x03, 0x03, 0x11, 0x11, 0x00, 0x0B, 0x07, 0x11, 0x11, 0x00, 0x00, 0x15, 0x19,
  /* RLE: 004 Pixels @ 022,143*/ 4, 0x09,
  /* RLE: 012 Pixels @ 026,143*/ 12, 0x02,
  /* RLE: 001 Pixels @ 038,143*/ 1, 0x04,
  /* RLE: 006 Pixels @ 039,143*/ 6, 0x03,
  /* ABS: 002 Pixels @ 045,143*/ 0, 2, 0x04, 0x0A,
  /* RLE: 045 Pixels @ 047,143*/ 45, 0x00,
  /* RLE: 001 Pixels @ 092,143*/ 1, 0x04,
  /* RLE: 008 Pixels @ 093,143*/ 8, 0x03,
  /* RLE: 001 Pixels @ 101,143*/ 1, 0x04,
  /* RLE: 017 Pixels @ 102,143*/ 17, 0x00,
  /* ABS: 008 Pixels @ 119,143*/ 0, 8, 0x0A, 0x00, 0x0A, 0x00, 0x00, 0x0A, 0x00, 0x0A,
  /* RLE: 004 Pixels @ 127,143*/ 4, 0x09,
  /* RLE: 005 Pixels @ 131,143*/ 5, 0x05,
  /* ABS: 006 Pixels @ 136,143*/ 0, 6, 0x07, 0x10, 0x07, 0x10, 0x10, 0x07,
  /* RLE: 004 Pixels @ 142,143*/ 4, 0x10,
  /* ABS: 005 Pixels @ 146,143*/ 0, 5, 0x07, 0x10, 0x10, 0x10, 0x07,
  /* RLE: 005 Pixels @ 151,143*/ 5, 0x10,
  /* ABS: 009 Pixels @ 156,143*/ 0, 9, 0x07, 0x10, 0x10, 0x07, 0x10, 0x07, 0x05, 0x05, 0x05,
  /* RLE: 004 Pixels @ 165,143*/ 4, 0x09,
  /* RLE: 037 Pixels @ 169,143*/ 37, 0x00,
  /* ABS: 005 Pixels @ 206,143*/ 0, 5, 0x03, 0x06, 0x12, 0x0B, 0x0A,
  /* RLE: 005 Pixels @ 211,143*/ 5, 0x03,
  /* RLE: 101 Pixels @ 216,143*/ 101, 0x00,
  /* ABS: 004 Pixels @ 317,143*/ 0, 4, 0x0D, 0x06, 0x06, 0x06,
  /* RLE: 018 Pixels @ 321,143*/ 18, 0x00,
  /* ABS: 005 Pixels @ 339,143*/ 0, 5, 0x06, 0x06, 0x03, 0x06, 0x06,
  /* RLE: 036 Pixels @ 344,143*/ 36, 0x01,
  /* ABS: 002 Pixels @ 380,143*/ 0, 2, 0x04, 0x04,
  /* RLE: 006 Pixels @ 382,143*/ 6, 0x03,
  /* RLE: 003 Pixels @ 000,144*/ 3, 0x02,
  /* ABS: 025 Pixels @ 003,144*/ 0, 25, 0x04, 0x03, 0x0A, 0x12, 0x06, 0x04, 0x03, 0x04, 0x07, 0x04, 0x03, 0x0F, 0x0D, 0x04, 0x12, 0x03, 0x0A, 0x00, 0x03, 0x03, 0x1A, 0x05, 0x09, 0x09, 0x09,
  /* RLE: 010 Pixels @ 028,144*/ 10, 0x02,
  /* RLE: 001 Pixels @ 038,144*/ 1, 0x04,
  /* RLE: 006 Pixels @ 039,144*/ 6, 0x03,
  /* RLE: 001 Pixels @ 045,144*/ 1, 0x04,
  /* RLE: 045 Pixels @ 046,144*/ 45, 0x00,
  /* ABS: 003 Pixels @ 091,144*/ 0, 3, 0x0A, 0x04, 0x04,
  /* RLE: 007 Pixels @ 094,144*/ 7, 0x03,
  /* RLE: 001 Pixels @ 101,144*/ 1, 0x04,
  /* RLE: 016 Pixels @ 102,144*/ 16, 0x00,
  /* ABS: 005 Pixels @ 118,144*/ 0, 5, 0x0A, 0x00, 0x00, 0x00, 0x0A,
  /* RLE: 004 Pixels @ 123,144*/ 4, 0x09,
  /* RLE: 009 Pixels @ 127,144*/ 9, 0x05,
  /* ABS: 006 Pixels @ 136,144*/ 0, 6, 0x07, 0x10, 0x07, 0x10, 0x10, 0x07,
  /* RLE: 004 Pixels @ 142,144*/ 4, 0x10,
  /* ABS: 019 Pixels @ 146,144*/ 0, 19, 0x07, 0x10, 0x10, 0x10, 0x07, 0x10, 0x07, 0x10, 0x10, 0x10, 0x07, 0x10, 0x10, 0x07, 0x10, 0x07, 0x09, 0x09, 0x09,
  /* RLE: 042 Pixels @ 165,144*/ 42, 0x00,
  /* RLE: 006 Pixels @ 207,144*/ 6, 0x03,
  /* ABS: 004 Pixels @ 213,144*/ 0, 4, 0x00, 0x0B, 0x12, 0x03,
  /* RLE: 101 Pixels @ 217,144*/ 101, 0x00,
  /* ABS: 005 Pixels @ 318,144*/ 0, 5, 0x0D, 0x0D, 0x06, 0x06, 0x06,
  /* RLE: 017 Pixels @ 323,144*/ 17, 0x00,
  /* ABS: 005 Pixels @ 340,144*/ 0, 5, 0x06, 0x06, 0x03, 0x06, 0x06,
  /* RLE: 036 Pixels @ 345,144*/ 36, 0x01,
  /* ABS: 002 Pixels @ 381,144*/ 0, 2, 0x04, 0x04,
  /* RLE: 005 Pixels @ 383,144*/ 5, 0x03,
  /* RLE: 004 Pixels @ 000,145*/ 4, 0x02,
  /* ABS: 002 Pixels @ 004,145*/ 0, 2, 0x00, 0x00,
  /* RLE: 004 Pixels @ 006,145*/ 4, 0x03,
  /* ABS: 020 Pixels @ 010,145*/ 0, 20, 0x12, 0x11, 0x03, 0x0B, 0x07, 0x0A, 0x03, 0x03, 0x04, 0x07, 0x07, 0x11, 0x00, 0x03, 0x05, 0x05, 0x05, 0x09, 0x09, 0x09,
  /* RLE: 008 Pixels @ 030,145*/ 8, 0x02,
  /* RLE: 001 Pixels @ 038,145*/ 1, 0x04,
  /* RLE: 006 Pixels @ 039,145*/ 6, 0x03,
  /* ABS: 003 Pixels @ 045,145*/ 0, 3, 0x04, 0x00, 0x0A,
  /* RLE: 045 Pixels @ 048,145*/ 45, 0x00,
  /* ABS: 002 Pixels @ 093,145*/ 0, 2, 0x0A, 0x04,
  /* RLE: 006 Pixels @ 095,145*/ 6, 0x03,
  /* RLE: 001 Pixels @ 101,145*/ 1, 0x04,
  /* RLE: 012 Pixels @ 102,145*/ 12, 0x00,
  /* ABS: 009 Pixels @ 114,145*/ 0, 9, 0x0A, 0x00, 0x00, 0x0A, 0x00, 0x0A, 0x09, 0x09, 0x09,
  /* RLE: 013 Pixels @ 123,145*/ 13, 0x05,
  /* ABS: 005 Pixels @ 136,145*/ 0, 5, 0x07, 0x10, 0x07, 0x10, 0x10,
  /* RLE: 005 Pixels @ 141,145*/ 5, 0x07,
  /* ABS: 016 Pixels @ 146,145*/ 0, 16, 0x10, 0x07, 0x07, 0x07, 0x10, 0x10, 0x10, 0x07, 0x07, 0x07, 0x10, 0x10, 0x10, 0x07, 0x10, 0x07,
  /* RLE: 045 Pixels @ 162,145*/ 45, 0x00,
  /* ABS: 009 Pixels @ 207,145*/ 0, 9, 0x03, 0x00, 0x0B, 0x12, 0x11, 0x07, 0x07, 0x07, 0x11,
  /* RLE: 103 Pixels @ 216,145*/ 103, 0x00,
  /* ABS: 005 Pixels @ 319,145*/ 0, 5, 0x0D, 0x06, 0x03, 0x06, 0x06,
  /* RLE: 017 Pixels @ 324,145*/ 17, 0x00,
  /* RLE: 005 Pixels @ 341,145*/ 5, 0x06,
  /* RLE: 036 Pixels @ 346,145*/ 36, 0x01,
  /* RLE: 003 Pixels @ 382,145*/ 3, 0x04,
  /* RLE: 003 Pixels @ 385,145*/ 3, 0x03,
  /* RLE: 007 Pixels @ 000,146*/ 7, 0x02,
  /* ABS: 025 Pixels @ 007,146*/ 0, 25, 0x09, 0x09, 0x03, 0x0E, 0x07, 0x0B, 0x0F, 0x06, 0x03, 0x03, 0x03, 0x00, 0x03, 0x03, 0x12, 0x11, 0x03, 0x19, 0x00, 0x00, 0x19, 0x05, 0x09, 0x09, 0x09,
  /* RLE: 006 Pixels @ 032,146*/ 6, 0x02,
  /* RLE: 001 Pixels @ 038,146*/ 1, 0x04,
  /* RLE: 007 Pixels @ 039,146*/ 7, 0x03,
  /* RLE: 001 Pixels @ 046,146*/ 1, 0x0A,
  /* RLE: 046 Pixels @ 047,146*/ 46, 0x00,
  /* ABS: 003 Pixels @ 093,146*/ 0, 3, 0x0A, 0x04, 0x04,
  /* RLE: 005 Pixels @ 096,146*/ 5, 0x03,
  /* RLE: 001 Pixels @ 101,146*/ 1, 0x04,
  /* RLE: 008 Pixels @ 102,146*/ 8, 0x00,
  /* ABS: 006 Pixels @ 110,146*/ 0, 6, 0x0A, 0x00, 0x00, 0x0A, 0x00, 0x0A,
  /* RLE: 004 Pixels @ 116,146*/ 4, 0x09,
  /* RLE: 016 Pixels @ 120,146*/ 16, 0x05,
  /* ABS: 003 Pixels @ 136,146*/ 0, 3, 0x07, 0x10, 0x07,
  /* RLE: 020 Pixels @ 139,146*/ 20, 0x10,
  /* ABS: 003 Pixels @ 159,146*/ 0, 3, 0x07, 0x10, 0x07,
  /* RLE: 045 Pixels @ 162,146*/ 45, 0x00,
  /* ABS: 010 Pixels @ 207,146*/ 0, 10, 0x03, 0x12, 0x07, 0x11, 0x12, 0x0B, 0x00, 0x03, 0x03, 0x03,
  /* RLE: 103 Pixels @ 217,146*/ 103, 0x00,
  /* ABS: 005 Pixels @ 320,146*/ 0, 5, 0x0D, 0x0D, 0x03, 0x06, 0x06,
  /* RLE: 018 Pixels @ 325,146*/ 18, 0x00,
  /* RLE: 004 Pixels @ 343,146*/ 4, 0x06,
  /* RLE: 038 Pixels @ 347,146*/ 38, 0x01,
  /* RLE: 003 Pixels @ 385,146*/ 3, 0x04,
  /* RLE: 009 Pixels @ 000,147*/ 9, 0x02,
  /* ABS: 025 Pixels @ 009,147*/ 0, 25, 0x04, 0x03, 0x0A, 0x00, 0x06, 0x0A, 0x03, 0x03, 0x0D, 0x07, 0x07, 0x06, 0x0F, 0x06, 0x03, 0x03, 0x0B, 0x0A, 0x03, 0x03, 0x1A, 0x05, 0x09, 0x09, 0x09,
  /* RLE: 005 Pixels @ 034,147*/ 5, 0x02,
  /* RLE: 001 Pixels @ 039,147*/ 1, 0x04,
  /* RLE: 006 Pixels @ 040,147*/ 6, 0x03,
  /* ABS: 002 Pixels @ 046,147*/ 0, 2, 0x04, 0x0A,
  /* RLE: 047 Pixels @ 048,147*/ 47, 0x00,
  /* ABS: 017 Pixels @ 095,147*/ 0, 17, 0x04, 0x04, 0x03, 0x03, 0x03, 0x04, 0x04, 0x00, 0x0A, 0x00, 0x00, 0x0A, 0x00, 0x00, 0x0A, 0x00, 0x0A,
  /* RLE: 004 Pixels @ 112,147*/ 4, 0x09,
  /* RLE: 020 Pixels @ 116,147*/ 20, 0x05,
  /* ABS: 003 Pixels @ 136,147*/ 0, 3, 0x07, 0x10, 0x07,
  /* RLE: 020 Pixels @ 139,147*/ 20, 0x10,
  /* ABS: 003 Pixels @ 159,147*/ 0, 3, 0x07, 0x10, 0x07,
  /* RLE: 045 Pixels @ 162,147*/ 45, 0x00,
  /* ABS: 010 Pixels @ 207,147*/ 0, 10, 0x03, 0x00, 0x03, 0x03, 0x00, 0x04, 0x00, 0x03, 0x03, 0x04,
  /* RLE: 105 Pixels @ 217,147*/ 105, 0x00,
  /* ABS: 004 Pixels @ 322,147*/ 0, 4, 0x0D, 0x03, 0x06, 0x06,
  /* RLE: 018 Pixels @ 326,147*/ 18, 0x00,
  /* ABS: 004 Pixels @ 344,147*/ 0, 4, 0x06, 0x06, 0x03, 0x06,
  /* RLE: 040 Pixels @ 348,147*/ 40, 0x01,
  /* RLE: 010 Pixels @ 000,148*/ 10, 0x02,
  /* ABS: 030 Pixels @ 010,148*/ 0, 30, 0x00, 0x04, 0x03, 0x00, 0x00, 0x15, 0x00, 0x07, 0x0B, 0x03, 0x0B, 0x07, 0x0A, 0x03, 0x06, 0x11, 0x11, 0x11, 0x00, 0x00, 0x05, 0x05, 0x05, 0x09, 0x09, 0x09, 0x02, 0x02, 0x02, 0x04,
  /* RLE: 006 Pixels @ 040,148*/ 6, 0x03,
  /* RLE: 001 Pixels @ 046,148*/ 1, 0x04,
  /* RLE: 048 Pixels @ 047,148*/ 48, 0x00,
  /* ABS: 013 Pixels @ 095,148*/ 0, 13, 0x04, 0x0A, 0x04, 0x0A, 0x04, 0x04, 0x00, 0x0A, 0x00, 0x00, 0x0A, 0x00, 0x0A,
  /* RLE: 004 Pixels @ 108,148*/ 4, 0x09,
  /* RLE: 024 Pixels @ 112,148*/ 24, 0x05,
  /* ABS: 003 Pixels @ 136,148*/ 0, 3, 0x07, 0x10, 0x10,
  /* RLE: 020 Pixels @ 139,148*/ 20, 0x07,
  /* ABS: 003 Pixels @ 159,148*/ 0, 3, 0x10, 0x10, 0x07,
  /* RLE: 045 Pixels @ 162,148*/ 45, 0x00,
  /* ABS: 010 Pixels @ 207,148*/ 0, 10, 0x03, 0x00, 0x0D, 0x03, 0x11, 0x07, 0x0F, 0x03, 0x03, 0x04,
  /* RLE: 106 Pixels @ 217,148*/ 106, 0x00,
  /* ABS: 004 Pixels @ 323,148*/ 0, 4, 0x0D, 0x0D, 0x06, 0x06,
  /* RLE: 018 Pixels @ 327,148*/ 18, 0x00,
  /* ABS: 004 Pixels @ 345,148*/ 0, 4, 0x06, 0x06, 0x03, 0x06,
  /* RLE: 039 Pixels @ 349,148*/ 39, 0x01,
  /* RLE: 013 Pixels @ 000,149*/ 13, 0x02,
  /* ABS: 027 Pixels @ 013,149*/ 0, 27, 0x09, 0x09, 0x06, 0x03, 0x06, 0x11, 0x0B, 0x0F, 0x11, 0x03, 0x03, 0x11, 0x0D, 0x03, 0x12, 0x12, 0x03, 0x15, 0x00, 0x19, 0x05, 0x05, 0x09, 0x09, 0x09, 0x02, 0x04,
  /* RLE: 006 Pixels @ 040,149*/ 6, 0x03,
  /* ABS: 002 Pixels @ 046,149*/ 0, 2, 0x04, 0x0A,
  /* RLE: 048 Pixels @ 048,149*/ 48, 0x00,
  /* ABS: 008 Pixels @ 096,149*/ 0, 8, 0x0A, 0x00, 0x0A, 0x00, 0x00, 0x0A, 0x00, 0x0A,
  /* RLE: 004 Pixels @ 104,149*/ 4, 0x09,
  /* RLE: 029 Pixels @ 108,149*/ 29, 0x05,
  /* RLE: 001 Pixels @ 137,149*/ 1, 0x07,
  /* RLE: 022 Pixels @ 138,149*/ 22, 0x10,
  /* RLE: 001 Pixels @ 160,149*/ 1, 0x07,
  /* RLE: 047 Pixels @ 161,149*/ 47, 0x00,
  /* ABS: 009 Pixels @ 208,149*/ 0, 9, 0x0B, 0x11, 0x00, 0x07, 0x04, 0x0D, 0x04, 0x03, 0x04,
  /* RLE: 107 Pixels @ 217,149*/ 107, 0x00,
  /* ABS: 004 Pixels @ 324,149*/ 0, 4, 0x0D, 0x06, 0x06, 0x06,
  /* RLE: 018 Pixels @ 328,149*/ 18, 0x00,
  /* ABS: 005 Pixels @ 346,149*/ 0, 5, 0x06, 0x06, 0x03, 0x06, 0x06,
  /* RLE: 037 Pixels @ 351,149*/ 37, 0x01,
  /* RLE: 015 Pixels @ 000,150*/ 15, 0x02,
  /* ABS: 025 Pixels @ 015,150*/ 0, 25, 0x09, 0x00, 0x03, 0x0A, 0x0A, 0x07, 0x04, 0x03, 0x00, 0x0A, 0x07, 0x0D, 0x0A, 0x0A, 0x03, 0x0B, 0x0A, 0x03, 0x03, 0x1A, 0x05, 0x05, 0x09, 0x09, 0x09,
  /* RLE: 006 Pixels @ 040,150*/ 6, 0x03,
  /* RLE: 001 Pixels @ 046,150*/ 1, 0x04,
  /* RLE: 048 Pixels @ 047,150*/ 48, 0x00,
  /* ABS: 005 Pixels @ 095,150*/ 0, 5, 0x0A, 0x00, 0x00, 0x00, 0x0A,
  /* RLE: 004 Pixels @ 100,150*/ 4, 0x09,
  /* RLE: 034 Pixels @ 104,150*/ 34, 0x05,
  /* RLE: 022 Pixels @ 138,150*/ 22, 0x07,
  /* RLE: 047 Pixels @ 160,150*/ 47, 0x00,
  /* ABS: 010 Pixels @ 207,150*/ 0, 10, 0x03, 0x0B, 0x0B, 0x0A, 0x07, 0x00, 0x12, 0x0B, 0x03, 0x04,
  /* RLE: 108 Pixels @ 217,150*/ 108, 0x00,
  /* ABS: 004 Pixels @ 325,150*/ 0, 4, 0x0D, 0x06, 0x03, 0x06,
  /* RLE: 018 Pixels @ 329,150*/ 18, 0x00,
  /* RLE: 005 Pixels @ 347,150*/ 5, 0x06,
  /* RLE: 036 Pixels @ 352,150*/ 36, 0x01,
  /* RLE: 017 Pixels @ 000,151*/ 17, 0x02,
  /* ABS: 023 Pixels @ 017,151*/ 0, 23, 0x18, 0x0A, 0x03, 0x00, 0x00, 0x03, 0x07, 0x0B, 0x00, 0x0F, 0x0D, 0x03, 0x06, 0x11, 0x11, 0x11, 0x00, 0x00, 0x1A, 0x15, 0x15, 0x19, 0x09,
  /* RLE: 006 Pixels @ 040,151*/ 6, 0x03,
  /* ABS: 004 Pixels @ 046,151*/ 0, 4, 0x04, 0x00, 0x00, 0x0A,
  /* RLE: 041 Pixels @ 050,151*/ 41, 0x00,
  /* ABS: 009 Pixels @ 091,151*/ 0, 9, 0x0A, 0x00, 0x00, 0x0A, 0x00, 0x0A, 0x09, 0x09, 0x09,
  /* RLE: 035 Pixels @ 100,151*/ 35, 0x05,
  /* RLE: 004 Pixels @ 135,151*/ 4, 0x09,
  /* RLE: 068 Pixels @ 139,151*/ 68, 0x00,
  /* ABS: 010 Pixels @ 207,151*/ 0, 10, 0x03, 0x04, 0x11, 0x04, 0x07, 0x0A, 0x0F, 0x04, 0x03, 0x03,
  /* RLE: 109 Pixels @ 217,151*/ 109, 0x00,
  /* ABS: 005 Pixels @ 326,151*/ 0, 5, 0x0D, 0x06, 0x03, 0x06, 0x06,
  /* RLE: 017 Pixels @ 331,151*/ 17, 0x00,
  /* RLE: 005 Pixels @ 348,151*/ 5, 0x06,
  /* RLE: 035 Pixels @ 353,151*/ 35, 0x01,
  /* RLE: 019 Pixels @ 000,152*/ 19, 0x02,
  /* ABS: 022 Pixels @ 019,152*/ 0, 22, 0x18, 0x0E, 0x0B, 0x03, 0x0E, 0x0F, 0x04, 0x06, 0x0F, 0x03, 0x11, 0x0D, 0x03, 0x12, 0x12, 0x03, 0x03, 0x00, 0x00, 0x03, 0x03, 0x1A,
  /* RLE: 006 Pixels @ 041,152*/ 6, 0x03,
  /* ABS: 013 Pixels @ 047,152*/ 0, 13, 0x00, 0x0A, 0x00, 0x00, 0x0A, 0x00, 0x00, 0x00, 0x0A, 0x00, 0x00, 0x00, 0x0A,
  /* RLE: 027 Pixels @ 060,152*/ 27, 0x00,
  /* ABS: 006 Pixels @ 087,152*/ 0, 6, 0x0A, 0x00, 0x00, 0x0A, 0x00, 0x0A,
  /* RLE: 004 Pixels @ 093,152*/ 4, 0x09,
  /* RLE: 034 Pixels @ 097,152*/ 34, 0x05,
  /* RLE: 004 Pixels @ 131,152*/ 4, 0x09,
  /* RLE: 073 Pixels @ 135,152*/ 73, 0x00,
  /* ABS: 010 Pixels @ 208,152*/ 0, 10, 0x03, 0x0D, 0x07, 0x0F, 0x03, 0x04, 0x03, 0x03, 0x03, 0x04,
  /* RLE: 109 Pixels @ 218,152*/ 109, 0x00,
  /* ABS: 005 Pixels @ 327,152*/ 0, 5, 0x0D, 0x0D, 0x03, 0x06, 0x06,
  /* RLE: 018 Pixels @ 332,152*/ 18, 0x00,
  /* RLE: 004 Pixels @ 350,152*/ 4, 0x06,
  /* RLE: 034 Pixels @ 354,152*/ 34, 0x01,
  /* RLE: 022 Pixels @ 000,153*/ 22, 0x02,
  /* ABS: 021 Pixels @ 022,153*/ 0, 21, 0x04, 0x03, 0x04, 0x12, 0x06, 0x04, 0x00, 0x0A, 0x07, 0x0D, 0x0A, 0x0A, 0x03, 0x0E, 0x07, 0x07, 0x0D, 0x00, 0x00, 0x05, 0x05,
  /* RLE: 004 Pixels @ 043,153*/ 4, 0x03,
  /* RLE: 003 Pixels @ 047,153*/ 3, 0x09,
  /* ABS: 022 Pixels @ 050,153*/ 0, 22, 0x0A, 0x00, 0x0A, 0x00, 0x0A, 0x00, 0x0A, 0x00, 0x0A, 0x00, 0x0A, 0x00, 0x00, 0x0A, 0x00, 0x00, 0x00, 0x0A, 0x00, 0x00, 0x00, 0x0A,
  /* RLE: 011 Pixels @ 072,153*/ 11, 0x00,
  /* ABS: 006 Pixels @ 083,153*/ 0, 6, 0x0A, 0x00, 0x00, 0x0A, 0x00, 0x0A,
  /* RLE: 004 Pixels @ 089,153*/ 4, 0x09,
  /* RLE: 034 Pixels @ 093,153*/ 34, 0x05,
  /* RLE: 004 Pixels @ 127,153*/ 4, 0x09,
  /* RLE: 001 Pixels @ 131,153*/ 1, 0x0A,
  /* RLE: 077 Pixels @ 132,153*/ 77, 0x00,
  /* ABS: 009 Pixels @ 209,153*/ 0, 9, 0x03, 0x0A, 0x03, 0x00, 0x00, 0x0A, 0x03, 0x03, 0x04,
  /* RLE: 110 Pixels @ 218,153*/ 110, 0x00,
  /* ABS: 005 Pixels @ 328,153*/ 0, 5, 0x0D, 0x0D, 0x06, 0x06, 0x06,
  /* RLE: 018 Pixels @ 333,153*/ 18, 0x00,
  /* ABS: 004 Pixels @ 351,153*/ 0, 4, 0x06, 0x03, 0x03, 0x06,
  /* RLE: 033 Pixels @ 355,153*/ 33, 0x01,
  /* RLE: 023 Pixels @ 000,154*/ 23, 0x02,
  /* RLE: 003 Pixels @ 023,154*/ 3, 0x00,
  /* ABS: 015 Pixels @ 026,154*/ 0, 15, 0x03, 0x03, 0x07, 0x0B, 0x00, 0x0F, 0x0D, 0x03, 0x04, 0x07, 0x0B, 0x03, 0x06, 0x12, 0x03,
  /* RLE: 009 Pixels @ 041,154*/ 9, 0x05,
  /* RLE: 012 Pixels @ 050,154*/ 12, 0x09,
  /* ABS: 023 Pixels @ 062,154*/ 0, 23, 0x0A, 0x00, 0x0A, 0x00, 0x0A, 0x00, 0x0A, 0x00, 0x0A, 0x00, 0x0A, 0x00, 0x00, 0x0A, 0x00, 0x00, 0x00, 0x0A, 0x00, 0x00, 0x0A, 0x00, 0x0A,
  /* RLE: 004 Pixels @ 085,154*/ 4, 0x09,
  /* RLE: 034 Pixels @ 089,154*/ 34, 0x05,
  /* RLE: 004 Pixels @ 123,154*/ 4, 0x09,
  /* RLE: 082 Pixels @ 127,154*/ 82, 0x00,
  /* ABS: 008 Pixels @ 209,154*/ 0, 8, 0x03, 0x03, 0x0E, 0x07, 0x07, 0x07, 0x06, 0x03,
  /* RLE: 113 Pixels @ 217,154*/ 113, 0x00,
  /* ABS: 004 Pixels @ 330,154*/ 0, 4, 0x0D, 0x06, 0x06, 0x06,
  /* RLE: 018 Pixels @ 334,154*/ 18, 0x00,
  /* ABS: 004 Pixels @ 352,154*/ 0, 4, 0x06, 0x06, 0x03, 0x06,
  /* RLE: 032 Pixels @ 356,154*/ 32, 0x01,
  /* RLE: 026 Pixels @ 000,155*/ 26, 0x02,
  /* ABS: 015 Pixels @ 026,155*/ 0, 15, 0x09, 0x03, 0x0E, 0x0F, 0x0B, 0x06, 0x0F, 0x03, 0x06, 0x0F, 0x11, 0x0E, 0x12, 0x06, 0x03,
  /* RLE: 021 Pixels @ 041,155*/ 21, 0x05,
  /* RLE: 012 Pixels @ 062,155*/ 12, 0x09,
  /* ABS: 007 Pixels @ 074,155*/ 0, 7, 0x0A, 0x00, 0x0A, 0x00, 0x0A, 0x00, 0x0A,
  /* RLE: 004 Pixels @ 081,155*/ 4, 0x09,
  /* RLE: 035 Pixels @ 085,155*/ 35, 0x05,
  /* RLE: 003 Pixels @ 120,155*/ 3, 0x09,
  /* RLE: 087 Pixels @ 123,155*/ 87, 0x00,
  /* ABS: 007 Pixels @ 210,155*/ 0, 7, 0x04, 0x07, 0x0E, 0x0D, 0x0A, 0x0F, 0x04,
  /* RLE: 114 Pixels @ 217,155*/ 114, 0x00,
  /* ABS: 004 Pixels @ 331,155*/ 0, 4, 0x0D, 0x06, 0x03, 0x06,
  /* RLE: 018 Pixels @ 335,155*/ 18, 0x00,
  /* RLE: 005 Pixels @ 353,155*/ 5, 0x06,
  /* RLE: 030 Pixels @ 358,155*/ 30, 0x01,
  /* RLE: 027 Pixels @ 000,156*/ 27, 0x02,
  /* ABS: 014 Pixels @ 027,156*/ 0, 14, 0x00, 0x03, 0x04, 0x12, 0x06, 0x0A, 0x03, 0x06, 0x0E, 0x03, 0x0B, 0x11, 0x0E, 0x03,
  /* RLE: 033 Pixels @ 041,156*/ 33, 0x05,
  /* RLE: 007 Pixels @ 074,156*/ 7, 0x09,
  /* RLE: 035 Pixels @ 081,156*/ 35, 0x05,
  /* RLE: 004 Pixels @ 116,156*/ 4, 0x09,
  /* RLE: 089 Pixels @ 120,156*/ 89, 0x00,
  /* ABS: 009 Pixels @ 209,156*/ 0, 9, 0x03, 0x0B, 0x12, 0x03, 0x07, 0x03, 0x06, 0x0B, 0x03,
  /* RLE: 114 Pixels @ 218,156*/ 114, 0x00,
  /* ABS: 004 Pixels @ 332,156*/ 0, 4, 0x0D, 0x0D, 0x03, 0x06,
  /* RLE: 018 Pixels @ 336,156*/ 18, 0x00,
  /* RLE: 005 Pixels @ 354,156*/ 5, 0x06,
  /* RLE: 029 Pixels @ 359,156*/ 29, 0x01,
  /* RLE: 028 Pixels @ 000,157*/ 28, 0x02,
  /* ABS: 013 Pixels @ 028,157*/ 0, 13, 0x00, 0x03, 0x03, 0x00, 0x00, 0x03, 0x04, 0x0F, 0x0B, 0x0E, 0x00, 0x03, 0x15,
  /* RLE: 071 Pixels @ 041,157*/ 71, 0x05,
  /* RLE: 004 Pixels @ 112,157*/ 4, 0x09,
  /* RLE: 001 Pixels @ 116,157*/ 1, 0x0A,
  /* RLE: 092 Pixels @ 117,157*/ 92, 0x00,
  /* ABS: 010 Pixels @ 209,157*/ 0, 10, 0x03, 0x04, 0x11, 0x0A, 0x06, 0x12, 0x07, 0x00, 0x03, 0x04,
  /* RLE: 114 Pixels @ 219,157*/ 114, 0x00,
  /* ABS: 005 Pixels @ 333,157*/ 0, 5, 0x0D, 0x06, 0x03, 0x06, 0x06,
  /* RLE: 017 Pixels @ 338,157*/ 17, 0x00,
  /* RLE: 005 Pixels @ 355,157*/ 5, 0x06,
  /* RLE: 028 Pixels @ 360,157*/ 28, 0x01,
  /* RLE: 032 Pixels @ 000,158*/ 32, 0x02,
  /* ABS: 008 Pixels @ 032,158*/ 0, 8, 0x09, 0x0B, 0x03, 0x04, 0x12, 0x06, 0x0A, 0x15,
  /* RLE: 068 Pixels @ 040,158*/ 68, 0x05,
  /* RLE: 004 Pixels @ 108,158*/ 4, 0x09,
  /* RLE: 098 Pixels @ 112,158*/ 98, 0x00,
  /* ABS: 009 Pixels @ 210,158*/ 0, 9, 0x03, 0x0D, 0x06, 0x0E, 0x0F, 0x04, 0x03, 0x03, 0x04,
  /* RLE: 115 Pixels @ 219,158*/ 115, 0x00,
  /* RLE: 001 Pixels @ 334,158*/ 1, 0x0D,
  /* RLE: 004 Pixels @ 335,158*/ 4, 0x06,
  /* RLE: 018 Pixels @ 339,158*/ 18, 0x00,
  /* ABS: 004 Pixels @ 357,158*/ 0, 4, 0x06, 0x03, 0x03, 0x06,
  /* RLE: 027 Pixels @ 361,158*/ 27, 0x01,
  /* RLE: 034 Pixels @ 000,159*/ 34, 0x02,
  /* ABS: 006 Pixels @ 034,159*/ 0, 6, 0x0E, 0x00, 0x03, 0x03, 0x03, 0x19,
  /* RLE: 064 Pixels @ 040,159*/ 64, 0x05,
  /* RLE: 004 Pixels @ 104,159*/ 4, 0x09,
  /* RLE: 103 Pixels @ 108,159*/ 103, 0x00,
  /* ABS: 008 Pixels @ 211,159*/ 0, 8, 0x03, 0x00, 0x00, 0x03, 0x03, 0x0A, 0x03, 0x04,
  /* RLE: 116 Pixels @ 219,159*/ 116, 0x00,
  /* ABS: 005 Pixels @ 335,159*/ 0, 5, 0x0D, 0x0D, 0x06, 0x06, 0x06,
  /* RLE: 018 Pixels @ 340,159*/ 18, 0x00,
  /* ABS: 004 Pixels @ 358,159*/ 0, 4, 0x06, 0x06, 0x03, 0x06,
  /* RLE: 026 Pixels @ 362,159*/ 26, 0x01,
  /* RLE: 036 Pixels @ 000,160*/ 36, 0x02,
  /* RLE: 003 Pixels @ 036,160*/ 3, 0x09,
  /* RLE: 061 Pixels @ 039,160*/ 61, 0x05,
  /* RLE: 004 Pixels @ 100,160*/ 4, 0x09,
  /* RLE: 106 Pixels @ 104,160*/ 106, 0x00,
  /* ABS: 008 Pixels @ 210,160*/ 0, 8, 0x03, 0x00, 0x0B, 0x12, 0x11, 0x07, 0x07, 0x03,
  /* RLE: 119 Pixels @ 218,160*/ 119, 0x00,
  /* ABS: 004 Pixels @ 337,160*/ 0, 4, 0x0D, 0x0D, 0x06, 0x06,
  /* RLE: 018 Pixels @ 341,160*/ 18, 0x00,
  /* RLE: 005 Pixels @ 359,160*/ 5, 0x06,
  /* RLE: 024 Pixels @ 364,160*/ 24, 0x01,
  /* RLE: 038 Pixels @ 000,161*/ 38, 0x02,
  /* RLE: 003 Pixels @ 038,161*/ 3, 0x09,
  /* RLE: 056 Pixels @ 041,161*/ 56, 0x05,
  /* RLE: 003 Pixels @ 097,161*/ 3, 0x09,
  /* RLE: 110 Pixels @ 100,161*/ 110, 0x00,
  /* ABS: 009 Pixels @ 210,161*/ 0, 9, 0x03, 0x12, 0x07, 0x11, 0x12, 0x0E, 0x0D, 0x00, 0x03,
  /* RLE: 119 Pixels @ 219,161*/ 119, 0x00,
  /* ABS: 004 Pixels @ 338,161*/ 0, 4, 0x0D, 0x03, 0x03, 0x06,
  /* RLE: 018 Pixels @ 342,161*/ 18, 0x00,
  /* RLE: 005 Pixels @ 360,161*/ 5, 0x06,
  /* RLE: 023 Pixels @ 365,161*/ 23, 0x01,
  /* RLE: 040 Pixels @ 000,162*/ 40, 0x02,
  /* RLE: 003 Pixels @ 040,162*/ 3, 0x09,
  /* RLE: 050 Pixels @ 043,162*/ 50, 0x05,
  /* RLE: 004 Pixels @ 093,162*/ 4, 0x09,
  /* RLE: 113 Pixels @ 097,162*/ 113, 0x00,
  /* ABS: 002 Pixels @ 210,162*/ 0, 2, 0x03, 0x00,
  /* RLE: 004 Pixels @ 212,162*/ 4, 0x03,
  /* ABS: 004 Pixels @ 216,162*/ 0, 4, 0x0E, 0x0B, 0x03, 0x04,
  /* RLE: 119 Pixels @ 220,162*/ 119, 0x00,
  /* ABS: 004 Pixels @ 339,162*/ 0, 4, 0x0D, 0x06, 0x03, 0x06,
  /* RLE: 018 Pixels @ 343,162*/ 18, 0x00,
  /* RLE: 005 Pixels @ 361,162*/ 5, 0x06,
  /* RLE: 022 Pixels @ 366,162*/ 22, 0x01,
  /* RLE: 043 Pixels @ 000,163*/ 43, 0x02,
  /* RLE: 007 Pixels @ 043,163*/ 7, 0x09,
  /* RLE: 039 Pixels @ 050,163*/ 39, 0x05,
  /* RLE: 004 Pixels @ 089,163*/ 4, 0x09,
  /* RLE: 118 Pixels @ 093,163*/ 118, 0x00,
  /* RLE: 004 Pixels @ 211,163*/ 4, 0x03,
  /* ABS: 005 Pixels @ 215,163*/ 0, 5, 0x0A, 0x11, 0x06, 0x03, 0x04,
  /* RLE: 120 Pixels @ 220,163*/ 120, 0x00,
  /* RLE: 001 Pixels @ 340,163*/ 1, 0x0D,
  /* RLE: 004 Pixels @ 341,163*/ 4, 0x06,
  /* RLE: 018 Pixels @ 345,163*/ 18, 0x00,
  /* ABS: 004 Pixels @ 363,163*/ 0, 4, 0x06, 0x03, 0x06, 0x06,
  /* RLE: 021 Pixels @ 367,163*/ 21, 0x01,
  /* RLE: 044 Pixels @ 000,164*/ 44, 0x02,
  /* ABS: 002 Pixels @ 044,164*/ 0, 2, 0x01, 0x04,
  /* RLE: 004 Pixels @ 046,164*/ 4, 0x00,
  /* RLE: 012 Pixels @ 050,164*/ 12, 0x09,
  /* RLE: 023 Pixels @ 062,164*/ 23, 0x05,
  /* RLE: 004 Pixels @ 085,164*/ 4, 0x09,
  /* RLE: 122 Pixels @ 089,164*/ 122, 0x00,
  /* ABS: 009 Pixels @ 211,164*/ 0, 9, 0x03, 0x12, 0x11, 0x07, 0x07, 0x07, 0x0A, 0x03, 0x03,
  /* RLE: 121 Pixels @ 220,164*/ 121, 0x00,
  /* ABS: 005 Pixels @ 341,164*/ 0, 5, 0x0D, 0x0D, 0x06, 0x06, 0x06,
  /* RLE: 018 Pixels @ 346,164*/ 18, 0x00,
  /* ABS: 004 Pixels @ 364,164*/ 0, 4, 0x06, 0x03, 0x03, 0x06,
  /* RLE: 020 Pixels @ 368,164*/ 20, 0x01,
  /* RLE: 043 Pixels @ 000,165*/ 43, 0x02,
  /* RLE: 005 Pixels @ 043,165*/ 5, 0x04,
  /* RLE: 014 Pixels @ 048,165*/ 14, 0x00,
  /* RLE: 012 Pixels @ 062,165*/ 12, 0x09,
  /* RLE: 007 Pixels @ 074,165*/ 7, 0x05,
  /* RLE: 005 Pixels @ 081,165*/ 5, 0x08,
  /* RLE: 001 Pixels @ 086,165*/ 1, 0x0B,
  /* RLE: 124 Pixels @ 087,165*/ 124, 0x00,
  /* ABS: 010 Pixels @ 211,165*/ 0, 10, 0x03, 0x0D, 0x06, 0x0B, 0x04, 0x00, 0x0D, 0x00, 0x0A, 0x03,
  /* RLE: 121 Pixels @ 221,165*/ 121, 0x00,
  /* ABS: 005 Pixels @ 342,165*/ 0, 5, 0x0D, 0x0D, 0x06, 0x0D, 0x06,
  /* RLE: 018 Pixels @ 347,165*/ 18, 0x00,
  /* RLE: 004 Pixels @ 365,165*/ 4, 0x06,
  /* RLE: 019 Pixels @ 369,165*/ 19, 0x01,
  /* RLE: 042 Pixels @ 000,166*/ 42, 0x02,
  /* ABS: 007 Pixels @ 042,166*/ 0, 7, 0x04, 0x04, 0x03, 0x03, 0x03, 0x04, 0x04,
  /* RLE: 025 Pixels @ 049,166*/ 25, 0x00,
  /* RLE: 003 Pixels @ 074,166*/ 3, 0x09,
  /* RLE: 009 Pixels @ 077,166*/ 9, 0x08,
  /* RLE: 001 Pixels @ 086,166*/ 1, 0x0B,
  /* RLE: 124 Pixels @ 087,166*/ 124, 0x00,
  /* ABS: 011 Pixels @ 211,166*/ 0, 11, 0x03, 0x03, 0x00, 0x0B, 0x06, 0x0F, 0x07, 0x07, 0x07, 0x04, 0x03,
  /* RLE: 122 Pixels @ 222,166*/ 122, 0x00,
  /* ABS: 004 Pixels @ 344,166*/ 0, 4, 0x0D, 0x03, 0x06, 0x06,
  /* RLE: 018 Pixels @ 348,166*/ 18, 0x00,
  /* RLE: 005 Pixels @ 366,166*/ 5, 0x06,
  /* RLE: 017 Pixels @ 371,166*/ 17, 0x01,
  /* RLE: 042 Pixels @ 000,167*/ 42, 0x02,
  /* RLE: 001 Pixels @ 042,167*/ 1, 0x04,
  /* RLE: 005 Pixels @ 043,167*/ 5, 0x03,
  /* RLE: 001 Pixels @ 048,167*/ 1, 0x04,
  /* RLE: 028 Pixels @ 049,167*/ 28, 0x00,
  /* RLE: 001 Pixels @ 077,167*/ 1, 0x0B,
  /* RLE: 009 Pixels @ 078,167*/ 9, 0x08,
  /* RLE: 001 Pixels @ 087,167*/ 1, 0x0B,
  /* RLE: 123 Pixels @ 088,167*/ 123, 0x00,
  /* ABS: 011 Pixels @ 211,167*/ 0, 11, 0x03, 0x12, 0x07, 0x07, 0x11, 0x12, 0x07, 0x0B, 0x12, 0x0E, 0x03,
  /* RLE: 034 Pixels @ 222,167*/ 34, 0x00,
  /* RLE: 005 Pixels @ 256,167*/ 5, 0x04,
  /* RLE: 084 Pixels @ 261,167*/ 84, 0x00,
  /* ABS: 004 Pixels @ 345,167*/ 0, 4, 0x0D, 0x03, 0x03, 0x06,
  /* RLE: 018 Pixels @ 349,167*/ 18, 0x00,
  /* RLE: 005 Pixels @ 367,167*/ 5, 0x06,
  /* RLE: 016 Pixels @ 372,167*/ 16, 0x01,
  /* RLE: 041 Pixels @ 000,168*/ 41, 0x02,
  /* ABS: 002 Pixels @ 041,168*/ 0, 2, 0x04, 0x04,
  /* RLE: 006 Pixels @ 043,168*/ 6, 0x03,
  /* RLE: 029 Pixels @ 049,168*/ 29, 0x00,
  /* RLE: 009 Pixels @ 078,168*/ 9, 0x08,
  /* RLE: 001 Pixels @ 087,168*/ 1, 0x0B,
  /* RLE: 123 Pixels @ 088,168*/ 123, 0x00,
  /* ABS: 011 Pixels @ 211,168*/ 0, 11, 0x03, 0x0A, 0x00, 0x03, 0x03, 0x03, 0x04, 0x00, 0x0A, 0x04, 0x03,
  /* RLE: 033 Pixels @ 222,168*/ 33, 0x00,
  /* ABS: 007 Pixels @ 255,168*/ 0, 7, 0x04, 0x04, 0x03, 0x03, 0x03, 0x04, 0x04,
  /* RLE: 084 Pixels @ 262,168*/ 84, 0x00,
  /* ABS: 004 Pixels @ 346,168*/ 0, 4, 0x0D, 0x06, 0x06, 0x06,
  /* RLE: 018 Pixels @ 350,168*/ 18, 0x00,
  /* ABS: 005 Pixels @ 368,168*/ 0, 5, 0x06, 0x06, 0x03, 0x06, 0x06,
  /* RLE: 015 Pixels @ 373,168*/ 15, 0x01,
  /* RLE: 042 Pixels @ 000,169*/ 42, 0x02,
  /* RLE: 001 Pixels @ 042,169*/ 1, 0x04,
  /* RLE: 006 Pixels @ 043,169*/ 6, 0x03,
  /* RLE: 001 Pixels @ 049,169*/ 1, 0x04,
  /* RLE: 028 Pixels @ 050,169*/ 28, 0x00,
  /* RLE: 001 Pixels @ 078,169*/ 1, 0x0B,
  /* RLE: 009 Pixels @ 079,169*/ 9, 0x08,
  /* RLE: 001 Pixels @ 088,169*/ 1, 0x0B,
  /* RLE: 121 Pixels @ 089,169*/ 121, 0x00,
  /* ABS: 011 Pixels @ 210,169*/ 0, 11, 0x04, 0x00, 0x00, 0x03, 0x03, 0x00, 0x00, 0x0A, 0x03, 0x03, 0x03,
  /* RLE: 034 Pixels @ 221,169*/ 34, 0x00,
  /* RLE: 001 Pixels @ 255,169*/ 1, 0x04,
  /* RLE: 005 Pixels @ 256,169*/ 5, 0x03,
  /* RLE: 001 Pixels @ 261,169*/ 1, 0x04,
  /* RLE: 085 Pixels @ 262,169*/ 85, 0x00,
  /* RLE: 001 Pixels @ 347,169*/ 1, 0x0D,
  /* RLE: 004 Pixels @ 348,169*/ 4, 0x06,
  /* RLE: 018 Pixels @ 352,169*/ 18, 0x00,
  /* ABS: 004 Pixels @ 370,169*/ 0, 4, 0x06, 0x03, 0x03, 0x06,
  /* RLE: 014 Pixels @ 374,169*/ 14, 0x01,
  /* RLE: 042 Pixels @ 000,170*/ 42, 0x02,
  /* RLE: 001 Pixels @ 042,170*/ 1, 0x04,
  /* RLE: 006 Pixels @ 043,170*/ 6, 0x03,
  /* RLE: 001 Pixels @ 049,170*/ 1, 0x04,
  /* RLE: 029 Pixels @ 050,170*/ 29, 0x00,
  /* RLE: 009 Pixels @ 079,170*/ 9, 0x08,
  /* RLE: 001 Pixels @ 088,170*/ 1, 0x0B,
  /* RLE: 117 Pixels @ 089,170*/ 117, 0x00,
  /* RLE: 003 Pixels @ 206,170*/ 3, 0x04,
  /* RLE: 005 Pixels @ 209,170*/ 5, 0x03,
  /* ABS: 006 Pixels @ 214,170*/ 0, 6, 0x0E, 0x07, 0x07, 0x07, 0x06, 0x03,
  /* RLE: 034 Pixels @ 220,170*/ 34, 0x00,
  /* RLE: 001 Pixels @ 254,170*/ 1, 0x04,
  /* RLE: 006 Pixels @ 255,170*/ 6, 0x03,
  /* RLE: 001 Pixels @ 261,170*/ 1, 0x04,
  /* RLE: 086 Pixels @ 262,170*/ 86, 0x00,
  /* ABS: 005 Pixels @ 348,170*/ 0, 5, 0x0D, 0x0D, 0x06, 0x06, 0x06,
  /* RLE: 018 Pixels @ 353,170*/ 18, 0x00,
  /* RLE: 004 Pixels @ 371,170*/ 4, 0x06,
  /* RLE: 013 Pixels @ 375,170*/ 13, 0x01,
  /* RLE: 042 Pixels @ 000,171*/ 42, 0x02,
  /* RLE: 001 Pixels @ 042,171*/ 1, 0x04,
  /* RLE: 006 Pixels @ 043,171*/ 6, 0x03,
  /* RLE: 001 Pixels @ 049,171*/ 1, 0x04,
  /* RLE: 029 Pixels @ 050,171*/ 29, 0x00,
  /* RLE: 001 Pixels @ 079,171*/ 1, 0x0B,
  /* RLE: 009 Pixels @ 080,171*/ 9, 0x08,
  /* RLE: 001 Pixels @ 089,171*/ 1, 0x0B,
  /* RLE: 112 Pixels @ 090,171*/ 112, 0x00,
  /* RLE: 003 Pixels @ 202,171*/ 3, 0x04,
  /* RLE: 008 Pixels @ 205,171*/ 8, 0x03,
  /* ABS: 008 Pixels @ 213,171*/ 0, 8, 0x0B, 0x07, 0x0E, 0x0D, 0x0A, 0x0F, 0x04, 0x03,
  /* RLE: 033 Pixels @ 221,171*/ 33, 0x00,
  /* RLE: 001 Pixels @ 254,171*/ 1, 0x04,
  /* RLE: 006 Pixels @ 255,171*/ 6, 0x03,
  /* RLE: 001 Pixels @ 261,171*/ 1, 0x04,
  /* RLE: 087 Pixels @ 262,171*/ 87, 0x00,
  /* ABS: 005 Pixels @ 349,171*/ 0, 5, 0x0D, 0x0D, 0x03, 0x0D, 0x06,
  /* RLE: 018 Pixels @ 354,171*/ 18, 0x00,
  /* RLE: 005 Pixels @ 372,171*/ 5, 0x06,
  /* RLE: 011 Pixels @ 377,171*/ 11, 0x01,
  /* RLE: 042 Pixels @ 000,172*/ 42, 0x02,
  /* RLE: 001 Pixels @ 042,172*/ 1, 0x04,
  /* RLE: 006 Pixels @ 043,172*/ 6, 0x03,
  /* RLE: 001 Pixels @ 049,172*/ 1, 0x04,
  /* RLE: 030 Pixels @ 050,172*/ 30, 0x00,
  /* RLE: 009 Pixels @ 080,172*/ 9, 0x08,
  /* RLE: 001 Pixels @ 089,172*/ 1, 0x0B,
  /* RLE: 108 Pixels @ 090,172*/ 108, 0x00,
  /* RLE: 003 Pixels @ 198,172*/ 3, 0x04,
  /* RLE: 012 Pixels @ 201,172*/ 12, 0x03,
  /* ABS: 009 Pixels @ 213,172*/ 0, 9, 0x0B, 0x12, 0x03, 0x07, 0x03, 0x06, 0x0B, 0x03, 0x04,
  /* RLE: 032 Pixels @ 222,172*/ 32, 0x00,
  /* RLE: 001 Pixels @ 254,172*/ 1, 0x04,
  /* RLE: 006 Pixels @ 255,172*/ 6, 0x03,
  /* RLE: 001 Pixels @ 261,172*/ 1, 0x04,
  /* RLE: 089 Pixels @ 262,172*/ 89, 0x00,
  /* ABS: 004 Pixels @ 351,172*/ 0, 4, 0x0D, 0x03, 0x06, 0x06,
  /* RLE: 018 Pixels @ 355,172*/ 18, 0x00,
  /* RLE: 005 Pixels @ 373,172*/ 5, 0x06,
  /* RLE: 010 Pixels @ 378,172*/ 10, 0x01,
  /* RLE: 042 Pixels @ 000,173*/ 42, 0x02,
  /* RLE: 001 Pixels @ 042,173*/ 1, 0x04,
  /* RLE: 006 Pixels @ 043,173*/ 6, 0x03,
  /* RLE: 001 Pixels @ 049,173*/ 1, 0x04,
  /* RLE: 030 Pixels @ 050,173*/ 30, 0x00,
  /* RLE: 001 Pixels @ 080,173*/ 1, 0x0B,
  /* RLE: 009 Pixels @ 081,173*/ 9, 0x08,
  /* RLE: 104 Pixels @ 090,173*/ 104, 0x00,
  /* RLE: 003 Pixels @ 194,173*/ 3, 0x04,
  /* RLE: 016 Pixels @ 197,173*/ 16, 0x03,
  /* ABS: 009 Pixels @ 213,173*/ 0, 9, 0x04, 0x11, 0x0A, 0x06, 0x12, 0x07, 0x0A, 0x03, 0x04,
  /* RLE: 032 Pixels @ 222,173*/ 32, 0x00,
  /* RLE: 001 Pixels @ 254,173*/ 1, 0x04,
  /* RLE: 006 Pixels @ 255,173*/ 6, 0x03,
  /* RLE: 001 Pixels @ 261,173*/ 1, 0x04,
  /* RLE: 090 Pixels @ 262,173*/ 90, 0x00,
  /* ABS: 004 Pixels @ 352,173*/ 0, 4, 0x0D, 0x03, 0x06, 0x06,
  /* RLE: 018 Pixels @ 356,173*/ 18, 0x00,
  /* ABS: 005 Pixels @ 374,173*/ 0, 5, 0x06, 0x06, 0x03, 0x06, 0x06,
  /* RLE: 009 Pixels @ 379,173*/ 9, 0x01,
  /* RLE: 042 Pixels @ 000,174*/ 42, 0x02,
  /* RLE: 001 Pixels @ 042,174*/ 1, 0x04,
  /* RLE: 007 Pixels @ 043,174*/ 7, 0x03,
  /* RLE: 031 Pixels @ 050,174*/ 31, 0x00,
  /* RLE: 009 Pixels @ 081,174*/ 9, 0x08,
  /* RLE: 001 Pixels @ 090,174*/ 1, 0x0B,
  /* RLE: 099 Pixels @ 091,174*/ 99, 0x00,
  /* RLE: 003 Pixels @ 190,174*/ 3, 0x04,
  /* RLE: 021 Pixels @ 193,174*/ 21, 0x03,
  /* ABS: 005 Pixels @ 214,174*/ 0, 5, 0x0D, 0x06, 0x0E, 0x0F, 0x04,
  /* RLE: 004 Pixels @ 219,174*/ 4, 0x03,
  /* RLE: 031 Pixels @ 223,174*/ 31, 0x00,
  /* RLE: 001 Pixels @ 254,174*/ 1, 0x04,
  /* RLE: 006 Pixels @ 255,174*/ 6, 0x03,
  /* RLE: 001 Pixels @ 261,174*/ 1, 0x04,
  /* RLE: 091 Pixels @ 262,174*/ 91, 0x00,
  /* ABS: 004 Pixels @ 353,174*/ 0, 4, 0x0D, 0x06, 0x06, 0x06,
  /* RLE: 019 Pixels @ 357,174*/ 19, 0x00,
  /* ABS: 004 Pixels @ 376,174*/ 0, 4, 0x06, 0x03, 0x06, 0x06,
  /* RLE: 008 Pixels @ 380,174*/ 8, 0x01,
  /* RLE: 043 Pixels @ 000,175*/ 43, 0x02,
  /* RLE: 001 Pixels @ 043,175*/ 1, 0x04,
  /* RLE: 006 Pixels @ 044,175*/ 6, 0x03,
  /* RLE: 001 Pixels @ 050,175*/ 1, 0x04,
  /* RLE: 030 Pixels @ 051,175*/ 30, 0x00,
  /* RLE: 001 Pixels @ 081,175*/ 1, 0x0B,
  /* RLE: 009 Pixels @ 082,175*/ 9, 0x08,
  /* RLE: 095 Pixels @ 091,175*/ 95, 0x00,
  /* RLE: 004 Pixels @ 186,175*/ 4, 0x04,
  /* RLE: 025 Pixels @ 190,175*/ 25, 0x03,
  /* ABS: 008 Pixels @ 215,175*/ 0, 8, 0x00, 0x00, 0x03, 0x03, 0x00, 0x0B, 0x12, 0x03,
  /* RLE: 031 Pixels @ 223,175*/ 31, 0x00,
  /* RLE: 001 Pixels @ 254,175*/ 1, 0x04,
  /* RLE: 006 Pixels @ 255,175*/ 6, 0x03,
  /* RLE: 001 Pixels @ 261,175*/ 1, 0x04,
  /* RLE: 092 Pixels @ 262,175*/ 92, 0x00,
  /* ABS: 004 Pixels @ 354,175*/ 0, 4, 0x0D, 0x06, 0x06, 0x06,
  /* RLE: 019 Pixels @ 358,175*/ 19, 0x00,
  /* ABS: 004 Pixels @ 377,175*/ 0, 4, 0x06, 0x03, 0x06, 0x06,
  /* RLE: 007 Pixels @ 381,175*/ 7, 0x01,
  /* RLE: 043 Pixels @ 000,176*/ 43, 0x02,
  /* RLE: 001 Pixels @ 043,176*/ 1, 0x04,
  /* RLE: 006 Pixels @ 044,176*/ 6, 0x03,
  /* RLE: 001 Pixels @ 050,176*/ 1, 0x04,
  /* RLE: 031 Pixels @ 051,176*/ 31, 0x00,
  /* RLE: 009 Pixels @ 082,176*/ 9, 0x08,
  /* RLE: 001 Pixels @ 091,176*/ 1, 0x0B,
  /* RLE: 090 Pixels @ 092,176*/ 90, 0x00,
  /* RLE: 004 Pixels @ 182,176*/ 4, 0x04,
  /* RLE: 024 Pixels @ 186,176*/ 24, 0x03,
  /* RLE: 003 Pixels @ 210,176*/ 3, 0x04,
  /* ABS: 009 Pixels @ 213,176*/ 0, 9, 0x03, 0x00, 0x0B, 0x12, 0x11, 0x07, 0x07, 0x07, 0x11,
  /* RLE: 032 Pixels @ 222,176*/ 32, 0x00,
  /* RLE: 001 Pixels @ 254,176*/ 1, 0x04,
  /* RLE: 006 Pixels @ 255,176*/ 6, 0x03,
  /* RLE: 001 Pixels @ 261,176*/ 1, 0x04,
  /* RLE: 093 Pixels @ 262,176*/ 93, 0x00,
  /* ABS: 005 Pixels @ 355,176*/ 0, 5, 0x0D, 0x0D, 0x03, 0x06, 0x06,
  /* RLE: 018 Pixels @ 360,176*/ 18, 0x00,
  /* RLE: 004 Pixels @ 378,176*/ 4, 0x06,
  /* RLE: 006 Pixels @ 382,176*/ 6, 0x01,
  /* RLE: 043 Pixels @ 000,177*/ 43, 0x02,
  /* RLE: 001 Pixels @ 043,177*/ 1, 0x04,
  /* RLE: 006 Pixels @ 044,177*/ 6, 0x03,
  /* RLE: 001 Pixels @ 050,177*/ 1, 0x04,
  /* RLE: 031 Pixels @ 051,177*/ 31, 0x00,
  /* RLE: 001 Pixels @ 082,177*/ 1, 0x0B,
  /* RLE: 009 Pixels @ 083,177*/ 9, 0x08,
  /* RLE: 086 Pixels @ 092,177*/ 86, 0x00,
  /* RLE: 004 Pixels @ 178,177*/ 4, 0x04,
  /* RLE: 024 Pixels @ 182,177*/ 24, 0x03,
  /* RLE: 004 Pixels @ 206,177*/ 4, 0x04,
  /* RLE: 003 Pixels @ 210,177*/ 3, 0x00,
  /* ABS: 010 Pixels @ 213,177*/ 0, 10, 0x03, 0x12, 0x07, 0x11, 0x12, 0x0B, 0x0A, 0x03, 0x03, 0x03,
  /* RLE: 030 Pixels @ 223,177*/ 30, 0x00,
  /* RLE: 001 Pixels @ 253,177*/ 1, 0x04,
  /* RLE: 007 Pixels @ 254,177*/ 7, 0x03,
  /* RLE: 095 Pixels @ 261,177*/ 95, 0x00,
  /* ABS: 005 Pixels @ 356,177*/ 0, 5, 0x0D, 0x06, 0x03, 0x06, 0x06,
  /* RLE: 018 Pixels @ 361,177*/ 18, 0x00,
  /* RLE: 005 Pixels @ 379,177*/ 5, 0x06,
  /* RLE: 004 Pixels @ 384,177*/ 4, 0x01,
  /* RLE: 043 Pixels @ 000,178*/ 43, 0x02,
  /* RLE: 001 Pixels @ 043,178*/ 1, 0x04,
  /* RLE: 006 Pixels @ 044,178*/ 6, 0x03,
  /* RLE: 001 Pixels @ 050,178*/ 1, 0x04,
  /* RLE: 032 Pixels @ 051,178*/ 32, 0x00,
  /* RLE: 009 Pixels @ 083,178*/ 9, 0x08,
  /* RLE: 001 Pixels @ 092,178*/ 1, 0x0B,
  /* RLE: 081 Pixels @ 093,178*/ 81, 0x00,
  /* RLE: 004 Pixels @ 174,178*/ 4, 0x04,
  /* RLE: 024 Pixels @ 178,178*/ 24, 0x03,
  /* RLE: 004 Pixels @ 202,178*/ 4, 0x04,
  /* RLE: 007 Pixels @ 206,178*/ 7, 0x00,
  /* ABS: 011 Pixels @ 213,178*/ 0, 11, 0x03, 0x00, 0x03, 0x03, 0x03, 0x00, 0x00, 0x03, 0x03, 0x03, 0x04,
  /* RLE: 029 Pixels @ 224,178*/ 29, 0x00,
  /* RLE: 001 Pixels @ 253,178*/ 1, 0x04,
  /* RLE: 006 Pixels @ 254,178*/ 6, 0x03,
  /* RLE: 001 Pixels @ 260,178*/ 1, 0x04,
  /* RLE: 096 Pixels @ 261,178*/ 96, 0x00,
  /* ABS: 005 Pixels @ 357,178*/ 0, 5, 0x0D, 0x0D, 0x03, 0x06, 0x06,
  /* RLE: 018 Pixels @ 362,178*/ 18, 0x00,
  /* ABS: 008 Pixels @ 380,178*/ 0, 8, 0x06, 0x06, 0x03, 0x06, 0x06, 0x01, 0x01, 0x01,
  /* RLE: 043 Pixels @ 000,179*/ 43, 0x02,
  /* RLE: 001 Pixels @ 043,179*/ 1, 0x04,
  /* RLE: 006 Pixels @ 044,179*/ 6, 0x03,
  /* RLE: 001 Pixels @ 050,179*/ 1, 0x04,
  /* RLE: 032 Pixels @ 051,179*/ 32, 0x00,
  /* RLE: 001 Pixels @ 083,179*/ 1, 0x0B,
  /* RLE: 009 Pixels @ 084,179*/ 9, 0x08,
  /* RLE: 077 Pixels @ 093,179*/ 77, 0x00,
  /* RLE: 004 Pixels @ 170,179*/ 4, 0x04,
  /* RLE: 024 Pixels @ 174,179*/ 24, 0x03,
  /* RLE: 004 Pixels @ 198,179*/ 4, 0x04,
  /* RLE: 013 Pixels @ 202,179*/ 13, 0x00,
  /* ABS: 009 Pixels @ 215,179*/ 0, 9, 0x03, 0x0E, 0x07, 0x07, 0x07, 0x06, 0x03, 0x03, 0x04,
  /* RLE: 029 Pixels @ 224,179*/ 29, 0x00,
  /* RLE: 001 Pixels @ 253,179*/ 1, 0x04,
  /* RLE: 006 Pixels @ 254,179*/ 6, 0x03,
  /* RLE: 001 Pixels @ 260,179*/ 1, 0x04,
  /* RLE: 098 Pixels @ 261,179*/ 98, 0x00,
  /* ABS: 004 Pixels @ 359,179*/ 0, 4, 0x0D, 0x06, 0x06, 0x06,
  /* RLE: 018 Pixels @ 363,179*/ 18, 0x00,
  /* ABS: 007 Pixels @ 381,179*/ 0, 7, 0x06, 0x06, 0x03, 0x06, 0x06, 0x01, 0x01,
  /* RLE: 043 Pixels @ 000,180*/ 43, 0x02,
  /* RLE: 001 Pixels @ 043,180*/ 1, 0x04,
  /* RLE: 007 Pixels @ 044,180*/ 7, 0x03,
  /* RLE: 032 Pixels @ 051,180*/ 32, 0x00,
  /* RLE: 001 Pixels @ 083,180*/ 1, 0x0B,
  /* RLE: 009 Pixels @ 084,180*/ 9, 0x08,
  /* RLE: 001 Pixels @ 093,180*/ 1, 0x0B,
  /* RLE: 072 Pixels @ 094,180*/ 72, 0x00,
  /* RLE: 004 Pixels @ 166,180*/ 4, 0x04,
  /* RLE: 024 Pixels @ 170,180*/ 24, 0x03,
  /* RLE: 004 Pixels @ 194,180*/ 4, 0x04,
  /* RLE: 016 Pixels @ 198,180*/ 16, 0x00,
  /* ABS: 010 Pixels @ 214,180*/ 0, 10, 0x03, 0x04, 0x07, 0x12, 0x04, 0x04, 0x0F, 0x04, 0x03, 0x04,
  /* RLE: 029 Pixels @ 224,180*/ 29, 0x00,
  /* RLE: 001 Pixels @ 253,180*/ 1, 0x04,
  /* RLE: 006 Pixels @ 254,180*/ 6, 0x03,
  /* RLE: 001 Pixels @ 260,180*/ 1, 0x04,
  /* RLE: 099 Pixels @ 261,180*/ 99, 0x00,
  /* ABS: 004 Pixels @ 360,180*/ 0, 4, 0x0D, 0x0D, 0x06, 0x06,
  /* RLE: 019 Pixels @ 364,180*/ 19, 0x00,
  /* ABS: 005 Pixels @ 383,180*/ 0, 5, 0x06, 0x03, 0x06, 0x06, 0x01,
  /* RLE: 044 Pixels @ 000,181*/ 44, 0x02,
  /* RLE: 001 Pixels @ 044,181*/ 1, 0x04,
  /* RLE: 006 Pixels @ 045,181*/ 6, 0x03,
  /* RLE: 001 Pixels @ 051,181*/ 1, 0x04,
  /* RLE: 032 Pixels @ 052,181*/ 32, 0x00,
  /* RLE: 001 Pixels @ 084,181*/ 1, 0x0B,
  /* RLE: 009 Pixels @ 085,181*/ 9, 0x08,
  /* RLE: 069 Pixels @ 094,181*/ 69, 0x00,
  /* RLE: 003 Pixels @ 163,181*/ 3, 0x04,
  /* RLE: 025 Pixels @ 166,181*/ 25, 0x03,
  /* RLE: 003 Pixels @ 191,181*/ 3, 0x04,
  /* RLE: 020 Pixels @ 194,181*/ 20, 0x00,
  /* ABS: 011 Pixels @ 214,181*/ 0, 11, 0x03, 0x0B, 0x0D, 0x03, 0x03, 0x03, 0x06, 0x0B, 0x03, 0x03, 0x03,
  /* RLE: 028 Pixels @ 225,181*/ 28, 0x00,
  /* RLE: 001 Pixels @ 253,181*/ 1, 0x04,
  /* RLE: 006 Pixels @ 254,181*/ 6, 0x03,
  /* RLE: 001 Pixels @ 260,181*/ 1, 0x04,
  /* RLE: 100 Pixels @ 261,181*/ 100, 0x00,
  /* ABS: 004 Pixels @ 361,181*/ 0, 4, 0x0D, 0x06, 0x03, 0x06,
  /* RLE: 019 Pixels @ 365,181*/ 19, 0x00,
  /* RLE: 004 Pixels @ 384,181*/ 4, 0x06,
  /* RLE: 044 Pixels @ 000,182*/ 44, 0x02,
  /* RLE: 001 Pixels @ 044,182*/ 1, 0x04,
  /* RLE: 006 Pixels @ 045,182*/ 6, 0x03,
  /* RLE: 001 Pixels @ 051,182*/ 1, 0x04,
  /* RLE: 032 Pixels @ 052,182*/ 32, 0x00,
  /* RLE: 001 Pixels @ 084,182*/ 1, 0x0B,
  /* RLE: 009 Pixels @ 085,182*/ 9, 0x08,
  /* RLE: 001 Pixels @ 094,182*/ 1, 0x0B,
  /* RLE: 067 Pixels @ 095,182*/ 67, 0x00,
  /* ABS: 002 Pixels @ 162,182*/ 0, 2, 0x04, 0x04,
  /* RLE: 023 Pixels @ 164,182*/ 23, 0x03,
  /* RLE: 003 Pixels @ 187,182*/ 3, 0x04,
  /* RLE: 024 Pixels @ 190,182*/ 24, 0x00,
  /* ABS: 011 Pixels @ 214,182*/ 0, 11, 0x03, 0x0A, 0x11, 0x00, 0x03, 0x0A, 0x0F, 0x12, 0x0D, 0x0F, 0x03,
  /* RLE: 028 Pixels @ 225,182*/ 28, 0x00,
  /* RLE: 001 Pixels @ 253,182*/ 1, 0x04,
  /* RLE: 006 Pixels @ 254,182*/ 6, 0x03,
  /* RLE: 001 Pixels @ 260,182*/ 1, 0x04,
  /* RLE: 101 Pixels @ 261,182*/ 101, 0x00,
  /* ABS: 005 Pixels @ 362,182*/ 0, 5, 0x0D, 0x06, 0x03, 0x06, 0x06,
  /* RLE: 018 Pixels @ 367,182*/ 18, 0x00,
  /* RLE: 003 Pixels @ 385,182*/ 3, 0x06,
  /* RLE: 044 Pixels @ 000,183*/ 44, 0x02,
  /* RLE: 001 Pixels @ 044,183*/ 1, 0x04,
  /* RLE: 006 Pixels @ 045,183*/ 6, 0x03,
  /* RLE: 001 Pixels @ 051,183*/ 1, 0x04,
  /* RLE: 033 Pixels @ 052,183*/ 33, 0x00,
  /* RLE: 001 Pixels @ 085,183*/ 1, 0x0B,
  /* RLE: 009 Pixels @ 086,183*/ 9, 0x08,
  /* RLE: 065 Pixels @ 095,183*/ 65, 0x00,
  /* RLE: 003 Pixels @ 160,183*/ 3, 0x04,
  /* RLE: 020 Pixels @ 163,183*/ 20, 0x03,
  /* RLE: 003 Pixels @ 183,183*/ 3, 0x04,
  /* RLE: 029 Pixels @ 186,183*/ 29, 0x00,
  /* ABS: 002 Pixels @ 215,183*/ 0, 2, 0x03, 0x0D,
  /* RLE: 004 Pixels @ 217,183*/ 4, 0x07,
  /* ABS: 004 Pixels @ 221,183*/ 0, 4, 0x0F, 0x06, 0x0B, 0x03,
  /* RLE: 028 Pixels @ 225,183*/ 28, 0x00,
  /* RLE: 001 Pixels @ 253,183*/ 1, 0x04,
  /* RLE: 006 Pixels @ 254,183*/ 6, 0x03,
  /* RLE: 001 Pixels @ 260,183*/ 1, 0x04,
  /* RLE: 102 Pixels @ 261,183*/ 102, 0x00,
  /* ABS: 005 Pixels @ 363,183*/ 0, 5, 0x0D, 0x06, 0x03, 0x06, 0x06,
  /* RLE: 018 Pixels @ 368,183*/ 18, 0x00,
  /* ABS: 002 Pixels @ 386,183*/ 0, 2, 0x06, 0x06,
  /* RLE: 044 Pixels @ 000,184*/ 44, 0x02,
  /* RLE: 001 Pixels @ 044,184*/ 1, 0x04,
  /* RLE: 006 Pixels @ 045,184*/ 6, 0x03,
  /* RLE: 001 Pixels @ 051,184*/ 1, 0x04,
  /* RLE: 033 Pixels @ 052,184*/ 33, 0x00,
  /* RLE: 001 Pixels @ 085,184*/ 1, 0x0B,
  /* RLE: 009 Pixels @ 086,184*/ 9, 0x08,
  /* RLE: 001 Pixels @ 095,184*/ 1, 0x0B,
  /* RLE: 063 Pixels @ 096,184*/ 63, 0x00,
  /* ABS: 002 Pixels @ 159,184*/ 0, 2, 0x04, 0x04,
  /* RLE: 018 Pixels @ 161,184*/ 18, 0x03,
  /* RLE: 003 Pixels @ 179,184*/ 3, 0x04,
  /* RLE: 033 Pixels @ 182,184*/ 33, 0x00,
  /* ABS: 009 Pixels @ 215,184*/ 0, 9, 0x03, 0x11, 0x06, 0x0E, 0x04, 0x00, 0x03, 0x03, 0x03,
  /* RLE: 028 Pixels @ 224,184*/ 28, 0x00,
  /* RLE: 001 Pixels @ 252,184*/ 1, 0x04,
  /* RLE: 007 Pixels @ 253,184*/ 7, 0x03,
  /* RLE: 104 Pixels @ 260,184*/ 104, 0x00,
  /* RLE: 003 Pixels @ 364,184*/ 3, 0x0D,
  /* ABS: 002 Pixels @ 367,184*/ 0, 2, 0x06, 0x06,
  /* RLE: 018 Pixels @ 369,184*/ 18, 0x00,
  /* RLE: 001 Pixels @ 387,184*/ 1, 0x06,
  /* RLE: 044 Pixels @ 000,185*/ 44, 0x02,
  /* RLE: 001 Pixels @ 044,185*/ 1, 0x04,
  /* RLE: 006 Pixels @ 045,185*/ 6, 0x03,
  /* RLE: 001 Pixels @ 051,185*/ 1, 0x04,
  /* RLE: 034 Pixels @ 052,185*/ 34, 0x00,
  /* RLE: 001 Pixels @ 086,185*/ 1, 0x0B,
  /* RLE: 009 Pixels @ 087,185*/ 9, 0x08,
  /* RLE: 061 Pixels @ 096,185*/ 61, 0x00,
  /* ABS: 002 Pixels @ 157,185*/ 0, 2, 0x04, 0x04,
  /* RLE: 016 Pixels @ 159,185*/ 16, 0x03,
  /* RLE: 003 Pixels @ 175,185*/ 3, 0x04,
  /* RLE: 037 Pixels @ 178,185*/ 37, 0x00,
  /* RLE: 010 Pixels @ 215,185*/ 10, 0x03,
  /* RLE: 001 Pixels @ 225,185*/ 1, 0x04,
  /* RLE: 026 Pixels @ 226,185*/ 26, 0x00,
  /* RLE: 001 Pixels @ 252,185*/ 1, 0x04,
  /* RLE: 006 Pixels @ 253,185*/ 6, 0x03,
  /* RLE: 001 Pixels @ 259,185*/ 1, 0x04,
  /* RLE: 106 Pixels @ 260,185*/ 106, 0x00,
  /* ABS: 004 Pixels @ 366,185*/ 0, 4, 0x0D, 0x06, 0x06, 0x06,
  /* RLE: 018 Pixels @ 370,185*/ 18, 0x00,
  /* RLE: 044 Pixels @ 000,186*/ 44, 0x02,
  /* RLE: 001 Pixels @ 044,186*/ 1, 0x04,
  /* RLE: 007 Pixels @ 045,186*/ 7, 0x03,
  /* RLE: 034 Pixels @ 052,186*/ 34, 0x00,
  /* RLE: 001 Pixels @ 086,186*/ 1, 0x0B,
  /* RLE: 009 Pixels @ 087,186*/ 9, 0x08,
  /* RLE: 001 Pixels @ 096,186*/ 1, 0x0B,
  /* RLE: 059 Pixels @ 097,186*/ 59, 0x00,
  /* ABS: 002 Pixels @ 156,186*/ 0, 2, 0x04, 0x04,
  /* RLE: 013 Pixels @ 158,186*/ 13, 0x03,
  /* RLE: 003 Pixels @ 171,186*/ 3, 0x04,
  /* RLE: 044 Pixels @ 174,186*/ 44, 0x00,
  /* RLE: 001 Pixels @ 218,186*/ 1, 0x04,
  /* RLE: 006 Pixels @ 219,186*/ 6, 0x03,
  /* RLE: 001 Pixels @ 225,186*/ 1, 0x04,
  /* RLE: 026 Pixels @ 226,186*/ 26, 0x00,
  /* RLE: 001 Pixels @ 252,186*/ 1, 0x04,
  /* RLE: 006 Pixels @ 253,186*/ 6, 0x03,
  /* RLE: 001 Pixels @ 259,186*/ 1, 0x04,
  /* RLE: 107 Pixels @ 260,186*/ 107, 0x00,
  /* ABS: 004 Pixels @ 367,186*/ 0, 4, 0x0D, 0x06, 0x03, 0x06,
  /* RLE: 017 Pixels @ 371,186*/ 17, 0x00,
  /* RLE: 045 Pixels @ 000,187*/ 45, 0x02,
  /* RLE: 001 Pixels @ 045,187*/ 1, 0x04,
  /* RLE: 006 Pixels @ 046,187*/ 6, 0x03,
  /* RLE: 001 Pixels @ 052,187*/ 1, 0x04,
  /* RLE: 034 Pixels @ 053,187*/ 34, 0x00,
  /* RLE: 001 Pixels @ 087,187*/ 1, 0x0B,
  /* RLE: 009 Pixels @ 088,187*/ 9, 0x08,
  /* RLE: 057 Pixels @ 097,187*/ 57, 0x00,
  /* ABS: 002 Pixels @ 154,187*/ 0, 2, 0x04, 0x04,
  /* RLE: 011 Pixels @ 156,187*/ 11, 0x03,
  /* RLE: 003 Pixels @ 167,187*/ 3, 0x04,
  /* RLE: 048 Pixels @ 170,187*/ 48, 0x00,
  /* RLE: 001 Pixels @ 218,187*/ 1, 0x04,
  /* RLE: 006 Pixels @ 219,187*/ 6, 0x03,
  /* RLE: 001 Pixels @ 225,187*/ 1, 0x04,
  /* RLE: 026 Pixels @ 226,187*/ 26, 0x00,
  /* RLE: 001 Pixels @ 252,187*/ 1, 0x04,
  /* RLE: 006 Pixels @ 253,187*/ 6, 0x03,
  /* RLE: 001 Pixels @ 259,187*/ 1, 0x04,
  /* RLE: 108 Pixels @ 260,187*/ 108, 0x00,
  /* ABS: 004 Pixels @ 368,187*/ 0, 4, 0x0D, 0x06, 0x03, 0x06,
  /* RLE: 016 Pixels @ 372,187*/ 16, 0x00,
  /* RLE: 045 Pixels @ 000,188*/ 45, 0x02,
  /* RLE: 001 Pixels @ 045,188*/ 1, 0x04,
  /* RLE: 006 Pixels @ 046,188*/ 6, 0x03,
  /* RLE: 001 Pixels @ 052,188*/ 1, 0x04,
  /* RLE: 034 Pixels @ 053,188*/ 34, 0x00,
  /* RLE: 001 Pixels @ 087,188*/ 1, 0x0B,
  /* RLE: 009 Pixels @ 088,188*/ 9, 0x08,
  /* RLE: 001 Pixels @ 097,188*/ 1, 0x0B,
  /* RLE: 054 Pixels @ 098,188*/ 54, 0x00,
  /* RLE: 003 Pixels @ 152,188*/ 3, 0x04,
  /* RLE: 011 Pixels @ 155,188*/ 11, 0x03,
  /* ABS: 002 Pixels @ 166,188*/ 0, 2, 0x04, 0x04,
  /* RLE: 050 Pixels @ 168,188*/ 50, 0x00,
  /* RLE: 001 Pixels @ 218,188*/ 1, 0x04,
  /* RLE: 007 Pixels @ 219,188*/ 7, 0x03,
  /* RLE: 026 Pixels @ 226,188*/ 26, 0x00,
  /* RLE: 001 Pixels @ 252,188*/ 1, 0x04,
  /* RLE: 006 Pixels @ 253,188*/ 6, 0x03,
  /* RLE: 001 Pixels @ 259,188*/ 1, 0x04,
  /* RLE: 109 Pixels @ 260,188*/ 109, 0x00,
  /* ABS: 005 Pixels @ 369,188*/ 0, 5, 0x0D, 0x0D, 0x03, 0x06, 0x06,
  /* RLE: 014 Pixels @ 374,188*/ 14, 0x00,
  /* RLE: 045 Pixels @ 000,189*/ 45, 0x02,
  /* RLE: 001 Pixels @ 045,189*/ 1, 0x04,
  /* RLE: 006 Pixels @ 046,189*/ 6, 0x03,
  /* RLE: 001 Pixels @ 052,189*/ 1, 0x04,
  /* RLE: 035 Pixels @ 053,189*/ 35, 0x00,
  /* RLE: 001 Pixels @ 088,189*/ 1, 0x0B,
  /* RLE: 008 Pixels @ 089,189*/ 8, 0x08,
  /* RLE: 001 Pixels @ 097,189*/ 1, 0x0B,
  /* RLE: 054 Pixels @ 098,189*/ 54, 0x00,
  /* ABS: 002 Pixels @ 152,189*/ 0, 2, 0x04, 0x04,
  /* RLE: 010 Pixels @ 154,189*/ 10, 0x03,
  /* ABS: 002 Pixels @ 164,189*/ 0, 2, 0x04, 0x04,
  /* RLE: 053 Pixels @ 166,189*/ 53, 0x00,
  /* RLE: 001 Pixels @ 219,189*/ 1, 0x04,
  /* RLE: 006 Pixels @ 220,189*/ 6, 0x03,
  /* RLE: 001 Pixels @ 226,189*/ 1, 0x04,
  /* RLE: 025 Pixels @ 227,189*/ 25, 0x00,
  /* RLE: 001 Pixels @ 252,189*/ 1, 0x04,
  /* RLE: 006 Pixels @ 253,189*/ 6, 0x03,
  /* RLE: 001 Pixels @ 259,189*/ 1, 0x04,
  /* RLE: 110 Pixels @ 260,189*/ 110, 0x00,
  /* RLE: 001 Pixels @ 370,189*/ 1, 0x0D,
  /* RLE: 004 Pixels @ 371,189*/ 4, 0x06,
  /* RLE: 013 Pixels @ 375,189*/ 13, 0x00,
  /* RLE: 045 Pixels @ 000,190*/ 45, 0x02,
  /* RLE: 001 Pixels @ 045,190*/ 1, 0x04,
  /* RLE: 006 Pixels @ 046,190*/ 6, 0x03,
  /* RLE: 001 Pixels @ 052,190*/ 1, 0x04,
  /* RLE: 035 Pixels @ 053,190*/ 35, 0x00,
  /* RLE: 001 Pixels @ 088,190*/ 1, 0x0B,
  /* RLE: 009 Pixels @ 089,190*/ 9, 0x08,
  /* RLE: 001 Pixels @ 098,190*/ 1, 0x0B,
  /* RLE: 053 Pixels @ 099,190*/ 53, 0x00,
  /* RLE: 001 Pixels @ 152,190*/ 1, 0x04,
  /* RLE: 009 Pixels @ 153,190*/ 9, 0x03,
  /* RLE: 003 Pixels @ 162,190*/ 3, 0x04,
  /* RLE: 054 Pixels @ 165,190*/ 54, 0x00,
  /* RLE: 001 Pixels @ 219,190*/ 1, 0x04,
  /* RLE: 006 Pixels @ 220,190*/ 6, 0x03,
  /* RLE: 001 Pixels @ 226,190*/ 1, 0x04,
  /* RLE: 024 Pixels @ 227,190*/ 24, 0x00,
  /* ABS: 002 Pixels @ 251,190*/ 0, 2, 0x04, 0x04,
  /* RLE: 006 Pixels @ 253,190*/ 6, 0x03,
  /* RLE: 001 Pixels @ 259,190*/ 1, 0x04,
  /* RLE: 111 Pixels @ 260,190*/ 111, 0x00,
  /* ABS: 005 Pixels @ 371,190*/ 0, 5, 0x0D, 0x0D, 0x06, 0x06, 0x06,
  /* RLE: 012 Pixels @ 376,190*/ 12, 0x00,
  /* RLE: 045 Pixels @ 000,191*/ 45, 0x02,
  /* RLE: 001 Pixels @ 045,191*/ 1, 0x04,
  /* RLE: 006 Pixels @ 046,191*/ 6, 0x03,
  /* RLE: 001 Pixels @ 052,191*/ 1, 0x04,
  /* RLE: 036 Pixels @ 053,191*/ 36, 0x00,
  /* RLE: 001 Pixels @ 089,191*/ 1, 0x0B,
  /* RLE: 008 Pixels @ 090,191*/ 8, 0x08,
  /* RLE: 001 Pixels @ 098,191*/ 1, 0x0B,
  /* RLE: 051 Pixels @ 099,191*/ 51, 0x00,
  /* RLE: 011 Pixels @ 150,191*/ 11, 0x03,
  /* ABS: 002 Pixels @ 161,191*/ 0, 2, 0x04, 0x04,
  /* RLE: 056 Pixels @ 163,191*/ 56, 0x00,
  /* RLE: 001 Pixels @ 219,191*/ 1, 0x04,
  /* RLE: 006 Pixels @ 220,191*/ 6, 0x03,
  /* RLE: 001 Pixels @ 226,191*/ 1, 0x04,
  /* RLE: 024 Pixels @ 227,191*/ 24, 0x00,
  /* RLE: 001 Pixels @ 251,191*/ 1, 0x04,
  /* RLE: 007 Pixels @ 252,191*/ 7, 0x03,
  /* RLE: 114 Pixels @ 259,191*/ 114, 0x00,
  /* ABS: 004 Pixels @ 373,191*/ 0, 4, 0x0D, 0x0D, 0x06, 0x06,
  /* RLE: 011 Pixels @ 377,191*/ 11, 0x00,
  /* RLE: 045 Pixels @ 000,192*/ 45, 0x02,
  /* RLE: 001 Pixels @ 045,192*/ 1, 0x04,
  /* RLE: 007 Pixels @ 046,192*/ 7, 0x03,
  /* RLE: 036 Pixels @ 053,192*/ 36, 0x00,
  /* RLE: 001 Pixels @ 089,192*/ 1, 0x0B,
  /* RLE: 009 Pixels @ 090,192*/ 9, 0x08,
  /* RLE: 050 Pixels @ 099,192*/ 50, 0x00,
  /* ABS: 006 Pixels @ 149,192*/ 0, 6, 0x03, 0x00, 0x0B, 0x0B, 0x04, 0x00,
  /* RLE: 004 Pixels @ 155,192*/ 4, 0x03,
  /* RLE: 003 Pixels @ 159,192*/ 3, 0x04,
  /* RLE: 058 Pixels @ 162,192*/ 58, 0x00,
  /* RLE: 007 Pixels @ 220,192*/ 7, 0x03,
  /* RLE: 001 Pixels @ 227,192*/ 1, 0x04,
  /* RLE: 022 Pixels @ 228,192*/ 22, 0x00,
  /* ABS: 002 Pixels @ 250,192*/ 0, 2, 0x04, 0x04,
  /* RLE: 006 Pixels @ 252,192*/ 6, 0x03,
  /* RLE: 001 Pixels @ 258,192*/ 1, 0x04,
  /* RLE: 115 Pixels @ 259,192*/ 115, 0x00,
  /* ABS: 004 Pixels @ 374,192*/ 0, 4, 0x0D, 0x03, 0x03, 0x06,
  /* RLE: 010 Pixels @ 378,192*/ 10, 0x00,
  /* RLE: 046 Pixels @ 000,193*/ 46, 0x02,
  /* RLE: 001 Pixels @ 046,193*/ 1, 0x04,
  /* RLE: 006 Pixels @ 047,193*/ 6, 0x03,
  /* RLE: 001 Pixels @ 053,193*/ 1, 0x04,
  /* RLE: 036 Pixels @ 054,193*/ 36, 0x00,
  /* RLE: 001 Pixels @ 090,193*/ 1, 0x0B,
  /* RLE: 008 Pixels @ 091,193*/ 8, 0x08,
  /* RLE: 001 Pixels @ 099,193*/ 1, 0x0B,
  /* RLE: 048 Pixels @ 100,193*/ 48, 0x00,
  /* ABS: 003 Pixels @ 148,193*/ 0, 3, 0x03, 0x00, 0x0F,
  /* RLE: 004 Pixels @ 151,193*/ 4, 0x07,
  /* ABS: 005 Pixels @ 155,193*/ 0, 5, 0x06, 0x03, 0x03, 0x04, 0x04,
  /* RLE: 060 Pixels @ 160,193*/ 60, 0x00,
  /* RLE: 001 Pixels @ 220,193*/ 1, 0x04,
  /* RLE: 006 Pixels @ 221,193*/ 6, 0x03,
  /* RLE: 001 Pixels @ 227,193*/ 1, 0x04,
  /* RLE: 022 Pixels @ 228,193*/ 22, 0x00,
  /* RLE: 001 Pixels @ 250,193*/ 1, 0x04,
  /* RLE: 007 Pixels @ 251,193*/ 7, 0x03,
  /* RLE: 001 Pixels @ 258,193*/ 1, 0x04,
  /* RLE: 116 Pixels @ 259,193*/ 116, 0x00,
  /* ABS: 004 Pixels @ 375,193*/ 0, 4, 0x0D, 0x06, 0x03, 0x06,
  /* RLE: 009 Pixels @ 379,193*/ 9, 0x00,
  /* RLE: 046 Pixels @ 000,194*/ 46, 0x02,
  /* RLE: 001 Pixels @ 046,194*/ 1, 0x04,
  /* RLE: 006 Pixels @ 047,194*/ 6, 0x03,
  /* RLE: 001 Pixels @ 053,194*/ 1, 0x04,
  /* RLE: 036 Pixels @ 054,194*/ 36, 0x00,
  /* RLE: 001 Pixels @ 090,194*/ 1, 0x0B,
  /* RLE: 008 Pixels @ 091,194*/ 8, 0x08,
  /* RLE: 001 Pixels @ 099,194*/ 1, 0x0B,
  /* RLE: 050 Pixels @ 100,194*/ 50, 0x00,
  /* ABS: 008 Pixels @ 150,194*/ 0, 8, 0x11, 0x03, 0x03, 0x00, 0x04, 0x04, 0x03, 0x03,
  /* RLE: 062 Pixels @ 158,194*/ 62, 0x00,
  /* RLE: 001 Pixels @ 220,194*/ 1, 0x04,
  /* RLE: 006 Pixels @ 221,194*/ 6, 0x03,
  /* ABS: 002 Pixels @ 227,194*/ 0, 2, 0x04, 0x04,
  /* RLE: 020 Pixels @ 229,194*/ 20, 0x00,
  /* RLE: 001 Pixels @ 249,194*/ 1, 0x04,
  /* RLE: 007 Pixels @ 250,194*/ 7, 0x03,
  /* RLE: 001 Pixels @ 257,194*/ 1, 0x04,
  /* RLE: 118 Pixels @ 258,194*/ 118, 0x00,
  /* RLE: 001 Pixels @ 376,194*/ 1, 0x0D,
  /* RLE: 004 Pixels @ 377,194*/ 4, 0x06,
  /* RLE: 007 Pixels @ 381,194*/ 7, 0x00,
  /* RLE: 046 Pixels @ 000,195*/ 46, 0x02,
  /* RLE: 001 Pixels @ 046,195*/ 1, 0x04,
  /* RLE: 006 Pixels @ 047,195*/ 6, 0x03,
  /* RLE: 001 Pixels @ 053,195*/ 1, 0x04,
  /* RLE: 037 Pixels @ 054,195*/ 37, 0x00,
  /* RLE: 009 Pixels @ 091,195*/ 9, 0x08,
  /* RLE: 001 Pixels @ 100,195*/ 1, 0x0B,
  /* RLE: 048 Pixels @ 101,195*/ 48, 0x00,
  /* ABS: 002 Pixels @ 149,195*/ 0, 2, 0x03, 0x11,
  /* RLE: 006 Pixels @ 151,195*/ 6, 0x03,
  /* RLE: 001 Pixels @ 157,195*/ 1, 0x04,
  /* RLE: 062 Pixels @ 158,195*/ 62, 0x00,
  /* RLE: 001 Pixels @ 220,195*/ 1, 0x04,
  /* RLE: 007 Pixels @ 221,195*/ 7, 0x03,
  /* RLE: 001 Pixels @ 228,195*/ 1, 0x04,
  /* RLE: 019 Pixels @ 229,195*/ 19, 0x00,
  /* ABS: 002 Pixels @ 248,195*/ 0, 2, 0x04, 0x04,
  /* RLE: 007 Pixels @ 250,195*/ 7, 0x03,
  /* RLE: 001 Pixels @ 257,195*/ 1, 0x04,
  /* RLE: 119 Pixels @ 258,195*/ 119, 0x00,
  /* RLE: 001 Pixels @ 377,195*/ 1, 0x0D,
  /* RLE: 004 Pixels @ 378,195*/ 4, 0x06,
  /* RLE: 006 Pixels @ 382,195*/ 6, 0x00,
  /* RLE: 046 Pixels @ 000,196*/ 46, 0x02,
  /* RLE: 001 Pixels @ 046,196*/ 1, 0x04,
  /* RLE: 006 Pixels @ 047,196*/ 6, 0x03,
  /* RLE: 001 Pixels @ 053,196*/ 1, 0x04,
  /* RLE: 037 Pixels @ 054,196*/ 37, 0x00,
  /* RLE: 001 Pixels @ 091,196*/ 1, 0x0B,
  /* RLE: 008 Pixels @ 092,196*/ 8, 0x08,
  /* RLE: 001 Pixels @ 100,196*/ 1, 0x0B,
  /* RLE: 047 Pixels @ 101,196*/ 47, 0x00,
  /* ABS: 010 Pixels @ 148,196*/ 0, 10, 0x03, 0x0B, 0x07, 0x0F, 0x06, 0x0B, 0x0B, 0x03, 0x03, 0x04,
  /* RLE: 063 Pixels @ 158,196*/ 63, 0x00,
  /* RLE: 001 Pixels @ 221,196*/ 1, 0x04,
  /* RLE: 006 Pixels @ 222,196*/ 6, 0x03,
  /* ABS: 002 Pixels @ 228,196*/ 0, 2, 0x04, 0x04,
  /* RLE: 018 Pixels @ 230,196*/ 18, 0x00,
  /* RLE: 001 Pixels @ 248,196*/ 1, 0x04,
  /* RLE: 007 Pixels @ 249,196*/ 7, 0x03,
  /* RLE: 001 Pixels @ 256,196*/ 1, 0x04,
  /* RLE: 121 Pixels @ 257,196*/ 121, 0x00,
  /* RLE: 003 Pixels @ 378,196*/ 3, 0x0D,
  /* ABS: 002 Pixels @ 381,196*/ 0, 2, 0x06, 0x06,
  /* RLE: 005 Pixels @ 383,196*/ 5, 0x00,
  /* RLE: 046 Pixels @ 000,197*/ 46, 0x02,
  /* RLE: 001 Pixels @ 046,197*/ 1, 0x04,
  /* RLE: 006 Pixels @ 047,197*/ 6, 0x03,
  /* RLE: 001 Pixels @ 053,197*/ 1, 0x04,
  /* RLE: 037 Pixels @ 054,197*/ 37, 0x00,
  /* RLE: 001 Pixels @ 091,197*/ 1, 0x0B,
  /* RLE: 009 Pixels @ 092,197*/ 9, 0x08,
  /* RLE: 049 Pixels @ 101,197*/ 49, 0x00,
  /* ABS: 005 Pixels @ 150,197*/ 0, 5, 0x0B, 0x06, 0x0D, 0x07, 0x07,
  /* RLE: 066 Pixels @ 155,197*/ 66, 0x00,
  /* RLE: 001 Pixels @ 221,197*/ 1, 0x04,
  /* RLE: 007 Pixels @ 222,197*/ 7, 0x03,
  /* RLE: 001 Pixels @ 229,197*/ 1, 0x04,
  /* RLE: 017 Pixels @ 230,197*/ 17, 0x00,
  /* ABS: 002 Pixels @ 247,197*/ 0, 2, 0x04, 0x04,
  /* RLE: 006 Pixels @ 249,197*/ 6, 0x03,
  /* ABS: 002 Pixels @ 255,197*/ 0, 2, 0x04, 0x04,
  /* RLE: 123 Pixels @ 257,197*/ 123, 0x00,
  /* ABS: 004 Pixels @ 380,197*/ 0, 4, 0x0D, 0x03, 0x0D, 0x06,
  /* RLE: 004 Pixels @ 384,197*/ 4, 0x00,
  /* RLE: 046 Pixels @ 000,198*/ 46, 0x02,
  /* RLE: 001 Pixels @ 046,198*/ 1, 0x04,
  /* RLE: 006 Pixels @ 047,198*/ 6, 0x03,
  /* RLE: 001 Pixels @ 053,198*/ 1, 0x04,
  /* RLE: 038 Pixels @ 054,198*/ 38, 0x00,
  /* RLE: 001 Pixels @ 092,198*/ 1, 0x0B,
  /* RLE: 008 Pixels @ 093,198*/ 8, 0x08,
  /* RLE: 001 Pixels @ 101,198*/ 1, 0x0B,
  /* RLE: 046 Pixels @ 102,198*/ 46, 0x00,
  /* RLE: 006 Pixels @ 148,198*/ 6, 0x03,
  /* ABS: 002 Pixels @ 154,198*/ 0, 2, 0x00, 0x03,
  /* RLE: 065 Pixels @ 156,198*/ 65, 0x00,
  /* ABS: 002 Pixels @ 221,198*/ 0, 2, 0x04, 0x04,
  /* RLE: 007 Pixels @ 223,198*/ 7, 0x03,
  /* RLE: 001 Pixels @ 230,198*/ 1, 0x04,
  /* RLE: 016 Pixels @ 231,198*/ 16, 0x00,
  /* RLE: 001 Pixels @ 247,198*/ 1, 0x04,
  /* RLE: 007 Pixels @ 248,198*/ 7, 0x03,
  /* RLE: 001 Pixels @ 255,198*/ 1, 0x04,
  /* RLE: 125 Pixels @ 256,198*/ 125, 0x00,
  /* ABS: 007 Pixels @ 381,198*/ 0, 7, 0x0D, 0x03, 0x03, 0x06, 0x00, 0x00, 0x00,
  /* RLE: 047 Pixels @ 000,199*/ 47, 0x02,
  /* RLE: 007 Pixels @ 047,199*/ 7, 0x03,
  /* RLE: 038 Pixels @ 054,199*/ 38, 0x00,
  /* RLE: 001 Pixels @ 092,199*/ 1, 0x0B,
  /* RLE: 008 Pixels @ 093,199*/ 8, 0x08,
  /* RLE: 001 Pixels @ 101,199*/ 1, 0x0B,
  /* RLE: 046 Pixels @ 102,199*/ 46, 0x00,
  /* ABS: 008 Pixels @ 148,199*/ 0, 8, 0x03, 0x0B, 0x07, 0x07, 0x0A, 0x0E, 0x03, 0x03,
  /* RLE: 066 Pixels @ 156,199*/ 66, 0x00,
  /* RLE: 001 Pixels @ 222,199*/ 1, 0x04,
  /* RLE: 007 Pixels @ 223,199*/ 7, 0x03,
  /* ABS: 002 Pixels @ 230,199*/ 0, 2, 0x04, 0x04,
  /* RLE: 014 Pixels @ 232,199*/ 14, 0x00,
  /* RLE: 001 Pixels @ 246,199*/ 1, 0x04,
  /* RLE: 007 Pixels @ 247,199*/ 7, 0x03,
  /* ABS: 002 Pixels @ 254,199*/ 0, 2, 0x04, 0x04,
  /* RLE: 126 Pixels @ 256,199*/ 126, 0x00,
  /* ABS: 006 Pixels @ 382,199*/ 0, 6, 0x0D, 0x06, 0x06, 0x06, 0x00, 0x00,
  /* RLE: 046 Pixels @ 000,200*/ 46, 0x02,
  /* RLE: 001 Pixels @ 046,200*/ 1, 0x00,
  /* RLE: 006 Pixels @ 047,200*/ 6, 0x03,
  /* ABS: 003 Pixels @ 053,200*/ 0, 3, 0x00, 0x04, 0x03,
  /* RLE: 037 Pixels @ 056,200*/ 37, 0x00,
  /* RLE: 009 Pixels @ 093,200*/ 9, 0x08,
  /* RLE: 001 Pixels @ 102,200*/ 1, 0x0B,
  /* RLE: 044 Pixels @ 103,200*/ 44, 0x00,
  /* ABS: 008 Pixels @ 147,200*/ 0, 8, 0x03, 0x00, 0x07, 0x04, 0x0D, 0x04, 0x0F, 0x04,
  /* RLE: 068 Pixels @ 155,200*/ 68, 0x00,
  /* RLE: 001 Pixels @ 223,200*/ 1, 0x04,
  /* RLE: 007 Pixels @ 224,200*/ 7, 0x03,
  /* RLE: 001 Pixels @ 231,200*/ 1, 0x04,
  /* RLE: 013 Pixels @ 232,200*/ 13, 0x00,
  /* ABS: 002 Pixels @ 245,200*/ 0, 2, 0x04, 0x04,
  /* RLE: 007 Pixels @ 247,200*/ 7, 0x03,
  /* RLE: 001 Pixels @ 254,200*/ 1, 0x04,
  /* RLE: 128 Pixels @ 255,200*/ 128, 0x00,
  /* ABS: 005 Pixels @ 383,200*/ 0, 5, 0x0D, 0x06, 0x06, 0x06, 0x00,
  /* RLE: 046 Pixels @ 000,201*/ 46, 0x02,
  /* ABS: 009 Pixels @ 046,201*/ 0, 9, 0x03, 0x0A, 0x0B, 0x0E, 0x06, 0x11, 0x07, 0x07, 0x07,
  /* RLE: 038 Pixels @ 055,201*/ 38, 0x00,
  /* RLE: 001 Pixels @ 093,201*/ 1, 0x0B,
  /* RLE: 008 Pixels @ 094,201*/ 8, 0x08,
  /* RLE: 001 Pixels @ 102,201*/ 1, 0x0B,
  /* RLE: 045 Pixels @ 103,201*/ 45, 0x00,
  /* ABS: 008 Pixels @ 148,201*/ 0, 8, 0x0B, 0x06, 0x03, 0x0F, 0x03, 0x0E, 0x0B, 0x03,
  /* RLE: 067 Pixels @ 156,201*/ 67, 0x00,
  /* ABS: 002 Pixels @ 223,201*/ 0, 2, 0x04, 0x04,
  /* RLE: 006 Pixels @ 225,201*/ 6, 0x03,
  /* ABS: 002 Pixels @ 231,201*/ 0, 2, 0x04, 0x04,
  /* RLE: 011 Pixels @ 233,201*/ 11, 0x00,
  /* ABS: 002 Pixels @ 244,201*/ 0, 2, 0x04, 0x04,
  /* RLE: 007 Pixels @ 246,201*/ 7, 0x03,
  /* RLE: 001 Pixels @ 253,201*/ 1, 0x04,
  /* RLE: 130 Pixels @ 254,201*/ 130, 0x00,
  /* ABS: 004 Pixels @ 384,201*/ 0, 4, 0x0D, 0x06, 0x06, 0x06,
  /* RLE: 046 Pixels @ 000,202*/ 46, 0x02,
  /* ABS: 002 Pixels @ 046,202*/ 0, 2, 0x03, 0x06,
  /* RLE: 004 Pixels @ 048,202*/ 4, 0x07,
  /* ABS: 005 Pixels @ 052,202*/ 0, 5, 0x0E, 0x0B, 0x07, 0x04, 0x03,
  /* RLE: 036 Pixels @ 057,202*/ 36, 0x00,
  /* RLE: 001 Pixels @ 093,202*/ 1, 0x0B,
  /* RLE: 009 Pixels @ 094,202*/ 9, 0x08,
  /* RLE: 044 Pixels @ 103,202*/ 44, 0x00,
  /* ABS: 009 Pixels @ 147,202*/ 0, 9, 0x03, 0x00, 0x07, 0x0E, 0x0F, 0x0A, 0x11, 0x0B, 0x03,
  /* RLE: 068 Pixels @ 156,202*/ 68, 0x00,
  /* RLE: 001 Pixels @ 224,202*/ 1, 0x04,
  /* RLE: 007 Pixels @ 225,202*/ 7, 0x03,
  /* RLE: 001 Pixels @ 232,202*/ 1, 0x04,
  /* RLE: 010 Pixels @ 233,202*/ 10, 0x00,
  /* ABS: 002 Pixels @ 243,202*/ 0, 2, 0x04, 0x04,
  /* RLE: 007 Pixels @ 245,202*/ 7, 0x03,
  /* ABS: 002 Pixels @ 252,202*/ 0, 2, 0x04, 0x04,
  /* RLE: 131 Pixels @ 254,202*/ 131, 0x00,
  /* ABS: 003 Pixels @ 385,202*/ 0, 3, 0x0D, 0x06, 0x03,
  /* RLE: 046 Pixels @ 000,203*/ 46, 0x02,
  /* ABS: 011 Pixels @ 046,203*/ 0, 11, 0x03, 0x00, 0x0A, 0x03, 0x00, 0x07, 0x00, 0x03, 0x11, 0x0B, 0x03,
  /* RLE: 037 Pixels @ 057,203*/ 37, 0x00,
  /* RLE: 001 Pixels @ 094,203*/ 1, 0x0B,
  /* RLE: 008 Pixels @ 095,203*/ 8, 0x08,
  /* RLE: 001 Pixels @ 103,203*/ 1, 0x0B,
  /* RLE: 044 Pixels @ 104,203*/ 44, 0x00,
  /* ABS: 007 Pixels @ 148,203*/ 0, 7, 0x03, 0x0E, 0x07, 0x07, 0x07, 0x0D, 0x03,
  /* RLE: 069 Pixels @ 155,203*/ 69, 0x00,
  /* ABS: 002 Pixels @ 224,203*/ 0, 2, 0x04, 0x04,
  /* RLE: 007 Pixels @ 226,203*/ 7, 0x03,
  /* RLE: 001 Pixels @ 233,203*/ 1, 0x04,
  /* RLE: 008 Pixels @ 234,203*/ 8, 0x00,
  /* ABS: 002 Pixels @ 242,203*/ 0, 2, 0x04, 0x04,
  /* RLE: 008 Pixels @ 244,203*/ 8, 0x03,
  /* RLE: 001 Pixels @ 252,203*/ 1, 0x04,
  /* RLE: 133 Pixels @ 253,203*/ 133, 0x00,
  /* ABS: 002 Pixels @ 386,203*/ 0, 2, 0x0D, 0x06,
  /* RLE: 046 Pixels @ 000,204*/ 46, 0x02,
  /* ABS: 011 Pixels @ 046,204*/ 0, 11, 0x00, 0x00, 0x03, 0x03, 0x03, 0x07, 0x04, 0x03, 0x06, 0x0E, 0x03,
  /* RLE: 037 Pixels @ 057,204*/ 37, 0x00,
  /* RLE: 001 Pixels @ 094,204*/ 1, 0x0B,
  /* RLE: 008 Pixels @ 095,204*/ 8, 0x08,
  /* RLE: 001 Pixels @ 103,204*/ 1, 0x0B,
  /* RLE: 044 Pixels @ 104,204*/ 44, 0x00,
  /* RLE: 003 Pixels @ 148,204*/ 3, 0x03,
  /* ABS: 005 Pixels @ 151,204*/ 0, 5, 0x00, 0x0A, 0x03, 0x03, 0x04,
  /* RLE: 069 Pixels @ 156,204*/ 69, 0x00,
  /* RLE: 001 Pixels @ 225,204*/ 1, 0x04,
  /* RLE: 007 Pixels @ 226,204*/ 7, 0x03,
  /* ABS: 002 Pixels @ 233,204*/ 0, 2, 0x04, 0x04,
  /* RLE: 006 Pixels @ 235,204*/ 6, 0x00,
  /* ABS: 002 Pixels @ 241,204*/ 0, 2, 0x04, 0x04,
  /* RLE: 008 Pixels @ 243,204*/ 8, 0x03,
  /* ABS: 002 Pixels @ 251,204*/ 0, 2, 0x04, 0x04,
  /* RLE: 135 Pixels @ 253,204*/ 135, 0x00,
  /* RLE: 048 Pixels @ 000,205*/ 48, 0x02,
  /* RLE: 003 Pixels @ 048,205*/ 3, 0x03,
  /* ABS: 006 Pixels @ 051,205*/ 0, 6, 0x0D, 0x0B, 0x03, 0x0B, 0x06, 0x03,
  /* RLE: 038 Pixels @ 057,205*/ 38, 0x00,
  /* RLE: 009 Pixels @ 095,205*/ 9, 0x08,
  /* RLE: 001 Pixels @ 104,205*/ 1, 0x0B,
  /* RLE: 042 Pixels @ 105,205*/ 42, 0x00,
  /* ABS: 009 Pixels @ 147,205*/ 0, 9, 0x03, 0x0E, 0x0B, 0x00, 0x06, 0x12, 0x03, 0x03, 0x04,
  /* RLE: 070 Pixels @ 156,205*/ 70, 0x00,
  /* RLE: 001 Pixels @ 226,205*/ 1, 0x04,
  /* RLE: 007 Pixels @ 227,205*/ 7, 0x03,
  /* RLE: 001 Pixels @ 234,205*/ 1, 0x04,
  /* RLE: 005 Pixels @ 235,205*/ 5, 0x00,
  /* ABS: 002 Pixels @ 240,205*/ 0, 2, 0x04, 0x04,
  /* RLE: 008 Pixels @ 242,205*/ 8, 0x03,
  /* ABS: 002 Pixels @ 250,205*/ 0, 2, 0x04, 0x04,
  /* RLE: 136 Pixels @ 252,205*/ 136, 0x00,
  /* RLE: 048 Pixels @ 000,206*/ 48, 0x02,
  /* RLE: 001 Pixels @ 048,206*/ 1, 0x04,
  /* RLE: 008 Pixels @ 049,206*/ 8, 0x03,
  /* RLE: 038 Pixels @ 057,206*/ 38, 0x00,
  /* RLE: 001 Pixels @ 095,206*/ 1, 0x0B,
  /* RLE: 008 Pixels @ 096,206*/ 8, 0x08,
  /* RLE: 001 Pixels @ 104,206*/ 1, 0x0B,
  /* RLE: 041 Pixels @ 105,206*/ 41, 0x00,
  /* ABS: 009 Pixels @ 146,206*/ 0, 9, 0x03, 0x00, 0x0F, 0x00, 0x11, 0x11, 0x0F, 0x0B, 0x03,
  /* RLE: 071 Pixels @ 155,206*/ 71, 0x00,
  /* ABS: 002 Pixels @ 226,206*/ 0, 2, 0x04, 0x04,
  /* RLE: 006 Pixels @ 228,206*/ 6, 0x03,
  /* ABS: 007 Pixels @ 234,206*/ 0, 7, 0x04, 0x04, 0x00, 0x00, 0x00, 0x04, 0x04,
  /* RLE: 008 Pixels @ 241,206*/ 8, 0x03,
  /* ABS: 002 Pixels @ 249,206*/ 0, 2, 0x04, 0x04,
  /* RLE: 137 Pixels @ 251,206*/ 137, 0x00,
  /* RLE: 048 Pixels @ 000,207*/ 48, 0x02,
  /* RLE: 001 Pixels @ 048,207*/ 1, 0x00,
  /* RLE: 006 Pixels @ 049,207*/ 6, 0x03,
  /* RLE: 001 Pixels @ 055,207*/ 1, 0x04,
  /* RLE: 039 Pixels @ 056,207*/ 39, 0x00,
  /* RLE: 001 Pixels @ 095,207*/ 1, 0x0B,
  /* RLE: 009 Pixels @ 096,207*/ 9, 0x08,
  /* RLE: 041 Pixels @ 105,207*/ 41, 0x00,
  /* ABS: 009 Pixels @ 146,207*/ 0, 9, 0x03, 0x0B, 0x06, 0x0A, 0x07, 0x04, 0x0B, 0x0B, 0x03,
  /* RLE: 072 Pixels @ 155,207*/ 72, 0x00,
  /* RLE: 001 Pixels @ 227,207*/ 1, 0x04,
  /* RLE: 007 Pixels @ 228,207*/ 7, 0x03,
  /* ABS: 005 Pixels @ 235,207*/ 0, 5, 0x04, 0x00, 0x00, 0x04, 0x04,
  /* RLE: 008 Pixels @ 240,207*/ 8, 0x03,
  /* ABS: 002 Pixels @ 248,207*/ 0, 2, 0x04, 0x04,
  /* RLE: 138 Pixels @ 250,207*/ 138, 0x00,
  /* RLE: 047 Pixels @ 000,208*/ 47, 0x02,
  /* ABS: 008 Pixels @ 047,208*/ 0, 8, 0x00, 0x03, 0x0B, 0x0F, 0x07, 0x11, 0x0A, 0x03,
  /* RLE: 041 Pixels @ 055,208*/ 41, 0x00,
  /* RLE: 001 Pixels @ 096,208*/ 1, 0x0B,
  /* RLE: 008 Pixels @ 097,208*/ 8, 0x08,
  /* RLE: 001 Pixels @ 105,208*/ 1, 0x0B,
  /* RLE: 040 Pixels @ 106,208*/ 40, 0x00,
  /* ABS: 009 Pixels @ 146,208*/ 0, 9, 0x03, 0x0B, 0x11, 0x0D, 0x11, 0x03, 0x06, 0x04, 0x03,
  /* RLE: 072 Pixels @ 155,208*/ 72, 0x00,
  /* ABS: 002 Pixels @ 227,208*/ 0, 2, 0x04, 0x04,
  /* RLE: 007 Pixels @ 229,208*/ 7, 0x03,
  /* RLE: 003 Pixels @ 236,208*/ 3, 0x04,
  /* RLE: 008 Pixels @ 239,208*/ 8, 0x03,
  /* ABS: 002 Pixels @ 247,208*/ 0, 2, 0x04, 0x04,
  /* RLE: 139 Pixels @ 249,208*/ 139, 0x00,
  /* RLE: 047 Pixels @ 000,209*/ 47, 0x02,
  /* ABS: 007 Pixels @ 047,209*/ 0, 7, 0x03, 0x04, 0x07, 0x06, 0x0B, 0x06, 0x0F,
  /* RLE: 042 Pixels @ 054,209*/ 42, 0x00,
  /* RLE: 001 Pixels @ 096,209*/ 1, 0x0B,
  /* RLE: 008 Pixels @ 097,209*/ 8, 0x08,
  /* RLE: 001 Pixels @ 105,209*/ 1, 0x0B,
  /* RLE: 038 Pixels @ 106,209*/ 38, 0x00,
  /* ABS: 009 Pixels @ 144,209*/ 0, 9, 0x03, 0x00, 0x03, 0x03, 0x06, 0x0F, 0x04, 0x0B, 0x0F,
  /* RLE: 075 Pixels @ 153,209*/ 75, 0x00,
  /* RLE: 001 Pixels @ 228,209*/ 1, 0x04,
  /* RLE: 007 Pixels @ 229,209*/ 7, 0x03,
  /* ABS: 002 Pixels @ 236,209*/ 0, 2, 0x04, 0x04,
  /* RLE: 008 Pixels @ 238,209*/ 8, 0x03,
  /* ABS: 002 Pixels @ 246,209*/ 0, 2, 0x04, 0x04,
  /* RLE: 140 Pixels @ 248,209*/ 140, 0x00,
  /* RLE: 047 Pixels @ 000,210*/ 47, 0x02,
  /* ABS: 009 Pixels @ 047,210*/ 0, 9, 0x03, 0x0B, 0x0D, 0x03, 0x03, 0x03, 0x11, 0x04, 0x03,
  /* RLE: 041 Pixels @ 056,210*/ 41, 0x00,
  /* RLE: 009 Pixels @ 097,210*/ 9, 0x08,
  /* RLE: 001 Pixels @ 106,210*/ 1, 0x0B,
  /* RLE: 036 Pixels @ 107,210*/ 36, 0x00,
  /* ABS: 004 Pixels @ 143,210*/ 0, 4, 0x03, 0x00, 0x00, 0x00,
  /* RLE: 004 Pixels @ 147,210*/ 4, 0x03,
  /* ABS: 003 Pixels @ 151,210*/ 0, 3, 0x00, 0x00, 0x03,
  /* RLE: 075 Pixels @ 154,210*/ 75, 0x00,
  /* RLE: 001 Pixels @ 229,210*/ 1, 0x04,
  /* RLE: 015 Pixels @ 230,210*/ 15, 0x03,
  /* ABS: 002 Pixels @ 245,210*/ 0, 2, 0x04, 0x04,
  /* RLE: 141 Pixels @ 247,210*/ 141, 0x00,
  /* RLE: 047 Pixels @ 000,211*/ 47, 0x02,
  /* ABS: 010 Pixels @ 047,211*/ 0, 10, 0x03, 0x0B, 0x0D, 0x03, 0x03, 0x03, 0x0F, 0x04, 0x03, 0x04,
  /* RLE: 040 Pixels @ 057,211*/ 40, 0x00,
  /* RLE: 001 Pixels @ 097,211*/ 1, 0x0B,
  /* RLE: 008 Pixels @ 098,211*/ 8, 0x08,
  /* RLE: 001 Pixels @ 106,211*/ 1, 0x0B,
  /* RLE: 036 Pixels @ 107,211*/ 36, 0x00,
  /* ABS: 012 Pixels @ 143,211*/ 0, 12, 0x03, 0x06, 0x07, 0x07, 0x07, 0x0D, 0x06, 0x0B, 0x0B, 0x03, 0x03, 0x04,
  /* RLE: 074 Pixels @ 155,211*/ 74, 0x00,
  /* ABS: 002 Pixels @ 229,211*/ 0, 2, 0x04, 0x04,
  /* RLE: 013 Pixels @ 231,211*/ 13, 0x03,
  /* ABS: 002 Pixels @ 244,211*/ 0, 2, 0x04, 0x04,
  /* RLE: 142 Pixels @ 246,211*/ 142, 0x00,
  /* RLE: 047 Pixels @ 000,212*/ 47, 0x02,
  /* ABS: 009 Pixels @ 047,212*/ 0, 9, 0x03, 0x00, 0x07, 0x0D, 0x06, 0x0F, 0x0F, 0x03, 0x03,
  /* RLE: 041 Pixels @ 056,212*/ 41, 0x00,
  /* RLE: 001 Pixels @ 097,212*/ 1, 0x0B,
  /* RLE: 004 Pixels @ 098,212*/ 4, 0x08,
  /* ABS: 002 Pixels @ 102,212*/ 0, 2, 0x1B, 0x18,
  /* RLE: 004 Pixels @ 104,212*/ 4, 0x03,
  /* RLE: 035 Pixels @ 108,212*/ 35, 0x00,
  /* ABS: 012 Pixels @ 143,212*/ 0, 12, 0x03, 0x00, 0x0A, 0x0B, 0x0B, 0x06, 0x0D, 0x07, 0x07, 0x00, 0x03, 0x04,
  /* RLE: 075 Pixels @ 155,212*/ 75, 0x00,
  /* RLE: 001 Pixels @ 230,212*/ 1, 0x04,
  /* RLE: 012 Pixels @ 231,212*/ 12, 0x03,
  /* ABS: 002 Pixels @ 243,212*/ 0, 2, 0x04, 0x04,
  /* RLE: 143 Pixels @ 245,212*/ 143, 0x00,
  /* RLE: 047 Pixels @ 000,213*/ 47, 0x02,
  /* ABS: 010 Pixels @ 047,213*/ 0, 10, 0x18, 0x03, 0x0A, 0x0D, 0x0F, 0x12, 0x00, 0x03, 0x00, 0x03,
  /* RLE: 041 Pixels @ 057,213*/ 41, 0x00,
  /* ABS: 009 Pixels @ 098,213*/ 0, 9, 0x0A, 0x18, 0x03, 0x03, 0x03, 0x00, 0x04, 0x0E, 0x0D,
  /* RLE: 036 Pixels @ 107,213*/ 36, 0x00,
  /* RLE: 008 Pixels @ 143,213*/ 8, 0x03,
  /* ABS: 003 Pixels @ 151,213*/ 0, 3, 0x00, 0x03, 0x03,
  /* RLE: 076 Pixels @ 154,213*/ 76, 0x00,
  /* ABS: 002 Pixels @ 230,213*/ 0, 2, 0x04, 0x04,
  /* RLE: 010 Pixels @ 232,213*/ 10, 0x03,
  /* ABS: 002 Pixels @ 242,213*/ 0, 2, 0x04, 0x04,
  /* RLE: 144 Pixels @ 244,213*/ 144, 0x00,
  /* RLE: 048 Pixels @ 000,214*/ 48, 0x02,
  /* ABS: 009 Pixels @ 048,214*/ 0, 9, 0x00, 0x03, 0x0A, 0x0B, 0x12, 0x0D, 0x07, 0x0F, 0x03,
  /* RLE: 041 Pixels @ 057,214*/ 41, 0x00,
  /* ABS: 005 Pixels @ 098,214*/ 0, 5, 0x03, 0x00, 0x0B, 0x12, 0x0F,
  /* RLE: 004 Pixels @ 103,214*/ 4, 0x07,
  /* ABS: 002 Pixels @ 107,214*/ 0, 2, 0x04, 0x03,
  /* RLE: 033 Pixels @ 109,214*/ 33, 0x00,
  /* ABS: 012 Pixels @ 142,214*/ 0, 12, 0x03, 0x00, 0x06, 0x03, 0x12, 0x0E, 0x0B, 0x00, 0x00, 0x03, 0x03, 0x04,
  /* RLE: 077 Pixels @ 154,214*/ 77, 0x00,
  /* RLE: 001 Pixels @ 231,214*/ 1, 0x04,
  /* RLE: 009 Pixels @ 232,214*/ 9, 0x03,
  /* ABS: 002 Pixels @ 241,214*/ 0, 2, 0x04, 0x04,
  /* RLE: 145 Pixels @ 243,214*/ 145, 0x00,
  /* RLE: 049 Pixels @ 000,215*/ 49, 0x02,
  /* ABS: 008 Pixels @ 049,215*/ 0, 8, 0x00, 0x06, 0x07, 0x0F, 0x06, 0x11, 0x06, 0x03,
  /* RLE: 042 Pixels @ 057,215*/ 42, 0x00,
  /* ABS: 009 Pixels @ 099,215*/ 0, 9, 0x12, 0x07, 0x11, 0x0E, 0x0B, 0x0D, 0x07, 0x0B, 0x03,
  /* RLE: 036 Pixels @ 108,215*/ 36, 0x00,
  /* ABS: 003 Pixels @ 144,215*/ 0, 3, 0x0E, 0x03, 0x0D,
  /* RLE: 004 Pixels @ 147,215*/ 4, 0x07,
  /* ABS: 003 Pixels @ 151,215*/ 0, 3, 0x06, 0x03, 0x04,
  /* RLE: 078 Pixels @ 154,215*/ 78, 0x00,
  /* RLE: 001 Pixels @ 232,215*/ 1, 0x04,
  /* RLE: 007 Pixels @ 233,215*/ 7, 0x03,
  /* ABS: 002 Pixels @ 240,215*/ 0, 2, 0x04, 0x04,
  /* RLE: 146 Pixels @ 242,215*/ 146, 0x00,
  /* RLE: 049 Pixels @ 000,216*/ 49, 0x02,
  /* ABS: 002 Pixels @ 049,216*/ 0, 2, 0x03, 0x00,
  /* RLE: 004 Pixels @ 051,216*/ 4, 0x03,
  /* ABS: 003 Pixels @ 055,216*/ 0, 3, 0x07, 0x04, 0x03,
  /* RLE: 040 Pixels @ 058,216*/ 40, 0x00,
  /* ABS: 009 Pixels @ 098,216*/ 0, 9, 0x03, 0x00, 0x03, 0x03, 0x03, 0x0E, 0x07, 0x04, 0x03,
  /* RLE: 035 Pixels @ 107,216*/ 35, 0x00,
  /* ABS: 012 Pixels @ 142,216*/ 0, 12, 0x03, 0x00, 0x0A, 0x03, 0x03, 0x03, 0x00, 0x04, 0x0D, 0x04, 0x03, 0x04,
  /* RLE: 078 Pixels @ 154,216*/ 78, 0x00,
  /* ABS: 002 Pixels @ 232,216*/ 0, 2, 0x04, 0x04,
  /* RLE: 006 Pixels @ 234,216*/ 6, 0x03,
  /* ABS: 002 Pixels @ 240,216*/ 0, 2, 0x04, 0x04,
  /* RLE: 146 Pixels @ 242,216*/ 146, 0x00,
  /* RLE: 049 Pixels @ 000,217*/ 49, 0x02,
  /* RLE: 001 Pixels @ 049,217*/ 1, 0x18,
  /* RLE: 005 Pixels @ 050,217*/ 5, 0x03,
  /* RLE: 001 Pixels @ 055,217*/ 1, 0x04,
  /* RLE: 043 Pixels @ 056,217*/ 43, 0x00,
  /* ABS: 006 Pixels @ 099,217*/ 0, 6, 0x0A, 0x03, 0x03, 0x0D, 0x07, 0x04,
  /* RLE: 004 Pixels @ 105,217*/ 4, 0x03,
  /* RLE: 033 Pixels @ 109,217*/ 33, 0x00,
  /* ABS: 003 Pixels @ 142,217*/ 0, 3, 0x03, 0x0E, 0x12,
  /* RLE: 005 Pixels @ 145,217*/ 5, 0x03,
  /* ABS: 004 Pixels @ 150,217*/ 0, 4, 0x0B, 0x0B, 0x03, 0x04,
  /* RLE: 079 Pixels @ 154,217*/ 79, 0x00,
  /* RLE: 001 Pixels @ 233,217*/ 1, 0x04,
  /* RLE: 007 Pixels @ 234,217*/ 7, 0x03,
  /* RLE: 001 Pixels @ 241,217*/ 1, 0x04,
  /* RLE: 091 Pixels @ 242,217*/ 91, 0x00,
  /* RLE: 006 Pixels @ 333,217*/ 6, 0x04,
  /* RLE: 049 Pixels @ 339,217*/ 49, 0x00,
  /* RLE: 049 Pixels @ 000,218*/ 49, 0x02,
  /* RLE: 001 Pixels @ 049,218*/ 1, 0x18,
  /* RLE: 004 Pixels @ 050,218*/ 4, 0x03,
  /* ABS: 003 Pixels @ 054,218*/ 0, 3, 0x00, 0x03, 0x03,
  /* RLE: 043 Pixels @ 057,218*/ 43, 0x00,
  /* ABS: 008 Pixels @ 100,218*/ 0, 8, 0x03, 0x0D, 0x07, 0x0B, 0x0A, 0x0E, 0x06, 0x07,
  /* RLE: 035 Pixels @ 108,218*/ 35, 0x00,
  /* ABS: 011 Pixels @ 143,218*/ 0, 11, 0x03, 0x03, 0x0B, 0x07, 0x06, 0x12, 0x0B, 0x0F, 0x0B, 0x03, 0x04,
  /* RLE: 079 Pixels @ 154,218*/ 79, 0x00,
  /* ABS: 002 Pixels @ 233,218*/ 0, 2, 0x04, 0x04,
  /* RLE: 007 Pixels @ 235,218*/ 7, 0x03,
  /* RLE: 001 Pixels @ 242,218*/ 1, 0x04,
  /* RLE: 090 Pixels @ 243,218*/ 90, 0x00,
  /* ABS: 007 Pixels @ 333,218*/ 0, 7, 0x04, 0x04, 0x03, 0x03, 0x03, 0x04, 0x04,
  /* RLE: 048 Pixels @ 340,218*/ 48, 0x00,
  /* RLE: 049 Pixels @ 000,219*/ 49, 0x02,
  /* ABS: 008 Pixels @ 049,219*/ 0, 8, 0x03, 0x00, 0x12, 0x03, 0x0D, 0x07, 0x06, 0x03,
  /* RLE: 042 Pixels @ 057,219*/ 42, 0x00,
  /* ABS: 002 Pixels @ 099,219*/ 0, 2, 0x03, 0x12,
  /* RLE: 005 Pixels @ 101,219*/ 5, 0x07,
  /* ABS: 004 Pixels @ 106,219*/ 0, 4, 0x06, 0x0E, 0x00, 0x03,
  /* RLE: 034 Pixels @ 110,219*/ 34, 0x00,
  /* ABS: 010 Pixels @ 144,219*/ 0, 10, 0x03, 0x0A, 0x0E, 0x06, 0x11, 0x07, 0x0D, 0x03, 0x03, 0x04,
  /* RLE: 080 Pixels @ 154,219*/ 80, 0x00,
  /* RLE: 001 Pixels @ 234,219*/ 1, 0x04,
  /* RLE: 007 Pixels @ 235,219*/ 7, 0x03,
  /* ABS: 002 Pixels @ 242,219*/ 0, 2, 0x04, 0x04,
  /* RLE: 088 Pixels @ 244,219*/ 88, 0x00,
  /* ABS: 002 Pixels @ 332,219*/ 0, 2, 0x04, 0x04,
  /* RLE: 005 Pixels @ 334,219*/ 5, 0x03,
  /* RLE: 001 Pixels @ 339,219*/ 1, 0x04,
  /* RLE: 048 Pixels @ 340,219*/ 48, 0x00,
  /* RLE: 049 Pixels @ 000,220*/ 49, 0x02,
  /* ABS: 007 Pixels @ 049,220*/ 0, 7, 0x03, 0x0E, 0x0D, 0x0A, 0x07, 0x04, 0x0F,
  /* RLE: 043 Pixels @ 056,220*/ 43, 0x00,
  /* ABS: 010 Pixels @ 099,220*/ 0, 10, 0x03, 0x0E, 0x11, 0x0E, 0x0B, 0x00, 0x00, 0x04, 0x00, 0x0D,
  /* RLE: 036 Pixels @ 109,220*/ 36, 0x00,
  /* RLE: 008 Pixels @ 145,220*/ 8, 0x03,
  /* RLE: 082 Pixels @ 153,220*/ 82, 0x00,
  /* RLE: 001 Pixels @ 235,220*/ 1, 0x04,
  /* RLE: 007 Pixels @ 236,220*/ 7, 0x03,
  /* RLE: 001 Pixels @ 243,220*/ 1, 0x04,
  /* RLE: 088 Pixels @ 244,220*/ 88, 0x00,
  /* RLE: 001 Pixels @ 332,220*/ 1, 0x04,
  /* RLE: 006 Pixels @ 333,220*/ 6, 0x03,
  /* RLE: 001 Pixels @ 339,220*/ 1, 0x04,
  /* RLE: 048 Pixels @ 340,220*/ 48, 0x00,
  /* RLE: 049 Pixels @ 000,221*/ 49, 0x02,
  /* ABS: 007 Pixels @ 049,221*/ 0, 7, 0x03, 0x06, 0x0B, 0x0A, 0x07, 0x00, 0x06,
  /* RLE: 044 Pixels @ 056,221*/ 44, 0x00,
  /* ABS: 011 Pixels @ 100,221*/ 0, 11, 0x03, 0x00, 0x0B, 0x06, 0x0F, 0x07, 0x07, 0x0A, 0x06, 0x0A, 0x03,
  /* RLE: 033 Pixels @ 111,221*/ 33, 0x00,
  /* ABS: 009 Pixels @ 144,221*/ 0, 9, 0x03, 0x00, 0x0B, 0x0B, 0x0A, 0x00, 0x03, 0x03, 0x04,
  /* RLE: 082 Pixels @ 153,221*/ 82, 0x00,
  /* ABS: 002 Pixels @ 235,221*/ 0, 2, 0x04, 0x04,
  /* RLE: 006 Pixels @ 237,221*/ 6, 0x03,
  /* ABS: 002 Pixels @ 243,221*/ 0, 2, 0x04, 0x04,
  /* RLE: 086 Pixels @ 245,221*/ 86, 0x00,
  /* ABS: 002 Pixels @ 331,221*/ 0, 2, 0x04, 0x04,
  /* RLE: 006 Pixels @ 333,221*/ 6, 0x03,
  /* RLE: 001 Pixels @ 339,221*/ 1, 0x04,
  /* RLE: 048 Pixels @ 340,221*/ 48, 0x00,
  /* RLE: 049 Pixels @ 000,222*/ 49, 0x02,
  /* ABS: 007 Pixels @ 049,222*/ 0, 7, 0x03, 0x0B, 0x12, 0x0E, 0x07, 0x00, 0x07,
  /* RLE: 044 Pixels @ 056,222*/ 44, 0x00,
  /* ABS: 010 Pixels @ 100,222*/ 0, 10, 0x03, 0x12, 0x07, 0x11, 0x0E, 0x0B, 0x00, 0x03, 0x03, 0x03,
  /* RLE: 033 Pixels @ 110,222*/ 33, 0x00,
  /* ABS: 003 Pixels @ 143,222*/ 0, 3, 0x03, 0x00, 0x0F,
  /* RLE: 004 Pixels @ 146,222*/ 4, 0x07,
  /* ABS: 003 Pixels @ 150,222*/ 0, 3, 0x06, 0x03, 0x04,
  /* RLE: 083 Pixels @ 153,222*/ 83, 0x00,
  /* RLE: 001 Pixels @ 236,222*/ 1, 0x04,
  /* RLE: 007 Pixels @ 237,222*/ 7, 0x03,
  /* RLE: 001 Pixels @ 244,222*/ 1, 0x04,
  /* RLE: 086 Pixels @ 245,222*/ 86, 0x00,
  /* RLE: 001 Pixels @ 331,222*/ 1, 0x04,
  /* RLE: 007 Pixels @ 332,222*/ 7, 0x03,
  /* RLE: 001 Pixels @ 339,222*/ 1, 0x04,
  /* RLE: 048 Pixels @ 340,222*/ 48, 0x00,
  /* RLE: 049 Pixels @ 000,223*/ 49, 0x02,
  /* ABS: 009 Pixels @ 049,223*/ 0, 9, 0x03, 0x00, 0x0F, 0x07, 0x06, 0x03, 0x0B, 0x03, 0x03,
  /* RLE: 044 Pixels @ 058,223*/ 44, 0x00,
  /* ABS: 008 Pixels @ 102,223*/ 0, 8, 0x03, 0x03, 0x00, 0x0A, 0x00, 0x03, 0x03, 0x0B,
  /* RLE: 032 Pixels @ 110,223*/ 32, 0x00,
  /* ABS: 011 Pixels @ 142,223*/ 0, 11, 0x03, 0x00, 0x00, 0x11, 0x03, 0x03, 0x00, 0x04, 0x04, 0x03, 0x04,
  /* RLE: 083 Pixels @ 153,223*/ 83, 0x00,
  /* ABS: 002 Pixels @ 236,223*/ 0, 2, 0x04, 0x04,
  /* RLE: 007 Pixels @ 238,223*/ 7, 0x03,
  /* RLE: 001 Pixels @ 245,223*/ 1, 0x04,
  /* RLE: 084 Pixels @ 246,223*/ 84, 0x00,
  /* ABS: 002 Pixels @ 330,223*/ 0, 2, 0x04, 0x04,
  /* RLE: 006 Pixels @ 332,223*/ 6, 0x03,
  /* RLE: 001 Pixels @ 338,223*/ 1, 0x04,
  /* RLE: 049 Pixels @ 339,223*/ 49, 0x00,
  /* RLE: 049 Pixels @ 000,224*/ 49, 0x02,
  /* ABS: 010 Pixels @ 049,224*/ 0, 10, 0x18, 0x03, 0x00, 0x0A, 0x03, 0x03, 0x00, 0x0E, 0x00, 0x03,
  /* RLE: 041 Pixels @ 059,224*/ 41, 0x00,
  /* ABS: 009 Pixels @ 100,224*/ 0, 9, 0x0A, 0x18, 0x03, 0x0E, 0x07, 0x07, 0x07, 0x0D, 0x03,
  /* RLE: 032 Pixels @ 109,224*/ 32, 0x00,
  /* ABS: 005 Pixels @ 141,224*/ 0, 5, 0x03, 0x00, 0x00, 0x00, 0x11,
  /* RLE: 006 Pixels @ 146,224*/ 6, 0x03,
  /* RLE: 001 Pixels @ 152,224*/ 1, 0x04,
  /* RLE: 084 Pixels @ 153,224*/ 84, 0x00,
  /* RLE: 001 Pixels @ 237,224*/ 1, 0x04,
  /* RLE: 007 Pixels @ 238,224*/ 7, 0x03,
  /* RLE: 001 Pixels @ 245,224*/ 1, 0x04,
  /* RLE: 084 Pixels @ 246,224*/ 84, 0x00,
  /* RLE: 001 Pixels @ 330,224*/ 1, 0x04,
  /* RLE: 007 Pixels @ 331,224*/ 7, 0x03,
  /* RLE: 001 Pixels @ 338,224*/ 1, 0x04,
  /* RLE: 049 Pixels @ 339,224*/ 49, 0x00,
  /* RLE: 050 Pixels @ 000,225*/ 50, 0x02,
  /* ABS: 010 Pixels @ 050,225*/ 0, 10, 0x03, 0x03, 0x04, 0x0B, 0x12, 0x0D, 0x07, 0x07, 0x0E, 0x03,
  /* RLE: 040 Pixels @ 060,225*/ 40, 0x00,
  /* ABS: 010 Pixels @ 100,225*/ 0, 10, 0x0B, 0x03, 0x04, 0x07, 0x0E, 0x11, 0x0A, 0x11, 0x0B, 0x03,
  /* RLE: 031 Pixels @ 110,225*/ 31, 0x00,
  /* ABS: 012 Pixels @ 141,225*/ 0, 12, 0x03, 0x06, 0x07, 0x07, 0x07, 0x0F, 0x06, 0x0B, 0x0B, 0x03, 0x03, 0x04,
  /* RLE: 085 Pixels @ 153,225*/ 85, 0x00,
  /* RLE: 001 Pixels @ 238,225*/ 1, 0x04,
  /* RLE: 007 Pixels @ 239,225*/ 7, 0x03,
  /* RLE: 001 Pixels @ 246,225*/ 1, 0x04,
  /* RLE: 082 Pixels @ 247,225*/ 82, 0x00,
  /* ABS: 002 Pixels @ 329,225*/ 0, 2, 0x04, 0x04,
  /* RLE: 006 Pixels @ 331,225*/ 6, 0x03,
  /* RLE: 001 Pixels @ 337,225*/ 1, 0x04,
  /* RLE: 050 Pixels @ 338,225*/ 50, 0x00,
  /* RLE: 050 Pixels @ 000,226*/ 50, 0x02,
  /* ABS: 010 Pixels @ 050,226*/ 0, 10, 0x03, 0x12, 0x07, 0x07, 0x11, 0x06, 0x07, 0x04, 0x0A, 0x03,
  /* RLE: 041 Pixels @ 060,226*/ 41, 0x00,
  /* ABS: 010 Pixels @ 101,226*/ 0, 10, 0x03, 0x0B, 0x06, 0x03, 0x0F, 0x03, 0x06, 0x0B, 0x03, 0x0B,
  /* RLE: 030 Pixels @ 111,226*/ 30, 0x00,
  /* ABS: 012 Pixels @ 141,226*/ 0, 12, 0x03, 0x00, 0x00, 0x0B, 0x0B, 0x06, 0x0D, 0x07, 0x07, 0x0A, 0x03, 0x04,
  /* RLE: 085 Pixels @ 153,226*/ 85, 0x00,
  /* ABS: 002 Pixels @ 238,226*/ 0, 2, 0x04, 0x04,
  /* RLE: 006 Pixels @ 240,226*/ 6, 0x03,
  /* RLE: 001 Pixels @ 246,226*/ 1, 0x04,
  /* RLE: 081 Pixels @ 247,226*/ 81, 0x00,
  /* ABS: 002 Pixels @ 328,226*/ 0, 2, 0x04, 0x04,
  /* RLE: 007 Pixels @ 330,226*/ 7, 0x03,
  /* RLE: 001 Pixels @ 337,226*/ 1, 0x04,
  /* RLE: 050 Pixels @ 338,226*/ 50, 0x00,
  /* RLE: 050 Pixels @ 000,227*/ 50, 0x02,
  /* ABS: 009 Pixels @ 050,227*/ 0, 9, 0x03, 0x06, 0x12, 0x03, 0x03, 0x03, 0x06, 0x00, 0x03,
  /* RLE: 043 Pixels @ 059,227*/ 43, 0x00,
  /* ABS: 009 Pixels @ 102,227*/ 0, 9, 0x04, 0x0F, 0x0A, 0x12, 0x06, 0x07, 0x00, 0x18, 0x0B,
  /* RLE: 031 Pixels @ 111,227*/ 31, 0x00,
  /* RLE: 003 Pixels @ 142,227*/ 3, 0x03,
  /* ABS: 007 Pixels @ 145,227*/ 0, 7, 0x00, 0x0B, 0x0B, 0x0A, 0x00, 0x03, 0x03,
  /* RLE: 087 Pixels @ 152,227*/ 87, 0x00,
  /* RLE: 001 Pixels @ 239,227*/ 1, 0x04,
  /* RLE: 007 Pixels @ 240,227*/ 7, 0x03,
  /* RLE: 001 Pixels @ 247,227*/ 1, 0x04,
  /* RLE: 078 Pixels @ 248,227*/ 78, 0x00,
  /* ABS: 002 Pixels @ 326,227*/ 0, 2, 0x04, 0x04,
  /* RLE: 008 Pixels @ 328,227*/ 8, 0x03,
  /* RLE: 001 Pixels @ 336,227*/ 1, 0x04,
  /* RLE: 051 Pixels @ 337,227*/ 51, 0x00,
  /* RLE: 050 Pixels @ 000,228*/ 50, 0x02,
  /* ABS: 009 Pixels @ 050,228*/ 0, 9, 0x03, 0x0A, 0x0A, 0x12, 0x03, 0x0D, 0x07, 0x06, 0x03,
  /* RLE: 043 Pixels @ 059,228*/ 43, 0x00,
  /* ABS: 009 Pixels @ 102,228*/ 0, 9, 0x03, 0x06, 0x06, 0x0B, 0x0F, 0x04, 0x03, 0x1B, 0x0B,
  /* RLE: 034 Pixels @ 111,228*/ 34, 0x00,
  /* RLE: 001 Pixels @ 145,228*/ 1, 0x0F,
  /* RLE: 004 Pixels @ 146,228*/ 4, 0x07,
  /* ABS: 002 Pixels @ 150,228*/ 0, 2, 0x06, 0x03,
  /* RLE: 087 Pixels @ 152,228*/ 87, 0x00,
  /* ABS: 002 Pixels @ 239,228*/ 0, 2, 0x04, 0x04,
  /* RLE: 006 Pixels @ 241,228*/ 6, 0x03,
  /* RLE: 001 Pixels @ 247,228*/ 1, 0x04,
  /* RLE: 076 Pixels @ 248,228*/ 76, 0x00,
  /* ABS: 002 Pixels @ 324,228*/ 0, 2, 0x04, 0x04,
  /* RLE: 010 Pixels @ 326,228*/ 10, 0x03,
  /* RLE: 001 Pixels @ 336,228*/ 1, 0x04,
  /* RLE: 051 Pixels @ 337,228*/ 51, 0x00,
  /* RLE: 050 Pixels @ 000,229*/ 50, 0x02,
  /* ABS: 010 Pixels @ 050,229*/ 0, 10, 0x18, 0x03, 0x0E, 0x0D, 0x0A, 0x07, 0x0B, 0x0F, 0x00, 0x03,
  /* RLE: 041 Pixels @ 060,229*/ 41, 0x00,
  /* ABS: 010 Pixels @ 101,229*/ 0, 10, 0x0B, 0x03, 0x03, 0x00, 0x03, 0x00, 0x0A, 0x03, 0x03, 0x0B,
  /* RLE: 033 Pixels @ 111,229*/ 33, 0x00,
  /* ABS: 008 Pixels @ 144,229*/ 0, 8, 0x0A, 0x11, 0x03, 0x03, 0x0A, 0x04, 0x04, 0x03,
  /* RLE: 088 Pixels @ 152,229*/ 88, 0x00,
  /* RLE: 001 Pixels @ 240,229*/ 1, 0x04,
  /* RLE: 007 Pixels @ 241,229*/ 7, 0x03,
  /* RLE: 001 Pixels @ 248,229*/ 1, 0x04,
  /* RLE: 073 Pixels @ 249,229*/ 73, 0x00,
  /* RLE: 003 Pixels @ 322,229*/ 3, 0x04,
  /* RLE: 010 Pixels @ 325,229*/ 10, 0x03,
  /* RLE: 001 Pixels @ 335,229*/ 1, 0x04,
  /* RLE: 052 Pixels @ 336,229*/ 52, 0x00,
  /* RLE: 051 Pixels @ 000,230*/ 51, 0x02,
  /* ABS: 007 Pixels @ 051,230*/ 0, 7, 0x03, 0x06, 0x0B, 0x00, 0x07, 0x00, 0x06,
  /* RLE: 043 Pixels @ 058,230*/ 43, 0x00,
  /* ABS: 009 Pixels @ 101,230*/ 0, 9, 0x0B, 0x18, 0x03, 0x0E, 0x07, 0x07, 0x07, 0x0D, 0x03,
  /* RLE: 033 Pixels @ 110,230*/ 33, 0x00,
  /* ABS: 003 Pixels @ 143,230*/ 0, 3, 0x03, 0x03, 0x11,
  /* RLE: 005 Pixels @ 146,230*/ 5, 0x03,
  /* RLE: 089 Pixels @ 151,230*/ 89, 0x00,
  /* ABS: 002 Pixels @ 240,230*/ 0, 2, 0x04, 0x04,
  /* RLE: 006 Pixels @ 242,230*/ 6, 0x03,
  /* RLE: 001 Pixels @ 248,230*/ 1, 0x04,
  /* RLE: 054 Pixels @ 249,230*/ 54, 0x00,
  /* ABS: 007 Pixels @ 303,230*/ 0, 7, 0x03, 0x00, 0x00, 0x00, 0x03, 0x03, 0x03,
  /* RLE: 010 Pixels @ 310,230*/ 10, 0x00,
  /* RLE: 003 Pixels @ 320,230*/ 3, 0x04,
  /* RLE: 011 Pixels @ 323,230*/ 11, 0x03,
  /* ABS: 002 Pixels @ 334,230*/ 0, 2, 0x04, 0x04,
  /* RLE: 052 Pixels @ 336,230*/ 52, 0x00,
  /* RLE: 051 Pixels @ 000,231*/ 51, 0x02,
  /* ABS: 007 Pixels @ 051,231*/ 0, 7, 0x03, 0x0B, 0x12, 0x0E, 0x07, 0x00, 0x07,
  /* RLE: 045 Pixels @ 058,231*/ 45, 0x00,
  /* ABS: 009 Pixels @ 103,231*/ 0, 9, 0x04, 0x07, 0x12, 0x04, 0x0A, 0x0F, 0x04, 0x03, 0x0A,
  /* RLE: 031 Pixels @ 112,231*/ 31, 0x00,
  /* ABS: 008 Pixels @ 143,231*/ 0, 8, 0x03, 0x0B, 0x07, 0x0F, 0x06, 0x0B, 0x04, 0x03,
  /* RLE: 090 Pixels @ 151,231*/ 90, 0x00,
  /* RLE: 001 Pixels @ 241,231*/ 1, 0x04,
  /* RLE: 007 Pixels @ 242,231*/ 7, 0x03,
  /* RLE: 001 Pixels @ 249,231*/ 1, 0x04,
  /* RLE: 051 Pixels @ 250,231*/ 51, 0x00,
  /* ABS: 009 Pixels @ 301,231*/ 0, 9, 0x03, 0x00, 0x04, 0x03, 0x03, 0x03, 0x04, 0x06, 0x03,
  /* RLE: 009 Pixels @ 310,231*/ 9, 0x00,
  /* ABS: 002 Pixels @ 319,231*/ 0, 2, 0x04, 0x04,
  /* RLE: 012 Pixels @ 321,231*/ 12, 0x03,
  /* ABS: 002 Pixels @ 333,231*/ 0, 2, 0x04, 0x04,
  /* RLE: 053 Pixels @ 335,231*/ 53, 0x00,
  /* RLE: 051 Pixels @ 000,232*/ 51, 0x02,
  /* ABS: 009 Pixels @ 051,232*/ 0, 9, 0x03, 0x00, 0x0F, 0x07, 0x06, 0x03, 0x0B, 0x03, 0x03,
  /* RLE: 042 Pixels @ 060,232*/ 42, 0x00,
  /* ABS: 011 Pixels @ 102,232*/ 0, 11, 0x03, 0x0B, 0x0D, 0x03, 0x03, 0x03, 0x0E, 0x0B, 0x03, 0x00, 0x03,
  /* RLE: 028 Pixels @ 113,232*/ 28, 0x00,
  /* ABS: 010 Pixels @ 141,232*/ 0, 10, 0x03, 0x00, 0x03, 0x0A, 0x0B, 0x06, 0x0D, 0x07, 0x07, 0x0A,
  /* RLE: 090 Pixels @ 151,232*/ 90, 0x00,
  /* ABS: 002 Pixels @ 241,232*/ 0, 2, 0x04, 0x04,
  /* RLE: 006 Pixels @ 243,232*/ 6, 0x03,
  /* RLE: 001 Pixels @ 249,232*/ 1, 0x04,
  /* RLE: 046 Pixels @ 250,232*/ 46, 0x00,
  /* RLE: 006 Pixels @ 296,232*/ 6, 0x03,
  /* ABS: 008 Pixels @ 302,232*/ 0, 8, 0x0E, 0x11, 0x0A, 0x00, 0x0D, 0x11, 0x06, 0x03,
  /* RLE: 007 Pixels @ 310,232*/ 7, 0x00,
  /* ABS: 002 Pixels @ 317,232*/ 0, 2, 0x04, 0x04,
  /* RLE: 012 Pixels @ 319,232*/ 12, 0x03,
  /* RLE: 003 Pixels @ 331,232*/ 3, 0x04,
  /* RLE: 054 Pixels @ 334,232*/ 54, 0x00,
  /* RLE: 051 Pixels @ 000,233*/ 51, 0x02,
  /* ABS: 010 Pixels @ 051,233*/ 0, 10, 0x18, 0x03, 0x00, 0x00, 0x03, 0x03, 0x0A, 0x0E, 0x00, 0x03,
  /* RLE: 042 Pixels @ 061,233*/ 42, 0x00,
  /* ABS: 010 Pixels @ 103,233*/ 0, 10, 0x0A, 0x0F, 0x00, 0x03, 0x0A, 0x0F, 0x06, 0x0F, 0x07, 0x03,
  /* RLE: 027 Pixels @ 113,233*/ 27, 0x00,
  /* ABS: 003 Pixels @ 140,233*/ 0, 3, 0x03, 0x00, 0x00,
  /* RLE: 006 Pixels @ 143,233*/ 6, 0x03,
  /* ABS: 002 Pixels @ 149,233*/ 0, 2, 0x00, 0x03,
  /* RLE: 091 Pixels @ 151,233*/ 91, 0x00,
  /* RLE: 001 Pixels @ 242,233*/ 1, 0x04,
  /* RLE: 007 Pixels @ 243,233*/ 7, 0x03,
  /* RLE: 001 Pixels @ 250,233*/ 1, 0x04,
  /* RLE: 044 Pixels @ 251,233*/ 44, 0x00,
  /* ABS: 022 Pixels @ 295,233*/ 0, 22, 0x03, 0x00, 0x12, 0x07, 0x11, 0x00, 0x03, 0x12, 0x07, 0x0F, 0x00, 0x07, 0x06, 0x03, 0x03, 0x00, 0x03, 0x03, 0x03, 0x00, 0x04, 0x04,
  /* RLE: 012 Pixels @ 317,233*/ 12, 0x03,
  /* RLE: 003 Pixels @ 329,233*/ 3, 0x04,
  /* RLE: 056 Pixels @ 332,233*/ 56, 0x00,
  /* RLE: 052 Pixels @ 000,234*/ 52, 0x02,
  /* ABS: 010 Pixels @ 052,234*/ 0, 10, 0x00, 0x03, 0x04, 0x0B, 0x12, 0x0D, 0x07, 0x07, 0x0E, 0x03,
  /* RLE: 040 Pixels @ 062,234*/ 40, 0x00,
  /* ABS: 003 Pixels @ 102,234*/ 0, 3, 0x0A, 0x03, 0x06,
  /* RLE: 004 Pixels @ 105,234*/ 4, 0x07,
  /* ABS: 004 Pixels @ 109,234*/ 0, 4, 0x11, 0x12, 0x0B, 0x03,
  /* RLE: 027 Pixels @ 113,234*/ 27, 0x00,
  /* ABS: 011 Pixels @ 140,234*/ 0, 11, 0x03, 0x06, 0x06, 0x0B, 0x07, 0x0D, 0x06, 0x0B, 0x0B, 0x03, 0x03,
  /* RLE: 091 Pixels @ 151,234*/ 91, 0x00,
  /* ABS: 002 Pixels @ 242,234*/ 0, 2, 0x04, 0x04,
  /* RLE: 006 Pixels @ 244,234*/ 6, 0x03,
  /* RLE: 001 Pixels @ 250,234*/ 1, 0x04,
  /* RLE: 043 Pixels @ 251,234*/ 43, 0x00,
  /* ABS: 022 Pixels @ 294,234*/ 0, 22, 0x03, 0x0A, 0x07, 0x12, 0x0B, 0x11, 0x0B, 0x03, 0x0E, 0x0F, 0x0E, 0x03, 0x12, 0x0F, 0x03, 0x00, 0x03, 0x12, 0x11, 0x03, 0x04, 0x04,
  /* RLE: 012 Pixels @ 316,234*/ 12, 0x03,
  /* ABS: 002 Pixels @ 328,234*/ 0, 2, 0x04, 0x04,
  /* RLE: 058 Pixels @ 330,234*/ 58, 0x00,
  /* RLE: 052 Pixels @ 000,235*/ 52, 0x02,
  /* ABS: 010 Pixels @ 052,235*/ 0, 10, 0x03, 0x12, 0x07, 0x07, 0x11, 0x06, 0x07, 0x04, 0x0A, 0x03,
  /* RLE: 041 Pixels @ 062,235*/ 41, 0x00,
  /* ABS: 009 Pixels @ 103,235*/ 0, 9, 0x03, 0x0D, 0x06, 0x0B, 0x0A, 0x00, 0x03, 0x03, 0x03,
  /* RLE: 028 Pixels @ 112,235*/ 28, 0x00,
  /* ABS: 009 Pixels @ 140,235*/ 0, 9, 0x03, 0x00, 0x00, 0x00, 0x0B, 0x06, 0x0D, 0x07, 0x07,
  /* RLE: 094 Pixels @ 149,235*/ 94, 0x00,
  /* RLE: 001 Pixels @ 243,235*/ 1, 0x04,
  /* RLE: 007 Pixels @ 244,235*/ 7, 0x03,
  /* RLE: 001 Pixels @ 251,235*/ 1, 0x04,
  /* RLE: 042 Pixels @ 252,235*/ 42, 0x00,
  /* ABS: 003 Pixels @ 294,235*/ 0, 3, 0x03, 0x06, 0x0D,
  /* RLE: 006 Pixels @ 297,235*/ 6, 0x03,
  /* ABS: 010 Pixels @ 303,235*/ 0, 10, 0x0E, 0x07, 0x00, 0x00, 0x07, 0x04, 0x03, 0x03, 0x04, 0x04,
  /* RLE: 013 Pixels @ 313,235*/ 13, 0x03,
  /* ABS: 002 Pixels @ 326,235*/ 0, 2, 0x04, 0x04,
  /* RLE: 060 Pixels @ 328,235*/ 60, 0x00,
  /* RLE: 052 Pixels @ 000,236*/ 52, 0x02,
  /* ABS: 009 Pixels @ 052,236*/ 0, 9, 0x03, 0x06, 0x12, 0x03, 0x03, 0x03, 0x0E, 0x00, 0x03,
  /* RLE: 042 Pixels @ 061,236*/ 42, 0x00,
  /* RLE: 003 Pixels @ 103,236*/ 3, 0x03,
  /* ABS: 007 Pixels @ 106,236*/ 0, 7, 0x00, 0x04, 0x00, 0x03, 0x03, 0x08, 0x0B,
  /* RLE: 028 Pixels @ 113,236*/ 28, 0x00,
  /* RLE: 007 Pixels @ 141,236*/ 7, 0x03,
  /* ABS: 002 Pixels @ 148,236*/ 0, 2, 0x00, 0x03,
  /* RLE: 093 Pixels @ 150,236*/ 93, 0x00,
  /* ABS: 002 Pixels @ 243,236*/ 0, 2, 0x04, 0x04,
  /* RLE: 006 Pixels @ 245,236*/ 6, 0x03,
  /* RLE: 001 Pixels @ 251,236*/ 1, 0x04,
  /* RLE: 042 Pixels @ 252,236*/ 42, 0x00,
  /* ABS: 015 Pixels @ 294,236*/ 0, 15, 0x03, 0x0B, 0x07, 0x0D, 0x0F, 0x07, 0x07, 0x12, 0x03, 0x00, 0x07, 0x0B, 0x00, 0x11, 0x06,
  /* RLE: 015 Pixels @ 309,236*/ 15, 0x03,
  /* ABS: 002 Pixels @ 324,236*/ 0, 2, 0x04, 0x04,
  /* RLE: 062 Pixels @ 326,236*/ 62, 0x00,
  /* RLE: 052 Pixels @ 000,237*/ 52, 0x02,
  /* ABS: 008 Pixels @ 052,237*/ 0, 8, 0x03, 0x0A, 0x00, 0x03, 0x03, 0x03, 0x00, 0x03,
  /* RLE: 044 Pixels @ 060,237*/ 44, 0x00,
  /* ABS: 009 Pixels @ 104,237*/ 0, 9, 0x03, 0x0E, 0x07, 0x07, 0x07, 0x0D, 0x03, 0x18, 0x0B,
  /* RLE: 029 Pixels @ 113,237*/ 29, 0x00,
  /* ABS: 009 Pixels @ 142,237*/ 0, 9, 0x03, 0x0B, 0x07, 0x07, 0x0A, 0x0E, 0x03, 0x03, 0x04,
  /* RLE: 093 Pixels @ 151,237*/ 93, 0x00,
  /* RLE: 001 Pixels @ 244,237*/ 1, 0x04,
  /* RLE: 007 Pixels @ 245,237*/ 7, 0x03,
  /* RLE: 001 Pixels @ 252,237*/ 1, 0x04,
  /* RLE: 039 Pixels @ 253,237*/ 39, 0x00,
  /* ABS: 017 Pixels @ 292,237*/ 0, 17, 0x04, 0x04, 0x03, 0x03, 0x0B, 0x06, 0x12, 0x0B, 0x12, 0x07, 0x04, 0x03, 0x0E, 0x07, 0x0F, 0x04, 0x04,
  /* RLE: 013 Pixels @ 309,237*/ 13, 0x03,
  /* RLE: 003 Pixels @ 322,237*/ 3, 0x04,
  /* RLE: 063 Pixels @ 325,237*/ 63, 0x00,
  /* RLE: 052 Pixels @ 000,238*/ 52, 0x02,
  /* ABS: 009 Pixels @ 052,238*/ 0, 9, 0x03, 0x0A, 0x0B, 0x12, 0x0D, 0x07, 0x0F, 0x03, 0x04,
  /* RLE: 042 Pixels @ 061,238*/ 42, 0x00,
  /* ABS: 010 Pixels @ 103,238*/ 0, 10, 0x03, 0x04, 0x07, 0x0E, 0x11, 0x00, 0x11, 0x0B, 0x03, 0x0B,
  /* RLE: 028 Pixels @ 113,238*/ 28, 0x00,
  /* ABS: 010 Pixels @ 141,238*/ 0, 10, 0x03, 0x00, 0x07, 0x04, 0x0D, 0x0A, 0x0F, 0x0B, 0x03, 0x04,
  /* RLE: 093 Pixels @ 151,238*/ 93, 0x00,
  /* ABS: 002 Pixels @ 244,238*/ 0, 2, 0x04, 0x04,
  /* RLE: 006 Pixels @ 246,238*/ 6, 0x03,
  /* RLE: 001 Pixels @ 252,238*/ 1, 0x04,
  /* RLE: 035 Pixels @ 253,238*/ 35, 0x00,
  /* RLE: 007 Pixels @ 288,238*/ 7, 0x03,
  /* RLE: 001 Pixels @ 295,238*/ 1, 0x0A,
  /* RLE: 005 Pixels @ 296,238*/ 5, 0x03,
  /* ABS: 006 Pixels @ 301,238*/ 0, 6, 0x07, 0x0B, 0x03, 0x03, 0x0A, 0x00,
  /* RLE: 013 Pixels @ 307,238*/ 13, 0x03,
  /* RLE: 003 Pixels @ 320,238*/ 3, 0x04,
  /* RLE: 065 Pixels @ 323,238*/ 65, 0x00,
  /* RLE: 052 Pixels @ 000,239*/ 52, 0x02,
  /* ABS: 008 Pixels @ 052,239*/ 0, 8, 0x03, 0x06, 0x07, 0x0F, 0x06, 0x11, 0x06, 0x03,
  /* RLE: 043 Pixels @ 060,239*/ 43, 0x00,
  /* ABS: 010 Pixels @ 103,239*/ 0, 10, 0x03, 0x0B, 0x06, 0x03, 0x0F, 0x03, 0x06, 0x0B, 0x03, 0x0B,
  /* RLE: 029 Pixels @ 113,239*/ 29, 0x00,
  /* ABS: 009 Pixels @ 142,239*/ 0, 9, 0x0B, 0x06, 0x03, 0x0F, 0x03, 0x0E, 0x0B, 0x03, 0x04,
  /* RLE: 094 Pixels @ 151,239*/ 94, 0x00,
  /* RLE: 001 Pixels @ 245,239*/ 1, 0x04,
  /* RLE: 006 Pixels @ 246,239*/ 6, 0x03,
  /* ABS: 002 Pixels @ 252,239*/ 0, 2, 0x04, 0x04,
  /* RLE: 024 Pixels @ 254,239*/ 24, 0x00,
  /* RLE: 003 Pixels @ 278,239*/ 3, 0x03,
  /* RLE: 005 Pixels @ 281,239*/ 5, 0x00,
  /* ABS: 017 Pixels @ 286,239*/ 0, 17, 0x03, 0x03, 0x00, 0x11, 0x07, 0x0B, 0x03, 0x0E, 0x07, 0x07, 0x04, 0x07, 0x0B, 0x04, 0x0D, 0x0F, 0x00,
  /* RLE: 016 Pixels @ 303,239*/ 16, 0x03,
  /* ABS: 002 Pixels @ 319,239*/ 0, 2, 0x04, 0x04,
  /* RLE: 067 Pixels @ 321,239*/ 67, 0x00,
  /* RLE: 052 Pixels @ 000,240*/ 52, 0x02,
  /* ABS: 002 Pixels @ 052,240*/ 0, 2, 0x03, 0x00,
  /* RLE: 004 Pixels @ 054,240*/ 4, 0x03,
  /* ABS: 002 Pixels @ 058,240*/ 0, 2, 0x07, 0x04,
  /* RLE: 043 Pixels @ 060,240*/ 43, 0x00,
  /* ABS: 011 Pixels @ 103,240*/ 0, 11, 0x03, 0x04, 0x0F, 0x0A, 0x12, 0x06, 0x07, 0x0A, 0x18, 0x08, 0x0B,
  /* RLE: 027 Pixels @ 114,240*/ 27, 0x00,
  /* ABS: 010 Pixels @ 141,240*/ 0, 10, 0x03, 0x00, 0x07, 0x0E, 0x0F, 0x0A, 0x11, 0x0B, 0x03, 0x04,
  /* RLE: 094 Pixels @ 151,240*/ 94, 0x00,
  /* RLE: 001 Pixels @ 245,240*/ 1, 0x04,
  /* RLE: 007 Pixels @ 246,240*/ 7, 0x03,
  /* RLE: 001 Pixels @ 253,240*/ 1, 0x04,
  /* RLE: 024 Pixels @ 254,240*/ 24, 0x00,
  /* ABS: 003 Pixels @ 278,240*/ 0, 3, 0x03, 0x06, 0x04,
  /* RLE: 004 Pixels @ 281,240*/ 4, 0x03,
  /* ABS: 017 Pixels @ 285,240*/ 0, 17, 0x00, 0x00, 0x11, 0x06, 0x04, 0x06, 0x07, 0x00, 0x0E, 0x0E, 0x00, 0x03, 0x06, 0x07, 0x07, 0x12, 0x00,
  /* RLE: 015 Pixels @ 302,240*/ 15, 0x03,
  /* ABS: 002 Pixels @ 317,240*/ 0, 2, 0x04, 0x04,
  /* RLE: 069 Pixels @ 319,240*/ 69, 0x00,
  /* RLE: 052 Pixels @ 000,241*/ 52, 0x02,
  /* ABS: 002 Pixels @ 052,241*/ 0, 2, 0x18, 0x00,
  /* RLE: 004 Pixels @ 054,241*/ 4, 0x03,
  /* ABS: 003 Pixels @ 058,241*/ 0, 3, 0x04, 0x00, 0x03,
  /* RLE: 037 Pixels @ 061,241*/ 37, 0x00,
  /* RLE: 005 Pixels @ 098,241*/ 5, 0x03,
  /* ABS: 011 Pixels @ 103,241*/ 0, 11, 0x00, 0x03, 0x06, 0x06, 0x0B, 0x0F, 0x04, 0x03, 0x03, 0x18, 0x0B,
  /* RLE: 026 Pixels @ 114,241*/ 26, 0x00,
  /* ABS: 010 Pixels @ 140,241*/ 0, 10, 0x03, 0x00, 0x03, 0x0E, 0x07, 0x07, 0x07, 0x0D, 0x03, 0x03,
  /* RLE: 096 Pixels @ 150,241*/ 96, 0x00,
  /* RLE: 001 Pixels @ 246,241*/ 1, 0x04,
  /* RLE: 006 Pixels @ 247,241*/ 6, 0x03,
  /* ABS: 002 Pixels @ 253,241*/ 0, 2, 0x04, 0x04,
  /* RLE: 023 Pixels @ 255,241*/ 23, 0x00,
  /* ABS: 015 Pixels @ 278,241*/ 0, 15, 0x03, 0x11, 0x06, 0x00, 0x11, 0x07, 0x0B, 0x03, 0x03, 0x07, 0x06, 0x03, 0x00, 0x07, 0x0B,
  /* RLE: 025 Pixels @ 293,241*/ 25, 0x03,
  /* ABS: 002 Pixels @ 318,241*/ 0, 2, 0x04, 0x04,
  /* RLE: 068 Pixels @ 320,241*/ 68, 0x00,
  /* RLE: 053 Pixels @ 000,242*/ 53, 0x02,
  /* ABS: 003 Pixels @ 053,242*/ 0, 3, 0x03, 0x0A, 0x0A,
  /* RLE: 005 Pixels @ 056,242*/ 5, 0x03,
  /* RLE: 001 Pixels @ 061,242*/ 1, 0x04,
  /* RLE: 033 Pixels @ 062,242*/ 33, 0x00,
  /* RLE: 003 Pixels @ 095,242*/ 3, 0x03,
  /* RLE: 005 Pixels @ 098,242*/ 5, 0x13,
  /* RLE: 003 Pixels @ 103,242*/ 3, 0x03,
  /* ABS: 008 Pixels @ 106,242*/ 0, 8, 0x00, 0x03, 0x03, 0x03, 0x00, 0x04, 0x03, 0x0B,
  /* RLE: 025 Pixels @ 114,242*/ 25, 0x00,
  /* ABS: 011 Pixels @ 139,242*/ 0, 11, 0x03, 0x00, 0x00, 0x00, 0x03, 0x03, 0x0A, 0x00, 0x03, 0x03, 0x04,
  /* RLE: 096 Pixels @ 150,242*/ 96, 0x00,
  /* RLE: 001 Pixels @ 246,242*/ 1, 0x04,
  /* RLE: 007 Pixels @ 247,242*/ 7, 0x03,
  /* RLE: 001 Pixels @ 254,242*/ 1, 0x04,
  /* RLE: 023 Pixels @ 255,242*/ 23, 0x00,
  /* ABS: 015 Pixels @ 278,242*/ 0, 15, 0x03, 0x0B, 0x07, 0x0D, 0x04, 0x12, 0x07, 0x00, 0x03, 0x0E, 0x0F, 0x03, 0x03, 0x12, 0x11,
  /* RLE: 004 Pixels @ 293,242*/ 4, 0x03,
  /* RLE: 014 Pixels @ 297,242*/ 14, 0x04,
  /* RLE: 008 Pixels @ 311,242*/ 8, 0x03,
  /* ABS: 002 Pixels @ 319,242*/ 0, 2, 0x04, 0x04,
  /* RLE: 067 Pixels @ 321,242*/ 67, 0x00,
  /* RLE: 053 Pixels @ 000,243*/ 53, 0x02,
  /* ABS: 003 Pixels @ 053,243*/ 0, 3, 0x03, 0x06, 0x06,
  /* RLE: 005 Pixels @ 056,243*/ 5, 0x03,
  /* RLE: 001 Pixels @ 061,243*/ 1, 0x04,
  /* RLE: 032 Pixels @ 062,243*/ 32, 0x00,
  /* ABS: 004 Pixels @ 094,243*/ 0, 4, 0x03, 0x03, 0x13, 0x13,
  /* RLE: 005 Pixels @ 098,243*/ 5, 0x16,
  /* ABS: 011 Pixels @ 103,243*/ 0, 11, 0x13, 0x13, 0x03, 0x03, 0x0B, 0x06, 0x0F, 0x07, 0x07, 0x03, 0x0B,
  /* RLE: 025 Pixels @ 114,243*/ 25, 0x00,
  /* ABS: 010 Pixels @ 139,243*/ 0, 10, 0x03, 0x06, 0x07, 0x07, 0x07, 0x0D, 0x06, 0x0B, 0x0B, 0x03,
  /* RLE: 098 Pixels @ 149,243*/ 98, 0x00,
  /* RLE: 001 Pixels @ 247,243*/ 1, 0x04,
  /* RLE: 006 Pixels @ 248,243*/ 6, 0x03,
  /* ABS: 002 Pixels @ 254,243*/ 0, 2, 0x04, 0x04,
  /* RLE: 014 Pixels @ 256,243*/ 14, 0x00,
  /* ABS: 002 Pixels @ 270,243*/ 0, 2, 0x03, 0x03,
  /* RLE: 004 Pixels @ 272,243*/ 4, 0x00,
  /* RLE: 004 Pixels @ 276,243*/ 4, 0x03,
  /* ABS: 016 Pixels @ 280,243*/ 0, 16, 0x0F, 0x06, 0x03, 0x00, 0x07, 0x0B, 0x03, 0x00, 0x07, 0x04, 0x03, 0x0A, 0x07, 0x00, 0x03, 0x04,
  /* RLE: 014 Pixels @ 296,243*/ 14, 0x00,
  /* ABS: 002 Pixels @ 310,243*/ 0, 2, 0x04, 0x04,
  /* RLE: 008 Pixels @ 312,243*/ 8, 0x03,
  /* ABS: 002 Pixels @ 320,243*/ 0, 2, 0x04, 0x04,
  /* RLE: 066 Pixels @ 322,243*/ 66, 0x00,
  /* RLE: 053 Pixels @ 000,244*/ 53, 0x02,
  /* ABS: 003 Pixels @ 053,244*/ 0, 3, 0x03, 0x00, 0x00,
  /* RLE: 005 Pixels @ 056,244*/ 5, 0x03,
  /* RLE: 001 Pixels @ 061,244*/ 1, 0x04,
  /* RLE: 031 Pixels @ 062,244*/ 31, 0x00,
  /* ABS: 003 Pixels @ 093,244*/ 0, 3, 0x03, 0x03, 0x13,
  /* RLE: 007 Pixels @ 096,244*/ 7, 0x16,
  /* ABS: 010 Pixels @ 103,244*/ 0, 10, 0x17, 0x17, 0x13, 0x03, 0x03, 0x0D, 0x0E, 0x0E, 0x06, 0x03,
  /* RLE: 026 Pixels @ 113,244*/ 26, 0x00,
  /* ABS: 009 Pixels @ 139,244*/ 0, 9, 0x03, 0x00, 0x00, 0x0B, 0x0B, 0x06, 0x0D, 0x07, 0x07,
  /* RLE: 099 Pixels @ 148,244*/ 99, 0x00,
  /* RLE: 001 Pixels @ 247,244*/ 1, 0x04,
  /* RLE: 007 Pixels @ 248,244*/ 7, 0x03,
  /* RLE: 001 Pixels @ 255,244*/ 1, 0x04,
  /* RLE: 013 Pixels @ 256,244*/ 13, 0x00,
  /* ABS: 025 Pixels @ 269,244*/ 0, 25, 0x03, 0x04, 0x0B, 0x03, 0x00, 0x03, 0x03, 0x04, 0x06, 0x12, 0x03, 0x0E, 0x0F, 0x03, 0x03, 0x12, 0x11, 0x03, 0x03, 0x0D, 0x0D, 0x03, 0x03, 0x00, 0x03,
  /* RLE: 017 Pixels @ 294,244*/ 17, 0x00,
  /* ABS: 002 Pixels @ 311,244*/ 0, 2, 0x04, 0x04,
  /* RLE: 008 Pixels @ 313,244*/ 8, 0x03,
  /* ABS: 002 Pixels @ 321,244*/ 0, 2, 0x04, 0x04,
  /* RLE: 065 Pixels @ 323,244*/ 65, 0x00,
  /* RLE: 053 Pixels @ 000,245*/ 53, 0x02,
  /* ABS: 002 Pixels @ 053,245*/ 0, 2, 0x18, 0x00,
  /* RLE: 006 Pixels @ 055,245*/ 6, 0x03,
  /* RLE: 001 Pixels @ 061,245*/ 1, 0x04,
  /* RLE: 030 Pixels @ 062,245*/ 30, 0x00,
  /* ABS: 008 Pixels @ 092,245*/ 0, 8, 0x03, 0x03, 0x13, 0x16, 0x16, 0x16, 0x03, 0x03,
  /* RLE: 004 Pixels @ 100,245*/ 4, 0x16,
  /* ABS: 003 Pixels @ 104,245*/ 0, 3, 0x17, 0x17, 0x13,
  /* RLE: 004 Pixels @ 107,245*/ 4, 0x03,
  /* ABS: 004 Pixels @ 111,245*/ 0, 4, 0x0E, 0x0E, 0x03, 0x0B,
  /* RLE: 024 Pixels @ 115,245*/ 24, 0x00,
  /* ABS: 002 Pixels @ 139,245*/ 0, 2, 0x03, 0x04,
  /* RLE: 006 Pixels @ 141,245*/ 6, 0x03,
  /* ABS: 002 Pixels @ 147,245*/ 0, 2, 0x00, 0x03,
  /* RLE: 099 Pixels @ 149,245*/ 99, 0x00,
  /* RLE: 001 Pixels @ 248,245*/ 1, 0x04,
  /* RLE: 006 Pixels @ 249,245*/ 6, 0x03,
  /* ABS: 002 Pixels @ 255,245*/ 0, 2, 0x04, 0x04,
  /* RLE: 008 Pixels @ 257,245*/ 8, 0x00,
  /* RLE: 003 Pixels @ 265,245*/ 3, 0x03,
  /* ABS: 024 Pixels @ 268,245*/ 0, 24, 0x00, 0x03, 0x0E, 0x07, 0x03, 0x00, 0x03, 0x0D, 0x11, 0x0B, 0x07, 0x0B, 0x00, 0x07, 0x04, 0x03, 0x0A, 0x07, 0x00, 0x03, 0x0A, 0x04, 0x03, 0x0A,
  /* RLE: 020 Pixels @ 292,245*/ 20, 0x00,
  /* ABS: 002 Pixels @ 312,245*/ 0, 2, 0x04, 0x04,
  /* RLE: 008 Pixels @ 314,245*/ 8, 0x03,
  /* ABS: 002 Pixels @ 322,245*/ 0, 2, 0x04, 0x04,
  /* RLE: 064 Pixels @ 324,245*/ 64, 0x00,
  /* RLE: 054 Pixels @ 000,246*/ 54, 0x02,
  /* RLE: 001 Pixels @ 054,246*/ 1, 0x04,
  /* RLE: 006 Pixels @ 055,246*/ 6, 0x03,
  /* RLE: 001 Pixels @ 061,246*/ 1, 0x04,
  /* RLE: 030 Pixels @ 062,246*/ 30, 0x00,
  /* ABS: 005 Pixels @ 092,246*/ 0, 5, 0x03, 0x13, 0x17, 0x16, 0x16,
  /* RLE: 004 Pixels @ 097,246*/ 4, 0x03,
  /* RLE: 003 Pixels @ 101,246*/ 3, 0x16,
  /* RLE: 003 Pixels @ 104,246*/ 3, 0x17,
  /* ABS: 008 Pixels @ 107,246*/ 0, 8, 0x13, 0x03, 0x03, 0x0A, 0x0D, 0x06, 0x03, 0x0B,
  /* RLE: 024 Pixels @ 115,246*/ 24, 0x00,
  /* ABS: 003 Pixels @ 139,246*/ 0, 3, 0x03, 0x07, 0x0A,
  /* RLE: 004 Pixels @ 142,246*/ 4, 0x03,
  /* ABS: 004 Pixels @ 146,246*/ 0, 4, 0x00, 0x0E, 0x03, 0x04,
  /* RLE: 098 Pixels @ 150,246*/ 98, 0x00,
  /* RLE: 001 Pixels @ 248,246*/ 1, 0x04,
  /* RLE: 007 Pixels @ 249,246*/ 7, 0x03,
  /* RLE: 001 Pixels @ 256,246*/ 1, 0x04,
  /* RLE: 008 Pixels @ 257,246*/ 8, 0x00,
  /* ABS: 025 Pixels @ 265,246*/ 0, 25, 0x03, 0x06, 0x0B, 0x03, 0x03, 0x00, 0x07, 0x0B, 0x03, 0x03, 0x0D, 0x00, 0x0A, 0x0F, 0x11, 0x03, 0x06, 0x0D, 0x03, 0x03, 0x00, 0x03, 0x00, 0x00, 0x03,
  /* RLE: 023 Pixels @ 290,246*/ 23, 0x00,
  /* ABS: 002 Pixels @ 313,246*/ 0, 2, 0x04, 0x04,
  /* RLE: 008 Pixels @ 315,246*/ 8, 0x03,
  /* ABS: 002 Pixels @ 323,246*/ 0, 2, 0x04, 0x04,
  /* RLE: 063 Pixels @ 325,246*/ 63, 0x00,
  /* RLE: 054 Pixels @ 000,247*/ 54, 0x02,
  /* RLE: 001 Pixels @ 054,247*/ 1, 0x04,
  /* RLE: 007 Pixels @ 055,247*/ 7, 0x03,
  /* RLE: 030 Pixels @ 062,247*/ 30, 0x00,
  /* ABS: 005 Pixels @ 092,247*/ 0, 5, 0x03, 0x13, 0x17, 0x16, 0x16,
  /* RLE: 004 Pixels @ 097,247*/ 4, 0x03,
  /* RLE: 004 Pixels @ 101,247*/ 4, 0x16,
  /* ABS: 010 Pixels @ 105,247*/ 0, 10, 0x17, 0x17, 0x13, 0x03, 0x07, 0x07, 0x07, 0x0A, 0x03, 0x0B,
  /* RLE: 024 Pixels @ 115,247*/ 24, 0x00,
  /* ABS: 011 Pixels @ 139,247*/ 0, 11, 0x03, 0x0D, 0x0F, 0x00, 0x03, 0x00, 0x12, 0x07, 0x07, 0x03, 0x04,
  /* RLE: 099 Pixels @ 150,247*/ 99, 0x00,
  /* RLE: 001 Pixels @ 249,247*/ 1, 0x04,
  /* RLE: 006 Pixels @ 250,247*/ 6, 0x03,
  /* ABS: 002 Pixels @ 256,247*/ 0, 2, 0x04, 0x04,
  /* RLE: 007 Pixels @ 258,247*/ 7, 0x00,
  /* ABS: 021 Pixels @ 265,247*/ 0, 21, 0x03, 0x11, 0x11, 0x03, 0x03, 0x00, 0x0F, 0x11, 0x03, 0x03, 0x03, 0x0E, 0x0D, 0x0B, 0x07, 0x0A, 0x0A, 0x04, 0x03, 0x03, 0x0A,
  /* RLE: 028 Pixels @ 286,247*/ 28, 0x00,
  /* ABS: 002 Pixels @ 314,247*/ 0, 2, 0x04, 0x04,
  /* RLE: 008 Pixels @ 316,247*/ 8, 0x03,
  /* ABS: 002 Pixels @ 324,247*/ 0, 2, 0x04, 0x04,
  /* RLE: 062 Pixels @ 326,247*/ 62, 0x00,
  /* RLE: 055 Pixels @ 000,248*/ 55, 0x02,
  /* RLE: 001 Pixels @ 055,248*/ 1, 0x04,
  /* RLE: 006 Pixels @ 056,248*/ 6, 0x03,
  /* RLE: 001 Pixels @ 062,248*/ 1, 0x04,
  /* RLE: 029 Pixels @ 063,248*/ 29, 0x00,
  /* ABS: 008 Pixels @ 092,248*/ 0, 8, 0x03, 0x13, 0x17, 0x16, 0x16, 0x16, 0x03, 0x03,
  /* RLE: 005 Pixels @ 100,248*/ 5, 0x16,
  /* ABS: 010 Pixels @ 105,248*/ 0, 10, 0x17, 0x17, 0x13, 0x03, 0x0B, 0x0A, 0x03, 0x03, 0x18, 0x0B,
  /* RLE: 024 Pixels @ 115,248*/ 24, 0x00,
  /* ABS: 010 Pixels @ 139,248*/ 0, 10, 0x03, 0x00, 0x0F, 0x0D, 0x06, 0x07, 0x07, 0x12, 0x0A, 0x03,
  /* RLE: 100 Pixels @ 149,248*/ 100, 0x00,
  /* RLE: 001 Pixels @ 249,248*/ 1, 0x04,
  /* RLE: 007 Pixels @ 250,248*/ 7, 0x03,
  /* RLE: 001 Pixels @ 257,248*/ 1, 0x0B,
  /* RLE: 007 Pixels @ 258,248*/ 7, 0x00,
  /* ABS: 018 Pixels @ 265,248*/ 0, 18, 0x03, 0x04, 0x07, 0x04, 0x12, 0x07, 0x0F, 0x07, 0x0A, 0x03, 0x00, 0x07, 0x00, 0x00, 0x07, 0x0D, 0x03, 0x03,
  /* RLE: 032 Pixels @ 283,248*/ 32, 0x00,
  /* ABS: 002 Pixels @ 315,248*/ 0, 2, 0x04, 0x04,
  /* RLE: 008 Pixels @ 317,248*/ 8, 0x03,
  /* RLE: 001 Pixels @ 325,248*/ 1, 0x04,
  /* RLE: 062 Pixels @ 326,248*/ 62, 0x00,
  /* RLE: 055 Pixels @ 000,249*/ 55, 0x02,
  /* RLE: 001 Pixels @ 055,249*/ 1, 0x04,
  /* RLE: 006 Pixels @ 056,249*/ 6, 0x03,
  /* RLE: 001 Pixels @ 062,249*/ 1, 0x04,
  /* RLE: 029 Pixels @ 063,249*/ 29, 0x00,
  /* ABS: 003 Pixels @ 092,249*/ 0, 3, 0x03, 0x13, 0x17,
  /* RLE: 009 Pixels @ 095,249*/ 9, 0x16,
  /* RLE: 003 Pixels @ 104,249*/ 3, 0x17,
  /* ABS: 009 Pixels @ 107,249*/ 0, 9, 0x13, 0x03, 0x03, 0x00, 0x0B, 0x00, 0x03, 0x1B, 0x0B,
  /* RLE: 023 Pixels @ 116,249*/ 23, 0x00,
  /* ABS: 009 Pixels @ 139,249*/ 0, 9, 0x03, 0x03, 0x0A, 0x07, 0x07, 0x0B, 0x00, 0x03, 0x03,
  /* RLE: 102 Pixels @ 148,249*/ 102, 0x00,
  /* ABS: 002 Pixels @ 250,249*/ 0, 2, 0x04, 0x03,
  /* RLE: 006 Pixels @ 252,249*/ 6, 0x08,
  /* ABS: 024 Pixels @ 258,249*/ 0, 24, 0x0B, 0x0B, 0x00, 0x00, 0x00, 0x04, 0x04, 0x03, 0x03, 0x0F, 0x07, 0x12, 0x0A, 0x03, 0x0F, 0x12, 0x03, 0x00, 0x07, 0x11, 0x11, 0x00, 0x00, 0x03,
  /* RLE: 034 Pixels @ 282,249*/ 34, 0x00,
  /* ABS: 002 Pixels @ 316,249*/ 0, 2, 0x04, 0x04,
  /* RLE: 008 Pixels @ 318,249*/ 8, 0x03,
  /* RLE: 062 Pixels @ 326,249*/ 62, 0x00,
  /* RLE: 055 Pixels @ 000,250*/ 55, 0x02,
  /* RLE: 001 Pixels @ 055,250*/ 1, 0x04,
  /* RLE: 006 Pixels @ 056,250*/ 6, 0x03,
  /* RLE: 001 Pixels @ 062,250*/ 1, 0x04,
  /* RLE: 029 Pixels @ 063,250*/ 29, 0x00,
  /* ABS: 004 Pixels @ 092,250*/ 0, 4, 0x03, 0x13, 0x17, 0x17,
  /* RLE: 007 Pixels @ 096,250*/ 7, 0x16,
  /* RLE: 004 Pixels @ 103,250*/ 4, 0x17,
  /* ABS: 009 Pixels @ 107,250*/ 0, 9, 0x13, 0x03, 0x03, 0x0F, 0x07, 0x0F, 0x00, 0x18, 0x0B,
  /* RLE: 022 Pixels @ 116,250*/ 22, 0x00,
  /* ABS: 011 Pixels @ 138,250*/ 0, 11, 0x03, 0x00, 0x04, 0x00, 0x0B, 0x07, 0x04, 0x03, 0x03, 0x03, 0x0A,
  /* RLE: 101 Pixels @ 149,250*/ 101, 0x00,
  /* ABS: 009 Pixels @ 250,250*/ 0, 9, 0x04, 0x03, 0x1B, 0x18, 0x18, 0x18, 0x08, 0x08, 0x1B,
  /* RLE: 007 Pixels @ 259,250*/ 7, 0x03,
  /* ABS: 015 Pixels @ 266,250*/ 0, 15, 0x0A, 0x0E, 0x07, 0x00, 0x03, 0x03, 0x0E, 0x07, 0x00, 0x03, 0x00, 0x0B, 0x00, 0x03, 0x03,
  /* RLE: 036 Pixels @ 281,250*/ 36, 0x00,
  /* ABS: 002 Pixels @ 317,250*/ 0, 2, 0x04, 0x04,
  /* RLE: 007 Pixels @ 319,250*/ 7, 0x03,
  /* RLE: 001 Pixels @ 326,250*/ 1, 0x04,
  /* RLE: 061 Pixels @ 327,250*/ 61, 0x00,
  /* RLE: 055 Pixels @ 000,251*/ 55, 0x02,
  /* RLE: 001 Pixels @ 055,251*/ 1, 0x04,
  /* RLE: 006 Pixels @ 056,251*/ 6, 0x03,
  /* RLE: 001 Pixels @ 062,251*/ 1, 0x04,
  /* RLE: 029 Pixels @ 063,251*/ 29, 0x00,
  /* ABS: 002 Pixels @ 092,251*/ 0, 2, 0x03, 0x13,
  /* RLE: 004 Pixels @ 094,251*/ 4, 0x17,
  /* RLE: 004 Pixels @ 098,251*/ 4, 0x16,
  /* RLE: 005 Pixels @ 102,251*/ 5, 0x17,
  /* ABS: 009 Pixels @ 107,251*/ 0, 9, 0x13, 0x03, 0x0A, 0x07, 0x04, 0x06, 0x0B, 0x03, 0x0B,
  /* RLE: 022 Pixels @ 116,251*/ 22, 0x00,
  /* ABS: 002 Pixels @ 138,251*/ 0, 2, 0x03, 0x12,
  /* RLE: 004 Pixels @ 140,251*/ 4, 0x07,
  /* ABS: 005 Pixels @ 144,251*/ 0, 5, 0x0F, 0x12, 0x0B, 0x00, 0x03,
  /* RLE: 102 Pixels @ 149,251*/ 102, 0x00,
  /* ABS: 027 Pixels @ 251,251*/ 0, 27, 0x08, 0x18, 0x00, 0x04, 0x03, 0x1B, 0x18, 0x03, 0x04, 0x06, 0x06, 0x04, 0x03, 0x0E, 0x07, 0x07, 0x00, 0x07, 0x0B, 0x03, 0x03, 0x00, 0x06, 0x00, 0x03, 0x00, 0x03,
  /* RLE: 040 Pixels @ 278,251*/ 40, 0x00,
  /* ABS: 002 Pixels @ 318,251*/ 0, 2, 0x04, 0x04,
  /* RLE: 006 Pixels @ 320,251*/ 6, 0x03,
  /* RLE: 001 Pixels @ 326,251*/ 1, 0x04,
  /* RLE: 061 Pixels @ 327,251*/ 61, 0x00,
  /* RLE: 055 Pixels @ 000,252*/ 55, 0x02,
  /* RLE: 001 Pixels @ 055,252*/ 1, 0x04,
  /* RLE: 006 Pixels @ 056,252*/ 6, 0x03,
  /* RLE: 001 Pixels @ 062,252*/ 1, 0x04,
  /* RLE: 029 Pixels @ 063,252*/ 29, 0x00,
  /* ABS: 003 Pixels @ 092,252*/ 0, 3, 0x03, 0x03, 0x13,
  /* RLE: 011 Pixels @ 095,252*/ 11, 0x17,
  /* ABS: 010 Pixels @ 106,252*/ 0, 10, 0x13, 0x03, 0x03, 0x00, 0x07, 0x0A, 0x0E, 0x0B, 0x03, 0x0B,
  /* RLE: 023 Pixels @ 116,252*/ 23, 0x00,
  /* ABS: 010 Pixels @ 139,252*/ 0, 10, 0x03, 0x0A, 0x04, 0x0B, 0x12, 0x06, 0x07, 0x07, 0x0A, 0x03,
  /* RLE: 102 Pixels @ 149,252*/ 102, 0x00,
  /* ABS: 019 Pixels @ 251,252*/ 0, 19, 0x18, 0x03, 0x0E, 0x11, 0x0A, 0x03, 0x03, 0x0B, 0x0F, 0x0E, 0x12, 0x07, 0x04, 0x0E, 0x0E, 0x00, 0x03, 0x06, 0x11,
  /* RLE: 005 Pixels @ 270,252*/ 5, 0x03,
  /* RLE: 045 Pixels @ 275,252*/ 45, 0x00,
  /* RLE: 007 Pixels @ 320,252*/ 7, 0x03,
  /* RLE: 001 Pixels @ 327,252*/ 1, 0x04,
  /* RLE: 060 Pixels @ 328,252*/ 60, 0x00,
  /* RLE: 055 Pixels @ 000,253*/ 55, 0x02,
  /* RLE: 001 Pixels @ 055,253*/ 1, 0x04,
  /* RLE: 006 Pixels @ 056,253*/ 6, 0x03,
  /* RLE: 001 Pixels @ 062,253*/ 1, 0x04,
  /* RLE: 030 Pixels @ 063,253*/ 30, 0x00,
  /* ABS: 003 Pixels @ 093,253*/ 0, 3, 0x03, 0x03, 0x13,
  /* RLE: 009 Pixels @ 096,253*/ 9, 0x17,
  /* ABS: 011 Pixels @ 105,253*/ 0, 11, 0x13, 0x03, 0x03, 0x11, 0x0B, 0x07, 0x00, 0x11, 0x04, 0x03, 0x0B,
  /* RLE: 021 Pixels @ 116,253*/ 21, 0x00,
  /* ABS: 002 Pixels @ 137,253*/ 0, 2, 0x04, 0x04,
  /* RLE: 009 Pixels @ 139,253*/ 9, 0x03,
  /* RLE: 097 Pixels @ 148,253*/ 97, 0x00,
  /* ABS: 019 Pixels @ 245,253*/ 0, 19, 0x03, 0x00, 0x00, 0x00, 0x03, 0x00, 0x04, 0x03, 0x12, 0x07, 0x0F, 0x0A, 0x03, 0x11, 0x0E, 0x03, 0x03, 0x0D, 0x0D,
  /* RLE: 004 Pixels @ 264,253*/ 4, 0x03,
  /* ABS: 004 Pixels @ 268,253*/ 0, 4, 0x0A, 0x04, 0x03, 0x04,
  /* RLE: 048 Pixels @ 272,253*/ 48, 0x00,
  /* RLE: 001 Pixels @ 320,253*/ 1, 0x04,
  /* RLE: 006 Pixels @ 321,253*/ 6, 0x03,
  /* RLE: 001 Pixels @ 327,253*/ 1, 0x04,
  /* RLE: 060 Pixels @ 328,253*/ 60, 0x00,
  /* RLE: 056 Pixels @ 000,254*/ 56, 0x02,
  /* RLE: 007 Pixels @ 056,254*/ 7, 0x03,
  /* RLE: 001 Pixels @ 063,254*/ 1, 0x04,
  /* RLE: 030 Pixels @ 064,254*/ 30, 0x00,
  /* ABS: 004 Pixels @ 094,254*/ 0, 4, 0x03, 0x03, 0x13, 0x13,
  /* RLE: 005 Pixels @ 098,254*/ 5, 0x17,
  /* ABS: 014 Pixels @ 103,254*/ 0, 14, 0x13, 0x13, 0x03, 0x03, 0x03, 0x0D, 0x07, 0x0F, 0x03, 0x0A, 0x03, 0x03, 0x18, 0x0A,
  /* RLE: 018 Pixels @ 117,254*/ 18, 0x00,
  /* RLE: 003 Pixels @ 135,254*/ 3, 0x04,
  /* RLE: 007 Pixels @ 138,254*/ 7, 0x03,
  /* ABS: 002 Pixels @ 145,254*/ 0, 2, 0x04, 0x04,
  /* RLE: 096 Pixels @ 147,254*/ 96, 0x00,
  /* ABS: 021 Pixels @ 243,254*/ 0, 21, 0x03, 0x03, 0x00, 0x00, 0x03, 0x03, 0x03, 0x0E, 0x11, 0x0A, 0x0E, 0x0F, 0x0E, 0x03, 0x03, 0x11, 0x12, 0x03, 0x03, 0x0B, 0x0F,
  /* RLE: 005 Pixels @ 264,254*/ 5, 0x03,
  /* RLE: 051 Pixels @ 269,254*/ 51, 0x00,
  /* RLE: 001 Pixels @ 320,254*/ 1, 0x04,
  /* RLE: 007 Pixels @ 321,254*/ 7, 0x03,
  /* RLE: 060 Pixels @ 328,254*/ 60, 0x00,
  /* RLE: 056 Pixels @ 000,255*/ 56, 0x02,
  /* RLE: 001 Pixels @ 056,255*/ 1, 0x04,
  /* RLE: 006 Pixels @ 057,255*/ 6, 0x03,
  /* RLE: 001 Pixels @ 063,255*/ 1, 0x04,
  /* RLE: 028 Pixels @ 064,255*/ 28, 0x00,
  /* ABS: 006 Pixels @ 092,255*/ 0, 6, 0x0A, 0x00, 0x00, 0x03, 0x13, 0x00,
  /* RLE: 005 Pixels @ 098,255*/ 5, 0x13,
  /* RLE: 003 Pixels @ 103,255*/ 3, 0x03,
  /* ABS: 011 Pixels @ 106,255*/ 0, 11, 0x00, 0x03, 0x03, 0x0A, 0x03, 0x03, 0x0A, 0x11, 0x0E, 0x00, 0x03,
  /* RLE: 016 Pixels @ 117,255*/ 16, 0x00,
  /* ABS: 002 Pixels @ 133,255*/ 0, 2, 0x04, 0x04,
  /* RLE: 010 Pixels @ 135,255*/ 10, 0x03,
  /* RLE: 001 Pixels @ 145,255*/ 1, 0x04,
  /* RLE: 096 Pixels @ 146,255*/ 96, 0x00,
  /* ABS: 026 Pixels @ 242,255*/ 0, 26, 0x03, 0x00, 0x06, 0x07, 0x07, 0x07, 0x0E, 0x03, 0x12, 0x07, 0x0F, 0x0A, 0x0E, 0x07, 0x00, 0x03, 0x0B, 0x07, 0x0B, 0x0A, 0x11, 0x0E, 0x03, 0x03, 0x04, 0x04,
  /* RLE: 053 Pixels @ 268,255*/ 53, 0x00,
  /* RLE: 001 Pixels @ 321,255*/ 1, 0x04,
  /* RLE: 006 Pixels @ 322,255*/ 6, 0x03,
  /* RLE: 001 Pixels @ 328,255*/ 1, 0x04,
  /* RLE: 059 Pixels @ 329,255*/ 59, 0x00,
  /* RLE: 056 Pixels @ 000,256*/ 56, 0x02,
  /* RLE: 001 Pixels @ 056,256*/ 1, 0x04,
  /* RLE: 006 Pixels @ 057,256*/ 6, 0x03,
  /* RLE: 001 Pixels @ 063,256*/ 1, 0x04,
  /* RLE: 029 Pixels @ 064,256*/ 29, 0x00,
  /* ABS: 006 Pixels @ 093,256*/ 0, 6, 0x0A, 0x03, 0x13, 0x00, 0x13, 0x13,
  /* RLE: 005 Pixels @ 099,256*/ 5, 0x03,
  /* RLE: 004 Pixels @ 104,256*/ 4, 0x00,
  /* ABS: 009 Pixels @ 108,256*/ 0, 9, 0x03, 0x04, 0x0E, 0x11, 0x07, 0x07, 0x07, 0x06, 0x03,
  /* RLE: 013 Pixels @ 117,256*/ 13, 0x00,
  /* RLE: 003 Pixels @ 130,256*/ 3, 0x04,
  /* RLE: 011 Pixels @ 133,256*/ 11, 0x03,
  /* RLE: 001 Pixels @ 144,256*/ 1, 0x04,
  /* RLE: 096 Pixels @ 145,256*/ 96, 0x00,
  /* ABS: 023 Pixels @ 241,256*/ 0, 23, 0x03, 0x03, 0x11, 0x0D, 0x04, 0x00, 0x0E, 0x07, 0x0B, 0x0E, 0x0F, 0x0E, 0x03, 0x00, 0x07, 0x0B, 0x00, 0x03, 0x12, 0x07, 0x07, 0x0E, 0x03,
  /* RLE: 057 Pixels @ 264,256*/ 57, 0x00,
  /* RLE: 001 Pixels @ 321,256*/ 1, 0x04,
  /* RLE: 006 Pixels @ 322,256*/ 6, 0x03,
  /* RLE: 001 Pixels @ 328,256*/ 1, 0x04,
  /* RLE: 059 Pixels @ 329,256*/ 59, 0x00,
  /* RLE: 056 Pixels @ 000,257*/ 56, 0x02,
  /* RLE: 001 Pixels @ 056,257*/ 1, 0x04,
  /* RLE: 006 Pixels @ 057,257*/ 6, 0x03,
  /* RLE: 001 Pixels @ 063,257*/ 1, 0x04,
  /* RLE: 029 Pixels @ 064,257*/ 29, 0x00,
  /* ABS: 007 Pixels @ 093,257*/ 0, 7, 0x03, 0x03, 0x13, 0x00, 0x13, 0x13, 0x03,
  /* RLE: 007 Pixels @ 100,257*/ 7, 0x00,
  /* ABS: 009 Pixels @ 107,257*/ 0, 9, 0x03, 0x0E, 0x07, 0x0F, 0x06, 0x0B, 0x0F, 0x04, 0x03,
  /* RLE: 012 Pixels @ 116,257*/ 12, 0x00,
  /* RLE: 003 Pixels @ 128,257*/ 3, 0x04,
  /* RLE: 012 Pixels @ 131,257*/ 12, 0x03,
  /* ABS: 002 Pixels @ 143,257*/ 0, 2, 0x04, 0x04,
  /* RLE: 096 Pixels @ 145,257*/ 96, 0x00,
  /* ABS: 017 Pixels @ 241,257*/ 0, 17, 0x03, 0x04, 0x07, 0x0A, 0x03, 0x03, 0x03, 0x11, 0x0F, 0x03, 0x0E, 0x07, 0x00, 0x03, 0x0E, 0x07, 0x0F,
  /* RLE: 005 Pixels @ 258,257*/ 5, 0x03,
  /* RLE: 059 Pixels @ 263,257*/ 59, 0x00,
  /* RLE: 007 Pixels @ 322,257*/ 7, 0x03,
  /* RLE: 001 Pixels @ 329,257*/ 1, 0x04,
  /* RLE: 058 Pixels @ 330,257*/ 58, 0x00,
  /* RLE: 056 Pixels @ 000,258*/ 56, 0x02,
  /* RLE: 001 Pixels @ 056,258*/ 1, 0x04,
  /* RLE: 006 Pixels @ 057,258*/ 6, 0x03,
  /* RLE: 001 Pixels @ 063,258*/ 1, 0x04,
  /* RLE: 029 Pixels @ 064,258*/ 29, 0x00,
  /* ABS: 007 Pixels @ 093,258*/ 0, 7, 0x03, 0x13, 0x0A, 0x13, 0x13, 0x03, 0x03,
  /* RLE: 007 Pixels @ 100,258*/ 7, 0x00,
  /* ABS: 011 Pixels @ 107,258*/ 0, 11, 0x03, 0x0E, 0x06, 0x03, 0x03, 0x03, 0x00, 0x0B, 0x03, 0x08, 0x0B,
  /* RLE: 008 Pixels @ 118,258*/ 8, 0x00,
  /* ABS: 002 Pixels @ 126,258*/ 0, 2, 0x04, 0x04,
  /* RLE: 014 Pixels @ 128,258*/ 14, 0x03,
  /* ABS: 002 Pixels @ 142,258*/ 0, 2, 0x04, 0x04,
  /* RLE: 094 Pixels @ 144,258*/ 94, 0x00,
  /* ABS: 021 Pixels @ 238,258*/ 0, 21, 0x04, 0x04, 0x03, 0x03, 0x0B, 0x07, 0x00, 0x03, 0x03, 0x03, 0x0B, 0x07, 0x00, 0x00, 0x07, 0x0B, 0x00, 0x03, 0x0A, 0x00, 0x03,
  /* RLE: 004 Pixels @ 259,258*/ 4, 0x08,
  /* RLE: 001 Pixels @ 263,258*/ 1, 0x0B,
  /* RLE: 058 Pixels @ 264,258*/ 58, 0x00,
  /* RLE: 001 Pixels @ 322,258*/ 1, 0x04,
  /* RLE: 006 Pixels @ 323,258*/ 6, 0x03,
  /* RLE: 001 Pixels @ 329,258*/ 1, 0x04,
  /* RLE: 058 Pixels @ 330,258*/ 58, 0x00,
  /* RLE: 056 Pixels @ 000,259*/ 56, 0x02,
  /* RLE: 001 Pixels @ 056,259*/ 1, 0x04,
  /* RLE: 006 Pixels @ 057,259*/ 6, 0x03,
  /* RLE: 001 Pixels @ 063,259*/ 1, 0x04,
  /* RLE: 029 Pixels @ 064,259*/ 29, 0x00,
  /* ABS: 006 Pixels @ 093,259*/ 0, 6, 0x03, 0x13, 0x00, 0x13, 0x13, 0x03,
  /* RLE: 008 Pixels @ 099,259*/ 8, 0x00,
  /* ABS: 011 Pixels @ 107,259*/ 0, 11, 0x03, 0x00, 0x0A, 0x0E, 0x0D, 0x07, 0x07, 0x07, 0x03, 0x1B, 0x0B,
  /* RLE: 005 Pixels @ 118,259*/ 5, 0x00,
  /* RLE: 003 Pixels @ 123,259*/ 3, 0x04,
  /* RLE: 015 Pixels @ 126,259*/ 15, 0x03,
  /* ABS: 002 Pixels @ 141,259*/ 0, 2, 0x04, 0x04,
  /* RLE: 093 Pixels @ 143,259*/ 93, 0x00,
  /* ABS: 002 Pixels @ 236,259*/ 0, 2, 0x04, 0x04,
  /* RLE: 004 Pixels @ 238,259*/ 4, 0x03,
  /* ABS: 021 Pixels @ 242,259*/ 0, 21, 0x04, 0x07, 0x04, 0x03, 0x03, 0x03, 0x00, 0x07, 0x00, 0x03, 0x0E, 0x07, 0x0F, 0x03, 0x18, 0x18, 0x1B, 0x08, 0x08, 0x08, 0x0B,
  /* RLE: 059 Pixels @ 263,259*/ 59, 0x00,
  /* RLE: 001 Pixels @ 322,259*/ 1, 0x04,
  /* RLE: 007 Pixels @ 323,259*/ 7, 0x03,
  /* RLE: 058 Pixels @ 330,259*/ 58, 0x00,
  /* RLE: 057 Pixels @ 000,260*/ 57, 0x02,
  /* RLE: 007 Pixels @ 057,260*/ 7, 0x03,
  /* RLE: 001 Pixels @ 064,260*/ 1, 0x04,
  /* RLE: 027 Pixels @ 065,260*/ 27, 0x00,
  /* ABS: 011 Pixels @ 092,260*/ 0, 11, 0x03, 0x13, 0x00, 0x13, 0x13, 0x03, 0x03, 0x00, 0x0A, 0x00, 0x00,
  /* RLE: 007 Pixels @ 103,260*/ 7, 0x03,
  /* ABS: 013 Pixels @ 110,260*/ 0, 13, 0x07, 0x06, 0x0E, 0x0E, 0x11, 0x00, 0x18, 0x0B, 0x00, 0x00, 0x00, 0x04, 0x04,
  /* RLE: 016 Pixels @ 123,260*/ 16, 0x03,
  /* ABS: 002 Pixels @ 139,260*/ 0, 2, 0x04, 0x04,
  /* RLE: 092 Pixels @ 141,260*/ 92, 0x00,
  /* ABS: 002 Pixels @ 233,260*/ 0, 2, 0x04, 0x04,
  /* RLE: 008 Pixels @ 235,260*/ 8, 0x03,
  /* ABS: 013 Pixels @ 243,260*/ 0, 13, 0x0F, 0x11, 0x03, 0x03, 0x03, 0x12, 0x07, 0x03, 0x03, 0x03, 0x0A, 0x00, 0x03,
  /* RLE: 006 Pixels @ 256,260*/ 6, 0x08,
  /* RLE: 001 Pixels @ 262,260*/ 1, 0x0B,
  /* RLE: 060 Pixels @ 263,260*/ 60, 0x00,
  /* RLE: 001 Pixels @ 323,260*/ 1, 0x04,
  /* RLE: 006 Pixels @ 324,260*/ 6, 0x03,
  /* RLE: 001 Pixels @ 330,260*/ 1, 0x04,
  /* RLE: 057 Pixels @ 331,260*/ 57, 0x00,
  /* RLE: 057 Pixels @ 000,261*/ 57, 0x02,
  /* RLE: 001 Pixels @ 057,261*/ 1, 0x04,
  /* RLE: 006 Pixels @ 058,261*/ 6, 0x03,
  /* RLE: 001 Pixels @ 064,261*/ 1, 0x04,
  /* RLE: 026 Pixels @ 065,261*/ 26, 0x00,
  /* ABS: 007 Pixels @ 091,261*/ 0, 7, 0x03, 0x03, 0x13, 0x00, 0x13, 0x13, 0x03,
  /* RLE: 004 Pixels @ 098,261*/ 4, 0x00,
  /* ABS: 002 Pixels @ 102,261*/ 0, 2, 0x03, 0x03,
  /* RLE: 005 Pixels @ 104,261*/ 5, 0x13,
  /* RLE: 005 Pixels @ 109,261*/ 5, 0x03,
  /* ABS: 007 Pixels @ 114,261*/ 0, 7, 0x06, 0x0B, 0x03, 0x0B, 0x00, 0x04, 0x04,
  /* RLE: 015 Pixels @ 121,261*/ 15, 0x03,
  /* RLE: 003 Pixels @ 136,261*/ 3, 0x04,
  /* RLE: 092 Pixels @ 139,261*/ 92, 0x00,
  /* ABS: 002 Pixels @ 231,261*/ 0, 2, 0x04, 0x04,
  /* RLE: 010 Pixels @ 233,261*/ 10, 0x03,
  /* ABS: 013 Pixels @ 243,261*/ 0, 13, 0x0A, 0x07, 0x11, 0x0E, 0x06, 0x07, 0x04, 0x03, 0x00, 0x00, 0x00, 0x18, 0x1B,
  /* RLE: 006 Pixels @ 256,261*/ 6, 0x08,
  /* RLE: 001 Pixels @ 262,261*/ 1, 0x0B,
  /* RLE: 060 Pixels @ 263,261*/ 60, 0x00,
  /* RLE: 001 Pixels @ 323,261*/ 1, 0x04,
  /* RLE: 006 Pixels @ 324,261*/ 6, 0x03,
  /* RLE: 001 Pixels @ 330,261*/ 1, 0x04,
  /* RLE: 057 Pixels @ 331,261*/ 57, 0x00,
  /* RLE: 057 Pixels @ 000,262*/ 57, 0x02,
  /* RLE: 001 Pixels @ 057,262*/ 1, 0x04,
  /* RLE: 006 Pixels @ 058,262*/ 6, 0x03,
  /* RLE: 001 Pixels @ 064,262*/ 1, 0x04,
  /* RLE: 026 Pixels @ 065,262*/ 26, 0x00,
  /* ABS: 011 Pixels @ 091,262*/ 0, 11, 0x03, 0x13, 0x00, 0x13, 0x13, 0x03, 0x03, 0x00, 0x00, 0x0A, 0x03,
  /* RLE: 009 Pixels @ 102,262*/ 9, 0x13,
  /* ABS: 008 Pixels @ 111,262*/ 0, 8, 0x03, 0x1B, 0x03, 0x00, 0x00, 0x18, 0x0B, 0x04,
  /* RLE: 015 Pixels @ 119,262*/ 15, 0x03,
  /* ABS: 002 Pixels @ 134,262*/ 0, 2, 0x04, 0x04,
  /* RLE: 092 Pixels @ 136,262*/ 92, 0x00,
  /* ABS: 002 Pixels @ 228,262*/ 0, 2, 0x04, 0x04,
  /* RLE: 014 Pixels @ 230,262*/ 14, 0x03,
  /* ABS: 006 Pixels @ 244,262*/ 0, 6, 0x0A, 0x12, 0x06, 0x12, 0x0A, 0x03,
  /* RLE: 004 Pixels @ 250,262*/ 4, 0x00,
  /* RLE: 001 Pixels @ 254,262*/ 1, 0x0B,
  /* RLE: 006 Pixels @ 255,262*/ 6, 0x08,
  /* ABS: 003 Pixels @ 261,262*/ 0, 3, 0x03, 0x03, 0x04,
  /* RLE: 060 Pixels @ 264,262*/ 60, 0x00,
  /* RLE: 007 Pixels @ 324,262*/ 7, 0x03,
  /* RLE: 001 Pixels @ 331,262*/ 1, 0x04,
  /* RLE: 056 Pixels @ 332,262*/ 56, 0x00,
  /* RLE: 057 Pixels @ 000,263*/ 57, 0x02,
  /* RLE: 001 Pixels @ 057,263*/ 1, 0x04,
  /* RLE: 006 Pixels @ 058,263*/ 6, 0x03,
  /* RLE: 001 Pixels @ 064,263*/ 1, 0x04,
  /* RLE: 025 Pixels @ 065,263*/ 25, 0x00,
  /* ABS: 011 Pixels @ 090,263*/ 0, 11, 0x03, 0x03, 0x13, 0x00, 0x13, 0x13, 0x03, 0x00, 0x00, 0x00, 0x03,
  /* RLE: 011 Pixels @ 101,263*/ 11, 0x13,
  /* RLE: 001 Pixels @ 112,263*/ 1, 0x03,
  /* RLE: 004 Pixels @ 113,263*/ 4, 0x1B,
  /* RLE: 001 Pixels @ 117,263*/ 1, 0x08,
  /* RLE: 014 Pixels @ 118,263*/ 14, 0x03,
  /* ABS: 002 Pixels @ 132,263*/ 0, 2, 0x04, 0x04,
  /* RLE: 092 Pixels @ 134,263*/ 92, 0x00,
  /* ABS: 002 Pixels @ 226,263*/ 0, 2, 0x04, 0x04,
  /* RLE: 021 Pixels @ 228,263*/ 21, 0x03,
  /* RLE: 007 Pixels @ 249,263*/ 7, 0x00,
  /* RLE: 001 Pixels @ 256,263*/ 1, 0x0B,
  /* RLE: 006 Pixels @ 257,263*/ 6, 0x03,
  /* RLE: 001 Pixels @ 263,263*/ 1, 0x04,
  /* RLE: 060 Pixels @ 264,263*/ 60, 0x00,
  /* RLE: 001 Pixels @ 324,263*/ 1, 0x04,
  /* RLE: 006 Pixels @ 325,263*/ 6, 0x03,
  /* RLE: 001 Pixels @ 331,263*/ 1, 0x04,
  /* RLE: 056 Pixels @ 332,263*/ 56, 0x00,
  /* RLE: 057 Pixels @ 000,264*/ 57, 0x02,
  /* RLE: 001 Pixels @ 057,264*/ 1, 0x04,
  /* RLE: 006 Pixels @ 058,264*/ 6, 0x03,
  /* RLE: 001 Pixels @ 064,264*/ 1, 0x04,
  /* RLE: 024 Pixels @ 065,264*/ 24, 0x00,
  /* ABS: 012 Pixels @ 089,264*/ 0, 12, 0x0A, 0x03, 0x13, 0x00, 0x13, 0x13, 0x03, 0x03, 0x00, 0x00, 0x00, 0x03,
  /* RLE: 011 Pixels @ 101,264*/ 11, 0x13,
  /* RLE: 001 Pixels @ 112,264*/ 1, 0x03,
  /* RLE: 005 Pixels @ 113,264*/ 5, 0x08,
  /* RLE: 011 Pixels @ 118,264*/ 11, 0x03,
  /* RLE: 003 Pixels @ 129,264*/ 3, 0x04,
  /* RLE: 091 Pixels @ 132,264*/ 91, 0x00,
  /* ABS: 002 Pixels @ 223,264*/ 0, 2, 0x04, 0x04,
  /* RLE: 016 Pixels @ 225,264*/ 16, 0x03,
  /* ABS: 002 Pixels @ 241,264*/ 0, 2, 0x04, 0x04,
  /* RLE: 013 Pixels @ 243,264*/ 13, 0x00,
  /* RLE: 001 Pixels @ 256,264*/ 1, 0x04,
  /* RLE: 007 Pixels @ 257,264*/ 7, 0x03,
  /* RLE: 060 Pixels @ 264,264*/ 60, 0x00,
  /* RLE: 001 Pixels @ 324,264*/ 1, 0x04,
  /* RLE: 007 Pixels @ 325,264*/ 7, 0x03,
  /* RLE: 056 Pixels @ 332,264*/ 56, 0x00,
  /* RLE: 057 Pixels @ 000,265*/ 57, 0x02,
  /* RLE: 001 Pixels @ 057,265*/ 1, 0x04,
  /* RLE: 006 Pixels @ 058,265*/ 6, 0x03,
  /* RLE: 001 Pixels @ 064,265*/ 1, 0x04,
  /* RLE: 024 Pixels @ 065,265*/ 24, 0x00,
  /* ABS: 007 Pixels @ 089,265*/ 0, 7, 0x03, 0x03, 0x13, 0x00, 0x13, 0x13, 0x03,
  /* RLE: 004 Pixels @ 096,265*/ 4, 0x00,
  /* RLE: 001 Pixels @ 100,265*/ 1, 0x03,
  /* RLE: 011 Pixels @ 101,265*/ 11, 0x13,
  /* RLE: 001 Pixels @ 112,265*/ 1, 0x03,
  /* RLE: 005 Pixels @ 113,265*/ 5, 0x08,
  /* RLE: 009 Pixels @ 118,265*/ 9, 0x03,
  /* ABS: 002 Pixels @ 127,265*/ 0, 2, 0x04, 0x04,
  /* RLE: 092 Pixels @ 129,265*/ 92, 0x00,
  /* ABS: 002 Pixels @ 221,265*/ 0, 2, 0x04, 0x04,
  /* RLE: 016 Pixels @ 223,265*/ 16, 0x03,
  /* ABS: 002 Pixels @ 239,265*/ 0, 2, 0x04, 0x04,
  /* RLE: 016 Pixels @ 241,265*/ 16, 0x00,
  /* RLE: 001 Pixels @ 257,265*/ 1, 0x04,
  /* RLE: 006 Pixels @ 258,265*/ 6, 0x03,
  /* RLE: 001 Pixels @ 264,265*/ 1, 0x04,
  /* RLE: 060 Pixels @ 265,265*/ 60, 0x00,
  /* RLE: 001 Pixels @ 325,265*/ 1, 0x04,
  /* RLE: 006 Pixels @ 326,265*/ 6, 0x03,
  /* RLE: 001 Pixels @ 332,265*/ 1, 0x04,
  /* RLE: 055 Pixels @ 333,265*/ 55, 0x00,
  /* RLE: 058 Pixels @ 000,266*/ 58, 0x02,
  /* RLE: 007 Pixels @ 058,266*/ 7, 0x03,
  /* RLE: 001 Pixels @ 065,266*/ 1, 0x04,
  /* RLE: 023 Pixels @ 066,266*/ 23, 0x00,
  /* ABS: 007 Pixels @ 089,266*/ 0, 7, 0x03, 0x13, 0x0A, 0x13, 0x13, 0x03, 0x03,
  /* RLE: 004 Pixels @ 096,266*/ 4, 0x00,
  /* RLE: 001 Pixels @ 100,266*/ 1, 0x03,
  /* RLE: 011 Pixels @ 101,266*/ 11, 0x13,
  /* RLE: 001 Pixels @ 112,266*/ 1, 0x03,
  /* RLE: 005 Pixels @ 113,266*/ 5, 0x08,
  /* RLE: 006 Pixels @ 118,266*/ 6, 0x03,
  /* RLE: 003 Pixels @ 124,266*/ 3, 0x04,
  /* RLE: 091 Pixels @ 127,266*/ 91, 0x00,
  /* ABS: 002 Pixels @ 218,266*/ 0, 2, 0x04, 0x04,
  /* RLE: 016 Pixels @ 220,266*/ 16, 0x03,
  /* ABS: 002 Pixels @ 236,266*/ 0, 2, 0x04, 0x04,
  /* RLE: 019 Pixels @ 238,266*/ 19, 0x00,
  /* RLE: 001 Pixels @ 257,266*/ 1, 0x04,
  /* RLE: 006 Pixels @ 258,266*/ 6, 0x03,
  /* RLE: 001 Pixels @ 264,266*/ 1, 0x04,
  /* RLE: 060 Pixels @ 265,266*/ 60, 0x00,
  /* RLE: 001 Pixels @ 325,266*/ 1, 0x04,
  /* RLE: 006 Pixels @ 326,266*/ 6, 0x03,
  /* RLE: 001 Pixels @ 332,266*/ 1, 0x04,
  /* RLE: 055 Pixels @ 333,266*/ 55, 0x00,
  /* RLE: 058 Pixels @ 000,267*/ 58, 0x02,
  /* RLE: 001 Pixels @ 058,267*/ 1, 0x04,
  /* RLE: 006 Pixels @ 059,267*/ 6, 0x03,
  /* RLE: 001 Pixels @ 065,267*/ 1, 0x04,
  /* RLE: 022 Pixels @ 066,267*/ 22, 0x00,
  /* ABS: 012 Pixels @ 088,267*/ 0, 12, 0x03, 0x03, 0x13, 0x00, 0x13, 0x13, 0x03, 0x00, 0x04, 0x04, 0x03, 0x03,
  /* RLE: 011 Pixels @ 100,267*/ 11, 0x13,
  /* ABS: 002 Pixels @ 111,267*/ 0, 2, 0x03, 0x03,
  /* RLE: 005 Pixels @ 113,267*/ 5, 0x08,
  /* RLE: 004 Pixels @ 118,267*/ 4, 0x03,
  /* RLE: 003 Pixels @ 122,267*/ 3, 0x04,
  /* RLE: 090 Pixels @ 125,267*/ 90, 0x00,
  /* RLE: 003 Pixels @ 215,267*/ 3, 0x04,
  /* RLE: 016 Pixels @ 218,267*/ 16, 0x03,
  /* ABS: 002 Pixels @ 234,267*/ 0, 2, 0x04, 0x04,
  /* RLE: 021 Pixels @ 236,267*/ 21, 0x00,
  /* RLE: 001 Pixels @ 257,267*/ 1, 0x04,
  /* RLE: 007 Pixels @ 258,267*/ 7, 0x03,
  /* RLE: 061 Pixels @ 265,267*/ 61, 0x00,
  /* RLE: 007 Pixels @ 326,267*/ 7, 0x03,
  /* RLE: 001 Pixels @ 333,267*/ 1, 0x04,
  /* RLE: 054 Pixels @ 334,267*/ 54, 0x00,
  /* RLE: 058 Pixels @ 000,268*/ 58, 0x02,
  /* RLE: 001 Pixels @ 058,268*/ 1, 0x04,
  /* RLE: 006 Pixels @ 059,268*/ 6, 0x03,
  /* RLE: 001 Pixels @ 065,268*/ 1, 0x04,
  /* RLE: 021 Pixels @ 066,268*/ 21, 0x00,
  /* ABS: 013 Pixels @ 087,268*/ 0, 13, 0x03, 0x03, 0x13, 0x00, 0x13, 0x13, 0x03, 0x04, 0x03, 0x03, 0x03, 0x13, 0x13,
  /* RLE: 004 Pixels @ 100,268*/ 4, 0x03,
  /* RLE: 005 Pixels @ 104,268*/ 5, 0x13,
  /* RLE: 003 Pixels @ 109,268*/ 3, 0x03,
  /* RLE: 007 Pixels @ 112,268*/ 7, 0x08,
  /* ABS: 003 Pixels @ 119,268*/ 0, 3, 0x03, 0x04, 0x04,
  /* RLE: 090 Pixels @ 122,268*/ 90, 0x00,
  /* RLE: 003 Pixels @ 212,268*/ 3, 0x04,
  /* RLE: 016 Pixels @ 215,268*/ 16, 0x03,
  /* ABS: 002 Pixels @ 231,268*/ 0, 2, 0x04, 0x04,
  /* RLE: 025 Pixels @ 233,268*/ 25, 0x00,
  /* RLE: 001 Pixels @ 258,268*/ 1, 0x04,
  /* RLE: 006 Pixels @ 259,268*/ 6, 0x03,
  /* RLE: 001 Pixels @ 265,268*/ 1, 0x04,
  /* RLE: 060 Pixels @ 266,268*/ 60, 0x00,
  /* RLE: 001 Pixels @ 326,268*/ 1, 0x04,
  /* RLE: 006 Pixels @ 327,268*/ 6, 0x03,
  /* RLE: 001 Pixels @ 333,268*/ 1, 0x04,
  /* RLE: 054 Pixels @ 334,268*/ 54, 0x00,
  /* RLE: 058 Pixels @ 000,269*/ 58, 0x02,
  /* RLE: 001 Pixels @ 058,269*/ 1, 0x04,
  /* RLE: 006 Pixels @ 059,269*/ 6, 0x03,
  /* RLE: 001 Pixels @ 065,269*/ 1, 0x04,
  /* RLE: 021 Pixels @ 066,269*/ 21, 0x00,
  /* ABS: 011 Pixels @ 087,269*/ 0, 11, 0x03, 0x03, 0x13, 0x04, 0x13, 0x13, 0x03, 0x03, 0x03, 0x13, 0x13,
  /* RLE: 013 Pixels @ 098,269*/ 13, 0x03,
  /* RLE: 008 Pixels @ 111,269*/ 8, 0x08,
  /* RLE: 001 Pixels @ 119,269*/ 1, 0x0B,
  /* RLE: 091 Pixels @ 120,269*/ 91, 0x00,
  /* ABS: 002 Pixels @ 211,269*/ 0, 2, 0x04, 0x04,
  /* RLE: 016 Pixels @ 213,269*/ 16, 0x03,
  /* ABS: 002 Pixels @ 229,269*/ 0, 2, 0x04, 0x04,
  /* RLE: 027 Pixels @ 231,269*/ 27, 0x00,
  /* RLE: 001 Pixels @ 258,269*/ 1, 0x04,
  /* RLE: 006 Pixels @ 259,269*/ 6, 0x03,
  /* RLE: 001 Pixels @ 265,269*/ 1, 0x04,
  /* RLE: 060 Pixels @ 266,269*/ 60, 0x00,
  /* RLE: 001 Pixels @ 326,269*/ 1, 0x04,
  /* RLE: 007 Pixels @ 327,269*/ 7, 0x03,
  /* RLE: 054 Pixels @ 334,269*/ 54, 0x00,
  /* RLE: 058 Pixels @ 000,270*/ 58, 0x02,
  /* RLE: 001 Pixels @ 058,270*/ 1, 0x04,
  /* RLE: 006 Pixels @ 059,270*/ 6, 0x03,
  /* RLE: 001 Pixels @ 065,270*/ 1, 0x04,
  /* RLE: 016 Pixels @ 066,270*/ 16, 0x00,
  /* RLE: 004 Pixels @ 082,270*/ 4, 0x04,
  /* ABS: 010 Pixels @ 086,270*/ 0, 10, 0x03, 0x03, 0x13, 0x03, 0x13, 0x13, 0x13, 0x03, 0x13, 0x13,
  /* RLE: 015 Pixels @ 096,270*/ 15, 0x03,
  /* RLE: 008 Pixels @ 111,270*/ 8, 0x08,
  /* RLE: 001 Pixels @ 119,270*/ 1, 0x0B,
  /* RLE: 090 Pixels @ 120,270*/ 90, 0x00,
  /* ABS: 002 Pixels @ 210,270*/ 0, 2, 0x04, 0x04,
  /* RLE: 014 Pixels @ 212,270*/ 14, 0x03,
  /* ABS: 002 Pixels @ 226,270*/ 0, 2, 0x04, 0x04,
  /* RLE: 031 Pixels @ 228,270*/ 31, 0x00,
  /* RLE: 007 Pixels @ 259,270*/ 7, 0x03,
  /* RLE: 001 Pixels @ 266,270*/ 1, 0x04,
  /* RLE: 060 Pixels @ 267,270*/ 60, 0x00,
  /* RLE: 001 Pixels @ 327,270*/ 1, 0x04,
  /* RLE: 006 Pixels @ 328,270*/ 6, 0x03,
  /* RLE: 001 Pixels @ 334,270*/ 1, 0x04,
  /* RLE: 053 Pixels @ 335,270*/ 53, 0x00,
  /* RLE: 058 Pixels @ 000,271*/ 58, 0x02,
  /* RLE: 001 Pixels @ 058,271*/ 1, 0x04,
  /* RLE: 006 Pixels @ 059,271*/ 6, 0x03,
  /* RLE: 001 Pixels @ 065,271*/ 1, 0x04,
  /* RLE: 011 Pixels @ 066,271*/ 11, 0x00,
  /* RLE: 005 Pixels @ 077,271*/ 5, 0x04,
  /* RLE: 006 Pixels @ 082,271*/ 6, 0x03,
  /* ABS: 002 Pixels @ 088,271*/ 0, 2, 0x13, 0x03,
  /* RLE: 004 Pixels @ 090,271*/ 4, 0x13,
  /* RLE: 016 Pixels @ 094,271*/ 16, 0x03,
  /* RLE: 001 Pixels @ 110,271*/ 1, 0x0B,
  /* RLE: 008 Pixels @ 111,271*/ 8, 0x08,
  /* RLE: 001 Pixels @ 119,271*/ 1, 0x0B,
  /* RLE: 089 Pixels @ 120,271*/ 89, 0x00,
  /* ABS: 002 Pixels @ 209,271*/ 0, 2, 0x04, 0x04,
  /* RLE: 013 Pixels @ 211,271*/ 13, 0x03,
  /* ABS: 002 Pixels @ 224,271*/ 0, 2, 0x04, 0x04,
  /* RLE: 033 Pixels @ 226,271*/ 33, 0x00,
  /* RLE: 001 Pixels @ 259,271*/ 1, 0x04,
  /* RLE: 006 Pixels @ 260,271*/ 6, 0x03,
  /* RLE: 001 Pixels @ 266,271*/ 1, 0x04,
  /* RLE: 060 Pixels @ 267,271*/ 60, 0x00,
  /* RLE: 001 Pixels @ 327,271*/ 1, 0x04,
  /* RLE: 006 Pixels @ 328,271*/ 6, 0x03,
  /* RLE: 001 Pixels @ 334,271*/ 1, 0x04,
  /* RLE: 053 Pixels @ 335,271*/ 53, 0x00,
  /* RLE: 059 Pixels @ 000,272*/ 59, 0x02,
  /* RLE: 007 Pixels @ 059,272*/ 7, 0x03,
  /* RLE: 001 Pixels @ 066,272*/ 1, 0x04,
  /* RLE: 006 Pixels @ 067,272*/ 6, 0x00,
  /* RLE: 004 Pixels @ 073,272*/ 4, 0x04,
  /* RLE: 011 Pixels @ 077,272*/ 11, 0x03,
  /* RLE: 004 Pixels @ 088,272*/ 4, 0x13,
  /* RLE: 014 Pixels @ 092,272*/ 14, 0x03,
  /* RLE: 004 Pixels @ 106,272*/ 4, 0x04,
  /* ABS: 002 Pixels @ 110,272*/ 0, 2, 0x00, 0x0B,
  /* RLE: 008 Pixels @ 112,272*/ 8, 0x08,
  /* RLE: 001 Pixels @ 120,272*/ 1, 0x0B,
  /* RLE: 087 Pixels @ 121,272*/ 87, 0x00,
  /* ABS: 002 Pixels @ 208,272*/ 0, 2, 0x04, 0x04,
  /* RLE: 011 Pixels @ 210,272*/ 11, 0x03,
  /* ABS: 002 Pixels @ 221,272*/ 0, 2, 0x04, 0x04,
  /* RLE: 036 Pixels @ 223,272*/ 36, 0x00,
  /* RLE: 001 Pixels @ 259,272*/ 1, 0x04,
  /* RLE: 007 Pixels @ 260,272*/ 7, 0x03,
  /* RLE: 061 Pixels @ 267,272*/ 61, 0x00,
  /* RLE: 007 Pixels @ 328,272*/ 7, 0x03,
  /* RLE: 001 Pixels @ 335,272*/ 1, 0x04,
  /* RLE: 052 Pixels @ 336,272*/ 52, 0x00,
  /* RLE: 059 Pixels @ 000,273*/ 59, 0x02,
  /* RLE: 001 Pixels @ 059,273*/ 1, 0x04,
  /* RLE: 006 Pixels @ 060,273*/ 6, 0x03,
  /* ABS: 002 Pixels @ 066,273*/ 0, 2, 0x04, 0x00,
  /* RLE: 004 Pixels @ 068,273*/ 4, 0x04,
  /* RLE: 029 Pixels @ 072,273*/ 29, 0x03,
  /* RLE: 004 Pixels @ 101,273*/ 4, 0x04,
  /* RLE: 006 Pixels @ 105,273*/ 6, 0x00,
  /* RLE: 001 Pixels @ 111,273*/ 1, 0x0B,
  /* RLE: 008 Pixels @ 112,273*/ 8, 0x08,
  /* RLE: 001 Pixels @ 120,273*/ 1, 0x0B,
  /* RLE: 086 Pixels @ 121,273*/ 86, 0x00,
  /* ABS: 002 Pixels @ 207,273*/ 0, 2, 0x04, 0x04,
  /* RLE: 010 Pixels @ 209,273*/ 10, 0x03,
  /* ABS: 002 Pixels @ 219,273*/ 0, 2, 0x04, 0x04,
  /* RLE: 039 Pixels @ 221,273*/ 39, 0x00,
  /* RLE: 001 Pixels @ 260,273*/ 1, 0x04,
  /* RLE: 006 Pixels @ 261,273*/ 6, 0x03,
  /* RLE: 001 Pixels @ 267,273*/ 1, 0x04,
  /* RLE: 060 Pixels @ 268,273*/ 60, 0x00,
  /* RLE: 001 Pixels @ 328,273*/ 1, 0x04,
  /* RLE: 006 Pixels @ 329,273*/ 6, 0x03,
  /* RLE: 053 Pixels @ 335,273*/ 53, 0x00,
  /* RLE: 057 Pixels @ 000,274*/ 57, 0x02,
  /* RLE: 001 Pixels @ 057,274*/ 1, 0x18,
  /* RLE: 008 Pixels @ 058,274*/ 8, 0x03,
  /* ABS: 002 Pixels @ 066,274*/ 0, 2, 0x04, 0x04,
  /* RLE: 028 Pixels @ 068,274*/ 28, 0x03,
  /* RLE: 005 Pixels @ 096,274*/ 5, 0x04,
  /* RLE: 010 Pixels @ 101,274*/ 10, 0x00,
  /* RLE: 001 Pixels @ 111,274*/ 1, 0x0B,
  /* RLE: 008 Pixels @ 112,274*/ 8, 0x08,
  /* RLE: 001 Pixels @ 120,274*/ 1, 0x0B,
  /* RLE: 085 Pixels @ 121,274*/ 85, 0x00,
  /* ABS: 002 Pixels @ 206,274*/ 0, 2, 0x04, 0x04,
  /* RLE: 008 Pixels @ 208,274*/ 8, 0x03,
  /* ABS: 002 Pixels @ 216,274*/ 0, 2, 0x04, 0x04,
  /* RLE: 042 Pixels @ 218,274*/ 42, 0x00,
  /* RLE: 001 Pixels @ 260,274*/ 1, 0x04,
  /* RLE: 006 Pixels @ 261,274*/ 6, 0x03,
  /* RLE: 001 Pixels @ 267,274*/ 1, 0x04,
  /* RLE: 060 Pixels @ 268,274*/ 60, 0x00,
  /* RLE: 001 Pixels @ 328,274*/ 1, 0x04,
  /* RLE: 004 Pixels @ 329,274*/ 4, 0x03,
  /* RLE: 003 Pixels @ 333,274*/ 3, 0x00,
  /* ABS: 002 Pixels @ 336,274*/ 0, 2, 0x03, 0x03,
  /* RLE: 050 Pixels @ 338,274*/ 50, 0x00,
  /* RLE: 034 Pixels @ 000,275*/ 34, 0x02,
  /* ABS: 002 Pixels @ 034,275*/ 0, 2, 0x00, 0x00,
  /* RLE: 006 Pixels @ 036,275*/ 6, 0x03,
  /* ABS: 019 Pixels @ 042,275*/ 0, 19, 0x00, 0x18, 0x00, 0x03, 0x00, 0x18, 0x02, 0x02, 0x00, 0x00, 0x00, 0x18, 0x02, 0x03, 0x03, 0x03, 0x00, 0x0B, 0x00,
  /* RLE: 031 Pixels @ 061,275*/ 31, 0x03,
  /* RLE: 004 Pixels @ 092,275*/ 4, 0x04,
  /* RLE: 015 Pixels @ 096,275*/ 15, 0x00,
  /* RLE: 001 Pixels @ 111,275*/ 1, 0x0B,
  /* RLE: 008 Pixels @ 112,275*/ 8, 0x08,
  /* RLE: 001 Pixels @ 120,275*/ 1, 0x0B,
  /* RLE: 084 Pixels @ 121,275*/ 84, 0x00,
  /* ABS: 002 Pixels @ 205,275*/ 0, 2, 0x04, 0x04,
  /* RLE: 008 Pixels @ 207,275*/ 8, 0x03,
  /* ABS: 002 Pixels @ 215,275*/ 0, 2, 0x04, 0x04,
  /* RLE: 043 Pixels @ 217,275*/ 43, 0x00,
  /* RLE: 001 Pixels @ 260,275*/ 1, 0x04,
  /* RLE: 007 Pixels @ 261,275*/ 7, 0x03,
  /* RLE: 062 Pixels @ 268,275*/ 62, 0x00,
  /* ABS: 008 Pixels @ 330,275*/ 0, 8, 0x03, 0x0A, 0x11, 0x07, 0x07, 0x07, 0x06, 0x03,
  /* RLE: 050 Pixels @ 338,275*/ 50, 0x00,
  /* RLE: 034 Pixels @ 000,276*/ 34, 0x02,
  /* ABS: 013 Pixels @ 034,276*/ 0, 13, 0x03, 0x04, 0x00, 0x0E, 0x04, 0x00, 0x07, 0x00, 0x03, 0x03, 0x00, 0x0A, 0x00,
  /* RLE: 004 Pixels @ 047,276*/ 4, 0x03,
  /* ABS: 011 Pixels @ 051,276*/ 0, 11, 0x0A, 0x00, 0x03, 0x00, 0x03, 0x11, 0x06, 0x0F, 0x0F, 0x07, 0x00,
  /* RLE: 025 Pixels @ 062,276*/ 25, 0x03,
  /* RLE: 004 Pixels @ 087,276*/ 4, 0x04,
  /* RLE: 020 Pixels @ 091,276*/ 20, 0x00,
  /* RLE: 001 Pixels @ 111,276*/ 1, 0x0B,
  /* RLE: 008 Pixels @ 112,276*/ 8, 0x08,
  /* RLE: 001 Pixels @ 120,276*/ 1, 0x0B,
  /* RLE: 083 Pixels @ 121,276*/ 83, 0x00,
  /* ABS: 002 Pixels @ 204,276*/ 0, 2, 0x04, 0x04,
  /* RLE: 008 Pixels @ 206,276*/ 8, 0x03,
  /* ABS: 002 Pixels @ 214,276*/ 0, 2, 0x04, 0x04,
  /* RLE: 045 Pixels @ 216,276*/ 45, 0x00,
  /* RLE: 001 Pixels @ 261,276*/ 1, 0x04,
  /* RLE: 006 Pixels @ 262,276*/ 6, 0x03,
  /* RLE: 001 Pixels @ 268,276*/ 1, 0x04,
  /* RLE: 060 Pixels @ 269,276*/ 60, 0x00,
  /* ABS: 010 Pixels @ 329,276*/ 0, 10, 0x03, 0x0A, 0x07, 0x11, 0x0B, 0x0A, 0x04, 0x0F, 0x0B, 0x03,
  /* RLE: 049 Pixels @ 339,276*/ 49, 0x00,
  /* RLE: 025 Pixels @ 000,277*/ 25, 0x02,
  /* ABS: 005 Pixels @ 025,277*/ 0, 5, 0x18, 0x03, 0x03, 0x03, 0x18,
  /* RLE: 004 Pixels @ 030,277*/ 4, 0x02,
  /* ABS: 028 Pixels @ 034,277*/ 0, 28, 0x03, 0x07, 0x00, 0x0B, 0x04, 0x03, 0x07, 0x0B, 0x03, 0x0E, 0x07, 0x0D, 0x07, 0x0B, 0x03, 0x04, 0x07, 0x07, 0x07, 0x04, 0x03, 0x03, 0x0D, 0x0F, 0x03, 0x00, 0x07, 0x0B,
  /* RLE: 021 Pixels @ 062,277*/ 21, 0x03,
  /* RLE: 004 Pixels @ 083,277*/ 4, 0x04,
  /* RLE: 025 Pixels @ 087,277*/ 25, 0x00,
  /* RLE: 001 Pixels @ 112,277*/ 1, 0x0B,
  /* RLE: 008 Pixels @ 113,277*/ 8, 0x08,
  /* RLE: 001 Pixels @ 121,277*/ 1, 0x0B,
  /* RLE: 082 Pixels @ 122,277*/ 82, 0x00,
  /* ABS: 002 Pixels @ 204,277*/ 0, 2, 0x04, 0x04,
  /* RLE: 007 Pixels @ 206,277*/ 7, 0x03,
  /* ABS: 002 Pixels @ 213,277*/ 0, 2, 0x04, 0x04,
  /* RLE: 046 Pixels @ 215,277*/ 46, 0x00,
  /* RLE: 001 Pixels @ 261,277*/ 1, 0x04,
  /* RLE: 006 Pixels @ 262,277*/ 6, 0x03,
  /* RLE: 001 Pixels @ 268,277*/ 1, 0x04,
  /* RLE: 060 Pixels @ 269,277*/ 60, 0x00,
  /* ABS: 003 Pixels @ 329,277*/ 0, 3, 0x03, 0x06, 0x11,
  /* RLE: 004 Pixels @ 332,277*/ 4, 0x03,
  /* ABS: 003 Pixels @ 336,277*/ 0, 3, 0x0B, 0x0F, 0x03,
  /* RLE: 049 Pixels @ 339,277*/ 49, 0x00,
  /* RLE: 015 Pixels @ 000,278*/ 15, 0x02,
  /* ABS: 005 Pixels @ 015,278*/ 0, 5, 0x00, 0x03, 0x03, 0x03, 0x00,
  /* RLE: 005 Pixels @ 020,278*/ 5, 0x02,
  /* ABS: 008 Pixels @ 025,278*/ 0, 8, 0x03, 0x0A, 0x07, 0x00, 0x03, 0x03, 0x03, 0x18,
  /* RLE: 005 Pixels @ 033,278*/ 5, 0x03,
  /* ABS: 024 Pixels @ 038,278*/ 0, 24, 0x0B, 0x00, 0x07, 0x0B, 0x03, 0x07, 0x0B, 0x03, 0x00, 0x03, 0x03, 0x0F, 0x0E, 0x03, 0x0B, 0x07, 0x03, 0x03, 0x06, 0x0D, 0x03, 0x03, 0x07, 0x0E,
  /* RLE: 016 Pixels @ 062,278*/ 16, 0x03,
  /* RLE: 004 Pixels @ 078,278*/ 4, 0x04,
  /* RLE: 030 Pixels @ 082,278*/ 30, 0x00,
  /* RLE: 001 Pixels @ 112,278*/ 1, 0x0B,
  /* RLE: 008 Pixels @ 113,278*/ 8, 0x08,
  /* RLE: 001 Pixels @ 121,278*/ 1, 0x0B,
  /* RLE: 082 Pixels @ 122,278*/ 82, 0x00,
  /* RLE: 001 Pixels @ 204,278*/ 1, 0x04,
  /* RLE: 007 Pixels @ 205,278*/ 7, 0x03,
  /* ABS: 002 Pixels @ 212,278*/ 0, 2, 0x04, 0x04,
  /* RLE: 048 Pixels @ 214,278*/ 48, 0x00,
  /* RLE: 007 Pixels @ 262,278*/ 7, 0x03,
  /* RLE: 001 Pixels @ 269,278*/ 1, 0x04,
  /* RLE: 059 Pixels @ 270,278*/ 59, 0x00,
  /* ABS: 003 Pixels @ 329,278*/ 0, 3, 0x03, 0x07, 0x0B,
  /* RLE: 004 Pixels @ 332,278*/ 4, 0x03,
  /* ABS: 003 Pixels @ 336,278*/ 0, 3, 0x00, 0x07, 0x03,
  /* RLE: 049 Pixels @ 339,278*/ 49, 0x00,
  /* RLE: 003 Pixels @ 000,279*/ 3, 0x02,
  /* RLE: 001 Pixels @ 003,279*/ 1, 0x00,
  /* RLE: 006 Pixels @ 004,279*/ 6, 0x03,
  /* RLE: 001 Pixels @ 010,279*/ 1, 0x00,
  /* RLE: 004 Pixels @ 011,279*/ 4, 0x02,
  /* ABS: 007 Pixels @ 015,279*/ 0, 7, 0x00, 0x00, 0x07, 0x00, 0x00, 0x02, 0x18,
  /* RLE: 004 Pixels @ 022,279*/ 4, 0x03,
  /* ABS: 036 Pixels @ 026,279*/ 0, 36, 0x00, 0x07, 0x0B, 0x00, 0x0B, 0x00, 0x03, 0x03, 0x11, 0x0B, 0x03, 0x0A, 0x07, 0x0A, 0x06, 0x06, 0x03, 0x0D, 0x07, 0x07, 0x07, 0x12, 0x03, 0x07, 0x11, 0x06, 0x0F, 0x0F, 0x00, 0x03, 0x0B, 0x07, 0x03, 0x03, 0x06, 0x06,
  /* RLE: 011 Pixels @ 062,279*/ 11, 0x03,
  /* RLE: 004 Pixels @ 073,279*/ 4, 0x04,
  /* RLE: 035 Pixels @ 077,279*/ 35, 0x00,
  /* RLE: 001 Pixels @ 112,279*/ 1, 0x0B,
  /* RLE: 008 Pixels @ 113,279*/ 8, 0x08,
  /* RLE: 001 Pixels @ 121,279*/ 1, 0x0B,
  /* RLE: 081 Pixels @ 122,279*/ 81, 0x00,
  /* RLE: 001 Pixels @ 203,279*/ 1, 0x04,
  /* RLE: 007 Pixels @ 204,279*/ 7, 0x03,
  /* ABS: 002 Pixels @ 211,279*/ 0, 2, 0x04, 0x04,
  /* RLE: 049 Pixels @ 213,279*/ 49, 0x00,
  /* RLE: 001 Pixels @ 262,279*/ 1, 0x04,
  /* RLE: 006 Pixels @ 263,279*/ 6, 0x03,
  /* RLE: 001 Pixels @ 269,279*/ 1, 0x04,
  /* RLE: 059 Pixels @ 270,279*/ 59, 0x00,
  /* ABS: 003 Pixels @ 329,279*/ 0, 3, 0x03, 0x06, 0x0E,
  /* RLE: 004 Pixels @ 332,279*/ 4, 0x03,
  /* ABS: 003 Pixels @ 336,279*/ 0, 3, 0x12, 0x0F, 0x03,
  /* RLE: 049 Pixels @ 339,279*/ 49, 0x00,
  /* RLE: 004 Pixels @ 000,280*/ 4, 0x03,
  /* ABS: 058 Pixels @ 004,280*/ 0, 58, 0x0B, 0x06, 0x04, 0x0A, 0x07, 0x00, 0x03, 0x02, 0x00, 0x00, 0x03, 0x03, 0x00, 0x00, 0x00, 0x03, 0x03, 0x03, 0x00, 0x0B, 0x00, 0x03, 0x03, 0x07, 0x06, 0x0F, 0x11, 0x07, 0x00, 0x03, 0x11, 0x12, 0x03, 0x00, 0x07, 0x0B, 0x06, 0x0D, 0x03, 0x03, 0x0A, 0x04, 0x0E, 0x07, 0x00, 0x07, 0x12, 0x00, 0x00, 0x0A, 0x00, 0x03, 0x04, 0x07, 0x00, 0x03, 0x12, 0x11,
  /* RLE: 007 Pixels @ 062,280*/ 7, 0x03,
  /* RLE: 004 Pixels @ 069,280*/ 4, 0x04,
  /* RLE: 039 Pixels @ 073,280*/ 39, 0x00,
  /* RLE: 001 Pixels @ 112,280*/ 1, 0x0B,
  /* RLE: 008 Pixels @ 113,280*/ 8, 0x08,
  /* RLE: 001 Pixels @ 121,280*/ 1, 0x0B,
  /* RLE: 081 Pixels @ 122,280*/ 81, 0x00,
  /* RLE: 001 Pixels @ 203,280*/ 1, 0x04,
  /* RLE: 006 Pixels @ 204,280*/ 6, 0x03,
  /* ABS: 002 Pixels @ 210,280*/ 0, 2, 0x04, 0x04,
  /* RLE: 050 Pixels @ 212,280*/ 50, 0x00,
  /* RLE: 001 Pixels @ 262,280*/ 1, 0x04,
  /* RLE: 007 Pixels @ 263,280*/ 7, 0x03,
  /* RLE: 059 Pixels @ 270,280*/ 59, 0x00,
  /* ABS: 010 Pixels @ 329,280*/ 0, 10, 0x03, 0x04, 0x07, 0x0B, 0x00, 0x04, 0x06, 0x07, 0x0B, 0x03,
  /* RLE: 049 Pixels @ 339,280*/ 49, 0x00,
  /* ABS: 062 Pixels @ 000,281*/ 0, 62, 0x07, 0x0B, 0x03, 0x0A, 0x07, 0x0E, 0x03, 0x03, 0x07, 0x0B, 0x03, 0x03, 0x03, 0x00, 0x00, 0x03, 0x03, 0x11, 0x0E, 0x03, 0x11, 0x06, 0x0F, 0x0F, 0x07, 0x00, 0x03, 0x0D, 0x0F, 0x03, 0x00, 0x07, 0x0B, 0x03, 0x06, 0x06, 0x03, 0x03, 0x07, 0x0E, 0x0B, 0x07, 0x03, 0x12, 0x0D, 0x0B, 0x12, 0x07, 0x00, 0x0E, 0x07, 0x0E, 0x12, 0x0F, 0x00, 0x03, 0x0A, 0x06, 0x00, 0x03, 0x00, 0x0A,
  /* RLE: 005 Pixels @ 062,281*/ 5, 0x03,
  /* RLE: 001 Pixels @ 067,281*/ 1, 0x04,
  /* RLE: 044 Pixels @ 068,281*/ 44, 0x00,
  /* RLE: 001 Pixels @ 112,281*/ 1, 0x0B,
  /* RLE: 008 Pixels @ 113,281*/ 8, 0x08,
  /* RLE: 001 Pixels @ 121,281*/ 1, 0x0B,
  /* RLE: 081 Pixels @ 122,281*/ 81, 0x00,
  /* RLE: 001 Pixels @ 203,281*/ 1, 0x04,
  /* RLE: 006 Pixels @ 204,281*/ 6, 0x03,
  /* RLE: 001 Pixels @ 210,281*/ 1, 0x04,
  /* RLE: 052 Pixels @ 211,281*/ 52, 0x00,
  /* RLE: 001 Pixels @ 263,281*/ 1, 0x04,
  /* RLE: 006 Pixels @ 264,281*/ 6, 0x03,
  /* RLE: 001 Pixels @ 270,281*/ 1, 0x04,
  /* RLE: 059 Pixels @ 271,281*/ 59, 0x00,
  /* ABS: 009 Pixels @ 330,281*/ 0, 9, 0x03, 0x0E, 0x07, 0x07, 0x07, 0x0F, 0x0B, 0x03, 0x03,
  /* RLE: 049 Pixels @ 339,281*/ 49, 0x00,
  /* ABS: 054 Pixels @ 000,282*/ 0, 54, 0x07, 0x0B, 0x03, 0x0F, 0x0D, 0x03, 0x00, 0x03, 0x07, 0x0B, 0x03, 0x04, 0x07, 0x07, 0x07, 0x04, 0x03, 0x06, 0x06, 0x03, 0x0D, 0x0F, 0x03, 0x00, 0x07, 0x0B, 0x03, 0x06, 0x0D, 0x03, 0x03, 0x07, 0x0E, 0x03, 0x0E, 0x07, 0x03, 0x03, 0x07, 0x06, 0x04, 0x07, 0x00, 0x00, 0x0E, 0x06, 0x0E, 0x00, 0x03, 0x03, 0x0B, 0x06, 0x0E, 0x00,
  /* RLE: 005 Pixels @ 054,282*/ 5, 0x03,
  /* ABS: 002 Pixels @ 059,282*/ 0, 2, 0x00, 0x00,
  /* RLE: 006 Pixels @ 061,282*/ 6, 0x03,
  /* RLE: 001 Pixels @ 067,282*/ 1, 0x04,
  /* RLE: 045 Pixels @ 068,282*/ 45, 0x00,
  /* RLE: 001 Pixels @ 113,282*/ 1, 0x0B,
  /* RLE: 008 Pixels @ 114,282*/ 8, 0x08,
  /* RLE: 001 Pixels @ 122,282*/ 1, 0x0B,
  /* RLE: 080 Pixels @ 123,282*/ 80, 0x00,
  /* RLE: 001 Pixels @ 203,282*/ 1, 0x04,
  /* RLE: 006 Pixels @ 204,282*/ 6, 0x03,
  /* RLE: 001 Pixels @ 210,282*/ 1, 0x04,
  /* RLE: 052 Pixels @ 211,282*/ 52, 0x00,
  /* RLE: 001 Pixels @ 263,282*/ 1, 0x04,
  /* RLE: 006 Pixels @ 264,282*/ 6, 0x03,
  /* RLE: 001 Pixels @ 270,282*/ 1, 0x04,
  /* RLE: 060 Pixels @ 271,282*/ 60, 0x00,
  /* ABS: 010 Pixels @ 331,282*/ 0, 10, 0x03, 0x00, 0x0A, 0x00, 0x03, 0x0B, 0x0E, 0x0B, 0x03, 0x03,
  /* RLE: 047 Pixels @ 341,282*/ 47, 0x00,
  /* ABS: 043 Pixels @ 000,283*/ 0, 43, 0x07, 0x06, 0x12, 0x07, 0x04, 0x03, 0x03, 0x03, 0x06, 0x06, 0x03, 0x0F, 0x0E, 0x03, 0x0B, 0x07, 0x03, 0x12, 0x0D, 0x03, 0x06, 0x0D, 0x03, 0x03, 0x07, 0x0E, 0x03, 0x0B, 0x07, 0x03, 0x03, 0x06, 0x06, 0x03, 0x04, 0x07, 0x12, 0x12, 0x11, 0x11, 0x0A, 0x06, 0x00,
  /* RLE: 007 Pixels @ 043,283*/ 7, 0x03,
  /* ABS: 006 Pixels @ 050,283*/ 0, 6, 0x00, 0x03, 0x03, 0x00, 0x00, 0x04,
  /* RLE: 004 Pixels @ 056,283*/ 4, 0x02,
  /* RLE: 001 Pixels @ 060,283*/ 1, 0x04,
  /* RLE: 006 Pixels @ 061,283*/ 6, 0x03,
  /* RLE: 001 Pixels @ 067,283*/ 1, 0x04,
  /* RLE: 045 Pixels @ 068,283*/ 45, 0x00,
  /* RLE: 001 Pixels @ 113,283*/ 1, 0x0B,
  /* RLE: 008 Pixels @ 114,283*/ 8, 0x08,
  /* RLE: 001 Pixels @ 122,283*/ 1, 0x0B,
  /* RLE: 079 Pixels @ 123,283*/ 79, 0x00,
  /* RLE: 001 Pixels @ 202,283*/ 1, 0x04,
  /* RLE: 007 Pixels @ 203,283*/ 7, 0x03,
  /* RLE: 053 Pixels @ 210,283*/ 53, 0x00,
  /* RLE: 001 Pixels @ 263,283*/ 1, 0x04,
  /* RLE: 007 Pixels @ 264,283*/ 7, 0x03,
  /* RLE: 061 Pixels @ 271,283*/ 61, 0x00,
  /* RLE: 003 Pixels @ 332,283*/ 3, 0x03,
  /* ABS: 006 Pixels @ 335,283*/ 0, 6, 0x0B, 0x11, 0x07, 0x0F, 0x04, 0x03,
  /* RLE: 047 Pixels @ 341,283*/ 47, 0x00,
  /* ABS: 044 Pixels @ 000,284*/ 0, 44, 0x06, 0x0F, 0x07, 0x0F, 0x11, 0x03, 0x03, 0x03, 0x06, 0x0D, 0x03, 0x07, 0x11, 0x06, 0x0F, 0x0F, 0x00, 0x0B, 0x07, 0x03, 0x0B, 0x07, 0x03, 0x03, 0x06, 0x06, 0x03, 0x04, 0x07, 0x00, 0x03, 0x12, 0x11, 0x03, 0x03, 0x0E, 0x06, 0x0B, 0x00, 0x0A, 0x03, 0x03, 0x03, 0x00,
  /* RLE: 005 Pixels @ 044,284*/ 5, 0x04,
  /* RLE: 012 Pixels @ 049,284*/ 12, 0x02,
  /* RLE: 007 Pixels @ 061,284*/ 7, 0x03,
  /* RLE: 001 Pixels @ 068,284*/ 1, 0x04,
  /* RLE: 044 Pixels @ 069,284*/ 44, 0x00,
  /* RLE: 001 Pixels @ 113,284*/ 1, 0x0B,
  /* RLE: 008 Pixels @ 114,284*/ 8, 0x08,
  /* RLE: 001 Pixels @ 122,284*/ 1, 0x0B,
  /* RLE: 079 Pixels @ 123,284*/ 79, 0x00,
  /* RLE: 001 Pixels @ 202,284*/ 1, 0x04,
  /* RLE: 006 Pixels @ 203,284*/ 6, 0x03,
  /* RLE: 001 Pixels @ 209,284*/ 1, 0x04,
  /* RLE: 054 Pixels @ 210,284*/ 54, 0x00,
  /* RLE: 001 Pixels @ 264,284*/ 1, 0x04,
  /* RLE: 006 Pixels @ 265,284*/ 6, 0x03,
  /* RLE: 001 Pixels @ 271,284*/ 1, 0x04,
  /* RLE: 059 Pixels @ 272,284*/ 59, 0x00,
  /* ABS: 010 Pixels @ 331,284*/ 0, 10, 0x03, 0x03, 0x0E, 0x07, 0x07, 0x06, 0x07, 0x00, 0x03, 0x03,
  /* RLE: 047 Pixels @ 341,284*/ 47, 0x00,
  /* ABS: 033 Pixels @ 000,285*/ 0, 33, 0x12, 0x07, 0x12, 0x00, 0x07, 0x0D, 0x03, 0x03, 0x0B, 0x07, 0x03, 0x07, 0x12, 0x0A, 0x00, 0x0A, 0x00, 0x04, 0x07, 0x00, 0x04, 0x07, 0x00, 0x03, 0x12, 0x11, 0x03, 0x0A, 0x06, 0x00, 0x03, 0x00, 0x0A,
  /* RLE: 006 Pixels @ 033,285*/ 6, 0x03,
  /* ABS: 003 Pixels @ 039,285*/ 0, 3, 0x00, 0x00, 0x04,
  /* RLE: 019 Pixels @ 042,285*/ 19, 0x02,
  /* RLE: 001 Pixels @ 061,285*/ 1, 0x04,
  /* RLE: 006 Pixels @ 062,285*/ 6, 0x03,
  /* RLE: 001 Pixels @ 068,285*/ 1, 0x04,
  /* RLE: 044 Pixels @ 069,285*/ 44, 0x00,
  /* RLE: 001 Pixels @ 113,285*/ 1, 0x0B,
  /* RLE: 008 Pixels @ 114,285*/ 8, 0x08,
  /* RLE: 001 Pixels @ 122,285*/ 1, 0x0B,
  /* RLE: 079 Pixels @ 123,285*/ 79, 0x00,
  /* RLE: 001 Pixels @ 202,285*/ 1, 0x04,
  /* RLE: 006 Pixels @ 203,285*/ 6, 0x03,
  /* RLE: 001 Pixels @ 209,285*/ 1, 0x04,
  /* RLE: 054 Pixels @ 210,285*/ 54, 0x00,
  /* RLE: 001 Pixels @ 264,285*/ 1, 0x04,
  /* RLE: 006 Pixels @ 265,285*/ 6, 0x03,
  /* RLE: 001 Pixels @ 271,285*/ 1, 0x04,
  /* RLE: 060 Pixels @ 272,285*/ 60, 0x00,
  /* ABS: 010 Pixels @ 332,285*/ 0, 10, 0x0B, 0x07, 0x0B, 0x00, 0x03, 0x0B, 0x0E, 0x0B, 0x03, 0x03,
  /* RLE: 046 Pixels @ 342,285*/ 46, 0x00,
  /* ABS: 026 Pixels @ 000,286*/ 0, 26, 0x0B, 0x07, 0x00, 0x03, 0x0B, 0x07, 0x12, 0x03, 0x04, 0x07, 0x00, 0x0E, 0x07, 0x0E, 0x12, 0x0F, 0x00, 0x0A, 0x06, 0x00, 0x0A, 0x06, 0x00, 0x03, 0x00, 0x0A,
  /* RLE: 004 Pixels @ 026,286*/ 4, 0x03,
  /* RLE: 004 Pixels @ 030,286*/ 4, 0x00,
  /* RLE: 001 Pixels @ 034,286*/ 1, 0x04,
  /* RLE: 026 Pixels @ 035,286*/ 26, 0x02,
  /* RLE: 001 Pixels @ 061,286*/ 1, 0x04,
  /* RLE: 006 Pixels @ 062,286*/ 6, 0x03,
  /* RLE: 001 Pixels @ 068,286*/ 1, 0x04,
  /* RLE: 044 Pixels @ 069,286*/ 44, 0x00,
  /* RLE: 001 Pixels @ 113,286*/ 1, 0x0B,
  /* RLE: 008 Pixels @ 114,286*/ 8, 0x08,
  /* RLE: 001 Pixels @ 122,286*/ 1, 0x0B,
  /* RLE: 079 Pixels @ 123,286*/ 79, 0x00,
  /* RLE: 001 Pixels @ 202,286*/ 1, 0x04,
  /* RLE: 006 Pixels @ 203,286*/ 6, 0x03,
  /* RLE: 001 Pixels @ 209,286*/ 1, 0x04,
  /* RLE: 055 Pixels @ 210,286*/ 55, 0x00,
  /* RLE: 007 Pixels @ 265,286*/ 7, 0x03,
  /* RLE: 001 Pixels @ 272,286*/ 1, 0x04,
  /* RLE: 057 Pixels @ 273,286*/ 57, 0x00,
  /* ABS: 012 Pixels @ 330,286*/ 0, 12, 0x0A, 0x03, 0x00, 0x07, 0x00, 0x03, 0x04, 0x11, 0x07, 0x0F, 0x04, 0x03,
  /* RLE: 046 Pixels @ 342,286*/ 46, 0x00,
  /* ABS: 016 Pixels @ 000,287*/ 0, 16, 0x04, 0x07, 0x00, 0x03, 0x03, 0x0E, 0x07, 0x0E, 0x0A, 0x06, 0x00, 0x03, 0x0B, 0x06, 0x0E, 0x00,
  /* RLE: 007 Pixels @ 016,287*/ 7, 0x03,
  /* RLE: 004 Pixels @ 023,287*/ 4, 0x00,
  /* RLE: 001 Pixels @ 027,287*/ 1, 0x04,
  /* RLE: 033 Pixels @ 028,287*/ 33, 0x02,
  /* RLE: 001 Pixels @ 061,287*/ 1, 0x04,
  /* RLE: 006 Pixels @ 062,287*/ 6, 0x03,
  /* RLE: 001 Pixels @ 068,287*/ 1, 0x04,
  /* RLE: 045 Pixels @ 069,287*/ 45, 0x00,
  /* RLE: 001 Pixels @ 114,287*/ 1, 0x0B,
  /* RLE: 008 Pixels @ 115,287*/ 8, 0x08,
  /* RLE: 001 Pixels @ 123,287*/ 1, 0x0B,
  /* RLE: 077 Pixels @ 124,287*/ 77, 0x00,
  /* ABS: 002 Pixels @ 201,287*/ 0, 2, 0x04, 0x04,
  /* RLE: 006 Pixels @ 203,287*/ 6, 0x03,
  /* RLE: 056 Pixels @ 209,287*/ 56, 0x00,
  /* RLE: 001 Pixels @ 265,287*/ 1, 0x04,
  /* RLE: 006 Pixels @ 266,287*/ 6, 0x03,
  /* RLE: 001 Pixels @ 272,287*/ 1, 0x04,
  /* RLE: 059 Pixels @ 273,287*/ 59, 0x00,
  /* ABS: 009 Pixels @ 332,287*/ 0, 9, 0x03, 0x03, 0x0E, 0x07, 0x07, 0x06, 0x07, 0x00, 0x03,
  /* RLE: 047 Pixels @ 341,287*/ 47, 0x00,
  /* ABS: 003 Pixels @ 000,288*/ 0, 3, 0x0A, 0x06, 0x0A,
  /* RLE: 012 Pixels @ 003,288*/ 12, 0x03,
  /* ABS: 002 Pixels @ 015,288*/ 0, 2, 0x00, 0x00,
  /* RLE: 004 Pixels @ 017,288*/ 4, 0x04,
  /* RLE: 040 Pixels @ 021,288*/ 40, 0x02,
  /* RLE: 001 Pixels @ 061,288*/ 1, 0x04,
  /* RLE: 006 Pixels @ 062,288*/ 6, 0x03,
  /* RLE: 001 Pixels @ 068,288*/ 1, 0x04,
  /* RLE: 045 Pixels @ 069,288*/ 45, 0x00,
  /* RLE: 001 Pixels @ 114,288*/ 1, 0x0B,
  /* RLE: 008 Pixels @ 115,288*/ 8, 0x08,
  /* RLE: 001 Pixels @ 123,288*/ 1, 0x0B,
  /* RLE: 078 Pixels @ 124,288*/ 78, 0x00,
  /* RLE: 001 Pixels @ 202,288*/ 1, 0x04,
  /* RLE: 006 Pixels @ 203,288*/ 6, 0x03,
  /* RLE: 001 Pixels @ 209,288*/ 1, 0x04,
  /* RLE: 055 Pixels @ 210,288*/ 55, 0x00,
  /* RLE: 001 Pixels @ 265,288*/ 1, 0x04,
  /* RLE: 007 Pixels @ 266,288*/ 7, 0x03,
  /* RLE: 059 Pixels @ 273,288*/ 59, 0x00,
  /* ABS: 010 Pixels @ 332,288*/ 0, 10, 0x03, 0x0B, 0x07, 0x0B, 0x00, 0x03, 0x00, 0x00, 0x03, 0x04,
  /* RLE: 046 Pixels @ 342,288*/ 46, 0x00,
  /* RLE: 007 Pixels @ 000,289*/ 7, 0x03,
  /* RLE: 007 Pixels @ 007,289*/ 7, 0x04,
  /* RLE: 047 Pixels @ 014,289*/ 47, 0x02,
  /* RLE: 001 Pixels @ 061,289*/ 1, 0x04,
  /* RLE: 006 Pixels @ 062,289*/ 6, 0x03,
  /* RLE: 001 Pixels @ 068,289*/ 1, 0x04,
  /* RLE: 045 Pixels @ 069,289*/ 45, 0x00,
  /* RLE: 001 Pixels @ 114,289*/ 1, 0x0B,
  /* RLE: 008 Pixels @ 115,289*/ 8, 0x08,
  /* RLE: 001 Pixels @ 123,289*/ 1, 0x0B,
  /* RLE: 078 Pixels @ 124,289*/ 78, 0x00,
  /* RLE: 001 Pixels @ 202,289*/ 1, 0x04,
  /* RLE: 006 Pixels @ 203,289*/ 6, 0x03,
  /* RLE: 001 Pixels @ 209,289*/ 1, 0x04,
  /* RLE: 056 Pixels @ 210,289*/ 56, 0x00,
  /* RLE: 001 Pixels @ 266,289*/ 1, 0x04,
  /* RLE: 006 Pixels @ 267,289*/ 6, 0x03,
  /* RLE: 001 Pixels @ 273,289*/ 1, 0x04,
  /* RLE: 058 Pixels @ 274,289*/ 58, 0x00,
  /* ABS: 004 Pixels @ 332,289*/ 0, 4, 0x03, 0x00, 0x07, 0x00,
  /* RLE: 005 Pixels @ 336,289*/ 5, 0x03,
  /* RLE: 047 Pixels @ 341,289*/ 47, 0x00,
  /* RLE: 007 Pixels @ 000,290*/ 7, 0x04,
  /* RLE: 055 Pixels @ 007,290*/ 55, 0x02,
  /* RLE: 007 Pixels @ 062,290*/ 7, 0x03,
  /* RLE: 001 Pixels @ 069,290*/ 1, 0x04,
  /* RLE: 044 Pixels @ 070,290*/ 44, 0x00,
  /* RLE: 001 Pixels @ 114,290*/ 1, 0x0B,
  /* RLE: 008 Pixels @ 115,290*/ 8, 0x08,
  /* RLE: 001 Pixels @ 123,290*/ 1, 0x0B,
  /* RLE: 078 Pixels @ 124,290*/ 78, 0x00,
  /* RLE: 001 Pixels @ 202,290*/ 1, 0x04,
  /* RLE: 006 Pixels @ 203,290*/ 6, 0x03,
  /* RLE: 001 Pixels @ 209,290*/ 1, 0x04,
  /* RLE: 056 Pixels @ 210,290*/ 56, 0x00,
  /* RLE: 001 Pixels @ 266,290*/ 1, 0x04,
  /* RLE: 006 Pixels @ 267,290*/ 6, 0x03,
  /* RLE: 001 Pixels @ 273,290*/ 1, 0x04,
  /* RLE: 059 Pixels @ 274,290*/ 59, 0x00,
  /* RLE: 004 Pixels @ 333,290*/ 4, 0x03,
  /* ABS: 005 Pixels @ 337,290*/ 0, 5, 0x0B, 0x06, 0x06, 0x0A, 0x03,
  /* RLE: 046 Pixels @ 342,290*/ 46, 0x00,
  /* RLE: 062 Pixels @ 000,291*/ 62, 0x02,
  /* RLE: 001 Pixels @ 062,291*/ 1, 0x04,
  /* RLE: 006 Pixels @ 063,291*/ 6, 0x03,
  /* RLE: 001 Pixels @ 069,291*/ 1, 0x04,
  /* RLE: 044 Pixels @ 070,291*/ 44, 0x00,
  /* RLE: 001 Pixels @ 114,291*/ 1, 0x0B,
  /* RLE: 008 Pixels @ 115,291*/ 8, 0x08,
  /* RLE: 001 Pixels @ 123,291*/ 1, 0x0B,
  /* RLE: 079 Pixels @ 124,291*/ 79, 0x00,
  /* RLE: 007 Pixels @ 203,291*/ 7, 0x03,
  /* RLE: 001 Pixels @ 210,291*/ 1, 0x04,
  /* RLE: 055 Pixels @ 211,291*/ 55, 0x00,
  /* RLE: 001 Pixels @ 266,291*/ 1, 0x04,
  /* RLE: 007 Pixels @ 267,291*/ 7, 0x03,
  /* RLE: 061 Pixels @ 274,291*/ 61, 0x00,
  /* ABS: 006 Pixels @ 335,291*/ 0, 6, 0x03, 0x06, 0x07, 0x06, 0x12, 0x07,
  /* RLE: 047 Pixels @ 341,291*/ 47, 0x00,
  /* RLE: 062 Pixels @ 000,292*/ 62, 0x02,
  /* RLE: 001 Pixels @ 062,292*/ 1, 0x04,
  /* RLE: 006 Pixels @ 063,292*/ 6, 0x03,
  /* RLE: 001 Pixels @ 069,292*/ 1, 0x04,
  /* RLE: 045 Pixels @ 070,292*/ 45, 0x00,
  /* RLE: 001 Pixels @ 115,292*/ 1, 0x0B,
  /* RLE: 008 Pixels @ 116,292*/ 8, 0x08,
  /* RLE: 001 Pixels @ 124,292*/ 1, 0x0B,
  /* RLE: 078 Pixels @ 125,292*/ 78, 0x00,
  /* RLE: 001 Pixels @ 203,292*/ 1, 0x04,
  /* RLE: 006 Pixels @ 204,292*/ 6, 0x03,
  /* RLE: 001 Pixels @ 210,292*/ 1, 0x04,
  /* RLE: 056 Pixels @ 211,292*/ 56, 0x00,
  /* RLE: 001 Pixels @ 267,292*/ 1, 0x04,
  /* RLE: 006 Pixels @ 268,292*/ 6, 0x03,
  /* RLE: 001 Pixels @ 274,292*/ 1, 0x04,
  /* RLE: 060 Pixels @ 275,292*/ 60, 0x00,
  /* ABS: 008 Pixels @ 335,292*/ 0, 8, 0x03, 0x07, 0x0B, 0x03, 0x03, 0x0E, 0x0E, 0x03,
  /* RLE: 045 Pixels @ 343,292*/ 45, 0x00,
  /* RLE: 062 Pixels @ 000,293*/ 62, 0x02,
  /* RLE: 001 Pixels @ 062,293*/ 1, 0x04,
  /* RLE: 006 Pixels @ 063,293*/ 6, 0x03,
  /* RLE: 001 Pixels @ 069,293*/ 1, 0x04,
  /* RLE: 045 Pixels @ 070,293*/ 45, 0x00,
  /* RLE: 001 Pixels @ 115,293*/ 1, 0x0B,
  /* RLE: 008 Pixels @ 116,293*/ 8, 0x08,
  /* RLE: 001 Pixels @ 124,293*/ 1, 0x0B,
  /* RLE: 078 Pixels @ 125,293*/ 78, 0x00,
  /* RLE: 001 Pixels @ 203,293*/ 1, 0x04,
  /* RLE: 006 Pixels @ 204,293*/ 6, 0x03,
  /* RLE: 001 Pixels @ 210,293*/ 1, 0x04,
  /* RLE: 056 Pixels @ 211,293*/ 56, 0x00,
  /* RLE: 001 Pixels @ 267,293*/ 1, 0x04,
  /* RLE: 006 Pixels @ 268,293*/ 6, 0x03,
  /* RLE: 001 Pixels @ 274,293*/ 1, 0x04,
  /* RLE: 060 Pixels @ 275,293*/ 60, 0x00,
  /* ABS: 009 Pixels @ 335,293*/ 0, 9, 0x03, 0x07, 0x00, 0x03, 0x03, 0x12, 0x12, 0x03, 0x04,
  /* RLE: 044 Pixels @ 344,293*/ 44, 0x00,
  /* RLE: 062 Pixels @ 000,294*/ 62, 0x02,
  /* RLE: 001 Pixels @ 062,294*/ 1, 0x04,
  /* RLE: 006 Pixels @ 063,294*/ 6, 0x03,
  /* RLE: 001 Pixels @ 069,294*/ 1, 0x04,
  /* RLE: 045 Pixels @ 070,294*/ 45, 0x00,
  /* RLE: 001 Pixels @ 115,294*/ 1, 0x0B,
  /* RLE: 008 Pixels @ 116,294*/ 8, 0x08,
  /* RLE: 001 Pixels @ 124,294*/ 1, 0x0B,
  /* RLE: 078 Pixels @ 125,294*/ 78, 0x00,
  /* RLE: 001 Pixels @ 203,294*/ 1, 0x04,
  /* RLE: 006 Pixels @ 204,294*/ 6, 0x03,
  /* RLE: 001 Pixels @ 210,294*/ 1, 0x04,
  /* RLE: 057 Pixels @ 211,294*/ 57, 0x00,
  /* RLE: 007 Pixels @ 268,294*/ 7, 0x03,
  /* RLE: 001 Pixels @ 275,294*/ 1, 0x04,
  /* RLE: 059 Pixels @ 276,294*/ 59, 0x00,
  /* ABS: 009 Pixels @ 335,294*/ 0, 9, 0x03, 0x06, 0x0D, 0x0B, 0x06, 0x07, 0x04, 0x03, 0x04,
  /* RLE: 044 Pixels @ 344,294*/ 44, 0x00,
  /* RLE: 062 Pixels @ 000,295*/ 62, 0x02,
  /* RLE: 001 Pixels @ 062,295*/ 1, 0x04,
  /* RLE: 006 Pixels @ 063,295*/ 6, 0x03,
  /* RLE: 001 Pixels @ 069,295*/ 1, 0x04,
  /* RLE: 045 Pixels @ 070,295*/ 45, 0x00,
  /* RLE: 001 Pixels @ 115,295*/ 1, 0x0B,
  /* RLE: 008 Pixels @ 116,295*/ 8, 0x08,
  /* RLE: 001 Pixels @ 124,295*/ 1, 0x0B,
  /* RLE: 079 Pixels @ 125,295*/ 79, 0x00,
  /* RLE: 007 Pixels @ 204,295*/ 7, 0x03,
  /* RLE: 001 Pixels @ 211,295*/ 1, 0x04,
  /* RLE: 056 Pixels @ 212,295*/ 56, 0x00,
  /* RLE: 001 Pixels @ 268,295*/ 1, 0x04,
  /* RLE: 008 Pixels @ 269,295*/ 8, 0x03,
  /* RLE: 058 Pixels @ 277,295*/ 58, 0x00,
  /* ABS: 010 Pixels @ 335,295*/ 0, 10, 0x03, 0x03, 0x0E, 0x07, 0x11, 0x0A, 0x03, 0x03, 0x03, 0x04,
  /* RLE: 043 Pixels @ 345,295*/ 43, 0x00,
  /* RLE: 063 Pixels @ 000,296*/ 63, 0x02,
  /* RLE: 007 Pixels @ 063,296*/ 7, 0x03,
  /* RLE: 001 Pixels @ 070,296*/ 1, 0x04,
  /* RLE: 045 Pixels @ 071,296*/ 45, 0x00,
  /* RLE: 001 Pixels @ 116,296*/ 1, 0x0B,
  /* RLE: 008 Pixels @ 117,296*/ 8, 0x08,
  /* RLE: 001 Pixels @ 125,296*/ 1, 0x0B,
  /* RLE: 078 Pixels @ 126,296*/ 78, 0x00,
  /* RLE: 001 Pixels @ 204,296*/ 1, 0x04,
  /* RLE: 006 Pixels @ 205,296*/ 6, 0x03,
  /* RLE: 001 Pixels @ 211,296*/ 1, 0x04,
  /* RLE: 056 Pixels @ 212,296*/ 56, 0x00,
  /* RLE: 001 Pixels @ 268,296*/ 1, 0x04,
  /* RLE: 004 Pixels @ 269,296*/ 4, 0x03,
  /* ABS: 004 Pixels @ 273,296*/ 0, 4, 0x04, 0x0D, 0x12, 0x03,
  /* RLE: 059 Pixels @ 277,296*/ 59, 0x00,
  /* RLE: 003 Pixels @ 336,296*/ 3, 0x03,
  /* ABS: 006 Pixels @ 339,296*/ 0, 6, 0x0E, 0x0E, 0x03, 0x03, 0x03, 0x04,
  /* RLE: 043 Pixels @ 345,296*/ 43, 0x00,
  /* RLE: 063 Pixels @ 000,297*/ 63, 0x02,
  /* RLE: 001 Pixels @ 063,297*/ 1, 0x04,
  /* RLE: 006 Pixels @ 064,297*/ 6, 0x03,
  /* RLE: 001 Pixels @ 070,297*/ 1, 0x04,
  /* RLE: 045 Pixels @ 071,297*/ 45, 0x00,
  /* RLE: 001 Pixels @ 116,297*/ 1, 0x0B,
  /* RLE: 008 Pixels @ 117,297*/ 8, 0x08,
  /* RLE: 001 Pixels @ 125,297*/ 1, 0x0B,
  /* RLE: 078 Pixels @ 126,297*/ 78, 0x00,
  /* RLE: 001 Pixels @ 204,297*/ 1, 0x04,
  /* RLE: 006 Pixels @ 205,297*/ 6, 0x03,
  /* RLE: 001 Pixels @ 211,297*/ 1, 0x04,
  /* RLE: 056 Pixels @ 212,297*/ 56, 0x00,
  /* ABS: 009 Pixels @ 268,297*/ 0, 9, 0x03, 0x03, 0x00, 0x0E, 0x0F, 0x07, 0x11, 0x0B, 0x03,
  /* RLE: 060 Pixels @ 277,297*/ 60, 0x00,
  /* ABS: 004 Pixels @ 337,297*/ 0, 4, 0x04, 0x03, 0x0E, 0x0F,
  /* RLE: 005 Pixels @ 341,297*/ 5, 0x03,
  /* RLE: 042 Pixels @ 346,297*/ 42, 0x00,
  /* RLE: 063 Pixels @ 000,298*/ 63, 0x02,
  /* RLE: 001 Pixels @ 063,298*/ 1, 0x04,
  /* RLE: 006 Pixels @ 064,298*/ 6, 0x03,
  /* RLE: 001 Pixels @ 070,298*/ 1, 0x04,
  /* RLE: 045 Pixels @ 071,298*/ 45, 0x00,
  /* RLE: 001 Pixels @ 116,298*/ 1, 0x0B,
  /* RLE: 008 Pixels @ 117,298*/ 8, 0x08,
  /* RLE: 001 Pixels @ 125,298*/ 1, 0x0B,
  /* RLE: 078 Pixels @ 126,298*/ 78, 0x00,
  /* RLE: 001 Pixels @ 204,298*/ 1, 0x04,
  /* RLE: 007 Pixels @ 205,298*/ 7, 0x03,
  /* RLE: 054 Pixels @ 212,298*/ 54, 0x00,
  /* ABS: 010 Pixels @ 266,298*/ 0, 10, 0x04, 0x00, 0x0A, 0x0D, 0x07, 0x07, 0x0E, 0x00, 0x03, 0x03,
  /* RLE: 062 Pixels @ 276,298*/ 62, 0x00,
  /* ABS: 009 Pixels @ 338,298*/ 0, 9, 0x03, 0x00, 0x07, 0x00, 0x03, 0x04, 0x12, 0x0E, 0x03,
  /* RLE: 041 Pixels @ 347,298*/ 41, 0x00,
  /* RLE: 063 Pixels @ 000,299*/ 63, 0x02,
  /* RLE: 001 Pixels @ 063,299*/ 1, 0x04,
  /* RLE: 006 Pixels @ 064,299*/ 6, 0x03,
  /* RLE: 001 Pixels @ 070,299*/ 1, 0x04,
  /* RLE: 045 Pixels @ 071,299*/ 45, 0x00,
  /* RLE: 001 Pixels @ 116,299*/ 1, 0x0B,
  /* RLE: 008 Pixels @ 117,299*/ 8, 0x08,
  /* RLE: 001 Pixels @ 125,299*/ 1, 0x0B,
  /* RLE: 079 Pixels @ 126,299*/ 79, 0x00,
  /* RLE: 001 Pixels @ 205,299*/ 1, 0x04,
  /* RLE: 006 Pixels @ 206,299*/ 6, 0x03,
  /* RLE: 001 Pixels @ 212,299*/ 1, 0x04,
  /* RLE: 050 Pixels @ 213,299*/ 50, 0x00,
  /* ABS: 015 Pixels @ 263,299*/ 0, 15, 0x04, 0x04, 0x03, 0x03, 0x03, 0x04, 0x11, 0x0B, 0x03, 0x03, 0x00, 0x00, 0x03, 0x03, 0x04,
  /* RLE: 060 Pixels @ 278,299*/ 60, 0x00,
  /* ABS: 009 Pixels @ 338,299*/ 0, 9, 0x03, 0x03, 0x00, 0x0E, 0x11, 0x07, 0x0F, 0x0E, 0x03,
  /* RLE: 041 Pixels @ 347,299*/ 41, 0x00,
  /* RLE: 063 Pixels @ 000,300*/ 63, 0x02,
  /* RLE: 001 Pixels @ 063,300*/ 1, 0x04,
  /* RLE: 006 Pixels @ 064,300*/ 6, 0x03,
  /* RLE: 001 Pixels @ 070,300*/ 1, 0x04,
  /* RLE: 045 Pixels @ 071,300*/ 45, 0x00,
  /* RLE: 001 Pixels @ 116,300*/ 1, 0x0B,
  /* RLE: 008 Pixels @ 117,300*/ 8, 0x08,
  /* RLE: 001 Pixels @ 125,300*/ 1, 0x0B,
  /* RLE: 027 Pixels @ 126,300*/ 27, 0x00,
  /* RLE: 005 Pixels @ 153,300*/ 5, 0x04,
  /* RLE: 047 Pixels @ 158,300*/ 47, 0x00,
  /* RLE: 001 Pixels @ 205,300*/ 1, 0x04,
  /* RLE: 006 Pixels @ 206,300*/ 6, 0x03,
  /* RLE: 001 Pixels @ 212,300*/ 1, 0x04,
  /* RLE: 048 Pixels @ 213,300*/ 48, 0x00,
  /* ABS: 002 Pixels @ 261,300*/ 0, 2, 0x04, 0x04,
  /* RLE: 008 Pixels @ 263,300*/ 8, 0x03,
  /* ABS: 007 Pixels @ 271,300*/ 0, 7, 0x0A, 0x12, 0x07, 0x0D, 0x03, 0x03, 0x04,
  /* RLE: 059 Pixels @ 278,300*/ 59, 0x00,
  /* ABS: 009 Pixels @ 337,300*/ 0, 9, 0x03, 0x0A, 0x06, 0x07, 0x07, 0x07, 0x0B, 0x03, 0x03,
  /* RLE: 042 Pixels @ 346,300*/ 42, 0x00,
  /* RLE: 063 Pixels @ 000,301*/ 63, 0x02,
  /* RLE: 001 Pixels @ 063,301*/ 1, 0x04,
  /* RLE: 006 Pixels @ 064,301*/ 6, 0x03,
  /* RLE: 001 Pixels @ 070,301*/ 1, 0x04,
  /* RLE: 046 Pixels @ 071,301*/ 46, 0x00,
  /* RLE: 001 Pixels @ 117,301*/ 1, 0x0B,
  /* RLE: 008 Pixels @ 118,301*/ 8, 0x08,
  /* RLE: 001 Pixels @ 126,301*/ 1, 0x0B,
  /* RLE: 023 Pixels @ 127,301*/ 23, 0x00,
  /* ABS: 002 Pixels @ 150,301*/ 0, 2, 0x04, 0x04,
  /* RLE: 005 Pixels @ 152,301*/ 5, 0x03,
  /* ABS: 002 Pixels @ 157,301*/ 0, 2, 0x04, 0x04,
  /* RLE: 046 Pixels @ 159,301*/ 46, 0x00,
  /* RLE: 001 Pixels @ 205,301*/ 1, 0x04,
  /* RLE: 006 Pixels @ 206,301*/ 6, 0x03,
  /* RLE: 001 Pixels @ 212,301*/ 1, 0x04,
  /* RLE: 045 Pixels @ 213,301*/ 45, 0x00,
  /* ABS: 002 Pixels @ 258,301*/ 0, 2, 0x04, 0x04,
  /* RLE: 009 Pixels @ 260,301*/ 9, 0x03,
  /* ABS: 010 Pixels @ 269,301*/ 0, 10, 0x0B, 0x11, 0x07, 0x11, 0x06, 0x12, 0x00, 0x03, 0x03, 0x04,
  /* RLE: 058 Pixels @ 279,301*/ 58, 0x00,
  /* ABS: 007 Pixels @ 337,301*/ 0, 7, 0x03, 0x04, 0x11, 0x0E, 0x00, 0x06, 0x0E,
  /* RLE: 004 Pixels @ 344,301*/ 4, 0x03,
  /* RLE: 040 Pixels @ 348,301*/ 40, 0x00,
  /* RLE: 064 Pixels @ 000,302*/ 64, 0x02,
  /* RLE: 007 Pixels @ 064,302*/ 7, 0x03,
  /* RLE: 001 Pixels @ 071,302*/ 1, 0x04,
  /* RLE: 045 Pixels @ 072,302*/ 45, 0x00,
  /* RLE: 001 Pixels @ 117,302*/ 1, 0x0B,
  /* RLE: 008 Pixels @ 118,302*/ 8, 0x08,
  /* RLE: 001 Pixels @ 126,302*/ 1, 0x0B,
  /* RLE: 021 Pixels @ 127,302*/ 21, 0x00,
  /* ABS: 002 Pixels @ 148,302*/ 0, 2, 0x04, 0x04,
  /* RLE: 008 Pixels @ 150,302*/ 8, 0x03,
  /* RLE: 001 Pixels @ 158,302*/ 1, 0x04,
  /* RLE: 046 Pixels @ 159,302*/ 46, 0x00,
  /* RLE: 001 Pixels @ 205,302*/ 1, 0x04,
  /* RLE: 007 Pixels @ 206,302*/ 7, 0x03,
  /* RLE: 043 Pixels @ 213,302*/ 43, 0x00,
  /* ABS: 002 Pixels @ 256,302*/ 0, 2, 0x04, 0x04,
  /* RLE: 011 Pixels @ 258,302*/ 11, 0x03,
  /* ABS: 010 Pixels @ 269,302*/ 0, 10, 0x04, 0x12, 0x0A, 0x03, 0x03, 0x0E, 0x12, 0x03, 0x03, 0x04,
  /* RLE: 059 Pixels @ 279,302*/ 59, 0x00,
  /* RLE: 004 Pixels @ 338,302*/ 4, 0x03,
  /* ABS: 007 Pixels @ 342,302*/ 0, 7, 0x0B, 0x0F, 0x03, 0x00, 0x0E, 0x00, 0x03,
  /* RLE: 039 Pixels @ 349,302*/ 39, 0x00,
  /* RLE: 064 Pixels @ 000,303*/ 64, 0x02,
  /* RLE: 001 Pixels @ 064,303*/ 1, 0x04,
  /* RLE: 006 Pixels @ 065,303*/ 6, 0x03,
  /* RLE: 001 Pixels @ 071,303*/ 1, 0x04,
  /* RLE: 045 Pixels @ 072,303*/ 45, 0x00,
  /* RLE: 001 Pixels @ 117,303*/ 1, 0x0B,
  /* RLE: 008 Pixels @ 118,303*/ 8, 0x08,
  /* RLE: 001 Pixels @ 126,303*/ 1, 0x0B,
  /* RLE: 018 Pixels @ 127,303*/ 18, 0x00,
  /* ABS: 002 Pixels @ 145,303*/ 0, 2, 0x04, 0x04,
  /* RLE: 011 Pixels @ 147,303*/ 11, 0x03,
  /* RLE: 001 Pixels @ 158,303*/ 1, 0x04,
  /* RLE: 047 Pixels @ 159,303*/ 47, 0x00,
  /* RLE: 001 Pixels @ 206,303*/ 1, 0x04,
  /* RLE: 006 Pixels @ 207,303*/ 6, 0x03,
  /* RLE: 001 Pixels @ 213,303*/ 1, 0x04,
  /* RLE: 039 Pixels @ 214,303*/ 39, 0x00,
  /* ABS: 002 Pixels @ 253,303*/ 0, 2, 0x04, 0x04,
  /* RLE: 018 Pixels @ 255,303*/ 18, 0x03,
  /* ABS: 006 Pixels @ 273,303*/ 0, 6, 0x0B, 0x0F, 0x06, 0x03, 0x03, 0x04,
  /* RLE: 061 Pixels @ 279,303*/ 61, 0x00,
  /* RLE: 003 Pixels @ 340,303*/ 3, 0x03,
  /* ABS: 005 Pixels @ 343,303*/ 0, 5, 0x07, 0x11, 0x07, 0x07, 0x04,
  /* RLE: 040 Pixels @ 348,303*/ 40, 0x00,
  /* RLE: 064 Pixels @ 000,304*/ 64, 0x02,
  /* RLE: 001 Pixels @ 064,304*/ 1, 0x04,
  /* RLE: 006 Pixels @ 065,304*/ 6, 0x03,
  /* RLE: 001 Pixels @ 071,304*/ 1, 0x04,
  /* RLE: 045 Pixels @ 072,304*/ 45, 0x00,
  /* RLE: 001 Pixels @ 117,304*/ 1, 0x0B,
  /* RLE: 008 Pixels @ 118,304*/ 8, 0x08,
  /* RLE: 001 Pixels @ 126,304*/ 1, 0x0B,
  /* RLE: 016 Pixels @ 127,304*/ 16, 0x00,
  /* ABS: 002 Pixels @ 143,304*/ 0, 2, 0x04, 0x04,
  /* RLE: 013 Pixels @ 145,304*/ 13, 0x03,
  /* RLE: 001 Pixels @ 158,304*/ 1, 0x04,
  /* RLE: 047 Pixels @ 159,304*/ 47, 0x00,
  /* RLE: 001 Pixels @ 206,304*/ 1, 0x04,
  /* RLE: 006 Pixels @ 207,304*/ 6, 0x03,
  /* RLE: 001 Pixels @ 213,304*/ 1, 0x04,
  /* RLE: 037 Pixels @ 214,304*/ 37, 0x00,
  /* ABS: 002 Pixels @ 251,304*/ 0, 2, 0x04, 0x04,
  /* RLE: 016 Pixels @ 253,304*/ 16, 0x03,
  /* ABS: 011 Pixels @ 269,304*/ 0, 11, 0x00, 0x00, 0x0E, 0x07, 0x07, 0x0D, 0x0D, 0x00, 0x03, 0x03, 0x04,
  /* RLE: 059 Pixels @ 280,304*/ 59, 0x00,
  /* ABS: 009 Pixels @ 339,304*/ 0, 9, 0x03, 0x00, 0x0E, 0x0F, 0x07, 0x11, 0x0B, 0x00, 0x03,
  /* RLE: 040 Pixels @ 348,304*/ 40, 0x00,
  /* RLE: 064 Pixels @ 000,305*/ 64, 0x02,
  /* RLE: 001 Pixels @ 064,305*/ 1, 0x04,
  /* RLE: 006 Pixels @ 065,305*/ 6, 0x03,
  /* RLE: 001 Pixels @ 071,305*/ 1, 0x04,
  /* RLE: 045 Pixels @ 072,305*/ 45, 0x00,
  /* RLE: 001 Pixels @ 117,305*/ 1, 0x0B,
  /* RLE: 008 Pixels @ 118,305*/ 8, 0x08,
  /* RLE: 001 Pixels @ 126,305*/ 1, 0x0B,
  /* RLE: 013 Pixels @ 127,305*/ 13, 0x00,
  /* ABS: 002 Pixels @ 140,305*/ 0, 2, 0x04, 0x04,
  /* RLE: 015 Pixels @ 142,305*/ 15, 0x03,
  /* ABS: 002 Pixels @ 157,305*/ 0, 2, 0x04, 0x04,
  /* RLE: 047 Pixels @ 159,305*/ 47, 0x00,
  /* RLE: 001 Pixels @ 206,305*/ 1, 0x04,
  /* RLE: 006 Pixels @ 207,305*/ 6, 0x03,
  /* ABS: 002 Pixels @ 213,305*/ 0, 2, 0x04, 0x04,
  /* RLE: 033 Pixels @ 215,305*/ 33, 0x00,
  /* ABS: 002 Pixels @ 248,305*/ 0, 2, 0x04, 0x04,
  /* RLE: 016 Pixels @ 250,305*/ 16, 0x03,
  /* ABS: 014 Pixels @ 266,305*/ 0, 14, 0x04, 0x04, 0x00, 0x03, 0x00, 0x0F, 0x0E, 0x00, 0x03, 0x0A, 0x0F, 0x03, 0x03, 0x04,
  /* RLE: 059 Pixels @ 280,305*/ 59, 0x00,
  /* ABS: 005 Pixels @ 339,305*/ 0, 5, 0x03, 0x11, 0x07, 0x12, 0x0A,
  /* RLE: 004 Pixels @ 344,305*/ 4, 0x03,
  /* RLE: 001 Pixels @ 348,305*/ 1, 0x04,
  /* RLE: 039 Pixels @ 349,305*/ 39, 0x00,
  /* RLE: 064 Pixels @ 000,306*/ 64, 0x02,
  /* RLE: 001 Pixels @ 064,306*/ 1, 0x04,
  /* RLE: 006 Pixels @ 065,306*/ 6, 0x03,
  /* RLE: 001 Pixels @ 071,306*/ 1, 0x04,
  /* RLE: 046 Pixels @ 072,306*/ 46, 0x00,
  /* RLE: 001 Pixels @ 118,306*/ 1, 0x0B,
  /* RLE: 008 Pixels @ 119,306*/ 8, 0x08,
  /* RLE: 001 Pixels @ 127,306*/ 1, 0x0B,
  /* RLE: 009 Pixels @ 128,306*/ 9, 0x00,
  /* ABS: 002 Pixels @ 137,306*/ 0, 2, 0x04, 0x04,
  /* RLE: 017 Pixels @ 139,306*/ 17, 0x03,
  /* ABS: 002 Pixels @ 156,306*/ 0, 2, 0x04, 0x04,
  /* RLE: 048 Pixels @ 158,306*/ 48, 0x00,
  /* RLE: 001 Pixels @ 206,306*/ 1, 0x04,
  /* RLE: 007 Pixels @ 207,306*/ 7, 0x03,
  /* ABS: 002 Pixels @ 214,306*/ 0, 2, 0x04, 0x04,
  /* RLE: 030 Pixels @ 216,306*/ 30, 0x00,
  /* ABS: 002 Pixels @ 246,306*/ 0, 2, 0x04, 0x04,
  /* RLE: 016 Pixels @ 248,306*/ 16, 0x03,
  /* ABS: 002 Pixels @ 264,306*/ 0, 2, 0x04, 0x04,
  /* RLE: 004 Pixels @ 266,306*/ 4, 0x00,
  /* RLE: 004 Pixels @ 270,306*/ 4, 0x03,
  /* ABS: 007 Pixels @ 274,306*/ 0, 7, 0x00, 0x12, 0x07, 0x03, 0x03, 0x03, 0x04,
  /* RLE: 058 Pixels @ 281,306*/ 58, 0x00,
  /* ABS: 002 Pixels @ 339,306*/ 0, 2, 0x03, 0x00,
  /* RLE: 007 Pixels @ 341,306*/ 7, 0x03,
  /* RLE: 001 Pixels @ 348,306*/ 1, 0x04,
  /* RLE: 039 Pixels @ 349,306*/ 39, 0x00,
  /* RLE: 064 Pixels @ 000,307*/ 64, 0x02,
  /* RLE: 001 Pixels @ 064,307*/ 1, 0x04,
  /* RLE: 006 Pixels @ 065,307*/ 6, 0x03,
  /* RLE: 001 Pixels @ 071,307*/ 1, 0x04,
  /* RLE: 046 Pixels @ 072,307*/ 46, 0x00,
  /* RLE: 001 Pixels @ 118,307*/ 1, 0x0B,
  /* RLE: 008 Pixels @ 119,307*/ 8, 0x08,
  /* RLE: 001 Pixels @ 127,307*/ 1, 0x0B,
  /* RLE: 007 Pixels @ 128,307*/ 7, 0x00,
  /* ABS: 002 Pixels @ 135,307*/ 0, 2, 0x04, 0x04,
  /* RLE: 016 Pixels @ 137,307*/ 16, 0x03,
  /* ABS: 002 Pixels @ 153,307*/ 0, 2, 0x04, 0x04,
  /* RLE: 052 Pixels @ 155,307*/ 52, 0x00,
  /* RLE: 001 Pixels @ 207,307*/ 1, 0x04,
  /* RLE: 007 Pixels @ 208,307*/ 7, 0x03,
  /* ABS: 002 Pixels @ 215,307*/ 0, 2, 0x04, 0x04,
  /* RLE: 026 Pixels @ 217,307*/ 26, 0x00,
  /* ABS: 002 Pixels @ 243,307*/ 0, 2, 0x04, 0x04,
  /* RLE: 016 Pixels @ 245,307*/ 16, 0x03,
  /* ABS: 002 Pixels @ 261,307*/ 0, 2, 0x04, 0x04,
  /* RLE: 008 Pixels @ 263,307*/ 8, 0x00,
  /* ABS: 010 Pixels @ 271,307*/ 0, 10, 0x03, 0x0B, 0x11, 0x07, 0x0F, 0x04, 0x03, 0x03, 0x03, 0x04,
  /* RLE: 059 Pixels @ 281,307*/ 59, 0x00,
  /* ABS: 009 Pixels @ 340,307*/ 0, 9, 0x03, 0x03, 0x00, 0x00, 0x03, 0x11, 0x0E, 0x03, 0x03,
  /* RLE: 039 Pixels @ 349,307*/ 39, 0x00,

  0};  /* 20430 for 119504 pixels */

static const GUI_BITMAP bmMap = {
 388, /* XSize */
 308, /* YSize */
 388, /* BytesPerLine */
 GUI_COMPRESS_RLE8, /* BitsPerPixel */
 acMap,  /* Pointer to picture data (indices) */
 &PalMap  /* Pointer to palette */
 ,GUI_DRAW_RLE8
};

static NAVIMAP _NaviMap = {
  0, 0, 100, 70, 1, 0, 1200, 0, 0, &bmMap
};

/*********************************************************************
*
*       static data, needle polygons
*
**********************************************************************
*/

static const GUI_POINT _aNeedleSrc[5][5] = {
  {
    { FACTOR *  0, FACTOR *  -5},
    { FACTOR * -5, FACTOR *  25},
    { FACTOR *  0, FACTOR * 100},
    { FACTOR *  5, FACTOR *  25}
  },{
    { FACTOR * -4, FACTOR *   0},
    { FACTOR * -3, FACTOR *  60},
    { FACTOR *  0, FACTOR * 100},
    { FACTOR *  3, FACTOR *  60},
    { FACTOR *  4, FACTOR *   0}
  },{
    { FACTOR * -3, FACTOR * -13},
    { FACTOR * -3, FACTOR *  60},
    { FACTOR *  0, FACTOR * 100},
    { FACTOR *  3, FACTOR *  60},
    { FACTOR *  3, FACTOR * -13}
  },{
    { FACTOR * -5, FACTOR * -13},
    { FACTOR * -4, FACTOR *  20},
    { FACTOR *  0, FACTOR * 100},
    { FACTOR *  4, FACTOR *  20},
    { FACTOR *  5, FACTOR * -13}
  },{
    { FACTOR * -5, FACTOR * -13},
    { FACTOR * -4, FACTOR *  65},
    { FACTOR *  0, FACTOR * 100},
    { FACTOR *  4, FACTOR *  65},
    { FACTOR *  5, FACTOR * -13}
  }
};

static GUI_POINT _aNeedle[5];

/*********************************************************************
*
*       static code, helper functions
*
**********************************************************************
*/

/*********************************************************************
*
*       _CreateButton
*/
static void _CreateButton(const char* pText, int x, int y, int w, int h, WM_HWIN hParent, int Id) {
  WM_HWIN hBut;
  hBut = BUTTON_CreateAsChild(x, y, w, h, hParent, Id, WM_CF_SHOW);
  BUTTON_SetText(hBut, pText);
}

/*********************************************************************
*
*       _SetCheckbox
*/
static void _SetCheckbox(WM_HWIN hWin, int Id, int State) {
  WM_HWIN hItem;
  hItem = WM_GetDialogItem(hWin, Id);
  if (State) {
    CHECKBOX_Check(hItem);
  } else {
    CHECKBOX_Uncheck(hItem);
  }
}

/*********************************************************************
*
*       _SetSlider
*/
static void _SetSlider(WM_HWIN hWin, int Id, int Min, int Max, int Value) {
  WM_HWIN hItem;
  hItem = WM_GetDialogItem(hWin, Id);
  SLIDER_SetRange(hItem, Min, Max);
  SLIDER_SetValue(hItem, Value);
}

/*********************************************************************
*
*       _SetDialogColor
*/
static void _SetDialogColor(int Scale) {
  int ColorIndex, i;
  ColorIndex = DROPDOWN_GetSel(_hDropDownColor);
  _InitDialog = 1;
  for (i = 0; i < 4; i++) {
    _SetSlider(_hDialogColor, GUI_ID_SLIDER0 + i, 0, 255, _Scale[Scale].Color[ColorIndex].Sep[i]);
  }
  _InitDialog = 0;
}

/*********************************************************************
*
*       _SetDialogMark
*/
static void _SetDialogMark(int Scale) {
  _InitDialog = 1;
  _SetSlider(_hDialogMark, GUI_ID_SLIDER0, 0, 25, _Scale[Scale].NumMarkLines);
  _SetSlider(_hDialogMark, GUI_ID_SLIDER1, 0, 40, _Scale[Scale].LineLen1);
  _SetSlider(_hDialogMark, GUI_ID_SLIDER2, 0, 50, _Scale[Scale].LinePos1);
  _SetSlider(_hDialogMark, GUI_ID_SLIDER3, 1,  5, _Scale[Scale].PenSize1);
  _SetCheckbox(_hDialogMark, GUI_ID_USER, _Scale[Scale].Flags & (1 << FLAG_SHOW_MARK));
  _InitDialog = 0;
}

/*********************************************************************
*
*       _SetDialogPitch
*/
static void _SetDialogPitch(int Scale) {
  _InitDialog = 1;
  _SetSlider(_hDialogPitch, GUI_ID_SLIDER0, 0, 25, _Scale[Scale].NumPitchLines);
  _SetSlider(_hDialogPitch, GUI_ID_SLIDER1, 0, 40, _Scale[Scale].LineLen2);
  _SetSlider(_hDialogPitch, GUI_ID_SLIDER2, 0, 50, _Scale[Scale].LinePos2);
  _SetSlider(_hDialogPitch, GUI_ID_SLIDER3, 1,  5, _Scale[Scale].PenSize2);
  _SetCheckbox(_hDialogPitch, GUI_ID_USER, _Scale[Scale].Flags & (1 << FLAG_SHOW_PITCH));
  _InitDialog = 0;
}

/*********************************************************************
*
*       _SetDialogArc
*/
static void _SetDialogArc(int Scale) {
  int i;
  _InitDialog = 1;
  _SetSlider(_hDialogArc, GUI_ID_SLIDER0, 0, 359, _Scale[Scale].ArcArea1);
  _SetSlider(_hDialogArc, GUI_ID_SLIDER1, 0, 359, _Scale[Scale].ArcArea2);
  _SetSlider(_hDialogArc, GUI_ID_SLIDER2, 0,  40, _Scale[Scale].ArcWidth);
  _SetSlider(_hDialogArc, GUI_ID_SLIDER3, 0,  50, _Scale[Scale].ArcPos);
  _SetSlider(_hDialogArc, GUI_ID_SLIDER4, 1,   5, _Scale[Scale].PenSize3);
  for (i = 0; i < 5; i++) {
    _SetCheckbox(_hDialogArc, GUI_ID_USER + i, _Scale[Scale].Flags & (1 << (FLAG_SHOW_ARC+i)));
  }
  _InitDialog = 0;
}

/*********************************************************************
*
*       _SetDialogGrad
*/
static void _SetDialogGrad(int Scale) {
  _InitDialog = 1;
  _SetSlider(_hDialogGrad, GUI_ID_SLIDER0,   0, 80, _Scale[Scale].GradDist);
  _SetSlider(_hDialogGrad, GUI_ID_SLIDER1,   1, 20, _Scale[Scale].NumStep);
  _SetSlider(_hDialogGrad, GUI_ID_SLIDER2,   0,  9, _Scale[Scale].NumStart);
  _SetSlider(_hDialogGrad, GUI_ID_SLIDER3,   0,  3, _Scale[Scale].NumExp);
  _SetSlider(_hDialogGrad, GUI_ID_SLIDER4, -90, 90, _Scale[Scale].TextDist);
  _SetCheckbox(_hDialogGrad, GUI_ID_USER + 0, _Scale[Scale].Flags & (1 << FLAG_SHOW_GRAD));
  _SetCheckbox(_hDialogGrad, GUI_ID_USER + 1, _Scale[Scale].Flags & (1 << FLAG_SHOW_TEXT));
  _InitDialog = 0;
}

/*********************************************************************
*
*       _SetDialogScale
*/
static void _SetDialogScale(int Scale) {
  _InitDialog = 1;
  _SetSlider(_hDialogScale, GUI_ID_SLIDER0,  0, 360, _Scale[Scale].ArcStart);
  _SetSlider(_hDialogScale, GUI_ID_SLIDER1,  0, 360, _Scale[Scale].ArcEnd);
  _SetSlider(_hDialogScale, GUI_ID_SLIDER2, 35, 160, _Scale[Scale].ArcRadius);
  _SetSlider(_hDialogScale, GUI_ID_SLIDER3,  0, 639, _Scale[Scale].x);
  _SetSlider(_hDialogScale, GUI_ID_SLIDER4,  0, 479, _Scale[Scale].y);
  _SetCheckbox(_hDialogScale, GUI_ID_USER, _Scale[Scale].Flags & (1 << FLAG_SHOW_SCALE));
  _InitDialog = 0;
}

/*********************************************************************
*
*       _SetDialogMisc
*/
static void _SetDialogMisc(int Scale) {
  _InitDialog = 1;
  _SetSlider(_hDialogMisc, GUI_ID_SLIDER0,  0,   4, _Scale[Scale].NeedleType);
  _SetSlider(_hDialogMisc, GUI_ID_SLIDER1, 50, 100, _Scale[Scale].NeedleRadius);
  _SetSlider(_hDialogMisc, GUI_ID_SLIDER2, 10,  60, _Scale[Scale].AxisRadius);
  _SetCheckbox(_hDialogMisc, GUI_ID_USER + 0, _Scale[Scale].Flags & (1 << FLAG_NEEDLE_FRAME));
  _SetCheckbox(_hDialogMisc, GUI_ID_USER + 1, _Scale[Scale].Flags & (1 << FLAG_NEEDLE_LINE));
  _InitDialog = 0;
}

/*********************************************************************
*
*       _SetDialogs
*/
static void _SetDialogs(int Scale) {
  _SetDialogColor(Scale);
  _SetDialogMark(Scale);
  _SetDialogPitch(Scale);
  _SetDialogArc(Scale);
  _SetDialogGrad(Scale);
  _SetDialogScale(Scale);
  _SetDialogMisc(Scale);
}

/*********************************************************************
*
*       _GetArcLen
*/
static int _GetArcLen(const SCALE* pObj) {
  if (pObj->ArcStart > pObj->ArcEnd) {
    return 360 - (pObj->ArcStart - pObj->ArcEnd);
  } else {
    return pObj->ArcEnd - pObj->ArcStart;
  }
}

/*********************************************************************
*
*       _MagnifyPolygon
*/
static void _MagnifyPolygon(GUI_POINT* pDest, const GUI_POINT* pSrc, int NumPoints, float Mag) {
  int i;

  for (i=0; i<NumPoints; i++) {
    (pDest+i)->x = (int)((pSrc+i)->x * Mag);
    (pDest+i)->y = (int)((pSrc+i)->y * Mag);
  }
}

/*********************************************************************
*
*       _CalcNeedle
*/
static int _CalcNeedle(const SCALE* pObj, int Index, int Radius) {
  int NumPoints, Shape;
  float Angel;
  Shape = pObj->NeedleType;
  NumPoints = GUI_COUNTOF(_aNeedleSrc[Shape]);
  Angel = -((pObj->ArcStart * PI) + (_GetArcLen(pObj) * _Needle[Index].NeedlePos * PI) / NEEDLE_GRAD) / 180;
  _MagnifyPolygon(_aNeedle, _aNeedleSrc[Shape], NumPoints, Radius / 100.);
  GUI_RotatePolygon(_aNeedle, _aNeedle, NumPoints, Angel);
  return NumPoints;
}

/*********************************************************************
*
*       _CalcPointX
*/
static int _CalcPointX(int r, int Angel) {
  return (int)(r * cos((Angel - 90) * PI / 180.));
}

/*********************************************************************
*
*       _CalcPointY
*/
static int _CalcPointY(int r, int Angel) {
  return (int)(r * sin((Angel - 90) * PI / 180.));
}

/*********************************************************************
*
*       _Max
*/
static int _Max(int a, int b) {
  return((a > b) ? a : b);
}

/*********************************************************************
*
*       _Max3
*/
static int _Max3(int a, int b, int c) {
  int r;
  r = (a > b) ? a : b;
  r = (r > c) ? r : c;
  return r;
}

/*********************************************************************
*
*       _Min
*/
static int _Min(int a, int b) {
  return((a < b) ? a : b);
}

/*********************************************************************
*
*       _GetNeedleRect
*/
static void _GetNeedleRect(const SCALE* pObj, int Index, GUI_RECT* pRect) {
  int NumPoints;
  int i;
  int x;
  int y;
  int r;
  int x0;
  int y0;
  int x1;
  int y1;

  x0 =  4096;
  y0 =  4096;
  x1 = -4096;
  y1 = -4096;
  r = (pObj->ArcRadius + pObj->PenSize3) * pObj->NeedleRadius / 100;
  NumPoints = _CalcNeedle(pObj, Index, r);
  for (i = 0; i < NumPoints; i++) {
    x = _aNeedle[i].x / FACTOR;
    y = _aNeedle[i].y / FACTOR;
    x0 = _Min(x0, x);
    y0 = _Min(y0, y);
    x1 = _Max(x1, x);
    y1 = _Max(y1, y);
  }
  pRect->x0 = pObj->x0 + x0 - 1;
  pRect->y0 = pObj->y0 + y0 - 1;
  pRect->x1 = pObj->x0 + x1 + 1;
  pRect->y1 = pObj->y0 + y1 + 1;
}

/*********************************************************************
*
*       _MergeRects
*/
static void _MergeRects(GUI_RECT* pR1, const GUI_RECT* pR2) {
  pR1->x0 = _Min(pR1->x0, pR2->x0);
  pR1->y0 = _Min(pR1->y0, pR2->y0);
  pR1->x1 = _Max(pR1->x1, pR2->x1);
  pR1->y1 = _Max(pR1->y1, pR2->y1);
}

/*********************************************************************
*
*       _MoveNeedle
*/
static void _MoveNeedle(NEEDLE* pObj, int Index) {
  GUI_RECT rOld;
  GUI_RECT rNew;
  int      Dif;
  int      Time;

  if (pObj->NeedleUPM) {
    _GetNeedleRect(&_Scale[Index], Index, &rOld);
    Time = GUI_GetTime();
    Dif = (Time - pObj->NeedlePrevTime) / (60000 / pObj->NeedleUPM / NEEDLE_GRAD);
    if (Dif != 0) {
      pObj->NeedlePos += (Dif * pObj->NeedleDir);
      if (pObj->NeedlePos > NEEDLE_GRAD) {
        pObj->NeedlePos = NEEDLE_GRAD;
        pObj->NeedleDir = -pObj->NeedleDir;
      } else {
        if (pObj->NeedlePos < 0) {
          pObj->NeedlePos = 0;
          pObj->NeedleDir = -pObj->NeedleDir;
        }
      }
      _GetNeedleRect(&_Scale[Index], Index, &rNew);
      _MergeRects(&rNew, &rOld);
      WM_InvalidateRect(_Scale[Index].hWin, &rNew);
      pObj->NeedlePrevTime = Time;
    }
  }
}

/*********************************************************************
*
*       _UpdateScale
*/
static void _UpdateScale(int Index) {
  int Mod;

  Mod = 0;
  if (_Scale[Index].hMemDev == 0) {
    GUI_RECT r;
    WM_GetWindowRect(&r);
    _Scale[Index].hMemDev = GUI_MEMDEV_CreateEx(r.x0, r.y0, r.x1 - r.x0 + 1, r.y1 - r.y0 + 1, GUI_MEMDEV_HASTRANS);
    Mod = 1;
  }
  if (Mod | memcmp(&_ScalePrev[Index], &_Scale[Index], sizeof(SCALE))) {
    GUI_MEMDEV_Handle hPrev = GUI_MEMDEV_Select(_Scale[Index].hMemDev);
    GUI_SetBkColor(GUI_BLACK);
    GUI_MEMDEV_Write(_hBkMemDev);
    GUI_MEMDEV_Clear(_Scale[Index].hMemDev);
    _DrawScale(&_Scale[Index]);
    memcpy(&_ScalePrev[Index], &_Scale[Index], sizeof(SCALE));
    GUI_MEMDEV_Select(hPrev);
  }
}

/*********************************************************************
*
*       _InvalidateScale
*/
static void _InvalidateScale(SCALE* pObj) {
  if (pObj->hMemDev) {
    GUI_MEMDEV_Delete(pObj->hMemDev);
    pObj->hMemDev = 0;
  }
}

/*********************************************************************
*
*       _ChangeScaleSize

  This function optimize the window rect
*/
static void _ChangeScaleSize(SCALE* pObj) {
  int x0;
  int y0;
  int x;
  int y;
  int w;
  int h;
  int ArcStart;
  int ArcEnd;
  int rOff;
  int PenSize;
  int r;
  int rNeedle;
  int BitmapW;
  int BitmapY0;
  int BitmapY1;
  int TextW;
  int TextY0;
  int TextY1;
  //  
  // Calculate text position
  //
  GUI_SetFont(&GUI_Font8x8);
  if (pObj->Flags & (1 << FLAG_SHOW_TEXT)) {
    TextW  = GUI_GetStringDistX(pObj->acText) / 2;
    TextY0 = (pObj->ArcRadius * pObj->TextDist / 100);
    TextY1 = TextY0 + 8;
    if (pObj->TextDist < 0) {
      TextY0 = -TextY0;
      TextY1 = _Max(TextY1, 0);
    } else {
      TextY0 = _Max(-TextY0, 0);
    }
  } else {
    TextW  = 0;
    TextY0 = 0;
    TextY1 = 0;
  }
  // 
  // Calculate radius of scale
  //
  PenSize  = _Max3(pObj->PenSize1, pObj->PenSize2, pObj->PenSize3);
  r        = pObj->ArcRadius + PenSize;
  // 
  // Calculate radius of needle
  //
  rNeedle  = pObj->ArcRadius * pObj->AxisRadius / 200;
  if (pObj->NeedleType > 1) {
    rNeedle  = _Max(rNeedle, (pObj->NeedleRadius * 22) / 100 + 1);
  }
  //  
  // Get arcbow
  //
  ArcStart = pObj->ArcStart;
  ArcEnd   = pObj->ArcEnd;
  // 
  // Calculate bitmap position
  //
  if (pObj->pBitmap) {
    BitmapW  = (pObj->pBitmap->XSize / 2) + 1;
    BitmapY0 = -(pObj->pBitmap->YSize / 2) + (pObj->ArcRadius / pObj->BitmapY);
    BitmapY1 = BitmapY0 + pObj->pBitmap->YSize;
    if (pObj->BitmapY < 0) {
      BitmapY0 = -BitmapY0;
      BitmapY1 = _Max(BitmapY1, 0);
    } else {
      BitmapY0 = _Max(-BitmapY0, 0);
    }
  } else {
    BitmapW  = 0;
    BitmapY0 = 0;
    BitmapY1 = 0;
  }
  // 
  // Calculate window heigh
  //
  if (ArcStart >= ArcEnd) {
    h = r * 2 + 2;
  } else {
    rOff = _Max3(rNeedle, TextY1, BitmapY1);
    h = _Max(r + rOff, r - _CalcPointY(pObj->ArcRadius, ArcStart) + PenSize + 2);
    h = _Max(h,        r - _CalcPointY(pObj->ArcRadius, ArcEnd)   + PenSize + 2);
  }
  // 
  // Calculate window width
  //
  if (ArcStart < 270 && (ArcEnd > 270 || ArcEnd <= ArcStart)) {
    w = r * 2 + 2;
  } else {
    rOff = _Max3(rNeedle, TextW, BitmapW);
    w = _Max(r + rOff, r - _CalcPointX(pObj->ArcRadius, ArcStart) + PenSize + 2);
    w = _Max(w,        r - _CalcPointX(pObj->ArcRadius, ArcEnd)   + PenSize + 2);
  }
  // 
  // Calculate y-position of window
  //
  if (((ArcStart < ArcEnd) && ((ArcStart > 180 && ArcEnd > 180) || (ArcStart < 180 && ArcEnd < 180))) ||
       (ArcStart > 180 && ArcEnd < 180)) {
    rOff = _Max3(rNeedle, TextY0, BitmapY0);
    y = _Min(r - rOff, r - _CalcPointY(pObj->ArcRadius, ArcStart) - PenSize - 2);
    y = _Min(y,        r - _CalcPointY(pObj->ArcRadius, ArcEnd)   - PenSize - 2);
    y0        = pObj->y - r + y;
    pObj->y0  = r - y;
    h        -= y;
  } else {
    y0        = pObj->y - r - 2;
    pObj->y0  = r + 2;
    h        += 2;
  }
  // 
  // Calculate x-position of window
  //
  if (((ArcStart < ArcEnd) && ((ArcStart > 90 && ArcEnd > 90) || (ArcStart < 90 && ArcEnd < 90))) ||
       (ArcStart > 90 && ArcEnd < 90)) {
    rOff = _Max3(rNeedle, TextW, BitmapW);
    x = _Min(r - rOff, r - _CalcPointX(pObj->ArcRadius, ArcStart) - PenSize - 2);
    x = _Min(x,        r - _CalcPointX(pObj->ArcRadius, ArcEnd)   - PenSize - 2);
    x0        = pObj->x - r + x;
    pObj->x0  = r - x;
    w        -= x;
  } else {
    x0        = pObj->x - r - 2;
    pObj->x0  = r + 2;
    w        += 2;
  }
  // 
  // Set new window rect
  //
  _InvalidateScale(pObj);
  WM_MoveTo(pObj->hWin, x0, y0);
  WM_SetSize(pObj->hWin, w, h);
  WM_InvalidateWindow(pObj->hWin);
}

/*********************************************************************
*
*       _SetPreset
*/
static void _SetPreset(int Preset, int Scale) {
  int i;
  int iStart;
  int iEnd;

  if (Preset >= 0 && Preset <= 3) {
    WM_HWIN OldhWin;
    iStart = (Scale == -1) ? 0 : Scale;
    iEnd   = (Scale == -1) ? 3 : Scale;
    for (i = iStart; i <= iEnd; i++) {
      _InvalidateScale(&_Scale[i]);
      // Save old values which should not change
      OldhWin = _Scale[i].hWin;
      // Copy the preset
      _Scale[i] = _Presets[Preset][i];
      // Restore unchanged values
      _Scale[i].hWin = OldhWin;
      // Recalculate the scalewin
      _ChangeScaleSize(&_Scale[i]);
    }
    _SetDialogs((Scale != -1) ? Scale : 0);
  }
}

/*********************************************************************
*
*       _CalcColor
*/
static GUI_COLOR _CalcColor(const COLOR* pColor) {
  GUI_COLOR r;

  r  = ((U32) pColor->Sep[0]);
  r += ((U32) pColor->Sep[1]) << 8;
  r += ((U32) pColor->Sep[2]) << 16;
  return r;
}

/*********************************************************************
*
*       _GetSliderValue
*/
static int _GetSliderValue(WM_HWIN hDlg, int Id) {
  return SLIDER_GetValue(WM_GetDialogItem(hDlg, Id));
}

/*********************************************************************
*
*       _AddDialog
*/
static WM_HWIN _AddDialog(const char* pText, const GUI_WIDGET_CREATE_INFO* pDialog, int NumItems,
                          WM_CALLBACK* cb, WM_HWIN hMultiPage) {
  WM_HWIN hWin;

  hWin = GUI_CreateDialogBox(pDialog, NumItems, cb, WM_GetClientWindow(hMultiPage), 0, 0);
  MULTIPAGE_AddPage(hMultiPage, 0, pText);
  return hWin;
}

/*********************************************************************
*
*       _IntToString
*/
static void _IntToString(char* pStr, int Value) {
  char* Ptr;
  
  Ptr = pStr + 6;
  *(--Ptr) = 0;
  Value = _Min(Value, 32767);
  do {
    *(--Ptr) = (Value % 10) + '0';
    Value /= 10;
  } while (Value != 0);
  strcpy(pStr, Ptr);
}

/*********************************************************************
*
*       _SetClipRect
*/
static const GUI_RECT* _SetClipRect(GUI_RECT* pRect,int x0, int y0, int x1, int y1) {
  pRect->x0 = x0;
  pRect->y0 = y0;
  pRect->x1 = x1;
  pRect->y1 = y1;
  return WM_SetUserClipRect(pRect);
}

/*********************************************************************
*
*       _MoveMap
*/
static void _MoveMap(WM_HWIN hMap) {
  int Time;
  int Dif;

  Time = GUI_GetTime();
  Dif  = (Time - _NaviMap.PrevTime) / (60000 / _NaviMap.PPM);
  if ((_NaviMap.Dif + Dif) > 100) {
    Dif = Dif - _NaviMap.Dif - Dif + 100;
  }
  if (Dif > 0) {
    _NaviMap.Dif += Dif;
    _NaviMap.x += Dif * _NaviMap.DirX;
    _NaviMap.y += Dif * _NaviMap.DirY;
    WM_InvalidateWindow(hMap);
    _NaviMap.PrevTime = Time;
  }
  if (_NaviMap.Dif >= 100) {
    _NaviMap.Dif = 100 - _NaviMap.Dif;
    if (_NaviMap.DirX == -1 && _NaviMap.DirY == -1) {
       _NaviMap.DirX = 1;
       _NaviMap.DirY = 0;
    } else {
      if (_NaviMap.DirX == 0 && _NaviMap.DirY == 1) {
         _NaviMap.DirX = -1;
         _NaviMap.DirY = -1;
      }
      if (_NaviMap.DirX == 1 && _NaviMap.DirY == 0) {
         _NaviMap.DirX = 0;
         _NaviMap.DirY = 1;
      }
    }
  }
}

/*********************************************************************
*
*       static code, drawing functions
*
**********************************************************************
*/
/*********************************************************************
*
*       _AA_DrawPolygon
*/
static void _AA_DrawPolygon(const GUI_POINT* pSrc, int NumPoints, int x0, int y0) {
  int i;
  int x;
  int y;
  int xPrev;
  int yPrev;
  U8  OldPenShape;

  xPrev = x0 + (pSrc+NumPoints-1)->x;
  yPrev = y0 + (pSrc+NumPoints-1)->y;
  OldPenShape = GUI_SetPenShape(GUI_PS_FLAT);
  for (i = 0; i < NumPoints; i++) {
    x = x0 + (pSrc+i)->x;
    y = y0 + (pSrc+i)->y;
    GUI_AA_DrawLine(xPrev, yPrev, x, y);
    xPrev = x;
    yPrev = y;
  }
  GUI_SetPenShape(OldPenShape);
}

/*********************************************************************
*
*       _DrawLine
*/
static void _DrawLine(const SCALE* pObj, int r1, int r2, float Angel) {
  float co = cos(Angel / 180.) * FACTOR;
  float si = sin(Angel / 180.) * FACTOR;
  int x0 = (int)(pObj->x0 * FACTOR - r1 * co);
  int y0 = (int)(pObj->y0 * FACTOR - r1 * si);
  int x1 = (int)(pObj->x0 * FACTOR - r2 * co);
  int y1 = (int)(pObj->y0 * FACTOR - r2 * si);
  GUI_AA_DrawLine(x0, y0, x1, y1);
}

/*********************************************************************
*
*       _DrawLines
*/
static void _DrawLines(const SCALE* pObj, int iEnd, int rStart, int rEnd) {
  int i;
  int ArcLen;
  float Angel;

  ArcLen = _GetArcLen(pObj);
  GUI_SetColor(_CalcColor(&pObj->Color[0]));
  HIRES_ON();
  for (i = 0; i <= iEnd; i++) {
    Angel = (i * ArcLen * PI) / _Max(iEnd, 1) + (pObj->ArcStart - 90.) * PI;
    _DrawLine(pObj, rStart, rEnd, Angel);
  }
  HIRES_OFF();
}

/*********************************************************************
*
*       _DrawPitchLines
*/
static void _DrawPitchLines(const SCALE* pObj) {
  int iEnd;
  int rStart;
  int rEnd;

  if (pObj->NumPitchLines > 0 && (pObj->Flags & (1 << FLAG_SHOW_PITCH))) {
    iEnd = _Max(pObj->NumMarkLines - 1, 1) * (pObj->NumPitchLines + 1);
    rStart = pObj->ArcRadius - pObj->LinePos2;
    rEnd = rStart - pObj->LineLen2;
    GUI_SetPenSize(pObj->PenSize2);
    _DrawLines(pObj, iEnd, rStart, rEnd);
  }
}

/*********************************************************************
*
*       _DrawMarkLines
*/
static void _DrawMarkLines(const SCALE* pObj) {
  int iEnd;
  int rStart;
  int rEnd;

  if (pObj->NumMarkLines > 0 && (pObj->Flags & (1 << FLAG_SHOW_MARK))) {
    iEnd = pObj->NumMarkLines - 1;
    rStart = pObj->ArcRadius - pObj->LinePos1;
    rEnd = rStart - pObj->LineLen1;
    GUI_SetPenSize(pObj->PenSize1);
    _DrawLines(pObj, iEnd, rStart, rEnd);
  }
}

/*********************************************************************
*
*       _DrawGrad
*/
static void _DrawGrad(const SCALE* pObj) {
  int   xt;
  int   yt;
  int   w;
  int   h;
  int   i;
  int   ArcLen;
  int   rStart;
  int   Cnt;
  int   wMax = 0;
  float co;
  float si;
  float Angel;
  char  acText[6];

  if (pObj->NumMarkLines > 0 && (pObj->Flags & (1 << FLAG_SHOW_GRAD))) {
    GUI_SetColor(_CalcColor(&pObj->Color[1]));
    GUI_SetFont(&GUI_Font6x8);
    GUI_SetTextMode(GUI_TEXTMODE_TRANS);
    Cnt = pObj->NumStart * (pObj->NumStep * _Pow10[pObj->NumExp]);
    for (i = 0; i < pObj->NumMarkLines; i++) {
      _IntToString(acText, Cnt);
      wMax = _Max(GUI_GetStringDistX(acText), wMax);
      Cnt += pObj->NumStep * _Pow10[pObj->NumExp];
    }
    ArcLen = _GetArcLen(pObj);
    rStart = pObj->ArcRadius - pObj->GradDist - (wMax / 2);
    Cnt = pObj->NumStart * (pObj->NumStep * _Pow10[pObj->NumExp]);
    for (i = 0; i < pObj->NumMarkLines; i++) {
      Angel = (i * ArcLen * PI) / _Max(pObj->NumMarkLines - 1, 1) + (pObj->ArcStart - 90.) * PI;
      co = cos(Angel / 180.);
      si = sin(Angel / 180.);
      xt = (int)(pObj->x0 - rStart * co);
      yt = (int)(pObj->y0 - rStart * si);
      _IntToString(acText, Cnt);
      w = GUI_GetStringDistX(acText);
      h = (int)(si * _Max(wMax / 2 - 4, 0) + 4);
      GUI_DispStringAt(acText, xt - w / 2 + 1, yt - h + 1);
      Cnt += pObj->NumStep * _Pow10[pObj->NumExp];
    }
  }
}

/*********************************************************************
*
*       _DrawText
*/
static void _DrawText(const SCALE* pObj) {
  int x;
  int y;

  if (pObj->Flags & (1 << FLAG_SHOW_TEXT)) {
    GUI_SetColor(_CalcColor(&pObj->Color[1]));
    GUI_SetFont(&GUI_Font8x8);
    GUI_SetTextMode(GUI_TEXTMODE_TRANS);
    x = pObj->x0;
    y = pObj->y0 + (pObj->ArcRadius * pObj->TextDist / 100);
    GUI_DispStringHCenterAt(pObj->acText, x, y);
  }
}

/*********************************************************************
*
*       _DrawColorArcs
*/
static void _DrawColorArcs(const SCALE* pObj) {
  int r;
  int Start;
  int Angel1;
  int Angel2;
  int ArcLen;
  
  r      = pObj->ArcRadius - (pObj->ArcWidth / 2) - pObj->ArcPos;
  ArcLen = _GetArcLen(pObj);
  Start  = (pObj->ArcStart < pObj->ArcEnd) ? 270 - pObj->ArcStart : 630 - pObj->ArcStart;
  Angel1 = pObj->ArcArea1 * ArcLen / 359;
  Angel2 = pObj->ArcArea2 * ArcLen / 359;
  GUI_SetPenSize(pObj->ArcWidth);
  if (pObj->Flags & (1 << (FLAG_SHOW_ARC+0))) {
    GUI_SetColor(_CalcColor(&pObj->Color[3]));
    GUI_AA_DrawArc(pObj->x0, pObj->y0, r, r, Start - Angel1, Start);
  }
  if (pObj->Flags & (1 << (FLAG_SHOW_ARC+1))) {
    GUI_SetColor(_CalcColor(&pObj->Color[4]));
    GUI_AA_DrawArc(pObj->x0, pObj->y0, r, r, Start - Angel2, Start - Angel1);
  }
  if (pObj->Flags & (1 << (FLAG_SHOW_ARC+2))) {
    GUI_SetColor(_CalcColor(&pObj->Color[5]));
    GUI_AA_DrawArc(pObj->x0, pObj->y0, r, r, Start - ArcLen, Start - Angel2);
  }
}

/*********************************************************************
*
*       _DrawArcs
*/
static void _DrawArcs(const SCALE* pObj) {
  int Start;
  int End;
  int r1;
  int r2;

  Start = (pObj->ArcStart < pObj->ArcEnd) ? 270 - pObj->ArcStart : 630 - pObj->ArcStart;
  End   = 270 - pObj->ArcEnd;
  r1    = pObj->ArcRadius;
  r2    = pObj->ArcRadius - pObj->ArcPos - pObj->ArcWidth;
  GUI_SetColor(_CalcColor(&pObj->Color[0]));
  GUI_SetPenSize(pObj->PenSize3);
  if (pObj->Flags & (1 << (FLAG_SHOW_ARC+3))) {
    GUI_AA_DrawArc(pObj->x0, pObj->y0, r1, r1, End, Start);
  }
  if (pObj->Flags & (1 << (FLAG_SHOW_ARC+4))) {
    GUI_AA_DrawArc(pObj->x0, pObj->y0, r2, r2, End, Start);
    if (pObj->Flags & (1 << (FLAG_SHOW_ARC+3))) {
      GUI_SetPenSize(pObj->PenSize1);
      HIRES_ON();
      _DrawLine(pObj, r1, r2, (pObj->ArcStart - 90.) * PI);
      _DrawLine(pObj, r1, r2, (pObj->ArcEnd   - 90.) * PI);
      HIRES_OFF();
    }
  }
}

/*********************************************************************
*
*       _DrawNeedleThread
*/
static void _DrawNeedleThread(const SCALE* pObj, int Index) {
  int x0;
  int y0;
  int x1;
  int y1;
  int NumPoints;
  int Radius;

  Radius = (pObj->ArcRadius + pObj->PenSize3) * pObj->NeedleRadius / 100;
  GUI_SetPenSize(3);
  NumPoints = _CalcNeedle(pObj, Index, Radius - 3);
  x0 = pObj->x0 * FACTOR;
  y0 = pObj->y0 * FACTOR;
  x1 = x0 + _aNeedle[NumPoints/2].x;
  y1 = y0 + _aNeedle[NumPoints/2].y;
  if ((NumPoints % 2) != 0) {
    x0 += (_aNeedle[0].x + _aNeedle[NumPoints - 1].x) / 2;
    y0 += (_aNeedle[0].y + _aNeedle[NumPoints - 1].y) / 2;
  }
  GUI_SetColor(GUI_RED);
  GUI_AA_DrawLine(x0, y0, x1, y1);
}

/*********************************************************************
*
*       _DrawNeedleFrame
*/
static void _DrawNeedleFrame(const SCALE* pObj, int Index) {
  int NumPoints;
  int Radius;

  Radius = (pObj->ArcRadius + pObj->PenSize3) * pObj->NeedleRadius / 100;
  GUI_SetPenSize(2);
  NumPoints = _CalcNeedle(pObj, Index, Radius - 2);
  GUI_SetColor(_CalcColor(&pObj->Color[2]));
  _AA_DrawPolygon(_aNeedle, NumPoints, pObj->x0 * FACTOR, pObj->y0 * FACTOR);
}

/*********************************************************************
*
*       _DrawNeedle
*/
static void _DrawNeedle(const SCALE* pObj, int Index) {
  static GUI_MEMDEV_Handle hMemDev;
  GUI_MEMDEV_Handle        hPrev;
  GUI_RECT                 r;
  int                      NumPoints;
  int                      Radius;

  #if SHOW_RECTS
    _GetNeedleRect(pObj, Index, &r);
    GUI_SetColor(GUI_BLUE);
    GUI_FillRect(r.x0, r.y0, r.x1, r.y1);
  #endif
  WM_GetWindowRect(&r);
  HIRES_ON();
  hMemDev = GUI_MEMDEV_CreateEx(r.x0, r.y0, r.x1 - r.x0 + 1, r.y1 - r.y0 + 1, GUI_MEMDEV_HASTRANS);
  hPrev = GUI_MEMDEV_Select(hMemDev);
  GUI_MEMDEV_Write(pObj->hMemDev);
  GUI_MEMDEV_Clear(hMemDev);
  Radius = (pObj->ArcRadius + pObj->PenSize3) * pObj->NeedleRadius / 100;
  NumPoints = _CalcNeedle(pObj, Index, Radius);
  GUI_SetColor(_CalcColor(&pObj->Color[2]));
  GUI_AA_SetFactor(AA_FACTOR);
  GUI_AA_FillPolygon(_aNeedle, NumPoints, pObj->x0 * FACTOR, pObj->y0 * FACTOR);
  if (pObj->Flags & (1 << FLAG_NEEDLE_LINE)) {
    _DrawNeedleThread(pObj, Index);
  }
  GUI_MEMDEV_Select(hPrev);
  if (pObj->Color[2].Sep[3] != 0xFF) {
    GUI_MEMDEV_WriteAlpha(hMemDev, pObj->Color[2].Sep[3]);
    if (pObj->Flags & (1 << FLAG_NEEDLE_FRAME)) {
      _DrawNeedleFrame(pObj, Index);
    }
  } else {
    GUI_MEMDEV_Write(hMemDev);
    if ((pObj->Flags & (1 << FLAG_NEEDLE_FRAME)) && (pObj->Flags & (1 << FLAG_NEEDLE_LINE))) {
      _DrawNeedleFrame(pObj, Index);
    }
  }
  GUI_MEMDEV_Delete(hMemDev);
  HIRES_OFF();
  GUI_SetColor(_CalcColor(&pObj->Color[6]));
  GUI_AA_FillCircle(pObj->x0, pObj->y0, pObj->ArcRadius * pObj->AxisRadius / 200);
}

/*********************************************************************
*
*       _DrawScale
*/
static void _DrawScale(SCALE* pObj) {
  int x;
  int y;

  #if SHOW_RECTS
    GUI_SetBkColor(GUI_GRAY);
    GUI_Clear();
  #endif
  if (pObj->ArcStart != pObj->ArcEnd) {
    if (pObj->pBitmap) {
      x = pObj->x0 - (pObj->pBitmap->XSize / 2);
      y = pObj->y0 - (pObj->pBitmap->YSize / 2) + (pObj->ArcRadius / pObj->BitmapY);
      GUI_DrawBitmap(pObj->pBitmap, x, y);
    }
    GUI_AA_SetFactor(AA_FACTOR);
    GUI_SetPenShape(GUI_PS_ROUND);
    _DrawColorArcs(pObj);
    _DrawArcs(pObj);
    _DrawPitchLines(pObj);
    _DrawMarkLines(pObj);
    _DrawGrad(pObj);
    _DrawText(pObj);
  }
}

/*********************************************************************
*
*       _DrawFrame
*/
static void _DrawFrame(int xDist, int yDist, int rx, int ry, int yEnd,
                       int PenSize, GUI_COLOR Color) {
  const GUI_RECT * OldClip;
  GUI_RECT         ClipRect;

  OldClip = _SetClipRect(&ClipRect, xDist - rx, yDist - ry, xDist, yDist);
  GUI_SetColor(Color);
  GUI_DrawEllipse(xDist, yDist, rx, ry);
  _SetClipRect(&ClipRect, 639 - xDist, yDist - ry, 639 - xDist + rx, yDist);
  GUI_DrawEllipse(639 - xDist, yDist, rx, ry);
  WM_SetUserClipRect(OldClip);
  GUI_FillRect(xDist, yDist - ry, 639 - xDist, yDist - ry + PenSize - 1);
  GUI_FillRect(xDist - rx, yEnd - PenSize + 1, 639 - xDist + rx, yEnd);
  GUI_FillRect(xDist - rx, yDist, xDist - rx + PenSize - 1, yEnd);
  GUI_FillRect(639 - xDist + rx - PenSize + 1, yDist, 639 - xDist + rx, yEnd);
}

/*********************************************************************
*
*       _CreateBackGround
*/
static void _CreateBackGround(void) {
  GUI_MEMDEV_Handle hMemPrev;
  GUI_RECT          r;
  int               xSize;
  int               ySize;

  xSize = LCD_GetXSize();
  ySize = LCD_GetYSize();
  r.x0  = 0;
  r.x1  = xSize - 1;
  r.y0  = 0;
  r.y1  = ySize - 195;
  _hBkMemDev = GUI_MEMDEV_CreateEx(r.x0, r.y0, r.x1 + 1, r.y1 + 1, GUI_MEMDEV_NOTRANS);
  hMemPrev = GUI_MEMDEV_Select(_hBkMemDev);
  GUI_SetBkColor(GUI_BLACK);
  GUI_Clear();
  _DrawFrame(230, 170, 230, 170, 285, 1, 0x444444);
  _DrawFrame(230, 170, 229, 169, 284, 1, 0x808080);
  _DrawFrame(230, 170, 228, 168, 283, 1, 0xA0A0A0);
  _DrawFrame(230, 170, 227, 167, 282, 1, 0xC0C0C0);
  _DrawFrame(230, 170, 226, 166, 281, 1, 0xA0A0A0);
  _DrawFrame(230, 170, 225, 165, 280, 1, 0x808080);
  _DrawFrame(230, 170, 224, 164, 279, 1, 0x444444);
  GUI_MEMDEV_Select(hMemPrev);
}

/*********************************************************************
*
*       static code, window callbacks
*
**********************************************************************
*/

/*********************************************************************
*
*       _cbBkWindow
*/
static void _cbBkWindow(WM_MESSAGE* pMsg) {
  switch (pMsg->MsgId) {
  case WM_PAINT:
    GUI_MEMDEV_Write(_hBkMemDev);
    break;
  case WM_KEY:
    if (((WM_KEY_INFO *)pMsg->Data.p)->Key == GUI_KEY_ESCAPE) {
      _Break = 1;
    }
    break;
  default:
    WM_DefaultProc(pMsg);
  }
}

/*********************************************************************
*
*       _cbFrameWin
*/
static void _cbFrameWin(WM_MESSAGE* pMsg) {
  int Scale;
  int Id;
  
  Scale = 0;
  switch (pMsg->MsgId) {
  case WM_NOTIFY_PARENT:
    if (WM_IsWindow(_hDropDownScale)) {
      Scale = DROPDOWN_GetSel(_hDropDownScale) - 1;
    }
    Id    = WM_GetId(pMsg->hWinSrc);         // Id of widget
    switch (pMsg->Data.v) {
    case WM_NOTIFICATION_RELEASED:
      if (Id >= GUI_ID_BUTTON0 && Id <= GUI_ID_BUTTON3) {
        _SetPreset(Id - GUI_ID_BUTTON0, Scale);
      } else if (Id == GUI_ID_CHECK0) {
        WM_HWIN hItem;
        hItem = WM_GetDialogItem(pMsg->hWin, GUI_ID_CHECK0);
        _AutoMode = CHECKBOX_GetState(hItem);
      }
      break;
    case WM_NOTIFICATION_SEL_CHANGED:
      if (pMsg->hWinSrc == _hDropDownScale) {
        _SetDialogs((Scale != -1) ? Scale : 0);
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
*       _cbWinMap
*/
static void _cbWinMap(WM_MESSAGE* pMsg) {
  switch (pMsg->MsgId) {
  case WM_PAINT:
    GUI_DrawBitmap(_NaviMap.pBitmap, -_NaviMap.x, -_NaviMap.y);
    GUI_SetPenSize(3);
    GUI_SetDrawMode(GUI_DRAWMODE_NORMAL);
    GUI_SetColor(GUI_RED);
    GUI_DrawCircle(_NaviMap.xHere, _NaviMap.yHere, 5);
    GUI_SetTextMode(GUI_TM_TRANS);
    GUI_SetFont(&GUI_FontComic18B_ASCII);
    GUI_DispStringAt("You are here", _NaviMap.xHere - 20, _NaviMap.yHere - 20);
    /* Leave code for test purpose
    GUI_SetFont(&GUI_Font6x8);
    GUI_DispDecAt(_tDiff, 220, 15, 3);
    */
    break;
  default:
    WM_DefaultProc(pMsg);
  }
}

/*********************************************************************
*
*       _cbScaleWin
*/
static void _cbScaleWin(WM_MESSAGE* pMsg) {
  int Index;

  switch (pMsg->MsgId) {
  case WM_PAINT:
    for (Index = 0; Index < 4; Index++) {
      if (_Scale[Index].hWin == pMsg->hWin) {
        if (_Scale[Index].Flags & (1 << FLAG_SHOW_SCALE)) {
          _UpdateScale(Index);
          GUI_MEMDEV_Write(_Scale[Index].hMemDev);
          _DrawNeedle(&_Scale[Index], Index);
        }
        break;
      }
    }
    break;
  default:
    WM_DefaultProc(pMsg);
  }
}

/*********************************************************************
*
*       static code, dialog callbacks
*
**********************************************************************
*/

/*********************************************************************
*
*       _cbDialogColor
*/
static void _cbDialogColor(WM_MESSAGE * pMsg) {
  int Scale;
  int Index;
  int Id;

  switch (pMsg->MsgId) {
  case WM_INIT_DIALOG:
    _hDialogColor = pMsg->hWin;
    _hDropDownColor = WM_GetDialogItem(_hDialogColor, GUI_ID_USER);
    DROPDOWN_AddString(_hDropDownColor, "Scale");
    DROPDOWN_AddString(_hDropDownColor, "Text");
    DROPDOWN_AddString(_hDropDownColor, "Needle");
    DROPDOWN_AddString(_hDropDownColor, "Arc 1");
    DROPDOWN_AddString(_hDropDownColor, "Arc 2");
    DROPDOWN_AddString(_hDropDownColor, "Arc 3");
    DROPDOWN_AddString(_hDropDownColor, "Axis");
    _SetDialogColor(0);
    break;
  case WM_NOTIFY_PARENT:
    if (_InitDialog) break;
    switch (pMsg->Data.v) {
    case WM_NOTIFICATION_SEL_CHANGED: // Value has changed
      Scale = DROPDOWN_GetSel(_hDropDownScale) - 1;
      if (pMsg->hWinSrc == _hDropDownColor) {
        _SetDialogColor((Scale == -1) ? 0 : Scale);
      }
      break;
    case WM_NOTIFICATION_VALUE_CHANGED: // Value has changed
      Scale = DROPDOWN_GetSel(_hDropDownScale) - 1;
      Id    = WM_GetId(pMsg->hWinSrc);         // Id of widget
      Index = DROPDOWN_GetSel(_hDropDownColor);
      if (Id >= GUI_ID_SLIDER0 && Id <= GUI_ID_SLIDER3) {
        int v = _GetSliderValue(pMsg->hWin, Id);
        int i, iStart, iEnd;
        iStart = (Scale == -1) ? 0 : Scale;
        iEnd   = (Scale == -1) ? 3 : Scale;
        for (i = iStart; i <= iEnd; i++) {
          _Scale[i].Color[Index].Sep[Id - GUI_ID_SLIDER0] = v;
          WM_InvalidateWindow(_Scale[i].hWin);
        }
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
*       _cbDialogMark
*/
static void _cbDialogMark(WM_MESSAGE * pMsg) {
  int Scale;
  int Id;

  Scale = 0;
  switch (pMsg->MsgId) {
  case WM_INIT_DIALOG:
    _hDialogMark = pMsg->hWin;
    _SetDialogMark(0);
    break;
  case WM_NOTIFY_PARENT:
    if (_InitDialog) break;
    Id    = WM_GetId(pMsg->hWinSrc);         // Id of widget
    if (WM_IsWindow(_hDropDownScale)) {
      Scale = DROPDOWN_GetSel(_hDropDownScale) - 1;
    }
    switch (pMsg->Data.v) {
    case WM_NOTIFICATION_VALUE_CHANGED: // Value has changed
      if (Id >= GUI_ID_SLIDER0 && Id <= GUI_ID_SLIDER3) {
        int v = _GetSliderValue(pMsg->hWin, Id);
        int i, iStart, iEnd;
        iStart = (Scale == -1) ? 0 : Scale;
        iEnd   = (Scale == -1) ? 3 : Scale;
        for (i = iStart; i <= iEnd; i++) {
          switch (Id) {
          case GUI_ID_SLIDER0:
            _Scale[i].NumMarkLines = v;
            break;
          case GUI_ID_SLIDER1:
            _Scale[i].LineLen1 = v;
            break;
          case GUI_ID_SLIDER2:
            _Scale[i].LinePos1 = v;
            break;
          case GUI_ID_SLIDER3:
            _Scale[i].PenSize1 = v;
            _ChangeScaleSize(&_Scale[i]);
            break;
          }
          WM_InvalidateWindow(_Scale[i].hWin);
        }
      }
      break;
    case WM_NOTIFICATION_CLICKED:
      if (Id == GUI_ID_USER) {
        WM_HWIN hItem = WM_GetDialogItem(pMsg->hWin, Id);
        int v = CHECKBOX_IsChecked(hItem);
        int i, iStart, iEnd;
        iStart = (Scale == -1) ? 0 : Scale;
        iEnd   = (Scale == -1) ? 3 : Scale;
        for (i = iStart; i <= iEnd; i++) {
          _Scale[i].Flags &= ~(1 << FLAG_SHOW_MARK);
          _Scale[i].Flags |= v << FLAG_SHOW_MARK;
          WM_InvalidateWindow(_Scale[i].hWin);
        }
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
*       _cbDialogPitch
*/
static void _cbDialogPitch(WM_MESSAGE * pMsg) {
  int Scale;
  int Id;

  Scale = 0;
  switch (pMsg->MsgId) {
  case WM_INIT_DIALOG:
    _hDialogPitch = pMsg->hWin;
    _SetDialogPitch(0);
    break;
  case WM_NOTIFY_PARENT:
    if (_InitDialog) break;
    Id    = WM_GetId(pMsg->hWinSrc);         // Id of widget
    if (WM_IsWindow(_hDropDownScale)) {
      Scale = DROPDOWN_GetSel(_hDropDownScale) - 1;
    }
    switch (pMsg->Data.v) {
    case WM_NOTIFICATION_VALUE_CHANGED: // Value has changed
      if (Id >= GUI_ID_SLIDER0 && Id <= GUI_ID_SLIDER3) {
        int v = _GetSliderValue(pMsg->hWin, Id);
        int i, iStart, iEnd;
        iStart = (Scale == -1) ? 0 : Scale;
        iEnd   = (Scale == -1) ? 3 : Scale;
        for (i = iStart; i <= iEnd; i++) {
          switch (Id) {
          case GUI_ID_SLIDER0:
            _Scale[i].NumPitchLines = v;
            break;
          case GUI_ID_SLIDER1:
            _Scale[i].LineLen2 = v;
            break;
          case GUI_ID_SLIDER2:
            _Scale[i].LinePos2 = v;
            break;
          case GUI_ID_SLIDER3:
            _Scale[i].PenSize2 = v;
            _ChangeScaleSize(&_Scale[i]);
            break;
          }
          WM_InvalidateWindow(_Scale[i].hWin);
        }
      }
      break;
    case WM_NOTIFICATION_CLICKED:
      if (Id == GUI_ID_USER) {
        WM_HWIN hItem = WM_GetDialogItem(pMsg->hWin, Id);
        int v = CHECKBOX_IsChecked(hItem);
        int i, iStart, iEnd;
        iStart = (Scale == -1) ? 0 : Scale;
        iEnd   = (Scale == -1) ? 3 : Scale;
        for (i = iStart; i <= iEnd; i++) {
          _Scale[i].Flags &= ~(1 << FLAG_SHOW_PITCH);
          _Scale[i].Flags |= v << FLAG_SHOW_PITCH;
          WM_InvalidateWindow(_Scale[i].hWin);
        }
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
*       _cbDialogArc
*/
static void _cbDialogArc(WM_MESSAGE * pMsg) {
  WM_HWIN hItem;
  int     i;
  int     iStart;
  int     iEnd;
  int     Scale;
  int     Id;
  int     v;
  int     f;
  
  Scale = 0;
  switch (pMsg->MsgId) {
  case WM_INIT_DIALOG:
    _hDialogArc = pMsg->hWin;
    _SetDialogArc(0);
    break;
  case WM_NOTIFY_PARENT:
    if (_InitDialog) break;
    Id    = WM_GetId(pMsg->hWinSrc);         // Id of widget
    if (WM_IsWindow(_hDropDownScale)) {
      Scale = DROPDOWN_GetSel(_hDropDownScale) - 1;
    }
    switch (pMsg->Data.v) {
    case WM_NOTIFICATION_VALUE_CHANGED: // Value has changed
      if (Id >= GUI_ID_SLIDER0 && Id <= GUI_ID_SLIDER4) {
        v      = _GetSliderValue(pMsg->hWin, Id);
        iStart = (Scale == -1) ? 0 : Scale;
        iEnd   = (Scale == -1) ? 3 : Scale;
        for (i = iStart; i <= iEnd; i++) {
          switch (Id) {
          case GUI_ID_SLIDER0:
            _Scale[i].ArcArea1 = v;
            break;
          case GUI_ID_SLIDER1:
            _Scale[i].ArcArea2 = v;
            break;
          case GUI_ID_SLIDER2:
            _Scale[i].ArcWidth = v;
            break;
          case GUI_ID_SLIDER3:
            _Scale[i].ArcPos = v;
            break;
          case GUI_ID_SLIDER4:
            _Scale[i].PenSize3 = v;
            _ChangeScaleSize(&_Scale[i]);
            break;
          }
          WM_InvalidateWindow(_Scale[i].hWin);
        }
      }
      break;
    case WM_NOTIFICATION_CLICKED:
      if ((Id >= GUI_ID_USER+0) && (Id <= GUI_ID_USER+4)) {
        hItem  = WM_GetDialogItem(pMsg->hWin, Id);
        v      = CHECKBOX_IsChecked(hItem);
        f      = Id - GUI_ID_USER;
        iStart = (Scale == -1) ? 0 : Scale;
        iEnd   = (Scale == -1) ? 3 : Scale;
        for (i = iStart; i <= iEnd; i++) {
          _Scale[i].Flags &= ~(1 << (FLAG_SHOW_ARC+f));
          _Scale[i].Flags |= v << (FLAG_SHOW_ARC+f);
          WM_InvalidateWindow(_Scale[i].hWin);
        }
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
*       _cbDialogGrad
*/
static void _cbDialogGrad(WM_MESSAGE * pMsg) {
  WM_HWIN hItem;
  int     i;
  int     iStart;
  int     iEnd;
  int     Scale;
  int     Id;
  int     v;

  Scale  = 0;
  switch (pMsg->MsgId) {
  case WM_INIT_DIALOG:
    _hDialogGrad = pMsg->hWin;
    _SetDialogGrad(0);
    break;
  case WM_NOTIFY_PARENT:
    if (_InitDialog) break;
    Id    = WM_GetId(pMsg->hWinSrc);         // Id of widget
    if (WM_IsWindow(_hDropDownScale)) {
      Scale = DROPDOWN_GetSel(_hDropDownScale) - 1;
    }
    switch (pMsg->Data.v) {
    case WM_NOTIFICATION_VALUE_CHANGED: // Value has changed
      if (Id >= GUI_ID_SLIDER0 && Id <= GUI_ID_SLIDER4) {
        v      = _GetSliderValue(pMsg->hWin, Id);
        iStart = (Scale == -1) ? 0 : Scale;
        iEnd   = (Scale == -1) ? 3 : Scale;
        for (i = iStart; i <= iEnd; i++) {
          switch (Id) {
          case GUI_ID_SLIDER0:
            _Scale[i].GradDist = v;
            break;
          case GUI_ID_SLIDER1:
            _Scale[i].NumStep = v;
            break;
          case GUI_ID_SLIDER2:
            _Scale[i].NumStart = v;
            break;
          case GUI_ID_SLIDER3:
            _Scale[i].NumExp = v;
            break;
          case GUI_ID_SLIDER4:
            _Scale[i].TextDist = v;
            _ChangeScaleSize(&_Scale[i]);
            break;
          }
          if (Id < GUI_ID_SLIDER4) {
            WM_InvalidateWindow(_Scale[i].hWin);
          }
        }
      }
      break;
    case WM_NOTIFICATION_CLICKED:
      if ((Id >= GUI_ID_USER+0) && (Id <= GUI_ID_USER+1)) {
        hItem         = WM_GetDialogItem(pMsg->hWin, Id);
        v             = CHECKBOX_IsChecked(hItem);
        iStart        = (Scale == -1) ? 0 : Scale;
        iEnd          = (Scale == -1) ? 3 : Scale;
        for (i        = iStart; i <= iEnd; i++) {
          switch (Id) {
          case GUI_ID_USER+0:
            _Scale[i].Flags &= ~(1 << FLAG_SHOW_GRAD);
            _Scale[i].Flags |= v << FLAG_SHOW_GRAD;
            WM_InvalidateWindow(_Scale[i].hWin);
            break;
          case GUI_ID_USER+1:
            _Scale[i].Flags &= ~(1 << FLAG_SHOW_TEXT);
            _Scale[i].Flags |= v << FLAG_SHOW_TEXT;
            _ChangeScaleSize(&_Scale[i]);
            break;
          }
        }
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
*       _cbDialogScale
*/
static void _cbDialogScale(WM_MESSAGE * pMsg) {
  WM_HWIN hItem;
  int     i;
  int     iStart;
  int     iEnd;
  int     Scale;
  int     Id;
  int     v;

  Scale  = 0;
  switch (pMsg->MsgId) {
  case WM_INIT_DIALOG:
    _hDialogScale = pMsg->hWin;
    _SetDialogScale(0);
    break;
  case WM_NOTIFY_PARENT:
    if (_InitDialog) break;
    Id    = WM_GetId(pMsg->hWinSrc);         // Id of widget
    if (WM_IsWindow(_hDropDownScale)) {
      Scale = DROPDOWN_GetSel(_hDropDownScale) - 1;
    }
    switch (pMsg->Data.v) {
    case WM_NOTIFICATION_VALUE_CHANGED: // Value has changed
      if (Id >= GUI_ID_SLIDER0 && Id <= GUI_ID_SLIDER4) {
        v      = _GetSliderValue(pMsg->hWin, Id);
        iStart = (Scale == -1) ? 0 : Scale;
        iEnd   = (Scale == -1) ? 3 : Scale;
        for (i = iStart; i <= iEnd; i++) {
          switch (Id) {
          case GUI_ID_SLIDER0:
            _Scale[i].ArcStart = v;
            break;
          case GUI_ID_SLIDER1:
            _Scale[i].ArcEnd = v;
            break;
          case GUI_ID_SLIDER2:
            _Scale[i].ArcRadius = v;
            break;
          case GUI_ID_SLIDER3:
            _Scale[i].x = v;
            break;
          case GUI_ID_SLIDER4:
            _Scale[i].y = v;
            break;
          }
          _ChangeScaleSize(&_Scale[i]);
        }
      }
      break;
    case WM_NOTIFICATION_CLICKED:
      if (Id == GUI_ID_USER) {
        hItem  = WM_GetDialogItem(pMsg->hWin, Id);
        v      = CHECKBOX_IsChecked(hItem);
        iStart = (Scale == -1) ? 0 : Scale;
        iEnd   = (Scale == -1) ? 3 : Scale;
        for (i = iStart; i <= iEnd; i++) {
          _Scale[i].Flags &= ~(1 << FLAG_SHOW_SCALE);
          _Scale[i].Flags |= v << FLAG_SHOW_SCALE;
          WM_InvalidateWindow(_Scale[i].hWin);
        }
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
*       _cbDialogMisc
*/
static void _cbDialogMisc(WM_MESSAGE * pMsg) {
  WM_HWIN hItem;
  int     i;
  int     iStart;
  int     iEnd;
  int     Scale;
  int     Id;
  int     v;

  Scale  = 0;
  switch (pMsg->MsgId) {
  case WM_INIT_DIALOG:
    _hDialogMisc = pMsg->hWin;
    _SetDialogMisc(0);
    break;
  case WM_NOTIFY_PARENT:
    if (_InitDialog) break;
    Id    = WM_GetId(pMsg->hWinSrc);         // Id of widget
    if (WM_IsWindow(_hDropDownScale)) {
      Scale = DROPDOWN_GetSel(_hDropDownScale) - 1;
    }
    switch (pMsg->Data.v) {
    case WM_NOTIFICATION_VALUE_CHANGED: // Value has changed
      if (Id >= GUI_ID_SLIDER0 && Id <= GUI_ID_SLIDER2) {
        v      = _GetSliderValue(pMsg->hWin, Id);
        iStart = (Scale == -1) ? 0 : Scale;
        iEnd   = (Scale == -1) ? 3 : Scale;
        for (i = iStart; i <= iEnd; i++) {
          switch (Id) {
          case GUI_ID_SLIDER0:
            _Scale[i].NeedleType = v;
            break;
          case GUI_ID_SLIDER1:
            _Scale[i].NeedleRadius = v;
            break;
          case GUI_ID_SLIDER2:
            _Scale[i].AxisRadius = v;
            break;
          }
          _ChangeScaleSize(&_Scale[i]);
        }
      }
      break;
    case WM_NOTIFICATION_CLICKED:
      if ((Id >= GUI_ID_USER+0) && (Id <= GUI_ID_USER+1)) {
        hItem  = WM_GetDialogItem(pMsg->hWin, Id);
        v      = CHECKBOX_IsChecked(hItem);
        iStart = (Scale == -1) ? 0 : Scale;
        iEnd   = (Scale == -1) ? 3 : Scale;
        for (i = iStart; i <= iEnd; i++) {
          switch (Id) {
          case GUI_ID_USER+0:
            _Scale[i].Flags &= ~(1 << FLAG_NEEDLE_FRAME);
            _Scale[i].Flags |= v << FLAG_NEEDLE_FRAME;
            WM_InvalidateWindow(_Scale[i].hWin);
            break;
          case GUI_ID_USER+1:
            _Scale[i].Flags &= ~(1 << FLAG_NEEDLE_LINE);
            _Scale[i].Flags |= v << FLAG_NEEDLE_LINE;
            WM_InvalidateWindow(_Scale[i].hWin);
            break;
          }
        }
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
*       static code
*
**********************************************************************
*/
/*********************************************************************
*
*       _CreateScaleWindow
*/
static void _CreateScaleWindow(SCALE* pObj) {
  pObj->hWin = WM_CreateWindow(0, 0, 10, 10, WM_CF_SHOW | WM_CF_HASTRANS, &_cbScaleWin, 0);
  _ChangeScaleSize(pObj);
}

/*********************************************************************
*
*       _ClearState
*/
static void _ClearState(void) {
  GUI_PID_STATE State = {0};
  GUI_PID_StoreState(&State);
}

/*********************************************************************
*
*       _DashboardDemo
*/
static void _DashboardDemo(void) {
  MULTIPAGE_Handle hMultiPage;
  FRAMEWIN_Handle hFrame;
  FRAMEWIN_Handle hWinMap;
  CHECKBOX_Handle hCheck;
  GUI_PID_STATE   CurrentState;
  GUI_PID_STATE   State = {0};
  GUI_PID_STATE   OldState;
  WM_HWIN         hClient;
  WM_HWIN         hItem;
  int             tNow;
  int             tEnd;
  int             tNextTouchEvent;
  int             iTouchEvent;
  int             i;

  _Break    = 0;
  _AutoMode = 1;
  // 
  // Use memory devices
  //
  WM_SetCreateFlags(WM_CF_MEMDEV);
  WM_EnableMemdev(WM_HBKWIN);
  //
  // Set callback for background
  //
  WM_SetCallback(WM_HBKWIN, &_cbBkWindow);
  //
  // Set preset
  //
  _Scale[0] = _Presets[0][0];
  _Scale[1] = _Presets[0][1];
  _Scale[2] = _Presets[0][2];
  _Scale[3] = _Presets[0][3];
  //
  // Create backgrund
  //
  _CreateBackGround();
  //
  // Create the scale-windows
  //
  _CreateScaleWindow(&_Scale[0]);
  _CreateScaleWindow(&_Scale[1]);
  _CreateScaleWindow(&_Scale[2]);
  _CreateScaleWindow(&_Scale[3]);
  //
  // Create framewindow
  //
  hFrame = FRAMEWIN_Create("Controls", &_cbFrameWin, WM_CF_SHOW, 0, 285, 380, 195);
  FRAMEWIN_SetActive(hFrame, 1);
  //
  // Create multipage
  //
  hMultiPage = MULTIPAGE_CreateEx(10, 10, 280, 154, WM_GetClientWindow(hFrame), WM_CF_SHOW, 0, 0);
  _hDialogColor   = _AddDialog("Color",   ARRAY(_aDialogColor),   &_cbDialogColor,   hMultiPage);
  _hDialogMark    = _AddDialog("Mark",    ARRAY(_aDialogMark),    &_cbDialogMark,    hMultiPage);
  _hDialogPitch   = _AddDialog("Pitch",   ARRAY(_aDialogPitch),   &_cbDialogPitch,   hMultiPage);
  _hDialogArc     = _AddDialog("Arc",     ARRAY(_aDialogArc),     &_cbDialogArc,     hMultiPage);
  _hDialogGrad    = _AddDialog("Grad",    ARRAY(_aDialogGrad),    &_cbDialogGrad,    hMultiPage);
  _hDialogScale   = _AddDialog("Scale",   ARRAY(_aDialogScale),   &_cbDialogScale,   hMultiPage);
  _hDialogMisc    = _AddDialog("Misc",    ARRAY(_aDialogMisc),    &_cbDialogMisc,    hMultiPage);
  MULTIPAGE_SelectPage(hMultiPage, 0);
  //
  // Create drop-down box
  //
  hClient = WM_GetClientWindow(hFrame);
  _hDropDownScale = DROPDOWN_Create(hClient, 297, 35, 65, 73, WM_CF_SHOW);
  DROPDOWN_AddString(_hDropDownScale, "All");
  DROPDOWN_AddString(_hDropDownScale, "Scale 1");
  DROPDOWN_AddString(_hDropDownScale, "Scale 2");
  DROPDOWN_AddString(_hDropDownScale, "Scale 3");
  DROPDOWN_AddString(_hDropDownScale, "Scale 4");
  //
  // Create checkbox for automode
  //
  hCheck = CHECKBOX_CreateEx(297, 11, 65, 20, hClient, WM_CF_SHOW, 0, GUI_ID_CHECK0);
  CHECKBOX_SetText(hCheck, "Auto");
  CHECKBOX_SetState(hCheck, 1);
  //
  // Create buttons for presets
  //
  _CreateButton("Preset 1", 292,  65, 65, 20, hClient, GUI_ID_BUTTON0);
  _CreateButton("Preset 2", 292,  89, 65, 20, hClient, GUI_ID_BUTTON1);
  _CreateButton("Preset 3", 292, 113, 65, 20, hClient, GUI_ID_BUTTON2);
  _CreateButton("Preset 4", 292, 137, 65, 20, hClient, GUI_ID_BUTTON3);
  //
  // Create framewindow for map
  //
  hWinMap  = FRAMEWIN_Create("Map to Segger Hilden", &_cbWinMap, WM_CF_SHOW, 380, 285, 260, 195);
  FRAMEWIN_SetActive(hWinMap, 0);
  hWinMap = WM_GetClientWindow(hWinMap);
  //
  // Handle the demo
  //
  GUI_CURSOR_Show();
  iTouchEvent = 0;
  tNextTouchEvent = GUI_GetTime() + 4000; // First touch event
  tEnd = GUI_GetTime() + 44000;
  _ClearState();
  do {
    GUI_PID_GetState(&CurrentState);
    tNow = GUI_GetTime();
    _MoveMap(hWinMap);
    _MoveNeedle(&_Needle[0], 0);
    _MoveNeedle(&_Needle[1], 1);
    _MoveNeedle(&_Needle[2], 2);
    _MoveNeedle(&_Needle[3], 3);
    GUI_Exec();
    _tDiff = GUI_GetTime() - tNow;
    if (_tDiff < 25) {                 // Make sure we have no more than a certain number of frames/sec
      GUI_Delay(25 - _tDiff);
    }
    if (_AutoMode) {
      tEnd = tNow + 4000;
      if ((CurrentState.Pressed) && ((CurrentState.x != _aPID_Events[iTouchEvent].x) || (CurrentState.y != _aPID_Events[iTouchEvent].y))) {
        hItem = WM_GetDialogItem(hFrame, GUI_ID_CHECK0);
        CHECKBOX_SetState(hItem, 0);
        _AutoMode = 0;
      } else {
        if (tNow >= tNextTouchEvent) {
          State.x = _aPID_Events[iTouchEvent].x;
          State.y = _aPID_Events[iTouchEvent].y;
          State.Pressed = _aPID_Events[iTouchEvent].Pressed;
          tNextTouchEvent = tNow + _aPID_Events[iTouchEvent].Duration;
          GUI_PID_StoreState(&State);
          iTouchEvent++;
          if (iTouchEvent >= (int)GUI_COUNTOF(_aPID_Events)) {
            _Break = 1;
          }
        }
      }
    } else {
      if (memcmp(&OldState, &CurrentState, sizeof(GUI_PID_STATE)) != 0) {
        tEnd = tNow + 4000;
      }
      OldState = CurrentState;
    }
  } while (!_Break && (tNow < tEnd));
  //
  // Unset callback
  //
  WM_SetCallback(WM_HBKWIN, NULL);
  //
  // Clear memory
  //
  for (i = 0; i < (int)GUI_COUNTOF(_Scale); i++) {
    WM_DeleteWindow(_Scale[i].hWin);
    GUI_MEMDEV_Delete(_Scale[i].hMemDev);
  }
  GUI_MEMDEV_Delete(_hBkMemDev);
  WM_DeleteWindow(WM_GetParent(hWinMap));
  WM_DeleteWindow(hFrame);
  _hDropDownScale = 0;
}

/*********************************************************************
*
*       AppDashBoard
*
**********************************************************************
*/

int AppDashBoard(void);
int AppDashBoard(void) {
  WM_SetCallback(WM_HBKWIN, NULL);
  GUI_Clear();
  _DashboardDemo();
  return _Break;
}

/*************************** End of file ****************************/

