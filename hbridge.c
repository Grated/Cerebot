#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <peripheral/timer.h>
#include <peripheral/outcompare.h>
#include "peripheral/ports.h" // For macros used to read ports
#include "config.h"
#include "hbridge.h"
#include "pmod-hbridge/pmod-hbridge.h"

struct hbridge_setup
{
   struct hbridge_state state;
   uint16_t sensor_port[PMOD_HB_NUM_SENSORS]; // Port mask for each sensor
   uint16_t sensor_bit_pos[PMOD_HB_NUM_SENSORS]; // Bit mask for each sensor
   uint16_t dir_port; // Port mask for the direction pin
   uint16_t dir_bit; // Bit position for the direction pin
   uint16_t ocn; // Output compare index (1 for OC1, 2 for OC2...)
};

// Declare storage for hbridge info
static struct hbridge_setup Motors[NUM_MOTORS];

// Forward declarations of driver functions
static uint8_t read_sensor_state(uint8_t hbridge_id,
                                 enum hbridge_sensors sensor);
static uint32_t read_speed(uint8_t hbridge_id);
static void set_direction(uint8_t hbridge_id);
static void clear_direction(uint8_t hbridge_id);
static void set_speed(uint8_t hbridge_id, uint32_t speed);
static uint32_t get_speed(uint8_t hbridge_id);

/*
 * Intializes hbridges for the motors
 */
static void init_hbridge_data()
{
   memset(&Motors[LEFT_MOTOR], 0, sizeof(struct hbridge_setup));
   memset(&Motors[RIGHT_MOTOR], 0, sizeof(struct hbridge_setup));

   // Enable reading value of sensor A
   PORTSetPinsDigitalIn(HBRIDGE_LEFT_SA_PORT, HBRIDGE_LEFT_SA_BIT);
   PORTSetPinsDigitalIn(HBRIDGE_LEFT_SB_PORT, HBRIDGE_LEFT_SB_BIT);
   PORTSetPinsDigitalIn(HBRIDGE_RIGHT_SA_PORT, HBRIDGE_RIGHT_SA_BIT);
   PORTSetPinsDigitalIn(HBRIDGE_RIGHT_SB_PORT, HBRIDGE_RIGHT_SB_BIT);

   // Enable writing direction pin
   PORTSetPinsDigitalOut(HBRIDGE_LEFT_DIR_PORT, HBRIDGE_LEFT_DIR_BIT);
   PORTSetPinsDigitalOut(HBRIDGE_RIGHT_DIR_PORT, HBRIDGE_RIGHT_DIR_BIT);

   // Setup left motor
   Motors[LEFT_MOTOR].state.hbridge_id = LEFT_MOTOR;
   Motors[LEFT_MOTOR].sensor_bit_pos[PMOD_HB_SENSOR_A] = HBRIDGE_LEFT_SA_BIT;
   Motors[LEFT_MOTOR].sensor_port[PMOD_HB_SENSOR_A] = HBRIDGE_LEFT_SA_PORT;
   Motors[LEFT_MOTOR].sensor_bit_pos[PMOD_HB_SENSOR_B] = HBRIDGE_LEFT_SB_BIT;
   Motors[LEFT_MOTOR].sensor_port[PMOD_HB_SENSOR_B] = HBRIDGE_LEFT_SB_PORT;
   Motors[LEFT_MOTOR].dir_port = HBRIDGE_LEFT_DIR_PORT;
   Motors[LEFT_MOTOR].dir_bit = HBRIDGE_LEFT_DIR_BIT;
   Motors[LEFT_MOTOR].ocn = HBRIDGE_LEFT_OC;

   // Setup right motor
   Motors[RIGHT_MOTOR].state.hbridge_id = RIGHT_MOTOR;
   Motors[RIGHT_MOTOR].sensor_bit_pos[PMOD_HB_SENSOR_A] = HBRIDGE_RIGHT_SA_BIT;
   Motors[RIGHT_MOTOR].sensor_port[PMOD_HB_SENSOR_A] = HBRIDGE_RIGHT_SA_PORT;
   Motors[RIGHT_MOTOR].sensor_bit_pos[PMOD_HB_SENSOR_B] = HBRIDGE_RIGHT_SB_BIT;
   Motors[RIGHT_MOTOR].sensor_port[PMOD_HB_SENSOR_B] = HBRIDGE_RIGHT_SB_PORT;
   Motors[RIGHT_MOTOR].dir_port = HBRIDGE_RIGHT_DIR_PORT;
   Motors[RIGHT_MOTOR].dir_bit = HBRIDGE_RIGHT_DIR_BIT;
   Motors[RIGHT_MOTOR].ocn = HBRIDGE_RIGHT_OC;

   // Setup function pointers
   Motors[LEFT_MOTOR].state.read_sensor_state = &read_sensor_state;
   Motors[RIGHT_MOTOR].state.read_sensor_state = &read_sensor_state;
   Motors[LEFT_MOTOR].state.set_direction = &set_direction;
   Motors[RIGHT_MOTOR].state.set_direction = &set_direction;
   Motors[LEFT_MOTOR].state.clear_direction = &clear_direction;
   Motors[RIGHT_MOTOR].state.clear_direction = &clear_direction;
   Motors[LEFT_MOTOR].state.set_speed = &set_speed;
   Motors[RIGHT_MOTOR].state.set_speed = &set_speed;
   Motors[LEFT_MOTOR].state.get_speed = &get_speed;
   Motors[RIGHT_MOTOR].state.get_speed = &get_speed;

   // Intialize direction
   set_hbridge_direction(&(Motors[LEFT_MOTOR].state), PMOD_HB_CCW);
   set_hbridge_direction(&(Motors[RIGHT_MOTOR].state), PMOD_HB_CW);

}

///////////////////////////////////////////////////////////////////////////////
/// Driver functions for the hbridge
///////////////////////////////////////////////////////////////////////////////

/*
 * Reads the state of the specified sensor pin for the specified hbridge.
 */
static uint8_t read_sensor_state(uint8_t hbridge_id,
                                 enum hbridge_sensors sensor)
{
   uint8_t retval = 0;
   enum motor_list motor = (enum motor_list)hbridge_id;
   if (motor < NUM_MOTORS)
   {
      retval = PORTReadBits(Motors[motor].sensor_port[sensor],
                            Motors[motor].sensor_bit_pos[sensor]);
   }
   return retval;
}

/*
 * Sets the specified hbridge's direction pin
 */
static void set_direction(uint8_t hbridge_id)
{
   enum motor_list motor = (enum motor_list)hbridge_id;
   if (motor < NUM_MOTORS)
   {
      PORTSetBits(Motors[hbridge_id].dir_port,
                  Motors[hbridge_id].dir_bit);
   }
}

/*
 * Clears the specified hbridge's direction pin
 */
static void clear_direction(uint8_t hbridge_id)
{
   enum motor_list motor = (enum motor_list)hbridge_id;
   if (motor < NUM_MOTORS)
   {
      PORTClearBits(Motors[hbridge_id].dir_port,
                  Motors[hbridge_id].dir_bit);
   }
}

/*
 * Sets the value of OCnRS for the specified hbridge.
 */
static void set_speed(uint8_t hbridge_id, uint32_t speed)
{
   enum motor_list motor = (enum motor_list)hbridge_id;
   if (motor < NUM_MOTORS)
   {
      switch(Motors[motor].ocn)
      {
         case 1:
            SetDCOC1PWM(speed);
            break;
         case 2:
            SetDCOC2PWM(speed);
            break;
         case 3:
            SetDCOC3PWM(speed);
            break;
         case 4:
            SetDCOC4PWM(speed);
            break;
         case 5:
            SetDCOC5PWM(speed);
            break;
         default:
            break;
      }
   }
}

/*
 * Reads the current value of OCnR for the specified hbridge.
 */
static uint32_t get_speed(uint8_t hbridge_id)
{
   uint32_t ocnr = 0;
   enum motor_list motor = (enum motor_list)hbridge_id;
   if (motor < NUM_MOTORS)
   {
      switch(Motors[motor].ocn)
      {
         case 1:
            ocnr = ReadRegOC1(0);
            break;
         case 2:
            ocnr = ReadRegOC2(0);
            break;
         case 3:
            ocnr = ReadRegOC3(0);
            break;
         case 4:
            ocnr = ReadRegOC4(0);
            break;
         case 5:
            ocnr = ReadRegOC5(0);
            break;
         default:
            ocnr = 0;
            break;
      }
   }
   return ocnr;
}

///////////////////////////////////////////////////////////////////////////////
/// Functions for external interfacing with hbridges
///////////////////////////////////////////////////////////////////////////////

/*
 * Helper function to get the proper hbridge structure from the motor
 * list enum.
 */
static struct hbridge_state* get_hbridge(enum motor_list motor)
{
   struct hbridge_state *hbridge = NULL;
   if (motor < NUM_MOTORS)
   {
      hbridge = &(Motors[motor].state);
   }
   return hbridge;
}

/*
 * Initializes the hbridges
 */
void init_hbridges()
{
   enum motor_list motor;
   init_hbridge_data();

   // Start with the motors off.
   for (motor = FIRST_MOTOR; motor < NUM_MOTORS; motor++)
   {
      set_hbridge_speed(&(Motors[motor].state), 0);
   }

}

/*
 * Updates hbridge sensor data.
 */
void update_hbridge_sensor_info(uint32_t sample_period)
{
   enum motor_list motor;
   for (motor = FIRST_MOTOR; motor < NUM_MOTORS; motor++)
   {
      update_sensor_state(&(Motors[motor].state),
                          PMOD_HB_SENSOR_A, sample_period);
      update_sensor_state(&(Motors[motor].state),
                          PMOD_HB_SENSOR_B, sample_period);
   }
}

/*
 * Puts a brief description of the hbridge status into the
 * specified string.
 */
void print_hbridge_info(enum motor_list motor, char *buf, uint32_t max_len)
{
   struct hbridge_state* hbridge = get_hbridge(motor);
   if (hbridge != NULL)
   {
      snprintf(buf, max_len, "S:%d D:%d R:%d",
               hbridge->sensor[PMOD_HB_SENSOR_B].epsp,
               (uint8_t)hbridge->direction, !is_hbridge_stopped(hbridge));
   }
}

/*
 * Returns non-zero if the specified motor is stopped, returns 0 if it isn't.
 */
uint8_t is_motor_stopped(enum motor_list motor)
{
   uint8_t stopped = 1; // Assume it isn't
   struct hbridge_state* hbridge = get_hbridge(motor);
   if (hbridge != NULL)
   {
      stopped = is_hbridge_stopped(hbridge);
   }
   return stopped;
}

/*
 * Changes the motor direction.
 */
uint8_t change_motor_direction(enum motor_list motor)
{
   uint8_t changed = 0;
   struct hbridge_state *hbridge = get_hbridge(motor);
   if (hbridge != NULL)
   {
      changed = change_hbridge_direction(hbridge);
   }
   return changed;
}

