// Using the Nano 33 IoT IMU Accelerometer to change MIDI Notes via USB
// Install any missing libraries - Tools > Manage Libraries...
// by Billy Bennett, NYU ITP Dec 2020

#include "MIDIUSB.h"
#include "Arduino_LSM6DS3.h"
#include "MadgwickAHRS.h"

// initialize a Madgwick filter:
Madgwick filter;
// sensor's sample rate is fixed at 104 Hz:
const float sensorRate = 104.00;

// values for orientation:
float roll = 0.0;
float pitch = 0.0;
float heading = 0.0;
float prevRoll = 0.0; // for tracking previous values

int note1 = 60; // middle C in MIDI
int note2 = 67; // a fifth up to a G in MIDI
// we use bytes cause that's what MIDI looks for
 
void setup() {
  
  Serial.begin(9600);
  
  accelSetup();
  
  //  Set MIDI baud rate:
  Serial1.begin(31250);
}
 
void loop() {
  accelLoop();
  checkPosition();
}

/////////////////////////////////////////
///////////// ACCELEROMETER STUFF //////
///////////////////////////////////////

void accelSetup() { // set it up
    // attempt to start the IMU:
  if (!IMU.begin()) {
  Serial.println("Failed to initialize IMU");
  // stop here if you can't access the IMU:
  while (true);
  }
  
  // start the filter to run at the sample rate:
  filter.begin(sensorRate);
}

void accelLoop() { // put in loop to get the values from the IMU

  float xAcc, yAcc, zAcc;
  float xGyro, yGyro, zGyro;

  // check if the IMU is ready to read:
  if (IMU.accelerationAvailable() && IMU.gyroscopeAvailable()) {
    // read accelerometer and gyrometer:
    IMU.readAcceleration(xAcc, yAcc, zAcc);
    IMU.readGyroscope(xGyro, yGyro, zGyro);

    // update the filter, which computes orientation:
    filter.updateIMU(xGyro, yGyro, zGyro, xAcc, yAcc, zAcc);

    roll = filter.getRoll();
    pitch = filter.getPitch();
    heading = filter.getYaw();
  }
}

void checkPosition() {  // watch to see if roll is moving one way or the other
  
  if(roll > 0 && prevRoll <= 0) {
    Serial.println("going one way");
    noteOn(note1);
    noteOff(note2);
  } else if (roll < 0 && prevRoll >= 0) {
    Serial.println("going the other way");
    noteOn(note2);  
    noteOff(note1);  
  }
  prevRoll = roll;
}

/////////////////////////////////////////
///////////// SEND MIDI NOTE ON ////////
///////////////////////////////////////

void noteOn(byte midiNote) {

  byte chan = 0x90; // channel 1 (0x90)
  byte note = midiNote; // the note 0-127
  byte vel = 0x45; // a middle value
  
 /* First parameter is the event type (top 4 bits of the command byte).
  Second parameter is command byte combined with the channel.
  Third parameter is the first data byte
  Fourth parameter second data byte, if there is one: */
    
  midiEventPacket_t midiMsg = {chan >> 4, chan, note, vel};
  MidiUSB.sendMIDI(midiMsg);
}

/////////////////////////////////////////
///////////// SEND MIDI NOTE OFF ///////
///////////////////////////////////////

void noteOff(byte midiNote) {

  byte chan = 0x90; // channel 1 (0x90)
  byte note = midiNote; // the note 0-127
  byte vel = 0x00; // zero value
  
 /* First parameter is the event type (top 4 bits of the command byte).
  Second parameter is command byte combined with the channel.
  Third parameter is the first data byte
  Fourth parameter second data byte, if there is one: */
    
  midiEventPacket_t midiMsg = {chan >> 4, chan, note, vel};
  MidiUSB.sendMIDI(midiMsg);
}
