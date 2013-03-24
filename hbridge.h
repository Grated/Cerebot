
#ifndef HBRIDGE_H
#define HBRIDGE_H

#include <stdint.h>

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
uint8_t is_motor_stopped(enum motor_list motor);

#endif // HBRIDGE_H
