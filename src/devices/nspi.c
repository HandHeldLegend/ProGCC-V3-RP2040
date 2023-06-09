#include "nspi.h"

static uint _nspi_irq;
static PIO  _nspi_pio;
static uint _nspi_sm;
static uint _nspi_offset;
static uint32_t _nspi_buffer;

void nspi_load_buffer()
{
  _nspi_buffer = 0xA0;
  pio_sm_put_blocking(_nspi_pio, 0, _nspi_buffer);
}

void nspi_isr_handler(uint gpio, uint32_t events)
{
  gpio_acknowledge_irq(PGPIO_NS_LATCH, IO_IRQ_BANK0);
  nspi_load_buffer();
  nserial_latch_jump(_nspi_pio, _nspi_sm);
}

void nspi_init()
{
  // Set up GPIO for NSPI latch interrupt
  _nspi_irq = IO_IRQ_BANK0;
  gpio_init(PGPIO_NS_LATCH);
  gpio_set_dir(PGPIO_NS_LATCH, GPIO_IN);
  gpio_pull_up(PGPIO_NS_LATCH);

  irq_set_exclusive_handler(_nspi_irq, nspi_isr_handler);
  // Enable GPIO ISR
  gpio_set_irq_enabled(PGPIO_NS_LATCH, GPIO_IRQ_EDGE_FALL, true);

  // Set up PIO for NSPI
  _nspi_pio = pio0;
  _nspi_sm = 0;
  _nspi_offset = pio_add_program(_nspi_pio, &nserial_program);

  nserial_program_init(_nspi_pio, _nspi_sm, _nspi_offset, PGPIO_NS_DATA);
  irq_set_enabled(_nspi_irq, true);
}

void nspi_deinit()
{

}
