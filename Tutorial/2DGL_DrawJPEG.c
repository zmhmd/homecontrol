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
File        : 2DGL_DrawJPG.c
Purpose     : Example for drawing JPG files
Requirements: WindowManager - ( )
              MemoryDevices - ( )
              AntiAliasing  - ( )
              VNC-Server    - ( )
              PNG-Library   - ( )
              TrueTypeFonts - ( )

              Can be used in a MS Windows environment only!
----------------------------------------------------------------------
*/

#ifndef SKIP_TEST

#include <windows.h>
#include <stdio.h>

#include "GUI.h"

/*******************************************************************
*
*       Static functions
*
********************************************************************
*/
/*******************************************************************
*
*       _ShowJPG
*
* Function description
*   Shows the contents of a JPEG file
*/
static void _ShowJPG(const char * sFilename) {
  GUI_JPEG_INFO Info;
  DWORD         NumBytesRead;
  int           XSize;
  int           YSize;
  int           XPos;
  int           YPos;

  HANDLE hFile   = CreateFile(sFilename, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
  DWORD FileSize = GetFileSize(hFile, NULL);
  char * pFile   = malloc(FileSize);
  ReadFile(hFile, pFile, FileSize, &NumBytesRead, NULL);
  CloseHandle(hFile);
  GUI_ClearRect(0, 60, 319, 239);
  GUI_JPEG_GetInfo(pFile, FileSize, &Info);
  XSize = Info.XSize;
  YSize = Info.YSize;
  XPos = (XSize > 320) ?  0 : 160 - (XSize / 2);
  YPos = (YSize > 180) ? 60 : 150 - (YSize / 2);
  if (!GUI_JPEG_Draw(pFile, FileSize, XPos, YPos)) {
    GUI_Delay(2000);
  }
  free(pFile);
}

/*******************************************************************
*
*       _GetFirstBitmapDirectory
*
* Function description
*   Returns the first directory which contains one or more JPG files
*/
static int _GetFirstBitmapDirectory(char * pDir, char * pBuffer) {
  WIN32_FIND_DATA Context;
  HANDLE          hFind;
  char            acMask[_MAX_PATH];
  char            acPath[_MAX_PATH];

  sprintf(acMask, "%s\\*.jpg", pDir);
  hFind = FindFirstFile(acMask, &Context);
  if (hFind != INVALID_HANDLE_VALUE) {
    strcpy(pBuffer, pDir);
    return 1;
  }
  sprintf(acMask, "%s\\*.", pDir);
  hFind = FindFirstFile(acMask, &Context);
  if (hFind != INVALID_HANDLE_VALUE) {
    do {
      if ((strcmp(Context.cFileName, ".") != 0) && (strcmp(Context.cFileName, "..") != 0)) {
        if (Context.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
          sprintf(acPath, "%s\\%s", pDir, Context.cFileName);
          if (_GetFirstBitmapDirectory(acPath, pBuffer)) {
            return 1;
          }
        }
      }
    } while (FindNextFile(hFind, &Context));
  }
  return 0;
}

/*******************************************************************
*
*       _DrawWindowsDirectoryBitmaps
*
* Functin description
*   Iterates over all *.jpg-files of the windows directory
*/
static void _DrawWindowsDirectoryBitmaps(void) {
  WIN32_FIND_DATA Context;
  HANDLE          hFind;
  char            acPath[_MAX_PATH];
  char            acMask[_MAX_PATH];
  char            acFile[_MAX_PATH];
  char            acBuffer[_MAX_PATH];

  GUI_SetBkColor(GUI_BLACK);
  GUI_Clear();
  GUI_SetColor(GUI_WHITE);
  GUI_SetFont(&GUI_Font24_ASCII);
  GUI_DispStringHCenterAt("DrawJPG - Sample", 160, 5);
  GUI_SetFont(&GUI_Font8x16);
  GetWindowsDirectory(acBuffer, sizeof(acBuffer));
  _GetFirstBitmapDirectory(acBuffer, acPath);
  sprintf(acMask, "%s\\*.jpg", acPath);
  hFind = FindFirstFile(acMask, &Context);
  if (hFind != INVALID_HANDLE_VALUE) {
    do {
      sprintf(acFile, "%s\\%s", acPath, Context.cFileName);
      GUI_DispStringAtCEOL(acFile, 5, 40);
      _ShowJPG(acFile);
    } while (FindNextFile(hFind, &Context));
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
  while (1) {
    _DrawWindowsDirectoryBitmaps();
  }
}

#endif

/*************************** End of file ****************************/
