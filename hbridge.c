#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <peripheral/timer.h>
#include <peripheral/outcompare.h>
#include "peripheral/ports.h" // For macros used to read ports
#include "config.h"
#include "hbridge.h"
#include "pmod-hbridge/pmod-hbridge.h"

struct hbridge_info
{
   struct hbridge_state state;

   volatile uint32_t target_speed;
   volatile enum motor_direction target_direction;

   uint16_t sensor_port[PMOD_HB_NUM_SENSORS]; // Port mask for each sensor
   uint16_t sensor_bit_pos[PMOD_HB_NUM_SENSORS]; // Bit mask for each sensor
   uint16_t dir_port; // Port mask for the direction pin
   uint16_t dir_bit; // Bit position for the direction pin
   uint16_t ocn; // Output compare index (1 for OC1, 2 for OC2...)
   uint16_t tmrn; // Timer index (1 for timer 1, 2 for timer 2...)
};

// Declare storage for hbridge info
static struct hbridge_info Motors[NUM_MOTORS];

// Forward declarations of driver functions
static uint8_t read_sensor_state(uint8_t hbridge_id,
                                 enum hbridge_sensors sensor);
static uint32_t read_speed(uint8_t hbridge_id);
static void set_direction(uint8_t hbridge_id);
static void clear_direction(uint8_t hbridge_id);
static void set_speed(uint8_t hbridge_id, uint32_t speed);
static uint32_t get_speed(uint8_t hbridge_id);

// Static functions
static uint8_t is_motor_stopped(enum motor_list motor);
static uint8_t change_motor_direction(enum motor_list motor);
static struct hbridge_state* get_hbridge(enum motor_list motor);
static uint32_t get_timer_period(struct hbridge_info* hbridge);
static enum robot_direction get_motor_direction(enum motor_list motor,
        enum motor_direction motor_dir);
static void set_target_motor_direction(enum motor_list motor,
        enum robot_direction direction);

/*
 * Intializes hbridges for the motors
 */
static void init_hbridge_data()
{
   memset(&Motors[LEFT_MOTOR], 0, sizeof(struct hbridge_info));
   memset(&Motors[RIGHT_MOTOR], 0, sizeof(struct hbridge_info));

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
   Motors[LEFT_MOTOR].tmrn = HBRIDGE_LEFT_TMR;

   // Setup right motor
   Motors[RIGHT_MOTOR].state.hbridge_id = RIGHT_MOTOR;
   Motors[RIGHT_MOTOR].sensor_bit_pos[PMOD_HB_SENSOR_A] = HBRIDGE_RIGHT_SA_BIT;
   Motors[RIGHT_MOTOR].sensor_port[PMOD_HB_SENSOR_A] = HBRIDGE_RIGHT_SA_PORT;
   Motors[RIGHT_MOTOR].sensor_bit_pos[PMOD_HB_SENSOR_B] = HBRIDGE_RIGHT_SB_BIT;
   Motors[RIGHT_MOTOR].sensor_port[PMOD_HB_SENSOR_B] = HBRIDGE_RIGHT_SB_PORT;
   Motors[RIGHT_MOTOR].dir_port = HBRIDGE_RIGHT_DIR_PORT;
   Motors[RIGHT_MOTOR].dir_bit = HBRIDGE_RIGHT_DIR_BIT;
   Motors[RIGHT_MOTOR].ocn = HBRIDGE_RIGHT_OC;
   Motors[RIGHT_MOTOR].tmrn = HBRIDGE_RIGHT_TMR;

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

   // Intialize speed
   set_hbridge_speed(&(Motors[LEFT_MOTOR].state), 0);
   set_hbridge_speed(&(Motors[LEFT_MOTOR].state), 0);
   set_target_speed(LEFT_MOTOR, 0);
   set_target_speed(RIGHT_MOTOR, 0);
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
/// Local helper functions
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
 * Helper function for getting timer periods.
 * The timer period is the maximum value for a output compare register.
 * Setting the OCRn register to the timer period results in the fastest
 * motor speed.
 */
static uint32_t get_timer_period(struct hbridge_info* hbridge)
{
   uint32_t retval = 0;
   switch(hbridge->tmrn)
   {
      case 1:
         retval = ReadPeriod1();
         break;
      case 2:
         retval = ReadPeriod2();
         break;
      case 3:
         retval = ReadPeriod3();
         break;
      case 4:
         retval = ReadPeriod4();
         break;
      case 5:
         retval = ReadPeriod5();
         break;
   }
}

/*
 * Returns non-zero if the specified motor is stopped, returns 0 if it isn't.
 */
static uint8_t is_motor_stopped(enum motor_list motor)
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
 * Changes the motor direction.  Busy waits until the direction can
 * be changed.
 */
static uint8_t change_motor_direction(enum motor_list motor)
{
   uint8_t changed = 0;
   struct hbridge_state *hbridge = get_hbridge(motor);
   if (hbridge != NULL)
   {
      changed = change_hbridge_direction(hbridge);
   }
   return changed;
}

/**
 * Given a motor and that motors direction determines the direction that
 * motor helps move the robot.
 * @param motor LEFT_MOTOR or RIGHT_MOTOR
 * @param motor_dir PMOD_HB_CCW or PMOD_HB_CW
 * @return ROBOT_FORWARD or ROBOT_REVERSE
 */
static enum robot_direction get_motor_direction(
   enum motor_list motor, enum motor_direction motor_dir)
{
   enum robot_direction dir = ROBOT_FORWARD;

   switch (motor)
   {
      case LEFT_MOTOR:
         if (motor_dir == PMOD_HB_CCW)
         {
            dir = ROBOT_FORWARD;
         }
         else
         {
            dir = ROBOT_REVERSE;
         }
         break;

      case RIGHT_MOTOR:
         if (motor_dir == PMOD_HB_CW)
         {
            dir = ROBOT_FORWARD;
         }
         else
         {
            dir = ROBOT_REVERSE;
         }
         break;

      default:
         break;
   }
   return dir;
}

/**
 * Given a direction from the robots point of view updates the motor to
 * move in that direction.
 * @param motor
 * @param direction
 */
static void set_target_motor_direction(enum motor_list motor,
        enum robot_direction direction)
{
   switch (motor)
   {
      case LEFT_MOTOR:
         if (direction == ROBOT_FORWARD)
         {
            Motors[motor].target_direction = PMOD_HB_CCW;
         }
         else
         {
            Motors[motor].target_direction = PMOD_HB_CW;
         }
         break;

      case RIGHT_MOTOR:
         if (direction == ROBOT_FORWARD)
         {
            Motors[motor].target_direction = PMOD_HB_CW;
         }
         else
         {
            Motors[motor].target_direction = PMOD_HB_CCW;
         }
         break;

      default:
         break;
   }
}

///////////////////////////////////////////////////////////////////////////////
/// Functions for external interfacing with hbridges
///////////////////////////////////////////////////////////////////////////////

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
 * Sets the motor speed given a percentage.
 * Disable interrupts before calling.
 */
void set_target_speed(enum motor_list motor, int8_t percent)
{
   uint32_t speed = 0;

   // Clamp the percentage of power
   if (percent >= 100)
   {
      percent = 99;
   }

   if (percent <= -100)
   {
      percent = -99;
   }

   // If the percentage is positive we want to go forward, otherwise
   // we want to go in reverse.
   if (percent >= 0)
   {
      set_target_motor_direction(motor, ROBOT_FORWARD);
   }
   else
   {
      set_target_motor_direction(motor, ROBOT_REVERSE);
   }

   percent = abs(percent);

   speed = (uint32_t)((float)percent/100 * get_timer_period(&(Motors[motor])));

   Motors[motor].target_speed = speed;
}

#define SPEED_STEP 1
/*
 * Updates motor state. Expected to be called periodically
 */
void update_motor_state()
{
   enum motor_list motor;
   uint32_t current_speed;
   uint32_t target_speed;

   for (motor = FIRST_MOTOR; motor < NUM_MOTORS; motor++)
   {
      target_speed = Motors[motor].target_speed;
      current_speed = get_hbridge_speed(&(Motors[motor].state));

      // See if we need to change directions
      if (Motors[motor].state.direction != Motors[motor].target_direction)
      {
         // Yup, the target direction is different from the current.
         // Try to stop the motor.
         set_hbridge_speed(&(Motors[motor].state), 0);

         // Try a direction change, will return if the motors aren't stopped.
         // That's OK if it does, we'll try again on the next go around.
         set_hbridge_direction(&(Motors[motor].state),
            Motors[motor].target_direction);
      }
      // Step the motor up to the target speed
      else if (current_speed != target_speed)
      {
         // If we are slowing down, do it!
         if (current_speed > target_speed)
         {
            set_hbridge_speed(&(Motors[motor].state), target_speed);
         }
         else
         {
            // We are speeding up, sometimes the cerebot restarts
            // if the speed increase is too great, slew to the
            // target speed.
            // Probably because of the sudden need for power.
            if ((target_speed - current_speed) > SPEED_STEP)
            {
               set_hbridge_speed(&(Motors[motor].state),
                                 current_speed + SPEED_STEP);
            }
            else
            {
               set_hbridge_speed(&(Motors[motor].state), target_speed);
            }
         }
      }
   }
}
