/* 
 * File:   main.c
 * Author: sgreenman
 *
 * Created on October 20, 2012, 1:45 PM
 */

#include <stdio.h>
#include <stdlib.h>

#include "config.h"
#include "hbridge.h"
#include "battery_monitor.h"
#include "lcd.h"
#include "robot_messaging.h"

#include <peripheral/timer.h>
#include <peripheral/outcompare.h>

#ifndef OVERRIDE_CONFIG_BITS

#pragma config FPLLODIV = DIV_1, FPLLMUL = MUL_20, FPLLIDIV = DIV_2, FWDTEN = OFF, FCKSM = CSECME, FPBDIV = DIV_1
#pragma config OSCIOFNC = ON, POSCMOD = XT, FSOSCEN = ON, FNOSC = PRIPLL
#pragma config CP = OFF, BWP = OFF, PWP = OFF

#endif

#define GetSystemClock() (80000000L)
#define GetPeripheralClock() (GetSystemClock()/(1<<OSCCONbits.PBDIV))
#define GetInstructionClock() (GetSystemClock()/(1<<OSCCONbits.PBDIV))

// h-bridge timers are configured to rollover every millisecond, they will
// rollover at a count value of 1250.
#define TIMER_HBRIDGE_PRE 64
#define MS_PER_S 1000
#define TIMER_TICK_1MS (GetPeripheralClock() / TIMER_HBRIDGE_PRE / MS_PER_S)

// The timer interrupt triggers at a rate of 5KHz
#define TIMER_INT_PRE 8
#define TIMER_INT_TOGGLES 5000
#define TIMER_INT_TICK ((UINT32)((GetPeripheralClock()/TIMER_INT_PRE) / TIMER_INT_TOGGLES))

// Every 200 ms we will produce a new motor speed estimate.
// TIMER_INT_TOGGLES is the number of times the timer expires in one second,
// divide by 5 to get the number of times the timer expires in 200 ms.
#define SAMPLE_PERIOD_200MS (TIMER_INT_TOGGLES / 5)

// UART 1 is connected to the LCD screen.
#define LCD_UART UART1

// UART 2 is the serial or bluetooth connection
UART_MODULE serial = UART2;

void configureTimers();

static const char* Powerup_msg = "Robot startup";

/*
 * 
 */
int main(int argc, char** argv)
{
   // Initialize the LCD baud rate, backlight, wrapping
   init_lcd(LCD_UART, GetPeripheralClock());

   // Initialize the UART connection
   init_uart_cmd_interp(serial, GetPeripheralClock(), 115200);

   send_uart_line(serial, Powerup_msg, strlen(Powerup_msg));

   // Initialize h bridge data and status
   init_hbridges();

   // Timers
   configureTimers();

   // Enable battery monitor using ADC10
   init_battery_monitor();

   // enable multi-vector interrupts
   INTEnableSystemMultiVectoredInt();

   while(1)
   {
      read_robot_message();
   }

   return (EXIT_SUCCESS);
}

/*
 * Initializes the timers.
 *
 * Timer 1 causes an interrupt where some actions are performed.
 *
 * Timer 2, timer 3, output compare 2, and output compare 3 are used to
 * control the motors.
 *
 */
void configureTimers()
{
   // Timer 1 is used to cause an interrupt
   OpenTimer1(T1_ON | T1_SOURCE_INT | T1_PS_1_8, TIMER_INT_TICK);
   // set up the timer interrupt with a priority of 2
   ConfigIntTimer1(T1_INT_ON | T1_INT_PRIOR_2);

   // Timer 2 and OC2 are used to control the left hbridge
   // Initialize output compare 2 to use timer 2, PWM mode,
   // and set the compare values to 0 so we start in a stopped state.
   OpenOC2(OC_ON | OC_TIMER2_SRC | OC_PWM_FAULT_PIN_DISABLE, 0, 0);
   // Initialize timer with prescaler of 8 and a tick rate of 1ms
   // i.e. the timer expires 1000 times per second.
   OpenTimer2(T2_ON | T2_PS_1_64, TIMER_TICK_1MS);

   // Timer 3 and OC3 are used to control the right hbridge.
   // They use the same configuration as timer 2 and OC 2.
   OpenOC3(OC_ON | OC_TIMER3_SRC | OC_PWM_FAULT_PIN_DISABLE, 0, 0);
   OpenTimer3(T3_ON | T3_PS_1_64, TIMER_TICK_1MS);
}

/*
 * The ISR that is triggered when timer 1 expires.
 */
void __ISR(_TIMER_1_VECTOR, ipl2) Timer1Handler(void)
{
   update_motor_state();
   update_hbridge_sensor_info(SAMPLE_PERIOD_200MS);
   rotating_display(TIMER_INT_TOGGLES);

   // clear the interrupt flag
   mT1ClearIntFlag();
}
