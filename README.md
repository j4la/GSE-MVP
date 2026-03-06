# GSE-MVP

The purpose of this firmware is to provide an interface between TCP packets and the activation of various components responsible for the combustion system of the rocket.

The ESP32-PoE-ISO is used, with an onboard Ethernet port. The Ethernet port allows communication through the WiFi bridge using TCP. The WiFi bridge receives packets from Ground Control.

Based on the packets received the program will activate/deactivate various pins/ports.

# Connected pins:
* N2O Fill:
    This activates the solenoid that releases N2O into the combustion chamber.

* Purge:
    This activates the solenoids that empty the entire combustion system.

* O2 Fill:
    This activates the solenoid that releases O2 into the combustion chamber.

* Fire:
    This activates the igniter, where activating this pin allows 12V to flow through toward the igniter.

* Fire PWM:
    The PWM signal is to activate the sparker.