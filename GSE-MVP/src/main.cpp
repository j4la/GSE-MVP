#include <Arduino.h>

// Define PIN numbers for screw terminal outputs
const int GPIO_PIN_n2oValve;
const int GPIO_PIN_purge;
const int GPIO_PIN_o2Valve;

const int GPIO_PIN_fire;
const int GPIO_PIN_firePWM;

bool ACTIVATE_n2oValve;
bool ACTIVATE_purge;
bool ACTIVATE_o2Valve;
bool ACTIVATE_fire;

/* 
States:
Fuel Active
  Neutral
  Fill
  Purge
Standby
Ignition Active
{ Fuel Active Fill
  Fuel Active Purge
  Ignition Active Fire
  Igntion Active O2 }
    O2
    Fire
    Neither
*/

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

void set_n2oValve(bool* activate){
  if (*activate == true)  { // open valve
    digitalWrite(GPIO_PIN_n2oValve, HIGH);
  }
  else  { // close valve
    digitalWrite(GPIO_PIN_n2oValve, LOW);
  }
}

void set_purge(bool* activate){
  if (*activate == true)  {
    digitalWrite(GPIO_PIN_purge, HIGH);
  }
  else  {
     digitalWrite(GPIO_PIN_purge, LOW);
  }
}

void set_o2Valve(bool* activate){
  if (*activate == true)  {
    digitalWrite(GPIO_PIN_o2Valve, HIGH);
  }
  else  {
    digitalWrite(GPIO_PIN_o2Valve, LOW);
  }
}

void set_fire(bool* activate){
  if (*activate == true)  {
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
          // Set Fuel Purge Port to SET
        }
        else if (*previousState == ACT_purge | ACT_neutral) {
          // Set Fuel Purge Port to SET
        }
        *previousAction = ACT_purge;
      }
      else if (ACTION == ACT_fill)  {
        // Set Fuel Purge Port to RESET
        // Set Fuel Valve Port to SET

        *previousAction = ACT_fill;
      }
      else if (ACTION == ACT_neutral) {
        // Set Fuel Valve Port to RESET
        // Set Fuel Purge Port to RESET

        *previousAction = ACT_neutral;
      }

      *previousState = STATE_fuelActive;
      break;

    case STATE_ignActive:
      if (*previousState == STATE_fuelActive && *previousAction == ACT_fill)  {
        // Set Fuel Valve Port to RESET
      }
      else if (*previousState == STATE_fuelActive && *previousAction == ACT_purge)  {
        // Set Fuel Purge Port to RESET
      }
      
      if (ACTION == ACT_fire) {
        if (*previousState == STATE_ignActive && *previousAction == ACT_O2)
        {
          // Set O2 Port to RESET
        }
        // Set Ignition Relay Port to Set

        *previousAction = ACT_fire;
      }
      else if (ACTION == ACT_O2)  {
        // Set O2 Port to SET

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
}

void loop() {
  //
}

/*  Questions
Is it best for me to constantly poll for activation signals from the TCP packets in the main loop or should I setup some form of interrupt system. My concern
with this is that it could stop something important from happening if the interrupt is called midway through an important process.


*/