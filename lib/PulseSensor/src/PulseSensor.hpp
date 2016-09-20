/*
    Class representing the Pulse Sensor Amped v1.4
    http://www.pulsesensor.com

    by Eliza Weisman (http://hawkweisman.me)
    based on the example code by Joel Murphy and Yury Gitman

    @author Eliza Weisman
    @version 0.1
*/
#include "Arduino.h"

enum PSensorTimer { T1
                  , T2
                  };

template <int AnalogPin, PSensorTimer Timer>
class PulseSensor {
private:
    //-- make the constructor, etc private ----------------------------------
    PulseSensor<AnalogPin, Timer>() {};

    //-- volatile variables used in ISR -------------------------------------
    volatile int analogPin = AnalogPin //!< analog in pin
               , signal    //!< incoming raw data
               , bpm       //!< raw analog in. updated every 2ms
               , ibi = 600 //!< interval between beats. must be seeded to 600.
               , p = 512   //!< used to find peak in pulse wave. seeded.
               , t = 512   //!< used to find trough in pulse wave, seeded.
               , threshold = 525 //!< used to find instant moment of heartbeat, seeded
               , amp = 100 //!< amplitude of pulse waveform, seeded.
               ;

    volatile boolean pulse = false //!< true when a "live" heartbeat is detected
                   , qs = false //!< becomes true when we've found a beat
                   , firstBeat = true //!< used to seed beat array so we start with sane BPM
                   , secondBeat = false //!< used to seed beat array
                   ;

    volatile int rate[10]; //!< array to hold last ten interval values

    volatile unsigned long sampleCounter = 0
                         , lastBeatTime = 0 //!< used to find IBI
                         ;

    /**
     * Initializes Timer 2 to throw an interrupt every 2ms
     */
    void setupInterrupt(void);

    volatile int n (void) {
        return sampleCounter - lastBeatTime;
    }

public:

    /**
     * Returns the pulse sensor instance for this AnalogPin
     */
    static PulseSensor<AnalogPin, Timer> & getInstance (void)
    {
        static PulseSensor pSensor;
        pSensor.setupInterrupt();
        return pSensor;
    };

    PulseSensor(PulseSensor &) = delete;
    void operator=(PulseSensor const&) = delete;

};

template <int AnalogPin>
class PulseSensor<AnalogPin, PSensorTimer::T1> {
public:
    /**
     * Interrupt triggered by the timer.
     *
     * The `TIMER1_COMPA` interrupt must be declared as a friend function
     * in order to access private members of the PulseSensor class.
     *
     * @method TIMER1_COMPA_vect
     */
    friend void TIMER1_COMPA_vect (void);

    /**
     * Returns the pulse sensor instance for this AnalogPin
     */
    static PulseSensor<AnalogPin, PSensorTimer::T1> & getInstance (void)
    {
        static PulseSensor pSensor;
        pSensor.setupInterrupt();
        return pSensor;
    };
private:
    void setupInterrupt(void) {
        // Initializes Timer1 to throw an interrupt every 2mS.
        TCCR1A = 0x00; // DISABLE OUTPUTS AND PWM ON DIGITAL PINS 9 & 10
        TCCR1B = 0x11; // GO INTO 'PHASE AND FREQUENCY CORRECT' MODE, NO PRESCALER
        TCCR1C = 0x00; // DON'T FORCE COMPARE
        TIMSK1 = 0x01; // ENABLE OVERFLOW INTERRUPT (TOIE1)
        ICR1 = 16000;  // TRIGGER TIMER INTERRUPT EVERY 2mS
        sei();         // MAKE SURE GLOBAL INTERRUPTS ARE ENABLED
    }
};

template <int AnalogPin>
class PulseSensor<AnalogPin, PSensorTimer::T2> {
private:

    void setupInterrupt(void) {
        // Initializes Timer2 to throw an interrupt every 2mS.
        TCCR2A = 0x02;     // DISABLE PWM ON DIGITAL PINS 3 AND 11, AND GO INTO CTC MODE
        TCCR2B = 0x06;     // DON'T FORCE COMPARE, 256 PRESCALER
        OCR2A = 0X7C;      // SET THE TOP OF THE COUNT TO 124 FOR 500Hz SAMPLE RATE
        TIMSK2 = 0x02;     // ENABLE INTERRUPT ON MATCH BETWEEN TIMER2 AND OCR2A
        sei();             // MAKE SURE GLOBAL INTERRUPTS ARE ENABLED
    };

public:
    /**
     * Interrupt triggered by the timer.
     *
     * The `TIMER2_COMPA` interrupt must be declared as a friend function
     * in order to access private members of the PulseSensor class.
     *
     * @method TIMER2_COMPA_vect
     */
    friend void TIMER2_COMPA_vect (void);

    /**
     * Returns the pulse sensor instance for this AnalogPin
     */
    static PulseSensor<AnalogPin, PSensorTimer::T2> & getInstance (void)
    {
        static PulseSensor pSensor;
        pSensor.setupInterrupt();
        return pSensor;
    };
};
