
#include <peripheral/uart.h>
#include <stdint.h>

#include "pmod_cls_interface.h"

void init_pmod_cls(UART_MODULE uart, UINT32 system_clk_freq, UINT32 baud)
{
   UARTConfigure(uart, UART_ENABLE_PINS_TX_RX_ONLY);
   UARTSetLineControl(uart, UART_DATA_SIZE_8_BITS | UART_PARITY_NONE |
         UART_STOP_BITS_1);
   UARTSetDataRate(uart, system_clk_freq, baud);
   UARTEnable(uart, UART_ENABLE_FLAGS(UART_PERIPHERAL | UART_TX | UART_RX));
}

// send string
void lcd_send_string(UART_MODULE lcd, const char* string, UINT32 size)
{
   while (size != 0)
   {
      while(!UARTTransmitterIsReady(lcd));
      UARTSendDataByte(lcd, *string);
      string++;
      size--;
   }
   while(!UARTTransmissionHasCompleted(lcd));
}

// Set's text to wrap at 16 characters
void wrap_at_16(UART_MODULE lcd)
{
   char buf[4] = {ESC_CHAR, LEFT_BRACK, '0', 'h'};
   lcd_send_string(lcd, buf, 4);
}

// clear and home
void clear_and_home(UART_MODULE lcd)
{
   char buf[3] = {ESC_CHAR, LEFT_BRACK, 'j'};
   lcd_send_string(lcd, buf, 3);
}

// reset
void pmod_cls_reset(UART_MODULE lcd)
{
   char buf[3] = {ESC_CHAR, LEFT_BRACK, '*'};
   lcd_send_string(lcd, buf, 3);
}

void set_display(UART_MODULE lcd, enum display_mode mode)
{
   char buf[4] = {ESC_CHAR, LEFT_BRACK, mode, 'e'};
   lcd_send_string(lcd, buf, 4);
}
