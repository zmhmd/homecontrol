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
File        : GUIConf.h
Purpose     : Configuration of available features and default values
----------------------------------------------------------------------
*/

#ifndef GUICONF_H
#define GUICONF_H

/*********************************************************************
*
*       Configuration of available packages
*/
#define GUI_SUPPORT_TOUCH   (1)  // Support a touch screen (req. win-manager)
#define GUI_SUPPORT_MOUSE   (1)  // Support a mouse
#define GUI_WINSUPPORT      (1)  // Window manager package available
#define GUI_SUPPORT_MEMDEV  (1)  // Memory devices available

/*********************************************************************
*
*       Default font
*/
#define GUI_DEFAULT_FONT          &GUI_Font6x8

/*********************************************************************
*
*       Default widget scheme
*/
#define WIDGET_USE_SCHEME_LARGE  1

#endif  /* Avoid multiple inclusion */

/*************************** End of file ****************************/
