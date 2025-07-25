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
File        : MOVIE_ShowFromFS.c
Purpose     : Shows how to play a movie directly from a file system.
Requirements: WindowManager - ( )
              MemoryDevices - ( )
              AntiAliasing  - ( )
              VNC-Server    - ( )
              PNG-Library   - ( )
              TrueTypeFonts - ( )

              Requires either a MS Windows environment or emFile.
----------------------------------------------------------------------
*/

#ifndef SKIP_TEST

#include "GUI.h"

#ifdef WIN32
  #include <windows.h>
#else
  #include "FS.h"
#endif

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/
/*********************************************************************
*
*       _cbNotify
*
* Function description
*   Callback function for movie player. Uses multiple buffering if
*   available to avoid tearing effects.
*/
void _cbNotify(GUI_HMEM hMem, int Notification, U32 CurrentFrame) {
  switch (Notification) {
  case GUI_MOVIE_NOTIFICATION_PREDRAW:
    GUI_MULTIBUF_Begin();
    break;
  case GUI_MOVIE_NOTIFICATION_POSTDRAW:
    GUI_MULTIBUF_End();
    break;
  case GUI_MOVIE_NOTIFICATION_STOP:
    break;
  }
}

/*********************************************************************
*
*       _GetData
*
* Function description
*   Reading data directly from file system
*/
int _GetData(void * p, const U8 ** ppData, unsigned NumBytes, U32 Off) {
  U32 NumBytesRead;
#ifdef WIN32
  HANDLE hFile;

  hFile = *(HANDLE *)p;
  SetFilePointer(hFile, Off, 0, FILE_BEGIN);
  ReadFile(hFile, (U8 *)*ppData, NumBytes, &NumBytesRead, NULL);
#else
  FS_FILE * pFile;

  pFile = (FS_FILE *)p;
  FS_SetFilePos(pFile, Off, FS_FILE_BEGIN);
  NumBytesRead = FS_Read(pFile, (U8 *)*ppData, NumBytes);
#endif
  return NumBytesRead;
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       _DrawSmilie
*/
static void _DrawSmilie(int xPos, int yPos, int r) {
  int d;

  GUI_SetColor(GUI_YELLOW);
  GUI_FillCircle(xPos, yPos, r);
  GUI_SetColor(GUI_BLACK);
  GUI_SetPenSize(1);
  GUI_DrawCircle(xPos, yPos, r);
  d = (r * 2) / 5;
  GUI_FillCircle(xPos - d, yPos - d, r / 10);
  GUI_FillCircle(xPos + d, yPos - d, r / 10);
  GUI_DrawVLine(xPos, yPos - d / 2, yPos + d / 2);
  GUI_DrawArc(xPos, yPos + r + d, r, r, 70, 110);
}

/*********************************************************************
*
*       MainTask
*/
void MainTask(void) {
  GUI_MOVIE_INFO   Info;
  GUI_MOVIE_HANDLE hMovie;
  int              xSize, ySize;
  GUI_RECT         Rect;
  int              FontDistY;
#ifdef WIN32
  HANDLE           hFile;
  const char       acFileName[] = "C:\\MOVIE_ShowFromFS.emf";
  #define PARAM    &hFile
#else
  FS_FILE        * pFile;
  const char       acFileName[] = "\\MOVIE_ShowFromFS.emf";
  #define PARAM    pFile
#endif

  GUI_Init();
  //
  // Get display size
  //
  xSize = LCD_GetXSize();
  ySize = LCD_GetYSize();
  //
  // Create file handle
  //
#ifdef WIN32
  hFile = CreateFile(acFileName, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
#else
  FS_Init();
  pFile = FS_FOpen(acFileName, "r");
#endif
  //
  // Get physical size of movie
  //
  if (GUI_MOVIE_GetInfoEx(_GetData, PARAM, &Info) == 0) {
    //
    // Check if display size fits
    //
    if ((Info.xSize <= xSize) && (Info.ySize <= ySize)) {
      //
      // Create and play movie
      //
      hMovie = GUI_MOVIE_CreateEx(_GetData, PARAM, _cbNotify);
      if (hMovie) {
        GUI_MOVIE_Show(hMovie, (xSize - Info.xSize) / 2, (ySize - Info.ySize) / 2, 1);
      }
    } else {
      //
      // Error message
      //
      GUI_SetFont(GUI_FONT_13_ASCII);
      GUI_DispStringHCenterAt("Video can not be shown.\n\nDisplay size too small.", xSize / 2, (ySize - GUI_GetFontSizeY()) / 2);
    }
  } else {
    GUI_SetBkColor(GUI_RED);
    GUI_Clear();
    GUI_SetFont(GUI_FONT_16_ASCII);
    FontDistY = GUI_GetFontDistY();
    Rect.x0 = 0;
    Rect.y0 = ySize / 2;
    Rect.x1 = xSize - 1;
    Rect.y1 = Rect.y0 + FontDistY * 3 - 1;
    GUI_DispStringInRect("Error opening the movie file.\n"
                         "Please make sure that the given\n"
                         "movie file exists on the specified path:",
                         &Rect, GUI_TA_HCENTER | GUI_TA_TOP);
    Rect.y0 = Rect.y1 + 1;
    Rect.y1 = Rect.y0 + FontDistY;
    GUI_DispStringInRect(acFileName, &Rect, GUI_TA_HCENTER | GUI_TA_TOP);
    Rect.y0 = Rect.y1 + 1;
    Rect.y1 = ySize - 1;
    GUI_DispStringInRect("A sample video can be downloaded\non www.segger.com.", &Rect, GUI_TA_HCENTER | GUI_TA_VCENTER);
    _DrawSmilie(xSize / 2, ySize / 4, ySize / 8);
  }
  while (1) {
    GUI_Exec();
    GUI_X_Delay(1);
  }
}

#endif

/*************************** End of file ****************************/
