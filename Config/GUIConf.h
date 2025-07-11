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
Purpose     : Configures emWins abilities, fonts etc.
----------------------------------------------------------------------
*/

#ifndef GUICONF_H
#define GUICONF_H

/*********************************************************************
*
*       Multi layer/display support
*/
#define GUI_NUM_LAYERS       1    // Maximum number of available layers

/*********************************************************************
*
*       Multi tasking support
*/
#define GUI_OS               (0)  // Compile with multitasking support

/*********************************************************************
*
*       Configuration of touch support
*/
#define GUI_SUPPORT_TOUCH    (0)  // Support a touch screen (req. win-manager)

/*********************************************************************
*
*       Default font
*/
#define GUI_DEFAULT_FONT     &GUI_Font6x8

/*********************************************************************
*
*         Configuration of available packages
*/
#define GUI_SUPPORT_MOUSE    1    // Support a mouse
#define GUI_WINSUPPORT       1    // Use window manager
#define GUI_SUPPORT_MEMDEV   1    // Memory device package available
#define GUI_SUPPORT_DEVICES  1    // Enable use of device pointers

#endif  // Avoid multiple inclusion
