
#include <peripheral/uart.h>
#include <stdint.h>

#include "uart_funcs.h"

// Initialize the UART command interpreter
void init_uart_cmd_interp(UART_MODULE uart, UINT32 system_clk_freq, UINT32 baud)
{
   //UARTConfigure(uart, UART_ENABLE_PINS_CTS_RTS);
   UARTConfigure(uart, UART_ENABLE_PINS_TX_RX_ONLY);
   UARTSetLineControl(uart, UART_DATA_SIZE_8_BITS | UART_PARITY_NONE |
         UART_STOP_BITS_1);
   UARTSetDataRate(uart, system_clk_freq, baud);
   UARTEnable(uart, UART_ENABLE_FLAGS(UART_PERIPHERAL | UART_TX | UART_RX));
}

// Reads a single character from the UART.
UINT8 read_uart_byte(UART_MODULE uart)
{
   while(!UARTReceivedDataIsAvailable(uart));
   return UARTGetDataByte(uart);
}

// Read a line from the UART, blocks until data is available.
// Appends a null terminator.
// TODO: Should this continue reading until a newline is found?
int read_uart_line(UART_MODULE uart, char *output, UINT32 max_size, UINT8 echo)
{
   UINT32 num_read = 0;
   UINT8 byte = 0;

   while (num_read < max_size-1)
   {
      byte = read_uart_byte(uart);

      // Echo the received byte
      if (echo > 0)
      {
         send_uart_byte(uart, byte);
      }

      // End of the line
      if (byte == '\r')
      {
         break;
      }

      // Copy to our buffer
      output[num_read] = byte;
      num_read++;
   }

   if (echo > 0)
   {
      send_uart_byte(uart, '\r');
      send_uart_byte(uart, '\n');
   }

   // Append null terminator.
   output[num_read] = '\0';

   return num_read;
}

// Sends a single byte over the UART.
void send_uart_byte(UART_MODULE uart, UINT8 byte)
{
   while(!UARTTransmitterIsReady(uart));
   UARTSendDataByte(uart, byte);
}

// Send a line over the UART
void send_uart_line(UART_MODULE uart, char *string, UINT32 size)
{
   // Send the message.
   while (size != 0)
   {
      send_uart_byte(uart, *string);
      string++;
      size--;
   }

   // Send the newline.
   while(!UARTTransmitterIsReady(uart));
   UARTSendDataByte(uart, '\r');
   while(!UARTTransmitterIsReady(uart));
   UARTSendDataByte(uart, '\n');

   // Wait for all this business to finish.
   while(!UARTTransmissionHasCompleted(uart));
}
