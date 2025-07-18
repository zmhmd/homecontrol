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
File        : LCD_X_I2CBUS.c
Purpose     : Port routines
----------------------------------------------------------------------
*/

/*********************************************************************
*
*           Hardware configuration
*
**********************************************************************
  Needs to be adapted to your target hardware.
*/

/* Configuration example:

#define Chip_30600
#include <IOM16C.H>

#define LCD_SLAVE_ADR      0x3c
#define LCD_CLR_SDA_IN()   P8 &= ~(1<<0)
#define LCD_CLR_SCL()      P8 &= ~(1<<2)
#define LCD_CLR_RESET()    P8 &= ~(1<<3)
#define LCD_CLR_SA0()      P8 &= ~(1<<4)
#define LCD_SET_SDA_IN()   P8 |=  (1<<0)
#define LCD_SET_SCL()      P8 |=  (1<<2)
#define LCD_SET_RESET()    P8 |=  (1<<3)
#define LCD_SET_SA0()      P8 |=  (1<<4)
#define LCD_READ_BIT()    (P8 &   (1<<1))
#define LCD_INIT_PORTS() \
  P8D &= ~(1<<1);        \
  PUR2 = 0xff;           \

*/

/*********************************************************************
*
*           High level LCD access macros
*
**********************************************************************
  Usually, there is no need to modify these macros.
  It should be sufficient ot modify the low-level macros
  above.
*/

#ifndef NOP
  #define NOP()
#endif

#define LCD_START() \
  LCD_CLR_SDA_IN(); \
  NOP();            \
  LCD_CLR_SCL();    \
  NOP()

#define LCD_STOP()  \
  LCD_SET_SCL();    \
  NOP();            \
  LCD_SET_SDA_IN(); \
  NOP()

#define LCD_SLAVE_WRITE ((LCD_SLAVE_ADR << 1) + 0)
#define LCD_SLAVE_READ  ((LCD_SLAVE_ADR << 1) + 1)
#define LCD_CNTRLBT_WRITE_CMD  0x00
#define LCD_CNTRLBT_WRITE_DATA 0x40

/*********************************************************************
*
*           Initialisation
*
**********************************************************************
  This routine should be called from your application program
  to set port pins to their initial values
*/

void LCD_X_Init(void) {
  LCD_INIT_PORTS();
  if (LCD_SLAVE_ADR & 1)
    LCD_SET_SA0();
  else
    LCD_CLR_SA0();
  LCD_CLR_RESET();
  LCD_SET_SDA_IN();
  LCD_SET_SCL();
  LCD_SET_RESET();
}

/*********************************************************************
*
*           Access routines
*
**********************************************************************
  Usually, there is no need to modify these routines.
  It should be sufficient ot modify the low-level macros
  above.
*/

/* Read 1 byte, MSB first */
static unsigned char _Read1(void) {
  unsigned char c = 0;
  LCD_SET_SCL(); if (LCD_READ_BIT()) c |= (1<<7); else c &= ~(1<<7); LCD_CLR_SCL();
  LCD_SET_SCL(); if (LCD_READ_BIT()) c |= (1<<6); else c &= ~(1<<6); LCD_CLR_SCL();
  LCD_SET_SCL(); if (LCD_READ_BIT()) c |= (1<<5); else c &= ~(1<<5); LCD_CLR_SCL();
  LCD_SET_SCL(); if (LCD_READ_BIT()) c |= (1<<4); else c &= ~(1<<4); LCD_CLR_SCL();
  LCD_SET_SCL(); if (LCD_READ_BIT()) c |= (1<<3); else c &= ~(1<<3); LCD_CLR_SCL();
  LCD_SET_SCL(); if (LCD_READ_BIT()) c |= (1<<2); else c &= ~(1<<2); LCD_CLR_SCL();
  LCD_SET_SCL(); if (LCD_READ_BIT()) c |= (1<<1); else c &= ~(1<<1); LCD_CLR_SCL();
  LCD_SET_SCL(); if (LCD_READ_BIT()) c |= (1<<0); else c &= ~(1<<0); LCD_CLR_SCL();
  LCD_SET_SDA_IN(); LCD_SET_SCL(); LCD_CLR_SCL();
  return c;
}

/* Write 1 byte, MSB first */
static void _Send1(unsigned char Data) {
  if (Data&(1<<7)) { LCD_SET_SDA_IN(); } else { LCD_CLR_SDA_IN(); } NOP(); LCD_SET_SCL(); NOP(); LCD_CLR_SCL(); NOP(); 
  if (Data&(1<<6)) { LCD_SET_SDA_IN(); } else { LCD_CLR_SDA_IN(); } NOP(); LCD_SET_SCL(); NOP(); LCD_CLR_SCL(); NOP(); 
  if (Data&(1<<5)) { LCD_SET_SDA_IN(); } else { LCD_CLR_SDA_IN(); } NOP(); LCD_SET_SCL(); NOP(); LCD_CLR_SCL(); NOP(); 
  if (Data&(1<<4)) { LCD_SET_SDA_IN(); } else { LCD_CLR_SDA_IN(); } NOP(); LCD_SET_SCL(); NOP(); LCD_CLR_SCL(); NOP(); 
  if (Data&(1<<3)) { LCD_SET_SDA_IN(); } else { LCD_CLR_SDA_IN(); } NOP(); LCD_SET_SCL(); NOP(); LCD_CLR_SCL(); NOP(); 
  if (Data&(1<<2)) { LCD_SET_SDA_IN(); } else { LCD_CLR_SDA_IN(); } NOP(); LCD_SET_SCL(); NOP(); LCD_CLR_SCL(); NOP(); 
  if (Data&(1<<1)) { LCD_SET_SDA_IN(); } else { LCD_CLR_SDA_IN(); } NOP(); LCD_SET_SCL(); NOP(); LCD_CLR_SCL(); NOP(); 
  if (Data&(1<<0)) { LCD_SET_SDA_IN(); } else { LCD_CLR_SDA_IN(); } NOP(); LCD_SET_SCL(); NOP(); LCD_CLR_SCL(); NOP(); 
  LCD_SET_SCL(); NOP(); LCD_CLR_SCL();
}

/* Read status */
unsigned char LCD_X_Read00(void) {
  unsigned char c;
  LCD_STOP();
  LCD_START();
  _Send1(LCD_SLAVE_READ);
  c = _Read1();
  LCD_STOP();
  LCD_START();
  _Send1(LCD_SLAVE_WRITE);
  _Send1(LCD_CNTRLBT_WRITE_DATA);
  return c;
}

/* Write data */
void LCD_X_Write01(unsigned char c) {
  _Send1(c);
}

/* Write multiple data bytes*/
void LCD_X_WriteM01(unsigned char * pData, int NumBytes) {
  for (; NumBytes; NumBytes--) {
    _Send1(*pData++);
  }
}

/* Write command */
void LCD_X_Write00(unsigned char c) {
  LCD_STOP();
  LCD_START();
  _Send1(LCD_SLAVE_WRITE);
  _Send1(LCD_CNTRLBT_WRITE_CMD);
  _Send1(c);
  LCD_STOP();
  LCD_START();
  _Send1(LCD_SLAVE_WRITE);
  _Send1(LCD_CNTRLBT_WRITE_DATA);
}

