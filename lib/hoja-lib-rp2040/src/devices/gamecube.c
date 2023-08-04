#include "gamecube.h"

uint _gamecube_irq;
uint _gamecube_irq_tx;
uint _gamecube_offset;
pio_sm_config _gamecube_c;

uint8_t _gamecube_out_buffer[8] = {0};
volatile uint8_t _gamecube_in_buffer[8] = {0};

#define ALIGNED_JOYBUS_8(val) ((val) << 24)

void gamecube_send_probe()
{
  pio_sm_put_blocking(GAMEPAD_PIO, GAMEPAD_SM, ALIGNED_JOYBUS_8(0x09));
  pio_sm_put_blocking(GAMEPAD_PIO, GAMEPAD_SM, 0);
  pio_sm_put_blocking(GAMEPAD_PIO, GAMEPAD_SM, ALIGNED_JOYBUS_8(0x03));
}

void gamecube_send_origin()
{
  pio_sm_put_blocking(GAMEPAD_PIO, GAMEPAD_SM, 0);
  pio_sm_put_blocking(GAMEPAD_PIO, GAMEPAD_SM, ALIGNED_JOYBUS_8(0x80));
  pio_sm_put_blocking(GAMEPAD_PIO, GAMEPAD_SM, ALIGNED_JOYBUS_8(0x7F));
  pio_sm_put_blocking(GAMEPAD_PIO, GAMEPAD_SM, ALIGNED_JOYBUS_8(0x7F));
  pio_sm_put_blocking(GAMEPAD_PIO, GAMEPAD_SM, ALIGNED_JOYBUS_8(0x7F));
  pio_sm_put_blocking(GAMEPAD_PIO, GAMEPAD_SM, ALIGNED_JOYBUS_8(0x7F));

  pio_sm_put_blocking(GAMEPAD_PIO, GAMEPAD_SM, ALIGNED_JOYBUS_8(0x1));
  pio_sm_put_blocking(GAMEPAD_PIO, GAMEPAD_SM, ALIGNED_JOYBUS_8(0x1));
  pio_sm_put_blocking(GAMEPAD_PIO, GAMEPAD_SM, 0);
  pio_sm_put_blocking(GAMEPAD_PIO, GAMEPAD_SM, 0);
}

void gamecube_send_poll()
{
  pio_sm_put_blocking(GAMEPAD_PIO, GAMEPAD_SM, 0);
  pio_sm_put_blocking(GAMEPAD_PIO, GAMEPAD_SM, ALIGNED_JOYBUS_8(0x80));
  pio_sm_put_blocking(GAMEPAD_PIO, GAMEPAD_SM, ALIGNED_JOYBUS_8(0x7F));
  pio_sm_put_blocking(GAMEPAD_PIO, GAMEPAD_SM, ALIGNED_JOYBUS_8(0x7F));
  pio_sm_put_blocking(GAMEPAD_PIO, GAMEPAD_SM, ALIGNED_JOYBUS_8(0x7F));
  pio_sm_put_blocking(GAMEPAD_PIO, GAMEPAD_SM, ALIGNED_JOYBUS_8(0x7F));

  pio_sm_put_blocking(GAMEPAD_PIO, GAMEPAD_SM, ALIGNED_JOYBUS_8(0x1));
  pio_sm_put_blocking(GAMEPAD_PIO, GAMEPAD_SM, ALIGNED_JOYBUS_8(0x1));
}

volatile bool _gc_got_data = false;
bool _gc_running = false;
volatile uint8_t polls = 0;

void __time_critical_func(gamecube_command_handler)()
{
  volatile uint8_t cmd = pio_sm_get(GAMEPAD_PIO, GAMEPAD_SM);

  switch (cmd)
  {
    default:
      printf("Unknown: %x\n", cmd);
      break;
    case 0x00:
      joybus_set_in(false, GAMEPAD_PIO, GAMEPAD_SM, _gamecube_offset, &_gamecube_c, PGPIO_NS_SERIAL);
      gamecube_send_probe();
    break;

    case 0x40:
      _gamecube_in_buffer[0] = pio_sm_get(GAMEPAD_PIO, GAMEPAD_SM);
      _gamecube_in_buffer[1] = pio_sm_get(GAMEPAD_PIO, GAMEPAD_SM);

      joybus_set_in(false, GAMEPAD_PIO, GAMEPAD_SM, _gamecube_offset, &_gamecube_c, PGPIO_NS_SERIAL);
      gamecube_send_poll();
    break;

    case 0x41:
      joybus_set_in(false, GAMEPAD_PIO, GAMEPAD_SM, _gamecube_offset, &_gamecube_c, PGPIO_NS_SERIAL);
      gamecube_send_origin();
    break;
  }
}

static void gamecube_isr_handler(void)
{
  if (pio_interrupt_get(GAMEPAD_PIO, 0))
  {
    pio_interrupt_clear(GAMEPAD_PIO, 0);
    gamecube_command_handler();
  }
}

static void gamecube_isr_txdone(void)
{
  if (pio_interrupt_get(GAMEPAD_PIO, 1))
  {
    pio_interrupt_clear(GAMEPAD_PIO, 1);
    _gc_got_data=true;
  }
}

int counter = 0;

void gamecube_comms_task(button_data_s *buttons, a_data_s *analog)
{
  /*
  counter += 1;
  if (counter>10000)
  {
    printf("PC: %i\n", polls);
    counter = 0;
  }*/
  if (!_gc_running)
  {
    sleep_ms(150);
    _gamecube_offset = pio_add_program(GAMEPAD_PIO, &joybus_program);
    _gamecube_irq = PIO1_IRQ_0;
    _gamecube_irq_tx = PIO1_IRQ_1;

    pio_set_irq0_source_enabled(GAMEPAD_PIO, pis_interrupt0, true);
    pio_set_irq1_source_enabled(GAMEPAD_PIO, pis_interrupt1, true);

    irq_set_exclusive_handler(_gamecube_irq, gamecube_isr_handler);
    irq_set_exclusive_handler(_gamecube_irq_tx, gamecube_isr_txdone);
    joybus_program_init(GAMEPAD_PIO, GAMEPAD_SM, _gamecube_offset, PGPIO_NS_SERIAL, &_gamecube_c);
    irq_set_enabled(_gamecube_irq, true);
    irq_set_enabled(_gamecube_irq_tx, true);
    _gc_running = true;
  }
  else if (_gc_got_data)
  {
    joybus_set_in(true, GAMEPAD_PIO, GAMEPAD_SM, _gamecube_offset, &_gamecube_c, PGPIO_NS_SERIAL);
    _gc_got_data = false;
  }
}

void gamecube_init()
{

  // joybus_set_in(false, GAMEPAD_PIO, GAMEPAD_SM, _gamecube_offset, &_gamecube_c);
}