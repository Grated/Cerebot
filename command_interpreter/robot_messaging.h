/*
 * File:   robot_messaging.h
 * Author: sgreenman
 *
 * Created on October 20, 2012, 5:51 PM
 */

#ifndef ROBOT_MESSAGING_H
#define	ROBOT_MESSAGING_H

#include <peripheral/uart.h>
#include <stdint.h>

#ifdef	__cplusplus
extern "C" {
#endif

   // Initialize the UART command interpreter
   void init_robot_messaging(UART_MODULE uart, UINT32 peripheral_clk_freq);

   // Read one message from the UART.
   void read_robot_message(UART_MODULE uart);


#ifdef	__cplusplus
}
#endif

#endif	/* ROBOT_MESSAGING_H */


