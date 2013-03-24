
#include <stdint.h>
#include "pmod-hbridge.h"
#include "peripheral/ports.h" // For macros used to read ports
#include "peripheral/outcompare.h" // macros to read output compare registers

/*
 * Updates sensor information for the given hbridge sensor.  After
 * sample_period updates this will provide a new epsp value.
 */
void update_sensor_info(struct hbridge_sensor_info *sensor,
                        uint32_t sample_period)
{
   uint16_t new_edge;

   // Read the current state of the sensor
   new_edge = PORTReadBits(sensor->port, sensor->bit_pos);

   // Determine if we've found a new rising edge
   if(new_edge > 0 && new_edge != sensor->prev_edge)
   {
      // New rising edge, increment the number of edges.
      sensor->edges++;
   }
   sensor->prev_edge = new_edge;

   // Increment count
   sensor->count++;

   // Every sample_period calls to this function update the epsp value.
   // The epp value is the number of edges found in the last sample period.
   if (sensor->count >= sample_period)
   {
      // Determine the epsp value
      sensor->epsp = sensor->edges;

      // Reset counters
      sensor->edges = 0;
      sensor->count = 0;
   }
}

/*
 * Returns 1 if the motor is stopped, 0 otherwise
 */
uint8_t is_hbridge_stopped(struct hbridge_info *hbridge)
{
   uint8_t stopped = 0;

   // Is the output compare value set to 0?
   // Read OCnR and verify the value is 0
   // Is the hbridge sensor information reporting that the motor is stopped?
   // If both are true, then the motor is stopped.
   if ((get_hbridge_speed(hbridge) == 0) &&
       (hbridge->sensor_a.epsp == 0) && 
       (hbridge->sensor_b.epsp == 0))
   {
      stopped = 1;
   }

   return stopped;
}

// Sets the hbridge direction
void set_hbridge_direction(struct hbridge_info *hbridge,
                           enum motor_direction dir)
{
   if (dir == PMOD_HB_CW)
   {
      PORTClearBits(hbridge->dir_port, hbridge->dir_bit);
   }
   else
   {
      PORTSetBits(hbridge->dir_port, hbridge->dir_bit);
   }
   hbridge->direction = dir;
}

/*
 * Changes the direction of the given hbridge, only changes if the motor
 * is stopped. Returns 1 if the change is successful otherwise it returns
 * 0 if the motor is not stopped.
 */
uint8_t change_hbridge_direction(struct hbridge_info *hbridge)
{
   uint8_t changed = 0;

   // Make sure the hbridge is stopped.
   if (is_hbridge_stopped(hbridge) != 0)
   {
      if (hbridge->direction == PMOD_HB_CW)
      {
         set_hbridge_direction(hbridge, PMOD_HB_CCW);
      }
      else
      {
         set_hbridge_direction(hbridge, PMOD_HB_CW);
      }

      // We changed the direction
      changed = 1;
   }

   return changed;
}

/*
 * Sets the value of OCnRS for the specified hbridge.
 */
void set_hbridge_speed(struct hbridge_info *hbridge, uint32_t speed)
{
   switch(hbridge->ocn)
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

/*
 * Reads the current value of OCnR for the specified hbridge.
 */
uint32_t get_hbridge_speed(struct hbridge_info *hbridge)
{
   uint32_t ocnr = 0;

   switch(hbridge->ocn)
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
   return ocnr;
}
