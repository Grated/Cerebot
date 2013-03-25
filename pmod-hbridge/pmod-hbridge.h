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

// Sensors
enum hbridge_sensors
{
   PMOD_HB_SENSOR_A,
   PMOD_HB_SENSOR_B,
   PMOD_HB_NUM_SENSORS
};

struct hbridge_sensor_state
{
   uint16_t port; // Port mask for this sensor
   uint16_t bit_pos; // Bit position for this sensor
   uint16_t prev_edge; // State of the previous edge, 0 or non-zero
   uint32_t edges; // Number of rising edges found in the current sample period
   uint32_t count; // Number of samples taken in the current period
   uint32_t epsp; // Number of edges in the last sample period
};

struct hbridge_state
{
   uint8_t hbridge_id;
   struct hbridge_sensor_state sensor[PMOD_HB_NUM_SENSORS];
   enum motor_direction direction; // Current motor direction
   uint16_t dir_port; // Port mask for the direction pin
   uint16_t dir_bit; // Bit position for the direction pin
   uint16_t ocn; // Output compare index (1 for OC1, 2 for OC2...)
   uint8_t (*read_sensor_state)(uint8_t id, enum hbridge_sensors sensor);
   void (*set_direction)(uint8_t id);
   void (*clear_direction)(uint8_t id);
   void (*set_speed)(uint8_t hbridge_id, uint32_t speed);
   uint32_t (*get_speed)(uint8_t hbridge_id);
};

void update_sensor_state(struct hbridge_state *hbridge,
                         enum hbridge_sensors sensor,
                         uint32_t sample_period);
uint8_t is_hbridge_stopped(struct hbridge_state *hbridge);
void set_hbridge_direction(struct hbridge_state *hbridge,
                           enum motor_direction dir);
uint8_t change_hbridge_direction(struct hbridge_state *hbridge);
void set_hbridge_speed(struct hbridge_state *hbridge, uint32_t speed);
uint32_t get_hbridge_speed(struct hbridge_state *hbridge);

#endif // PMOD_HBRIDGE_H
