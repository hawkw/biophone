/**
 * Biophone
 * by Eliza Weisman
 */

#include "Arduino.h"
#include <PulseSensor.hpp>

PulseSensor<A0> heartbeat;

void setup () {
    heartbeat = PulseSensor<A0>::getInstance();
}

void loop () {

}
