#include "Arduino.h"
#include "PulseSensor.hpp"

//-- volatile variables used in ISR -------------------------------------
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
