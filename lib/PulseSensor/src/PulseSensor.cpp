#include "Arduino.h"
#include "PulseSensor.hpp"

PulseSensor pulseSensor;

void _isr_wrapper() {
    pulseSensor.read(); // aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
}

void PulseSensor::init_interrupts() {
    #if (PSENSOR_TIMER1)
        Timer1.initialize(_2MS_MICROSECONDS);
        Timer1.attachInterrupt(_isr_wrapper);
        Timer1.start();
    #elif (PSENSOR_TIMER2)
        MsTimer2::set(_2MS_MILLISECONDS, _isr_wrapper);
        MsTimer2::start();
    #else
        #error Must define a timer for the pulse sensor!
    #endif
}
