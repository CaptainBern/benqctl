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

// Represents all possible states.
enum state {
  S_0,
  S_1,
  S_2,
};

volatile bool busy                = false;
volatile enum state state         = S_0;
volatile enum command current_cmd = C_EXIT;

// Set up to send a command to the monitor.
// Returns true if the command is being sent, false otherwise.
bool sendCommand(enum command cmd)
{
  // Only touch the globals if we aren't currently executing a command.
  if (!busy) {
    current_cmd = cmd;
    busy = true;

    // Setting INT to high will make the monitor query us.
    digitalWrite(INT, HIGH);
  }

  return busy;
}

// Handle an I2C request event. Depending on our state,
// the monitor expects a different reply.
void handleRequest()
{
  // We only take requests if we are sending a command.
  if (!busy) {
    return;
  }
  
  switch (state) {
    case S_0:
      Wire.write(0x1);
      break;
    case S_1:
      Wire.write(command_data[current_cmd][0]);
      break;
    case S_2:
      Wire.write(command_data[current_cmd][1]);
      break; 
  }
}

// Handles I2C receive events.
void handleReceive(int n)
{
  // We only accept data if we are sending a command.
  if (!busy) {
    return;
  }

  if (n == 1) {
    int i = Wire.read();
    switch (i) {
      case 0x00:
        state = S_1;
        break;
      case 0x01:
        state = S_2;
        break;
      case 0x02:
        state = S_0;
        break;
      default:
        goto err_unknown_data;
    }
  } else if (n == 2) {
    int i = Wire.read();
    int j = Wire.read();
    if (i == 0 && j == 0) {
      // Command done, so no longer busy.
      busy = false;

      // This should have already happened in the previous block,
      // but just in case something went wrong, making sure we're
      // in S_0 pretty much guarantees next commands won't get
      // messed up.
      state = S_0;

      // The command is finished so set INT low.
      digitalWrite(INT, LOW);
    } else {
      goto err_unknown_data;
    }
  }
  return;
  
err_unknown_data:
  Serial.println("Unexpected data!");
}

void setup()
{
  // Interrupt pin
  pinMode(INT, OUTPUT);

  // Setup I2C
  Wire.onRequest(handleRequest);
  Wire.onReceive(handleReceive);
  Wire.begin(ADDRESS);

  Serial.begin(9600);
  while (!Serial); // Wait for serial monitor
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
