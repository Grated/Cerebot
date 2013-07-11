
#include <stdint.h>

#include "lcd.h"
#include "hbridge.h"
#include "battery_monitor.h"

// For now we only allow 16 characters to a line.
#define LINE_LENGTH 16

// Storage for the user message.  Uses a ping pong buffer to reduce
// the chance that we will be updating a message at the same time we
// are sending one.  This will work as long as the rotating display
// is called from a higher priority interrupt than the message
// updater.
static volatile UINT8 msg_index = 0;
static char user_line1[2][LINE_LENGTH] = {0};
static char user_line2[2][LINE_LENGTH] = {0};

// Display rotation speed
#define NUM_SEC_PER_SWAP 3

// Supported displays

enum displays
{
   USER_MSG,
   MOTOR_STATUS,
   BATTERY_STATUS
};

// Helper functions
static enum displays get_next_display(enum displays current);

/**
 * Rotating display
 * @param calls_per_sec The number of times this function is called in a second.
 */
void rotating_display(UINT32 calls_per_sec)
{
   static UINT32 current_count = 0;
   static enum displays cur_display = USER_MSG;
   char line1[LINE_LENGTH] = {0};
   char line2[LINE_LENGTH] = {0};
   UINT8 user_msg_index = 0;

   // Increment count
   current_count++;

   // See if the counter has expired
   if (current_count > (calls_per_sec * NUM_SEC_PER_SWAP))
   {
      current_count = 0;
      cur_display = get_next_display(cur_display);

      switch (cur_display)
      {
         case USER_MSG:
            // Read the index once so we can guarantee we send matching messages.
            user_msg_index = msg_index;
            lcd_write(&user_line1[user_msg_index][0],
                      &user_line2[user_msg_index][0]);
            break;

         case MOTOR_STATUS:
            // Left motor info
            snprintf(line1, sizeof (line1), "L ");
            print_hbridge_info(LEFT_MOTOR, &line1[strlen(line1)],
                    sizeof (line1) - strlen(line1));

            // Right motor info
            snprintf(line2, sizeof (line2), "R ");
            print_hbridge_info(RIGHT_MOTOR, &line2[strlen(line2)],
                    sizeof (line2) - strlen(line2));

            lcd_write(line1, line2);
            break;

         case BATTERY_STATUS:
            snprintf(line1, sizeof (line1), "Batt: %1.2f V",
                    read_battery_voltage());
            line2[0] = '\0';
            lcd_write(line1, line2);
            break;

         default:
            snprintf(line1, sizeof (line1), "Error: ");
            snprintf(line2, sizeof (line1), "Unimplemented");
            break;
      }
   }

}

/**
 * Sets the user message string using ping pong buffers.
 * @param top
 * @param bot
 */
void set_user_message(const char* top, const char* bot)
{
   UINT32 i;
   UINT32 index;

   // Set the index to write to
   if (msg_index == 0)
   {
      index = 1;
   }
   else
   {
      index = 0;
   }

   // Copy the user strings
   for (i = 0; i < LINE_LENGTH; i++)
   {
      user_line1[index][i] = *top++;
      user_line2[index][i] = *bot++;
   }

   // Swap buffers
   msg_index = index;
}

/**
 * Given the current display, returns the next one.
 * @param current
 * @return
 */
static enum displays get_next_display(enum displays current)
{
   enum displays retval = USER_MSG;

   switch (current)
   {
      case USER_MSG:
         retval = MOTOR_STATUS;
         break;

      case MOTOR_STATUS:
         retval = BATTERY_STATUS;
         break;

      case BATTERY_STATUS:
         retval = USER_MSG;
         break;

      default:
         retval = USER_MSG;
         break;
   }

   return retval;
}
