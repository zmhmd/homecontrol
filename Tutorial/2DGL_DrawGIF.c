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
File        : 2DGL_DrawGIF.c
Purpose     : Shows how to render GIF images
Requirements: WindowManager - ( )
              MemoryDevices - ( )
              AntiAliasing  - ( )
              VNC-Server    - ( )
              PNG-Library   - ( )
              TrueTypeFonts - ( )
----------------------------------------------------------------------
*/

#include "GUI.h"

/*******************************************************************
*
*       Static data
*
********************************************************************
*/
static unsigned const char _acImage0[] = {
  0x47, 0x49, 0x46, 0x38, 0x39, 0x61, 0x30, 0x00, 0x30, 0x00, 0xF3, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFB, 0xF3, 0x05, 0xFF, 0x64, 0x03, 0xDD, 0x09, 0x07, 0xF2, 0x08, 0x84, 0x47, 0x00, 0xA5, 0x00, 0x00, 0xD3, 0x02, 0xAB, 0xEA, 0x1F, 0xB7, 0x14,
  0x00, 0x64, 0x12, 0x56, 0x2C, 0x05, 0x90, 0x71, 0x3A, 0xBF, 0xBF, 0xBF, 0x80, 0x80, 0x80, 0x40, 0x40, 0x40, 0x00, 0x00, 0x00, 0x21, 0xFF, 0x0B, 0x4E, 0x45, 0x54, 0x53, 0x43, 0x41, 0x50, 0x45, 0x32, 0x2E, 0x30, 0x03, 0x01, 0x00, 0x00, 0x00,
  0x21, 0xFE, 0x30, 0x54, 0x68, 0x69, 0x73, 0x20, 0x47, 0x49, 0x46, 0x20, 0x46, 0x6F, 0x75, 0x6E, 0x64, 0x20, 0x40, 0x0D, 0x0A,
  0x68, 0x74, 0x74, 0x70, 0x3A, 0x2F, 0x2F, 0x61, 0x6E, 0x69, 0x6D, 0x61, 0x74, 0x69, 0x6F, 0x6E, 0x2D, 0x73, 0x74, 0x61, 0x74, 0x69, 0x6F, 0x6E, 0x2E, 0x63, 0x6F, 0x6D, 0x0D, 0x0A,
  0x00, 0x21, 0xF9, 0x04, 0x09, 0x0A,
  0x00, 0x0C, 0x00, 0x2C, 0x00, 0x00, 0x00, 0x00, 0x30, 0x00, 0x30, 0x00, 0x03, 0x04, 0xDB, 0x90, 0xC9, 0x49, 0xAB, 0xBD, 0x38, 0xEB, 0x2D, 0x9F, 0xFF, 0x0F, 0x27, 0x72, 0x60, 0x60, 0x9E, 0x01, 0x38, 0xAE, 0xDD, 0x83, 0xBE, 0xB0, 0xC7, 0x6E,
  0x1E, 0x6C, 0xC7, 0xE1, 0x5C, 0xD5, 0x77, 0xFF, 0xCA, 0x3A, 0x86, 0xCB, 0x47, 0xFC, 0xE9, 0x86, 0xC5, 0xA4, 0x29, 0x37, 0x42, 0x2A, 0x95, 0x4C, 0xDA, 0x73, 0xBA, 0x24, 0xDD, 0x3E, 0x45, 0xEC, 0x55, 0x8A, 0xD3, 0x5E, 0x4B, 0xB6, 0xA8, 0xC5,
  0x79, 0x02, 0x79, 0x7F, 0x66, 0x72, 0x39, 0xA3, 0x4E, 0xA5, 0xBF, 0xE6, 0xED, 0xA5, 0xED, 0x8E, 0x87, 0xDF, 0xF2, 0x5D, 0xCF, 0x0E, 0xE7, 0xE5, 0x27, 0x74, 0x65, 0x7E, 0x7B, 0x83, 0x7F, 0x42, 0x54, 0x88, 0x31, 0x7A, 0x89, 0x8C, 0x29, 0x8B,
  0x8D, 0x88, 0x51, 0x81, 0x90, 0x59, 0x80, 0x94, 0x8C, 0x4C, 0x93, 0x97, 0x7B, 0x96, 0x9B, 0x54, 0x99, 0x9E, 0x9F, 0x9D, 0xA1, 0x50, 0xA3, 0x77, 0x9A, 0x38, 0x44, 0x92, 0x5F, 0x91, 0x67, 0x46, 0x14, 0x6D, 0xAE, 0x4F, 0xB2, 0x55, 0xB0, 0x61,
  0x29, 0x98, 0xB8, 0x61, 0x8F, 0xA9, 0x9F, 0xAA, 0x63, 0x84, 0xB3, 0x9A, 0x62, 0x1D, 0x3E, 0x85, 0x77, 0x49, 0xC4, 0xC5, 0x84, 0x74, 0xC7, 0x86, 0xB6, 0xAA, 0x69, 0xCE, 0x9C, 0x6C, 0xA4, 0xBB, 0x1A, 0xA8, 0x94, 0xCA, 0xBC, 0xD6, 0xDB, 0xC0,
  0xD6, 0x8E, 0x2B, 0xD9, 0x53, 0xDE, 0xD5, 0x9E, 0xE5, 0xD8, 0xE3, 0x84, 0x41, 0xB0, 0xEA, 0x28, 0x40, 0xEC, 0x3B, 0xEA, 0xF0, 0xF1, 0x73, 0xB4, 0x2A, 0xF5, 0x4D, 0x69, 0xF9, 0xFC, 0xFD, 0xFE, 0x41, 0x11, 0x00, 0x21, 0xF9, 0x04, 0x09, 0x0A,
  0x00, 0x0C, 0x00, 0x2C, 0x00, 0x00, 0x00, 0x00, 0x30, 0x00, 0x30, 0x00, 0x03, 0x04, 0xDB, 0x90, 0xC9, 0x49, 0xAB, 0xBD, 0x38, 0xEB, 0x2D, 0x9F, 0xFF, 0x0F, 0x27, 0x72, 0x60, 0x60, 0x9E, 0x01, 0x38, 0xAE, 0xDD, 0x83, 0xBE, 0xB0, 0xC7, 0x6E,
  0x1E, 0x6C, 0xC7, 0xE1, 0x5C, 0xD5, 0x77, 0xFF, 0xCA, 0x3A, 0x86, 0xCB, 0x47, 0xFC, 0xE9, 0x86, 0xC5, 0xA4, 0x29, 0x37, 0x42, 0x2A, 0x95, 0x4C, 0xDA, 0x73, 0xBA, 0x24, 0xDD, 0x3E, 0x45, 0xEC, 0x55, 0x8A, 0xD3, 0x5E, 0x4B, 0xB6, 0xA8, 0xC5,
  0x79, 0x02, 0x79, 0x7F, 0x66, 0x72, 0x39, 0xA3, 0x4E, 0xA5, 0xBF, 0xE6, 0xED, 0xA5, 0xED, 0x8E, 0x87, 0xDF, 0xF2, 0x5D, 0xCF, 0x0E, 0xE7, 0xE5, 0x27, 0x74, 0x65, 0x7E, 0x7B, 0x83, 0x7F, 0x42, 0x54, 0x88, 0x31, 0x7A, 0x89, 0x8C, 0x29, 0x8B,
  0x8D, 0x88, 0x51, 0x81, 0x90, 0x59, 0x80, 0x94, 0x8C, 0x4C, 0x93, 0x97, 0x7B, 0x96, 0x9B, 0x54, 0x99, 0x9E, 0x9F, 0x9D, 0xA1, 0x50, 0xA3, 0x77, 0x9A, 0x38, 0x44, 0x92, 0x5F, 0x91, 0x67, 0x46, 0x14, 0x6D, 0xAE, 0x4F, 0xB2, 0x55, 0xB0, 0x61,
  0x29, 0x98, 0xB8, 0x61, 0x8F, 0xA9, 0x9F, 0xAA, 0x63, 0x70, 0x50, 0x85, 0x8A, 0xC0, 0xA7, 0xC3, 0x78, 0x95, 0x73, 0xC6, 0x69, 0x60, 0xA5, 0x18, 0x74, 0xCC, 0xB4, 0xC9, 0xCF, 0x7D, 0xC3, 0xB3, 0x5C, 0xA4, 0xAF, 0x1A, 0xA8, 0x8D, 0x62, 0x6C,
  0xD9, 0xB5, 0x4D, 0xD9, 0xDE, 0x56, 0x9E, 0xE4, 0x22, 0xD6, 0x53, 0x40, 0x41, 0x2D, 0x89, 0xEB, 0xEC, 0xB0, 0xDC, 0x4B, 0xE7, 0xF0, 0x2D, 0xD6, 0x2A, 0xF5, 0x4D, 0x69, 0xF9, 0xFC, 0xFD, 0xFE, 0x41, 0x11, 0x00, 0x21, 0xF9, 0x04, 0x09, 0x0A,
  0x00, 0x0C, 0x00, 0x2C, 0x00, 0x00, 0x00, 0x00, 0x30, 0x00, 0x30, 0x00, 0x03, 0x04, 0xDB, 0x90, 0xC9, 0x49, 0xAB, 0xBD, 0x38, 0xEB, 0x2D, 0x9F, 0xFF, 0x0F, 0x27, 0x72, 0x60, 0x60, 0x9E, 0x01, 0x38, 0xAE, 0xDD, 0x83, 0xBE, 0xB0, 0xC7, 0x6E,
  0x1E, 0x6C, 0xC7, 0xE1, 0x5C, 0xD5, 0x77, 0xFF, 0xCA, 0x3A, 0x86, 0xCB, 0x47, 0xFC, 0xE9, 0x86, 0xC5, 0xA4, 0x29, 0x37, 0x42, 0x2A, 0x95, 0x4C, 0xDA, 0x73, 0xBA, 0x24, 0xDD, 0x3E, 0x45, 0xEC, 0x55, 0x8A, 0xD3, 0x5E, 0x4B, 0xB6, 0xA8, 0xC5,
  0x79, 0x02, 0x79, 0x7F, 0x66, 0x72, 0x39, 0xA3, 0x4E, 0xA5, 0xBF, 0xE6, 0xED, 0xA5, 0xED, 0x8E, 0x87, 0xDF, 0xF2, 0x5D, 0xCF, 0x0E, 0xE7, 0xE5, 0x27, 0x74, 0x65, 0x7E, 0x7B, 0x83, 0x7F, 0x42, 0x54, 0x88, 0x31, 0x7A, 0x89, 0x8C, 0x29, 0x8B,
  0x8D, 0x88, 0x51, 0x81, 0x90, 0x59, 0x80, 0x94, 0x8C, 0x4C, 0x93, 0x97, 0x7B, 0x96, 0x9B, 0x54, 0x99, 0x9E, 0x9F, 0x9D, 0xA1, 0x50, 0xA3, 0x77, 0x9A, 0x38, 0x44, 0x92, 0x5F, 0x91, 0x67, 0x46, 0x14, 0x6D, 0xAE, 0x4F, 0xB2, 0x55, 0xB0, 0x61,
  0x29, 0x98, 0xB8, 0x61, 0x8F, 0xA9, 0x9F, 0xAA, 0x63, 0x84, 0xB3, 0x9A, 0x62, 0x1D, 0x3E, 0x85, 0x77, 0x49, 0xC4, 0xC5, 0x84, 0x74, 0xC7, 0x86, 0xB6, 0xAA, 0x69, 0xCE, 0x9C, 0x6C, 0xA4, 0xBB, 0x1A, 0xA8, 0x94, 0xCA, 0xBC, 0xD6, 0xDB, 0xC0,
  0xD6, 0x8E, 0x2B, 0xD9, 0x53, 0xDE, 0xD5, 0x9E, 0xE5, 0xD8, 0xE3, 0x84, 0x41, 0xB0, 0xEA, 0x28, 0x40, 0xEC, 0x3B, 0xEA, 0xF0, 0xF1, 0x73, 0xB4, 0x2A, 0xF5, 0x4D, 0x69, 0xF9, 0xFC, 0xFD, 0xFE, 0x41, 0x11, 0x00, 0x21, 0xF9, 0x04, 0x09, 0x0A,
  0x00, 0x0C, 0x00, 0x2C, 0x00, 0x00, 0x00, 0x00, 0x30, 0x00, 0x30, 0x00, 0x03, 0x04, 0xDB, 0x90, 0xC9, 0x49, 0xAB, 0xBD, 0x38, 0xEB, 0x2D, 0x9F, 0xFF, 0x0F, 0x27, 0x72, 0x60, 0x60, 0x9E, 0x01, 0x38, 0xAE, 0xDD, 0x83, 0xBE, 0xB0, 0xC7, 0x6E,
  0x1E, 0x6C, 0xC7, 0xE1, 0x5C, 0xD5, 0x77, 0xFF, 0xCA, 0x3A, 0x86, 0xCB, 0x47, 0xFC, 0xE9, 0x86, 0xC5, 0xA4, 0x29, 0x37, 0x42, 0x2A, 0x95, 0x4C, 0xDA, 0x73, 0xBA, 0x24, 0xDD, 0x3E, 0x45, 0xEC, 0x55, 0x8A, 0xD3, 0x5E, 0x4B, 0xB6, 0xA8, 0xC5,
  0x79, 0x02, 0x79, 0x7F, 0x66, 0x72, 0x39, 0xA3, 0x4E, 0xA5, 0xBF, 0xE6, 0xED, 0xA5, 0xED, 0x8E, 0x87, 0xDF, 0xF2, 0x5D, 0xCF, 0x0E, 0xE7, 0xE5, 0x27, 0x74, 0x65, 0x7E, 0x7B, 0x83, 0x7F, 0x42, 0x54, 0x88, 0x31, 0x7A, 0x89, 0x8C, 0x29, 0x8B,
  0x8D, 0x88, 0x51, 0x81, 0x90, 0x59, 0x80, 0x94, 0x8C, 0x4C, 0x93, 0x97, 0x7B, 0x96, 0x9B, 0x54, 0x99, 0x9E, 0x9F, 0x9D, 0xA1, 0x50, 0xA3, 0x5F, 0x9F, 0x85, 0x46, 0xA6, 0x68, 0xA9, 0x59, 0x74, 0x62, 0xAF, 0xAD, 0x49, 0x67, 0x6B, 0x14, 0xAF,
  0x29, 0x98, 0xB8, 0x8A, 0xB6, 0x44, 0x9A, 0x77, 0x9C, 0x8F, 0x70, 0x50, 0xB2, 0x62, 0x1D, 0xC3, 0x60, 0x28, 0x7C, 0xAA, 0x63, 0x53, 0x69, 0xCE, 0x6D, 0xC5, 0xAB, 0xC7, 0xCE, 0x86, 0xC1, 0xA8, 0xC8, 0xCB, 0x18, 0xBE, 0x94, 0xD1, 0xD6, 0xA4,
  0xDD, 0xCC, 0xA4, 0x55, 0x2B, 0xDB, 0xCD, 0x47, 0xA1, 0xE0, 0x34, 0xE5, 0x84, 0x41, 0xB6, 0xEB, 0xC9, 0xE9, 0x33, 0xB2, 0x77, 0xED, 0xEA, 0xAD, 0x2A, 0xF5, 0xE4, 0x69, 0xF9, 0xFC, 0xFD, 0xFE, 0xFC, 0x11, 0x00, 0x21, 0xF9, 0x04, 0x09, 0x0A,
  0x00, 0x0C, 0x00, 0x2C, 0x00, 0x00, 0x00, 0x00, 0x30, 0x00, 0x30, 0x00, 0x03, 0x04, 0xDB, 0x90, 0xC9, 0x49, 0xAB, 0xBD, 0x38, 0xEB, 0x2D, 0x9F, 0xFF, 0x0F, 0x27, 0x72, 0x60, 0x60, 0x9E, 0x01, 0x38, 0xAE, 0xDD, 0x83, 0xBE, 0xB0, 0xC7, 0x6E,
  0x1E, 0x6C, 0xC7, 0xE1, 0x5C, 0xD5, 0x77, 0xFF, 0xCA, 0x3A, 0x86, 0xCB, 0x47, 0xFC, 0xE9, 0x86, 0xC5, 0xA4, 0x29, 0x37, 0x42, 0x2A, 0x95, 0x4C, 0xDA, 0x73, 0xBA, 0x24, 0xDD, 0x3E, 0x45, 0xEC, 0x55, 0x8A, 0xD3, 0x5E, 0x4B, 0xB6, 0xA8, 0xC5,
  0x79, 0x02, 0x79, 0x7F, 0x66, 0x72, 0x39, 0xA3, 0x4E, 0xA5, 0xBF, 0xE6, 0xED, 0xA5, 0xED, 0x8E, 0x87, 0xDF, 0xF2, 0x5D, 0xCF, 0x0E, 0xE7, 0xE5, 0x27, 0x74, 0x65, 0x7E, 0x7B, 0x83, 0x7F, 0x42, 0x54, 0x88, 0x31, 0x7A, 0x89, 0x8C, 0x29, 0x8B,
  0x8D, 0x88, 0x51, 0x81, 0x90, 0x59, 0x80, 0x94, 0x8C, 0x4C, 0x93, 0x97, 0x7B, 0x96, 0x9B, 0x54, 0x99, 0x9E, 0x9F, 0x9D, 0xA1, 0x50, 0xA3, 0x77, 0x9A, 0x38, 0x44, 0x92, 0x5F, 0x91, 0x67, 0x46, 0x14, 0x6D, 0xAE, 0x4F, 0xB2, 0x55, 0xB0, 0x61,
  0x29, 0x98, 0xB8, 0x61, 0x8F, 0xA9, 0x9F, 0xAA, 0x63, 0x84, 0xB3, 0x9A, 0x62, 0x1D, 0x3E, 0x85, 0x77, 0x49, 0xC4, 0xC5, 0x84, 0x74, 0xC7, 0x86, 0xB6, 0xAA, 0x69, 0xCE, 0x9C, 0x6C, 0xA4, 0xBB, 0x1A, 0xA8, 0x94, 0xCA, 0xBC, 0xD6, 0xDB, 0xC0,
  0xD6, 0x8E, 0x2B, 0xD9, 0x53, 0xDE, 0xD5, 0x9E, 0xE5, 0xD8, 0xE3, 0x84, 0x41, 0xB0, 0xEA, 0x28, 0x40, 0xEC, 0x3B, 0xEA, 0xF0, 0xF1, 0x73, 0xB4, 0x2A, 0xF5, 0x4D, 0x69, 0xF9, 0xFC, 0xFD, 0xFE, 0x41, 0x11, 0x00, 0x21, 0xF9, 0x04, 0x09, 0x0A,
  0x00, 0x0C, 0x00, 0x2C, 0x00, 0x00, 0x00, 0x00, 0x30, 0x00, 0x30, 0x00, 0x03, 0x04, 0xF2, 0x90, 0xC9, 0x49, 0xAB, 0xBD, 0x38, 0xEB, 0x2D, 0x9F, 0xFF, 0x0F, 0x27, 0x72, 0x60, 0x60, 0x9E, 0x01, 0x38, 0xAE, 0xDD, 0x83, 0xBE, 0xB0, 0xC7, 0x6E,
  0x1E, 0x6C, 0xC7, 0xE1, 0x5C, 0xD5, 0x77, 0xFF, 0xCA, 0x3A, 0x86, 0xCB, 0x47, 0xFC, 0xE9, 0x86, 0xC5, 0xA4, 0x29, 0x37, 0x42, 0x2A, 0x95, 0x4C, 0x9A, 0xED, 0x93, 0xA4, 0xC6, 0x48, 0xD3, 0x92, 0x0F, 0xE4, 0x3C, 0x45, 0x2F, 0xDD, 0x14, 0x37,
  0xEC, 0x1D, 0x5F, 0x33, 0xE4, 0x31, 0x79, 0x69, 0x3E, 0x83, 0xB3, 0xDC, 0x9E, 0x7A, 0x8A, 0x59, 0xCF, 0x6F, 0x77, 0x37, 0xC5, 0xDE, 0x86, 0x6B, 0xF5, 0x1D, 0x72, 0x7D, 0x38, 0x71, 0x78, 0x3B, 0x5B, 0x7F, 0x78, 0x89, 0x80, 0x6B, 0x65, 0x8D,
  0x28, 0x56, 0x72, 0x87, 0x4F, 0x94, 0x92, 0x13, 0x8F, 0x95, 0x94, 0x4C, 0x98, 0x98, 0x78, 0x44, 0x9B, 0x9F, 0x99, 0x29, 0x9F, 0x97, 0x88, 0xA2, 0x9C, 0xA5, 0x8A, 0xA2, 0x62, 0x5B, 0xA9, 0x38, 0xAB, 0x62, 0x8D, 0xA0, 0x90, 0x85, 0xA7, 0xB5,
  0x4B, 0x7B, 0x82, 0xB6, 0x96, 0xAE, 0x59, 0x95, 0x3C, 0x86, 0xB9, 0xA6, 0x50, 0xA8, 0xC2, 0x82, 0x9D, 0xB1, 0xA4, 0x93, 0xC7, 0xB2, 0x91, 0xC1, 0xCB, 0x82, 0x03, 0x6A, 0x1E, 0xD2, 0x9D, 0x5F, 0xBD, 0x78, 0x03, 0xD5, 0x5C, 0xDA, 0xD6, 0x68,
  0x55, 0xDA, 0xE1, 0xE2, 0xC8, 0xD7, 0xC6, 0x88, 0xE2, 0xD5, 0x55, 0x52, 0xC4, 0xCE, 0xCA, 0x1A, 0xC8, 0xAB, 0xE5, 0x6F, 0xB0, 0xBC, 0x22, 0xF0, 0x4F, 0xF2, 0xEF, 0xF4, 0x46, 0x41, 0xC0, 0xF1, 0xF9, 0x4D, 0xEE, 0xD1, 0x0A,
  0x52, 0xE7, 0x1E, 0x10, 0x82, 0x68, 0xDA, 0xB1, 0x39, 0x88, 0xD0, 0xDE, 0x98, 0x86, 0x10, 0x23, 0x4A, 0x84, 0x18, 0x01, 0x00, 0x21, 0xFE, 0x38, 0x46, 0x49, 0x4C, 0x45, 0x20, 0x49, 0x44, 0x45, 0x4E, 0x54, 0x49, 0x54, 0x59, 0x0D, 0x0A,
  0x43, 0x72, 0x65, 0x61, 0x74, 0x65, 0x64, 0x20, 0x6F, 0x72, 0x20, 0x6D, 0x6F, 0x64, 0x69, 0x66, 0x69, 0x65, 0x64, 0x20, 0x62, 0x79, 0x0D, 0x0A,
  0x53, 0x79, 0x72, 0x75, 0x73, 0x73, 0x20, 0x4D, 0x63, 0x44, 0x61, 0x6E, 0x69, 0x65, 0x6C, 0x0D, 0x0A,
  0x00, 0x21, 0xFF, 0x0B, 0x47, 0x49, 0x46, 0x43, 0x4F, 0x4E, 0x6E, 0x62, 0x31, 0x2E, 0x30, 0x02, 0x06, 0x00, 0x0E, 0x01, 0x00, 0x02, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x0E, 0x01, 0x00, 0x02, 0x00,
  0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x0E, 0x01, 0x00, 0x02, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x0E, 0x01, 0x00, 0x02, 0x00, 0x0A,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x0E, 0x01, 0x00, 0x02, 0x00, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x0E, 0x01, 0x00, 0x02, 0x00, 0x0E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x3B, 0x00
};

/*******************************************************************
*
*       Static functions
*
********************************************************************
*/
/*******************************************************************
*
*       _ShowMovie
*
* Function description
*   Shows the contents of a GIF file as movie
*/
static void _ShowMovie(const char * pFile, int FileSize) {
  GUI_GIF_IMAGE_INFO ImageInfo = {0}; // Info structure of one particular GIF image of the GIF file
  GUI_GIF_INFO       GifInfo   = {0}; // Info structure of GIF file
  int                i;
  int                j;
  int                XPos;
  int                YPos;

  //
  // Display sample information
  //
  GUI_SetFont(&GUI_Font8x16);
  GUI_ClearRect(0, 40, 319, 59);
  GUI_DispStringHCenterAt("Show complete GIF file as movie", 160, 40);
  //
  // Show movie
  //
  GUI_ClearRect(0, 60, 319, 239);                                 // Clear the image area
  GUI_GIF_GetInfo(pFile, FileSize, &GifInfo);                     // Get GIF info structure
  XPos = (GifInfo.xSize > 320) ?  0 : 160 - (GifInfo.xSize / 2);
  YPos = (GifInfo.ySize > 180) ? 60 : 150 - (GifInfo.ySize / 2);
  for (i = 0; i < 2; i++) {                                       // Show the complete GIF 2 times ...
    for (j = 0; j < GifInfo.NumImages; j++) {
      GUI_GIF_DrawSub(pFile, FileSize, XPos, YPos, j);            // Draw sub image
      GUI_GIF_GetImageInfo(pFile, FileSize, &ImageInfo, j);       // Get sub image information
      GUI_Delay(ImageInfo.Delay ? ImageInfo.Delay * 10 : 100);    // Use the Delay member of the ImageInfo structure for waiting a while
    }
    GUI_Delay(2000);                                              // Wait a while
  }
}

/*******************************************************************
*
*       _ShowSubImages
*
* Function description
*   Shows all sub images of a GIF file side by side
*/
static void _ShowSubImages(const char * pFile, int FileSize) {
  GUI_GIF_INFO       GifInfo   = {0}; // Info structure of GIF file
  int                j;
  int                XPos;
  int                YPos;
  //
  // Display sample information
  //
  GUI_SetFont(&GUI_Font8x16);
  GUI_ClearRect(0, 40, 319, 59);
  GUI_DispStringHCenterAt("Show all sub images of a GIF file", 160, 40);
  //
  // Show sub images
  //
  GUI_ClearRect(0, 60, 319, 239);                                 // Clear the image area
  GUI_GIF_GetInfo(pFile, FileSize, &GifInfo);                     // Get GIF info structure
  XPos = 160 - GifInfo.xSize * GifInfo.NumImages / 2;
  YPos = (GifInfo.ySize > 180) ? 60 : 150 - (GifInfo.ySize / 2);
  for (j = 0; j < GifInfo.NumImages; j++) {
    char acNumber[3] = "#";
    acNumber[1] = '0' + j;
    GUI_DispStringHCenterAt(acNumber, XPos + GifInfo.xSize / 2, 90);
    GUI_GIF_DrawSub(pFile, FileSize, XPos, YPos, j);              // Draw sub image
    XPos += GifInfo.xSize;
  }
  GUI_Delay(4000);                                                // Wait a while
}

/*******************************************************************
*
*       _ShowComments
*
* Function description
*   Shows all comments of a GIF file
*/
static void _ShowComments(const char * pFile, int FileSize) {
  GUI_RECT Rect          = {80, 100, 239, 199};
  char     acBuffer[256] = {0};
  int      CommentCnt;

  //
  // Display sample information
  //
  GUI_SetFont(&GUI_Font8x16);
  GUI_ClearRect(0, 40, 319, 59);
  GUI_DispStringHCenterAt("Show all comments of a GIF file", 160, 40);
  //
  // Show all comments
  //
  GUI_ClearRect(0, 60, 319, 239);                                 // Clear the image area
  CommentCnt = 0;
  while (!GUI_GIF_GetComment(pFile, FileSize, (unsigned char *)acBuffer, sizeof(acBuffer), CommentCnt)) {
    char acNumber[12] = "Comment #0:";
    acNumber[9] = '0' + CommentCnt;
    GUI_DispStringHCenterAt(acNumber, 160, 80);
    GUI_SetBkColor(GUI_BLACK);
    GUI_SetColor(GUI_WHITE);
    GUI_ClearRectEx(&Rect);
    GUI_DispStringInRectWrap(acBuffer, &Rect, GUI_TA_HCENTER | GUI_TA_VCENTER, GUI_WRAPMODE_WORD);
    GUI_SetBkColor(GUI_WHITE);
    GUI_SetColor(GUI_BLACK);
    GUI_Delay(4000);                                              // Wait a while
    CommentCnt++;
  }
}

/*******************************************************************
*
*       Public code
*
********************************************************************
*/
/*******************************************************************
*
*       MainTask
*/
void MainTask(void) {
  GUI_Init();
  GUI_SetBkColor(GUI_WHITE);
  GUI_Clear();
  GUI_SetColor(GUI_BLACK);
  GUI_SetFont(&GUI_Font24_ASCII);
  GUI_DispStringHCenterAt("DrawGIF - Sample", 160, 5);
  while (1) {
    _ShowMovie    ((char *)_acImage0, sizeof(_acImage0));
    _ShowSubImages((char *)_acImage0, sizeof(_acImage0));
    _ShowComments ((char *)_acImage0, sizeof(_acImage0));
  }
}

/*************************** End of file ****************************/

