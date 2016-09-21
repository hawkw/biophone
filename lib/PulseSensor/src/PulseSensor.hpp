/*
    Class representing the Pulse Sensor Amped v1.4
    http://www.pulsesensor.com

    by Eliza Weisman (http://hawkweisman.me)
    based on the example code by Joel Murphy and Yury Gitman

    @author Eliza Weisman
    @version 0.1
*/
#ifndef PulseSensor_hpp_
#define PulseSensor_hpp_

#if defined(ARDUINO) && ARDUINO >= 100
    #include "Arduino.h"
#else
    #include "WProgram.h"
#endif

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
    #define _2MS_MICROSECONDS 2000
#elif PSENSOR_TIMER2
    #include "MsTimer2.h"
    #define _2MS_MILLISECONDS 2
#else
    #ifndef PSENSOR_TIMER1
        #error "No PSensor Timer1 defined!"
    #endif
    #ifndef PSENSOR_TIMER2
        #error "No PSensor Timer2 defined!"
    #endif
#endif

#ifndef PSENSOR_MAX_CALLBACKS
    #define PSENSOR_MAX_CALLBACKS 5
#endif



/**
 * A pulse sensor callback.
 * @method void
 * @param  Callback [description]
 * @return [description]
 */
typedef void (*Callback)();

class PulseSensor {
public:
    /**
     * Initializes pulse sensor with no callbacks
     */
    PulseSensor & init() {

    }

    /**
     * Initializes pulse sensor with N callbacks.
     */
    PulseSensor & init( Callback callbacks[PSENSOR_MAX_CALLBACKS] );

    /**
     * Adds a callback to the pulse sensor.
     * @method add_callback
     * @param  callback [description]
     * @return [description]
     */
    PulseSensor & add_callback( Callback callback );

    friend void _isr_wrapper();

private:

    //-- volatile variables used in ISR -------------------------------------
    //! Array of callback functions used by the ISR
    volatile Callback callbacks[PSENSOR_MAX_CALLBACKS];
    volatile int signal    //!< incoming raw data
               , bpm       //!< raw analog in. updated every 2ms
               , ibi = 600 //!< interval between beats. must be seeded to 600.
               , p = 512   //!< used to find peak in pulse wave. seeded.
               , t = 512   //!< used to find trough in pulse wave, seeded.
               , threshold = 525 //!< used to find instant moment of heartbeat
               , amp = 100 //!< amplitude of pulse waveform, seeded.
               ;


    volatile boolean pulse = false //!< true when a "live" heartbeat is detected
                   , qs = false //!< becomes true when we've found a beat
                   , firstBeat = true //!< used to seed beat array
                   , secondBeat = false //!< used to seed beat array
                   ;

    volatile int rate[10]; //!< array to hold last ten interval values

    volatile unsigned long sampleCounter = 0
                         , lastBeatTime = 0 //!< used to find ibi
                         ;

    void read (void) {
        signal = analogRead(PSENSOR_ANALOG_PIN);   // read the Pulse Sensor
        sampleCounter += 2;  // keep track of the time in mS with this variable
        int n = sampleCounter - lastBeatTime;     // monitor the time since the last beat to avoid noise

        //  find the peak and trough of the pulse wave
        if (signal < threshold && n > (ibi/5)*3) {
            // avoid dichrotic noise by waiting 3/5 of last IBI
            if (signal < t){                        // t is the trough
                t = signal; // keep track of lowest point in pulse wave
            }
        }

        if (signal > threshold && signal > p) {
            // threshold condition helps avoid noise
            p = signal; // P is the peak
        }               // keep track of highest point in pulse wave

        //  NOW It'S tIME tO LOOK FOR tHE HEARt BEAt
        // signal surges up in value every time there is a pulse
        if (n > 250) { // avoid high frequency noise

            if ( (signal > threshold) && (pulse == false) && (n > (ibi/5)*3) ) {
                pulse = true; // set the Pulse flag when we think there is a pulse
                ibi = sampleCounter - lastBeatTime;  // measure time between beats in mS
                lastBeatTime = sampleCounter; // keep track of time for next pulse

                if (secondBeat) {  // if this is the second beat, if secondBeat == TRUE
                    secondBeat = false; // clear secondBeat flag
                    for (int i=0; i<=9; i++) { // seed the running total to get a realisitic bpm at startup
                        rate[i] = ibi;
                    }
                }

                if (firstBeat) {
                    // if it's the first time we found a beat, if firstBeat == TRUE
                    firstBeat = false; // clear firstBeat flag
                    secondBeat = true;  // set the second beat flag
                    //  sei();          // enable interrupts again
                    return;             // ibi value is unreliable so discard it
                }

                // keep a running total of the last 10 ibi values
                word runningtotal = 0;       // clear the runningtotal variable

                for (int i=0; i<=8; i++) {    // shift data in the rate array
                    rate[i] = rate[i+1];      // and drop the oldest IBI value
                    runningtotal += rate[i];  // add up the 9 oldest IBI values
                }

               rate[9] = ibi;               // add the latest IBI to the rate array
               runningtotal += rate[9];     // add the latest IBI to runningtotal
               runningtotal /= 10;          // average the last 10 IBI values
               bpm = 60000/runningtotal;    // how many beats can fit into a minute? that's bpm!
               qs = true;                   // set Quantified Self flag
               // QS FLAG IS NOt CLEARED INSIDE THIS ISR
            }
        }

        // when the values are going down, the beat is over
        if (signal < threshold && pulse == true) {
            pulse = false;         // reset the Pulse flag so we can do it again
            amp = p - t;           // get amplitude of the pulse wave
            threshold = amp/2 + t; // set threshold at 50% of the amplitude
            p = threshold;         // reset these for next time
            t = threshold;
        }

        if (n > 2500) {  // if 2.5 seconds go by without a beat
            threshold = 512;              // set threshold default
            p = 512;                      // set P default
            t = 512;                      // set t default
            lastBeatTime = sampleCounter; // bring the lastBeatTime up to date
            firstBeat = true;             // set these to avoid noise
            secondBeat = false;           // when we get the heartbeat back
        }
        return;
    };

    void init_interrupts();

};

extern PulseSensor pulseSensor;
#endif
