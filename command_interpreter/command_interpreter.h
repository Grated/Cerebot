/* 
 * File:   command_interpreter.h
 * Author: sgreenman
 *
 * Created on October 20, 2012, 5:51 PM
 */

#ifndef COMMAND_INTERPRETER_H
#define	COMMAND_INTERPRETER_H

#include <peripheral/uart.h>
#include <stdint.h>

#ifdef	__cplusplus
extern "C" {
#endif

   // Initialize the UART command interpreter
   void init_uart_cmd_interp(UART_MODULE uart, UINT32 system_clk_freq,
         UINT32 baud);

   // Reads a single byte from the UART.
   UINT8 read_uart_byte(UART_MODULE uart);
   // Read a line from the UART
   int read_uart_line(UART_MODULE uart, char *output, UINT32 max_size);

   // Send a single byte over the UART.
   void send_uart_byte(UART_MODULE uart, char byte);
   // Send a line over the UART
   void send_uart_line(UART_MODULE uart, char *string, UINT32 size);


#ifdef	__cplusplus
}
#endif

#endif	/* COMMAND_INTERPRETER_H */

