#include <Arduino.h>
#include "ESP3Parser.h"

ESP3Parser parser(NULL);

void setup()
{
  // Open serial communications and wait for port to open:
  Serial.begin(57600); // Init UART0
  while (!Serial) {
    ; // wait for serial port to connect.
  }
  parser.initialization();
}

void loop()
{
  ;
}
