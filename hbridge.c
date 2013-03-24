#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <peripheral/timer.h>
#include <peripheral/outcompare.h>
#include "config.h"
#include "hbridge.h"
#include "pmod-hbridge/pmod-hbridge.h"

enum sensor_id
{
   PMOD_HB_SENSOR_A,
   PMOD_HB_SENSOR_B,
   PMOD_HB_NUM_SENSORS
};

struct hbridge_sensor
{
   uint16_t port;
   uint16_t bit_pos;
};

struct hbridge_setup
{
   struct hbridge_info hbridge;
   struct hbridge_sensor sensors[PMOD_HB_NUM_SENSORS];
};

// Declare storage for hbridge info
static struct hbridge_info Motors[NUM_MOTORS];

static void init_hbridge_data(struct hbridge_info *left,
                              struct hbridge_info *right)
{
   memset(left, 0, sizeof(struct hbridge_info));
   memset(right, 0, sizeof(struct hbridge_info));

   // Enable reading value of sensor A
   PORTSetPinsDigitalIn(HBRIDGE_LEFT_SA_PORT, HBRIDGE_LEFT_SA_BIT);
   PORTSetPinsDigitalIn(HBRIDGE_LEFT_SB_PORT, HBRIDGE_LEFT_SB_BIT);
   PORTSetPinsDigitalIn(HBRIDGE_RIGHT_SA_PORT, HBRIDGE_RIGHT_SA_BIT);
   PORTSetPinsDigitalIn(HBRIDGE_RIGHT_SB_PORT, HBRIDGE_RIGHT_SB_BIT);

   // Enable writing direction pin
   PORTSetPinsDigitalOut(HBRIDGE_LEFT_DIR_PORT, HBRIDGE_LEFT_DIR_BIT);
   PORTSetPinsDigitalOut(HBRIDGE_RIGHT_DIR_PORT, HBRIDGE_RIGHT_DIR_BIT);

   // Setup left hbridge
   left->sensor_a.port = HBRIDGE_LEFT_SA_PORT;
   left->sensor_a.bit_pos = HBRIDGE_LEFT_SA_BIT;
   left->sensor_b.port = HBRIDGE_LEFT_SB_PORT;
   left->sensor_b.bit_pos = HBRIDGE_LEFT_SB_BIT;
   left->dir_port = HBRIDGE_LEFT_DIR_PORT;
   left->dir_bit = HBRIDGE_LEFT_DIR_BIT;
   left->ocn = HBRIDGE_LEFT_OC;

   // Setup right hbridge
   right->sensor_a.port = HBRIDGE_RIGHT_SA_PORT;
   right->sensor_a.bit_pos = HBRIDGE_RIGHT_SA_BIT;
   right->sensor_b.port = HBRIDGE_RIGHT_SB_PORT;
   right->sensor_b.bit_pos = HBRIDGE_RIGHT_SB_BIT;
   right->dir_port = HBRIDGE_RIGHT_DIR_PORT;
   right->dir_bit = HBRIDGE_RIGHT_DIR_BIT;
   right->ocn = HBRIDGE_RIGHT_OC;

   // Intialize direction
   set_hbridge_direction(left, PMOD_HB_CCW);
   set_hbridge_direction(right, PMOD_HB_CW);

}

/*
 * Helper function to get the proper hbridge structure from the motor
 * list enum.
 */
static struct hbridge_info* get_hbridge(enum motor_list motor)
{
   struct hbridge_info *hbridge = NULL;
   if (motor < NUM_MOTORS)
   {
      hbridge = &Motors[motor];
   }
   return hbridge;
}

/*
 * Initializes the hbridges
 */
void init_hbridges()
{
   init_hbridge_data(&Motors[LEFT_MOTOR], &Motors[RIGHT_MOTOR]);

   // Start with the motors off.
   SetDCOC2PWM(0);
   SetDCOC3PWM(0);
}
/*
 * Updates hbridge sensor data.
 */
void update_hbridge_sensor_info(uint32_t sample_period)
{
   enum motor_list motor;
   for (motor = FIRST_MOTOR; motor < NUM_MOTORS; motor++)
   {
      update_sensor_info(&(Motors[motor].sensor_a), sample_period);
      update_sensor_info(&(Motors[motor].sensor_b), sample_period);
   }
}

/*
 * Puts a brief description of the hbridge status into the
 * specified string.
 */
void print_hbridge_info(enum motor_list motor, char *buf, uint32_t max_len)
{
   struct hbridge_info* hbridge = get_hbridge(motor);
   if (hbridge != NULL)
   {
      snprintf(buf, max_len, "S:%d D:%d R:%d", hbridge->sensor_b.epsp,
               (uint8_t)hbridge->direction, !is_hbridge_stopped(hbridge));
   }
}

/*
 * Returns non-zero if the specified motor is stopped, returns 0 if it isn't.
 */
uint8_t is_motor_stopped(enum motor_list motor)
{
   uint8_t stopped = 1; // Assume it isn't
   struct hbridge_info* hbridge = get_hbridge(motor);
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
   struct hbridge_info *hbridge = get_hbridge(motor);
   if (hbridge != NULL)
   {
      changed = change_hbridge_direction(hbridge);
   }
   return changed;
}

