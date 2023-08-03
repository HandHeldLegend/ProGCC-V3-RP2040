#include "gamecube.h"

uint _gamecube_irq;
uint _gamecube_offset;
pio_sm_config _gamecube_c;

uint8_t _gamecube_out_buffer[8] = {0};
uint8_t _gamecube_in_buffer[8] = {0};

volatile bool gotisr = false;

void gamecube_send_probe()
{
  pio_sm_put_blocking(GAMEPAD_PIO, GAMEPAD_SM, 0x09 << 24);
  pio_sm_put_blocking(GAMEPAD_PIO, GAMEPAD_SM, 0x00 << 24);
  pio_sm_put_blocking(GAMEPAD_PIO, GAMEPAD_SM, 0x03 << 24);

}

volatile bool _gc_got_data = false;
bool _gc_running = false;

void __time_critical_func(gamecube_command_handler)()
{
  _gamecube_in_buffer[0] = pio_sm_get_blocking(GAMEPAD_PIO, GAMEPAD_SM);
  switch (_gamecube_in_buffer[0])
  {
    default:
    case 0x00:
      joybus_set_in(false, GAMEPAD_PIO, GAMEPAD_SM, _gamecube_offset, &_gamecube_c, PGPIO_NS_SERIAL);
      gamecube_send_probe();
    break;
  }
}

static void gamecube_isr_handler(void)
{
  if (pio_interrupt_get(GAMEPAD_PIO, 0))
  {
    gamecube_command_handler();
    _gc_got_data=true;
    pio_interrupt_clear(GAMEPAD_PIO, 0);
  }
}

void gamecube_comms_task(button_data_s *buttons, a_data_s *analog)
{
  if (!_gc_running)
  {
    sleep_ms(150);
    _gamecube_offset = pio_add_program(GAMEPAD_PIO, &joybus_program);
    _gamecube_irq = PIO1_IRQ_0;
    pio_set_irq0_source_enabled(GAMEPAD_PIO, pis_interrupt0, true);
    irq_set_exclusive_handler(_gamecube_irq, gamecube_isr_handler);
    joybus_program_init(GAMEPAD_PIO, GAMEPAD_SM, _gamecube_offset, PGPIO_NS_SERIAL, &_gamecube_c);
    irq_set_enabled(_gamecube_irq, true);
    _gc_running = true;
  }
  else if (_gc_got_data)
  {
    sleep_us(10);
    printf("Re: %x\n", _gamecube_in_buffer[0]);
    joybus_set_in(true, GAMEPAD_PIO, GAMEPAD_SM, _gamecube_offset, &_gamecube_c, PGPIO_NS_SERIAL);
    _gc_got_data = false;
  }
}

void gamecube_init()
{

  // joybus_set_in(false, GAMEPAD_PIO, GAMEPAD_SM, _gamecube_offset, &_gamecube_c);
}