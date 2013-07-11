/**
 * Module for communicating robot messages.
 */
#include <stdint.h>
#include <assert.h>
#include "uart_funcs.h"

#include <peripheral/uart.h>

// The UART used for communication
static UART_MODULE Uart = 0xFF;

// Default BAUD rate of the Bluetooth PMOD.
#define BAUD_RATE 115200

// Largest message string we accept
#define MAX_LINE_LEN 16

typedef struct
{
   UINT8 version;
   UINT8 left_speed;
   UINT8 right_speed;
   char top_msg[MAX_LINE_LEN+1];
   char bot_msg[MAX_LINE_LEN+1];
} robot_msg_t;

#define MESSAGE_SIZE sizeof(robot_msg_t)

/**
 * Initializes the locally stored UART and sets it up.
 */
void init_robot_messaging(UART_MODULE uart, UINT32 peripheral_clock)
{
   Uart = uart;

   // Intialize a connection with the Bluetooth PMOD.
   init_uart_cmd_interp(uart, peripheral_clock, BAUD_RATE);
}

/**
 * Reads exactly one message from the UART.
 * Requires that Uart is initialized.
 */
void read_robot_message()
{
   char buffer[MESSAGE_SIZE];
   uint8_t cur_byte;

   assert(Uart != 0xFF);

   // Overlay onto the buffer.
   robot_msg_t *message = &(buffer[0]);

   // Read in MESSAGE_SIZE bytes and handle the message.
   for(cur_byte = 0; cur_byte < MESSAGE_SIZE; cur_byte++)
   {
      buffer[cur_byte] = read_uart_byte(Uart);
   }
}
