#include <Arduino.h>
#include <map>

#include <ETH.h>
#include <WiFiClient.h>
#include <WiFiServer.h>


// Olimex ESP32-PoE-ISO Ethernet hardware definitions
// Based on LAN8710A PHY operating in RMII mode  with SMI address 0x00.
#define ETH_PHY_ADDR 0
#define ETH_PHY_POWER 12
#define ETH_PHY_MDC 23
#define ETH_PHY_MDIO 18
#define ETH_PHY_TYPE \
  ETH_PHY_LAN8720  // The LAN8710A is perfectly compatible with the standard
                   // LAN8720 driver
#define ETH_CLK_MODE ETH_CLOCK_GPIO17_OUT

#define SERIAL_BAUD 115200


const int TCP_PORT = 5000;
WiFiServer server(TCP_PORT);

// --- Static IP Configuration ---
IPAddress local_IP(192, 168, 0,
                   150);            // The static IP for this specific HIVE node
IPAddress gateway(192, 168, 0, 1);  // Your network router/gateway
IPAddress subnet(255, 255, 255, 0);  // Subnet mask
IPAddress primaryDNS(8, 8, 8, 8);    // Optional: Primary DNS (Google)
IPAddress secondaryDNS(1, 1, 1, 1);  // Optional: Secondary DNS (Cloudflare)

// Allocated PIN numbers
const int GPIO_PIN_n2oValve;
const int GPIO_PIN_purge;
const int GPIO_PIN_o2Valve;
const int GPIO_PIN_fire;
const int GPIO_PIN_firePWM;


bool ACTIVATE_PIN = true;
bool DEACTIVATE_PIN = false;

// 
const int STATE_fuelActive = 1;
const int STATE_ignActive = 2;
const int STATE_standby = 3;

const int ACT_fill;
const int ACT_purge;
const int ACT_neutral;
const int ACT_fire;
const int ACT_O2;

int STATE_current;
int ACT_current;

/*
Create Hash Map for each Input of GCS and the value for each key to be instructions for what solenoids to activate

Input is some 4 byte sequence representing input from GCS control pendant

Switch solenoids as soon as you have new input and keep them in that state until you get a new input.
If you do not get a new input within 5 seconds you need to cut power to all solenoid outputs.

*/

void set_n2oValve(bool activate){
  if (activate)  { // open valve
    digitalWrite(GPIO_PIN_n2oValve, HIGH);
  }
  else  { // close valve
    digitalWrite(GPIO_PIN_n2oValve, LOW);
  }
}

void set_purge(bool activate){
  if (activate)  {
    digitalWrite(GPIO_PIN_purge, HIGH);
  }
  else  {
     digitalWrite(GPIO_PIN_purge, LOW);
  }
}

void set_o2Valve(bool activate){
  if (activate)  {
    digitalWrite(GPIO_PIN_o2Valve, HIGH);
  }
  else  {
    digitalWrite(GPIO_PIN_o2Valve, LOW);
  }
}

void set_fire(bool activate){
  if (activate)  {
    digitalWrite(GPIO_PIN_fire, HIGH);
  }
  else  {
    digitalWrite(GPIO_PIN_fire, LOW);
  }
}

void set_firePWM(){

}

void switchState(int STATE, int ACTION, int* previousState, int* previousAction)
{
  switch (STATE)  {
    case STATE_fuelActive:
      if (ACTION == ACT_purge)  {
        if (*previousState == ACT_fill) {
          // Set Fuel Valve Port to RESET
          set_n2oValve(DEACTIVATE_PIN);
          // Set Fuel Purge Port to SET
          set_purge(ACTIVATE_PIN);
        }
        else if (*previousState == ACT_purge | ACT_neutral) {
          // Set Fuel Purge Port to SET
          set_purge(ACTIVATE_PIN);
        }
        *previousAction = ACT_purge;
      }
      else if (ACTION == ACT_fill)  {
        // Set Fuel Purge Port to RESET
        set_purge(DEACTIVATE_PIN);
        // Set Fuel Valve Port to SET
        set_n2oValve(ACTIVATE_PIN);

        *previousAction = ACT_fill;
      }
      else if (ACTION == ACT_neutral) {
        // Set Fuel Valve Port to RESET
        set_n2oValve(DEACTIVATE_PIN);
        // Set Fuel Purge Port to RESET
        set_purge(DEACTIVATE_PIN);

        *previousAction = ACT_neutral;
      }

      *previousState = STATE_fuelActive;
      break;

    case STATE_ignActive:
      if (*previousState == STATE_fuelActive && *previousAction == ACT_fill)  {
        // Set Fuel Valve Port to RESET
        set_n2oValve(DEACTIVATE_PIN);
      }
      else if (*previousState == STATE_fuelActive && *previousAction == ACT_purge)  {
        // Set Fuel Purge Port to RESET
        set_purge(DEACTIVATE_PIN);
      }
      
      if (ACTION == ACT_fire) {
        if (*previousState == STATE_ignActive && *previousAction == ACT_O2)
        {
          // Set O2 Port to RESET
          set_o2Valve(DEACTIVATE_PIN);
        }
        // Set Ignition Relay Port to Set
        set_fire(ACTIVATE_PIN);

        *previousAction = ACT_fire;
      }
      else if (ACTION == ACT_O2)  {
        // Set O2 Port to SET
        set_o2Valve(ACTIVATE_PIN);

        *previousAction = ACT_O2;
      }

      *previousState = STATE_ignActive;
      break;

    case STATE_standby:
      
      break;
  }
}

void setup() {
  pinMode(GPIO_PIN_n2oValve, OUTPUT);
  pinMode(GPIO_PIN_purge, OUTPUT);
  pinMode(GPIO_PIN_o2Valve, OUTPUT);
  pinMode(GPIO_PIN_fire, OUTPUT);

  // Outputting PWM signal, can we use: void analogWrite(uint8_t pin, int value);
  pinMode(GPIO_PIN_firePWM, OUTPUT);


  // Ethernet/TCP Setup 
  Serial.begin(SERIAL_BAUD);

  // Wait for serial to connect
  while (!Serial) {
  } 
  // ^ Should this be here

  Serial.println("\n--- Project Horizon: Ethernet TCP Hex Dumper ---");
  Serial.println("Initializing Ethernet PHY...");

  // Start the Ethernet interface using the specific Olimex pinout
  if (!ETH.begin(ETH_PHY_ADDR, ETH_PHY_POWER, ETH_PHY_MDC, ETH_PHY_MDIO,
                 ETH_PHY_TYPE, ETH_CLK_MODE)) {
    Serial.println("Failed to initialize Ethernet!");
    return;
  }

  // Apply the Static IP settings
  if (!ETH.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println(
        "Warning: Failed to configure Static IP! Falling back to DHCP.");
  } else {
    Serial.println("Static IP configuration applied.");
  }

  // Wait until the board confirms the IP address
  while (ETH.localIP()[0] == 0) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("HIVE Telemetry Node IP: ");
  Serial.println(ETH.localIP());

  Serial.println("Ethernet Started. Waiting for DHCP IP address...");

  // Wait until the board gets a valid IP address
  while (ETH.localIP()[0] == 0) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("HIVE Telemetry Node IP: ");
  Serial.println(ETH.localIP());

  // Start the TCP Server
  server.begin();
  Serial.printf("Listening for TCP packets on port %d...\n", TCP_PORT);
}

void loop() {
  // read TCP packets
  // and send TCP packets?
  // change state based on TCP packet info


   // Check if a client has connected
  WiFiClient client = server.available();

  if (client) {
    Serial.print("\n[New Connection from ");
    Serial.print(client.remoteIP());
    Serial.println("]");

    // Read and process data while the client remains connected
    while (client.connected()) {
      if (client.available()) {
        int bytesAvailable = client.available();
        uint8_t buffer[128];

        // Read up to 128 bytes at a time
        int bytesRead = client.read(buffer, min(bytesAvailable, 128));

        // Print the payload in HEX format
        for (int i = 0; i < bytesRead; i++) {
          Serial.printf("%02X ", buffer[i]);
        }
      }
    }

    // Formatting break when the connection drops
    Serial.println("\n[Connection Closed]");
    client.stop();
}

/*  Questions
Is it best for me to constantly poll for activation signals from the TCP packets in the main loop or should I setup some form of interrupt system. My concern
with this is that it could stop something important from happening if the interrupt is called midway through an important process.


*/