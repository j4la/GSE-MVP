#include <Arduino.h>

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
}

void loop() {
  // read TCP packets
  // and send TCP packets?
  // change state based on TCP packet info
}

/*  Questions
Is it best for me to constantly poll for activation signals from the TCP packets in the main loop or should I setup some form of interrupt system. My concern
with this is that it could stop something important from happening if the interrupt is called midway through an important process.


*/