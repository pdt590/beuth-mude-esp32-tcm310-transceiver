/*
  ESP32 UART0 - Forward serial packet to PC
    - TXD - Pin 1
    - RXD - Pin 3
  ESP32 UART1 - Reserved for memory communication 
  ESP32 UART2 - Receive enocean packets from TCM310
    - TX2 - Pin 17
    - RX2 - Pin 16
*/

#include <Arduino.h>
#include "SerialCommunication.h"

#include "driver/uart.h"
#define ENOCEAN_UART_NUM UART_NUM_2 // Use UART2 to receice Enocen packets
#define ENOCEAN_UART_TX 17
#define ENOCEAN_UART_RX 16
#define ENOCEAN_UART UART2
#define BUF_SIZE (1024)
static intr_handle_t handle_console;

static uint8_t empty(char aChar)
{
  return 0;
}

const ReceptionOpe dummySet[] = {
    empty};

static ReceptionOpe *pReceptOpeSet = (ReceptionOpe *)dummySet;
static uint8_t state = 0;

static void IRAM_ATTR enocean_uart_intr_handle(void *arg)
{
  uint16_t rx_fifo_len;

  rx_fifo_len = ENOCEAN_UART.status.rxfifo_cnt; // read number of bytes in UART buffer

  while (rx_fifo_len)
  {
    rx_fifo_len--;
    unsigned char c = ENOCEAN_UART.fifo.rw_byte;
    state = (pReceptOpeSet[state])(c);
  }

  // after reading bytes from buffer clear UART interrupt status
  uart_clear_intr_status(ENOCEAN_UART_NUM, UART_RXFIFO_FULL_INT_CLR | UART_RXFIFO_TOUT_INT_CLR);

  // send packet to enocean module
  //uart_write_bytes(ENOCEAN_UART_NUM, (const char *)"RX Done", 7);
}


void SerialCommunication::Initialization(void)
{

  //Configure parameters of an UART driver, communication pins and install the driver
  uart_config_t uart_config = {
      .baud_rate = 57600,
      .data_bits = UART_DATA_8_BITS,
      .parity = UART_PARITY_DISABLE,
      .stop_bits = UART_STOP_BITS_1,
      .flow_ctrl = UART_HW_FLOWCTRL_DISABLE};

  ESP_ERROR_CHECK(uart_param_config(ENOCEAN_UART_NUM, &uart_config));

  //Set UART pins (using UART2)
  ESP_ERROR_CHECK(uart_set_pin(ENOCEAN_UART_NUM, ENOCEAN_UART_TX, ENOCEAN_UART_RX, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE)); // TODO: Change UART_PIN_NO_CHANGE

  //Install UART driver, and get the queue.
  ESP_ERROR_CHECK(uart_driver_install(ENOCEAN_UART_NUM, BUF_SIZE * 2, 0, 0, NULL, 0));

  // release the pre registered UART handler/subroutine
  ESP_ERROR_CHECK(uart_isr_free(ENOCEAN_UART_NUM));

  // register new UART subroutine
  ESP_ERROR_CHECK(uart_isr_register(ENOCEAN_UART_NUM, enocean_uart_intr_handle, NULL, ESP_INTR_FLAG_IRAM, &handle_console));

  // enable RX interrupt
  ESP_ERROR_CHECK(uart_enable_rx_intr(ENOCEAN_UART_NUM));

  state = 0;
}

void SerialCommunication::SetReceptOpe(ReceptionOpe *pRcvOpeSet)
{
  pReceptOpeSet = pRcvOpeSet;
}
