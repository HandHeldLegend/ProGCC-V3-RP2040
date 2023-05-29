#include "switch_commands.h"

// This C file handles various Switch gamepad commands (OUT reports)

// Handles an OUT report and responds accordingly.
void switch_commands_report_handle(uint8_t report_id, const uint8_t *data, uint16_t len)
{
  printf("Report ID received: %X\n", report_id);

    for(uint16_t i = 0; i < len; i++)
    {
      printf("%X, ", data[i]);
    }
    printf("\n");
}

// Handles a command
void switch_commands_command_handle(uint8_t command, uint8_t *data)
{

}
