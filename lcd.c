
#include <peripheral/uart.h>
#include <stdint.h>

#include "pmod_cls/pmod_cls_interface.h"

#include "lcd.h"

static UART_MODULE lcd_uart = 255;

// Helper functions
static void lcd_clear();

///////////////////////////////////////////////////////////////////////////////
/// Initialization
///////////////////////////////////////////////////////////////////////////////
/**
 * Initializes the LCD
 * @param uart
 * @param peripheral_clk_freq
 */
void init_lcd(UART_MODULE uart, UINT32 peripheral_clk_freq)
{
   lcd_uart = uart;

   init_pmod_cls(lcd_uart, peripheral_clk_freq, 9600);
   set_display(lcd_uart, DISP_ON_BL_ON);
   wrap_at_16(lcd_uart);
   lcd_clear();
}

///////////////////////////////////////////////////////////////////////////////
/// Local helper functions
///////////////////////////////////////////////////////////////////////////////
/**
 * Clears the LCD
 */
static void lcd_clear()
{
   clear_and_home(lcd_uart);
}

///////////////////////////////////////////////////////////////////////////////
/// Interface functions
///////////////////////////////////////////////////////////////////////////////
/**
 * Clears the LCD then sends the provided strings.
 * @param top_line
 * @param bot_line
 */
void lcd_write(const char* top_line, const char* bot_line)
{
   lcd_clear();
   lcd_send_string(lcd_uart, top_line, strlen(top_line));
   set_cursor_to_line_start(lcd_uart, PMOD_CLS_LINE_2);
   lcd_send_string(lcd_uart, bot_line, strlen(bot_line));
}

