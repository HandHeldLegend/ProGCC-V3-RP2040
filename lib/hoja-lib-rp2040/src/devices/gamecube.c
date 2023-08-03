#include "gamecube.h"

static uint _gamecube_irq;
static uint _gamecube_offset;
static pio_sm_config _gamecube_c;

void gamecube_load_buffer()
{
  pio_sm_put_blocking(GAMEPAD_PIO, GAMEPAD_SM, 0xFF);
  pio_sm_put_blocking(GAMEPAD_PIO, GAMEPAD_SM, 0x00);
  pio_sm_put_blocking(GAMEPAD_PIO, GAMEPAD_SM, 0xFF);
  pio_sm_put_blocking(GAMEPAD_PIO, GAMEPAD_SM, 0x00);
  pio_sm_put_blocking(GAMEPAD_PIO, GAMEPAD_SM, 0x00);
  pio_sm_put_blocking(GAMEPAD_PIO, GAMEPAD_SM, 0xFF);
  sleep_us(64);
}

void gamecube_isr_handler(void)
{
  if (pio_interrupt_get(GAMEPAD_PIO, 0))
  {
    pio_interrupt_clear(GAMEPAD_PIO, 0);
  }
}

void gamecube_init()
{
  // Set up GPIO for gamecube latch interrupt
  _gamecube_irq = IO_IRQ_BANK0;
  pio_set_irq0_source_enabled(GAMEPAD_PIO, pis_interrupt0, true);
  irq_set_exclusive_handler(_gamecube_irq, gamecube_isr_handler);
  irq_set_enabled(_gamecube_irq, true);


  _gamecube_offset = pio_add_program(GAMEPAD_PIO, &joybus_program);

  irq_set_enabled(_gamecube_irq, true);

  joybus_program_init(GAMEPAD_PIO, GAMEPAD_SM, _gamecube_offset, PGPIO_NS_SERIAL, &_gamecube_c);

  joybus_set_in(false, GAMEPAD_PIO, GAMEPAD_SM, _gamecube_offset, &_gamecube_c);

  sleep_us(100);

  gamecube_load_buffer();
}