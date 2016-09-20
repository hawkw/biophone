/*
    Class representing the Pulse Sensor Amped v1.4
    http://www.pulsesensor.com

    by Eliza Weisman (http://hawkweisman.me)
    based on the example code by Joel Murphy and Yury Gitman

    @author Eliza Weisman
    @version 0.1
*/
#include "Arduino.h"

template <int AnalogPin>
class PulseSensor {
private:
    //-- make the constructor, etc private ----------------------------------
    PulseSensor<AnalogPin>() {};
    static PulseSensor<AnalogPin> * pSensor;

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
    void setupInterrupt()
    {
        TCCR2A = 0x02; // disable PWM on digital pins 3 and 11, and go into CTC mode
        TCCR2B = 0x06; // don't force compare, 256 prescaler
        OCR2A = 0x7c;  // set the top of the count to 124 for 500hz sample rate
        TIMSK2 = 0x02; // enable IRQ on match between Timer2 and ORC2A;
        sei();         // enable global interrupts
    };

    volatile int n (void) {
        return sampleCounter - lastBeatTime;
    }

public:
    friend void TIMER2_COMPA_vect (void);

    static PulseSensor<AnalogPin> & init (void)
    {
        pSensor = new PulseSensor<AnalogPin>();
        setupInterrupt();
        return pSensor;
    };
    //
    // PulseSensor(PulseSensor &) = delete;
    // void operator=(PulseSensor const&) = delete;

};
