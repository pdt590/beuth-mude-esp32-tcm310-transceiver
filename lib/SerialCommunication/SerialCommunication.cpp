/*
  ESP32 UART0 - Forward serial packet to PC
    - TXD
    - RXD
  ESP32 UART1 - Reserved for memory communication 
  ESP32 UART2 - Receive enocean packets from TCM310
    - TX2 - Pin 17
    - RX2 - Pin 16

*/

#include <Arduino.h>
#include "SerialCommunication.h"

#if defined(ESP32)
#include "driver/uart.h"
#define ENOCEAN_UART_NUM UART_NUM_2 // Use UART2 to receice Enocen packets
#define ENOCEAN_UART_TX 17
#define ENOCEAN_UART_RX 16
static const char *TAG = "enocean_uart_events";
#define BUF_SIZE (1024)
//static uart_isr_handle_t *handle_console;
static intr_handle_t handle_console;

#elif defined(ESP8266)
#else

#include <avr/io.h>
#include <avr/interrupt.h>
#endif

static uint8_t empty(char aChar)
{
  return 0;
}

const ReceptionOpe dummySet[] = {
    empty};

static ReceptionOpe *pReceptOpeSet = (ReceptionOpe *)dummySet;
static uint8_t state = 0;

#if defined(ESP32)
static void IRAM_ATTR enocean_uart_intr_handle(void *arg)
{
  uint16_t rx_fifo_len;

  rx_fifo_len = UART2.status.rxfifo_cnt; // read number of bytes in UART buffer

  while (rx_fifo_len)
  {
    rx_fifo_len--;
    unsigned char c = UART2.fifo.rw_byte;
    state = (pReceptOpeSet[state])(c);
  }

  // after reading bytes from buffer clear UART interrupt status
  uart_clear_intr_status(ENOCEAN_UART_NUM, UART_RXFIFO_FULL_INT_CLR | UART_RXFIFO_TOUT_INT_CLR);
}

#elif defined(ESP8266)
static void uart0_rx_intr_handler(void *para)
{
  if (!(USIS(UART0) & 0x1c))
  {
    while (USS(UART0) & 0x00ff)
    {
      unsigned char c = USF(UART0) & 0xff;
      state = (pReceptOpeSet[state])(c);
    }
  }
  else
  {
    // Overflow/Frame/Parity error, read byte but discard it
    unsigned char c = USF(UART0) & 0xff;
  }
  USIC((UART0)) = 0xff; /* Clear interrupt factor */
}

#else
#if defined(USART_RX_vect)
ISR(USART_RX_vect)

#elif defined(USART0_RX_vect)
ISR(USART0_RX_vect)

#elif defined(USART_RXC_vect)
ISR(USART_RXC_vect) // ATmega8

#else
#error "Don't know what the Data Received vector is called for Serial"
#endif

{
  if (bit_is_clear(UCSR0A, UPE0))
  {
    unsigned char c = UDR0;
    state = (pReceptOpeSet[state])(c);
  }
  else
  {
    // Parity error, read byte but discard it
    unsigned char c = UDR0;
  }
}

#endif

void SerialCommunication::Initialization(void)
{

#if defined(ESP32)
  //Configure parameters of an UART driver, communication pins and install the driver
  uart_config_t uart_config = {
      .baud_rate = 57600,
      .data_bits = UART_DATA_8_BITS,
      .parity = UART_PARITY_DISABLE,
      .stop_bits = UART_STOP_BITS_1,
      .flow_ctrl = UART_HW_FLOWCTRL_DISABLE};

  ESP_ERROR_CHECK(uart_param_config(ENOCEAN_UART_NUM, &uart_config));

  //Set UART log level
  esp_log_level_set(TAG, ESP_LOG_INFO);

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

#elif defined(ESP8266)
  volatile unsigned char c;
  USIE(UART0) = 0x00; /* Disable interrupt */
  USC1(UART0) = 1;    /* Rx fifo full threshold is 1 */

  while ((USS(UART0) >> USRXC) & 0xff)
  {
    c = USF(UART0) & 0xff;
  }
  ETS_UART_INTR_ATTACH(uart0_rx_intr_handler, NULL);
  PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0RXD_U, FUNC_U0RXD);
  ETS_UART_INTR_ENABLE();

  USIC((UART0)) = 0xff;         /* Clear interrupt factor */
  USIE(UART0) = (0x01 << UIFF); /* Enable interrupt for rx fifo full */

#else
  volatile unsigned char c;
#define RXCn 0x80
  while (UCSR0A & RXCn)
  {
    c = UDR0;
  }

#endif

  state = 0;
}

void SerialCommunication::SetReceptOpe(ReceptionOpe *pRcvOpeSet)
{
  pReceptOpeSet = pRcvOpeSet;
}
