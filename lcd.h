/* 
 * File:   lcd.h
 * Author: sgreenman
 *
 * Created on April 14, 2013, 12:45 PM
 */

#ifndef LCD_H
#define	LCD_H

#include <peripheral/uart.h>
#include <stdint.h>

#ifdef	__cplusplus
extern "C" {
#endif

   void init_lcd(UART_MODULE uart, UINT32 peripheral_clk_freq);
   void lcd_write(const char* top_line, const char* bot_line);

#ifdef	__cplusplus
}
#endif

#endif	/* LCD_H */

