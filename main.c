/* 
 * File:   main.c
 * Author: sgreenman
 *
 * Created on October 20, 2012, 1:45 PM
 */

#include <stdio.h>
#include <stdlib.h>

#include "pmod_cls/pmod_cls_interface.h"

#ifndef OVERRIDE_CONFIG_BITS

#pragma config FPLLODIV = DIV_1, FPLLMUL = MUL_20, FPLLIDIV = DIV_2, FWDTEN = OFF, FCKSM = CSECME, FPBDIV = DIV_1
#pragma config OSCIOFNC = ON, POSCMOD = XT, FSOSCEN = ON, FNOSC = PRIPLL
#pragma config CP = OFF, BWP = OFF, PWP = OFF

#endif

#define GetSystemClock() (80000000L)
#define GetPeripheralClock() (GetSystemClock()/(1<<OSCCONbits.PBDIV))
#define GetInstructionClock() (GetSystemClock()/(1<<OSCCONbits.PBDIV))

/*
 * 
 */
int main(int argc, char** argv)
{
   UINT32 i = 0;
   UINT32 received = 0;
   char receive_buffer[256] = {0};
   char *buf = "Hello world.....Blargh!";
   char *buf2 = "Might be working???";
   char *buf3 = "Stuff received!!!";

   UART_MODULE lcd = UART1;
   UART_MODULE serial = UART2;

   init_pmod_cls(lcd, GetPeripheralClock(), 9600);
   set_display(lcd, DISP_ON_BL_ON);
   wrap_at_16(lcd);
   clear_and_home(lcd);
   lcd_send_string(lcd, buf, strlen(buf));

   init_uart_cmd_interp(serial, GetPeripheralClock(), 9600);
   send_uart_line(serial, buf, strlen(buf));

   while(1)
   {
      received = read_uart_line(serial, receive_buffer, 256);
      send_uart_line(serial, buf3, strlen(buf3));
      clear_and_home(lcd);
      lcd_send_string(lcd, receive_buffer, received);
   }

   return (EXIT_SUCCESS);
}
