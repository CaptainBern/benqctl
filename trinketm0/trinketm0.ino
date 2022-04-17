#include <Wire.h>

// The pin that will be used to send an interrupt to the monitor.
#define INT 3

// Our address on the I2C bus.
#define ADDRESS 0x28

// Supported commands to send to the monitor.
enum command {
  C_EXIT,
  C_SELECT,
  C_SCROLL_DOWN,
  C_SCROLL_UP,
  C_AUTO,
  C_P1,
  C_P2,
  C_P3,
};

// The state we are currently in.
enum state {
  S_READY,
  S_ONE,
  S_TWO,
};

volatile enum state current_state = S_READY;
volatile enum command current_cmd = C_EXIT;

// An array with the first and second argument for each command.
static const byte command_data[8][2] = {
  {0x10, 0x00}, // C_EXIT
  {0x40, 0x00}, // C_SELECT
  {0x00, 0x80}, // C_SCROLL_DOWN
  {0x00, 0x40}, // C_SCROLL_UP
  {0x08, 0x00}, // C_AUTO
  {0x01, 0x00}, // C_P1
  {0x02, 0x00}, // C_P2
  {0x04, 0x00}, // C_P3
};

// Set up to send a command to the monitor.
void sendCommand(enum command cmd)
{
  current_cmd = cmd;
  digitalWrite(INT, HIGH);
}

void handleReceive(int n)
{
  if (n == 1) {
    int i = Wire.read();
    switch (i) {
      case 0x00:
        current_state = S_ONE;
        break;
      case 0x01:
        current_state = S_TWO;
        break;
      case 0x02:
        current_state = S_READY;
        break;
      default:
        goto err_unknown_data;
    }
  } else if (n == 2) {
    int i = Wire.read();
    int j = Wire.read();
    if (i == 0 && j == 0) {
      digitalWrite(INT, LOW);
    } else {
      goto err_unknown_data;
    }
  }
  return;
  
err_unknown_data:
  Serial.println("Unexpected data!");
}

void handleRequest()
{
  Serial.println("got request");
  switch (current_state) {
    case S_READY:
      Serial.println("\tIDLE");
      Wire.write(0x1);
      break;
    case S_ONE:
      Serial.println("\tONE");
      Wire.write(command_data[current_cmd][0]);
      break;
    case S_TWO:
      Serial.println("\tTWO");
      Wire.write(command_data[current_cmd][1]);
      break;
    default:
      Serial.println("idk man");  
  }
}

void setup()
{
  // Interrupt pin
  pinMode(INT, OUTPUT);

  // Setup I2C
  Wire.onReceive(handleReceive);
  Wire.onRequest(handleRequest);
  Wire.begin(ADDRESS);

  Serial.begin(9600);
  while (!Serial); // Wait for serial monitor

  sendCommand(C_P1);
}

void loop()
{
  if (Serial.available() > 0) {
    String cmd = Serial.readString();
    cmd.trim();
    if (cmd.equalsIgnoreCase("exit")) {
      sendCommand(C_EXIT);
    } else if (cmd.equalsIgnoreCase("select")) {
      sendCommand(C_SELECT);
    } else if (cmd.equalsIgnoreCase("up")) {
      sendCommand(C_SCROLL_UP);
    } else if (cmd.equalsIgnoreCase("down")) {
      sendCommand(C_SCROLL_DOWN);
    } else {
      Serial.print("Unknown command: ");
      Serial.println(cmd);
    }
  }
  delay(500);
}
