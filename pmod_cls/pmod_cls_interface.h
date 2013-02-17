/* 
 * File:   pmod_cls_interface.h
 * Author: sgreenman
 *
 * Created on October 20, 2012, 1:31 PM
 */

#ifndef UART_MODULE_INTERFACE_H
#define	UART_MODULE_INTERFACE_H

#include <peripheral/uart.h>
#include <stdint.h>

#ifdef	__cplusplus
extern "C" {
#endif

   enum display_mode
   {
      DISP_OFF_BL_OFF = '0',
      DISP_ON_BL_OFF = '1',
      DISP_OFF_BL_ON = '2',
      DISP_ON_BL_ON = '3'
   };

   #define ESC_CHAR (0x1B)
   #define LEFT_BRACK ('[')

   // Initialize pmod
   void init_pmod_cls(UART_MODULE uart, UINT32 system_clk_freq, UINT32 baud);

   // send string
   void lcd_send_string(UART_MODULE lcd, const char* string, UINT32 size);

   // clear and home
   void clear_and_home(UART_MODULE lcd);

   // reset
   void pmod_cls_reset(UART_MODULE lcd);

   // enable display
   void set_display(UART_MODULE lcd, enum display_mode mode);

   // There's more we can do but I probably won't use them.
   // See the PMOD CLS reference manual.

#ifdef	__cplusplus
}
#endif

#endif	/* UART_MODULE_INTERFACE_H */

