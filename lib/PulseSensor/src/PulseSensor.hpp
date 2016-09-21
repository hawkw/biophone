/*
    Class representing the Pulse Sensor Amped v1.4
    http://www.pulsesensor.com

    by Eliza Weisman (http://hawkweisman.me)
    based on the example code by Joel Murphy and Yury Gitman

    @author Eliza Weisman
    @version 0.1
*/

#include "Arduino.h"

#ifndef PSENSOR_ANALOG_PIN
    #define PSENSOR_ANALOG_PIN A0
#endif

#ifndef PSENSOR_TIMER1
    #define PSENSOR_TIMER1 true
#endif

#ifndef PSENSOR_TIMER2
    #define PSENSOR_TIMER2 false
#endif

#if PSENSOR_TIMER1
    #include "TimerOne.h"
#elif PSENSOR_TIMER2
    #include "MsTimer2.h"
#else
    #ifndef PSENSOR_TIMER1
        #error "No PSensor Timer1 defined!"
    #endif
    #ifndef PSENSOR_TIMER2
        #error "No PSensor Timer2 defined!"
    #endif
#endif

/**
 * A pulse sensor callback.
 * @method void
 * @param  Callback [description]
 * @return [description]
 */
typedef void (*Callback)();

template <size_t N_CALLBACKS>
class PulseSensor {
public:
    // /**
    //  * Initializes pulse sensor with no callbacks
    //  */
    // static PulseSensor<N_CALLBACKS> init();
    //
    // /**
    //  * Initializes pulse sensor with N callbacks.
    //  */
    // static PulseSensor<N_CALLBACKS> & init( Callback callbacks[N_CALLBACKS] );

    /**
     * Adds a callback to the pulse sensor.
     * @method add_callback
     * @param  callback [description]
     * @return [description]
     */
    static boolean add_callback( Callback callback );


private:
    volatile Callback callbacks[N_CALLBACKS];
};

void init_interrupts(void);
// #include "PulseSensor.cpp"
