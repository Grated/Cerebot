
#include <stdint.h>
#include "pmod-hbridge.h"

/*
 * Updates sensor information for the given hbridge sensor.  After
 * sample_period updates this will provide a new epsp value.
 */
void update_sensor_state(struct hbridge_state *hbridge,
                         enum hbridge_sensors sensor_id,
                         uint32_t sample_period)
{
   uint16_t new_edge;
   struct hbridge_sensor_state *sensor = &(hbridge->sensor[sensor_id]);

   // Read the current state of the sensor
   new_edge = hbridge->read_sensor_state(hbridge->hbridge_id, sensor_id);

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
uint8_t is_hbridge_stopped(struct hbridge_state *hbridge)
{
   uint8_t stopped = 0;

   // Is the output compare value set to 0?
   // Read OCnR and verify the value is 0
   // Is the hbridge sensor information reporting that the motor is stopped?
   // If both are true, then the motor is stopped.
   if ((get_hbridge_speed(hbridge) == 0) &&
       (hbridge->sensor[PMOD_HB_SENSOR_A].epsp == 0) &&
       (hbridge->sensor[PMOD_HB_SENSOR_B].epsp == 0))
   {
      stopped = 1;
   }

   return stopped;
}

// Sets the hbridge direction
void set_hbridge_direction(struct hbridge_state *hbridge,
                           enum motor_direction dir)
{
   // Stop the motor
   while (is_hbridge_stopped(hbridge) == 0)
   {
      set_hbridge_speed(hbridge, 0);
   }

   if (dir == PMOD_HB_CW)
   {
      hbridge->clear_direction(hbridge->hbridge_id);
   }
   else
   {
      hbridge->set_direction(hbridge->hbridge_id);
   }

   hbridge->direction = dir;
}

/*
 * Changes the direction of the given hbridge, only changes if the motor
 * is stopped. Returns 1 if the change is successful otherwise it returns
 * 0 if the motor is not stopped.
 */
uint8_t change_hbridge_direction(struct hbridge_state *hbridge)
{
   uint8_t changed = 0;

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

   return changed;
}

/*
 * Sets the value of OCnRS for the specified hbridge.
 */
void set_hbridge_speed(struct hbridge_state *hbridge, uint32_t speed)
{
   hbridge->set_speed(hbridge->hbridge_id, speed);
}

/*
 * Reads the current value of OCnR for the specified hbridge.
 */
uint32_t get_hbridge_speed(struct hbridge_state *hbridge)
{
   return hbridge->get_speed(hbridge->hbridge_id);
}
