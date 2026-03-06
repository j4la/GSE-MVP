# GSE-MVP

The program functions by intaking TCP packets from the attached ethernet port and activating/deactivating GPIO ports.

# Connected pins:
* N2O Fill
    This activates the solenoid that releases N2O into the combustion chamber.

* Purge
    This activates the solenoids that empty the entire combustion system.

* O2 Fill
    This activates the solenoid that releases O2 into the combustion chamber.

* Fire
    This activates the igniter, where activating this pin allows 12V to flow through toward the igniter.

* Fire PWM
    The PWM signal is to activate the sparker.