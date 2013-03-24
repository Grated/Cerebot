
#ifndef CONFIG_H
#define CONFIG_H

#include <peripheral/ports.h> // For IOPORT_x and BIT_x defines
#include <peripheral/outcompare.h>

// Defines for hbridge ports
#define HBRIDGE_LEFT_SA_PORT  IOPORT_D
#define HBRIDGE_LEFT_SA_BIT   BIT_9
#define HBRIDGE_LEFT_SB_PORT  IOPORT_C
#define HBRIDGE_LEFT_SB_BIT   BIT_1
#define HBRIDGE_LEFT_DIR_PORT IOPORT_D
#define HBRIDGE_LEFT_DIR_BIT  BIT_7
#define HBRIDGE_LEFT_OC       2

#define HBRIDGE_RIGHT_SA_PORT  IOPORT_D
#define HBRIDGE_RIGHT_SA_BIT   BIT_10
#define HBRIDGE_RIGHT_SB_PORT  IOPORT_C
#define HBRIDGE_RIGHT_SB_BIT   BIT_2
#define HBRIDGE_RIGHT_DIR_PORT IOPORT_D
#define HBRIDGE_RIGHT_DIR_BIT  BIT_6
#define HBRIDGE_RIGHT_OC       3

#endif // CONFIG_H
