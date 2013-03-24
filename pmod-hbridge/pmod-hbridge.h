/*
 * Definitions for hbridges
 */
#ifndef PMOD_HBRIDGE_H
#define PMOD_HBRIDGE_H

#include <stdint.h>

// Looking at the motor, which way is it spinning?
enum motor_direction
{
   PMOD_HB_CW,
   PMOD_HB_CCW
};

struct hbridge_sensor_info
{
   uint16_t port; // Port mask for this sensor
   uint16_t bit_pos; // Bit position for this sensor
   uint16_t prev_edge; // State of the previous edge, 0 or non-zero
   uint32_t edges; // Number of rising edges found in the current sample period
   uint32_t count; // Number of samples taken in the current period
   uint32_t epsp; // Number of edges in the last sample period
};

struct hbridge_info
{
   struct hbridge_sensor_info sensor_a; // h-bridge has two sensors
   struct hbridge_sensor_info sensor_b;
   enum motor_direction direction; // Current motor direction
   uint16_t dir_port; // Port mask for the direction pin
   uint16_t dir_bit; // Bit position for the direction pin
   uint16_t ocn; // Output compare index (1 for OC1, 2 for OC2...)
};

void update_sensor_info(struct hbridge_sensor_info *sensor,
                        uint32_t sample_period);
uint8_t is_hbridge_stopped(struct hbridge_info *hbridge);
void set_hbridge_direction(struct hbridge_info *hbridge,
                           enum motor_direction dir);
uint8_t change_hbridge_direction(struct hbridge_info *hbridge);
void set_hbridge_speed(struct hbridge_info *hbridge, uint32_t speed);
void get_hbridge_speed(struct hbridge_info *hbridge, uint32_t speed);

#endif // PMOD_HBRIDGE_H
