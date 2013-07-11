
#ifndef HBRIDGE_H
#define HBRIDGE_H

#include <stdint.h>

enum robot_direction
{
   ROBOT_FORWARD,
   ROBOT_REVERSE
};

enum motor_list
{
   LEFT_MOTOR,
   RIGHT_MOTOR,
   NUM_MOTORS,
   FIRST_MOTOR = LEFT_MOTOR
};

void init_hbridges();
void update_hbridge_sensor_info(uint32_t sample_period);
void print_hbridge_info(enum motor_list motor, char *buf, uint32_t max_len);
void set_target_speed(enum motor_list motor, int8_t speed);
void update_motor_state();

#endif // HBRIDGE_H
