/* 
 * File:   rotating_display.h
 * Author: sgreenman
 *
 * Created on April 14, 2013, 1:04 PM
 */

#ifndef ROTATING_DISPLAY_H
#define	ROTATING_DISPLAY_H

#include <stdint.h>

#ifdef	__cplusplus
extern "C" {
#endif

   void rotating_display(UINT32 system_clk_freq);
   void set_user_message(const char* top, const char* bot);

#ifdef	__cplusplus
}
#endif

#endif	/* ROTATING_DISPLAY_H */

