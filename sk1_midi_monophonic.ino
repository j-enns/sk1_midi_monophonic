#include "key_matrix.h"

#define portOfPin(P)\
  (((P)>=0&&(P)<8)?&PORTD:(((P)>7&&(P)<14)?&PORTB:&PORTC))
#define ddrOfPin(P)\
  (((P)>=0&&(P)<8)?&DDRD:(((P)>7&&(P)<14)?&DDRB:&DDRC))
#define pinOfPin(P)\
  (((P)>=0&&(P)<8)?&PIND:(((P)>7&&(P)<14)?&PINB:&PINC))
#define pinIndex(P)((uint8_t)(P>13?P-14:P&7))
#define pinMask(P)((uint8_t)(1<<pinIndex(P)))

#define pinAsInput(P) *(ddrOfPin(P))&=~pinMask(P)                         // pinMode( pin, INPUT ); with pinAsInput( pin );
#define pinAsInputPullUp(P) *(ddrOfPin(P))&=~pinMask(P);digitalHigh(P)    // pinMode( pin, INPUT_PULLUP); with pinAsInputPullUp( pin );
#define pinAsOutput(P) *(ddrOfPin(P))|=pinMask(P)                         // pinMode( pin, OUTPUT ); with pinAsOutput( pin );
#define digitalLow(P) *(portOfPin(P))&=~pinMask(P)                        // digitalWrite( pin, LOW ); with digitalLow( pin );
#define digitalHigh(P) *(portOfPin(P))|=pinMask(P)                        // digitalWrite( pin, HIGH ); with digitalHigh( pin );
#define isHigh(P)((*(pinOfPin(P))& pinMask(P))>0)                         // 
#define isLow(P)((*(pinOfPin(P))& pinMask(P))==0)                         // 
#define digitalState(P)((uint8_t)isHigh(P))                               // digitalRead( pin ); with digitalState( pin );

byte midiIN = 0x00;
byte KO = 0;
byte KI = 0;
byte midiSTATUS = 0x00; // keep track of running status store 0x80 to 0xEF, clear 0xF0 to 0xF7, status unchanged for 0xF8 to 0xFE
byte midiDATA [ 2 ] = { 0x00, 0x00 }; // the real information.
byte readDATA = 0; // the number of data bits to read/trash

byte midiNOTE = 0x00;
const byte midiNOTEon = 0x90; // followed by note, then velocity. if velocity 0x00 note off
const byte midiNOTEoff = 0x80; // followed by note, then velocity. ignore velocity

void setup() {
  // put your setup code here, to run once:

  Serial.begin( 31250 );

  pinAsOutput( 12 ); // KI 1
  digitalLow( 12 );
  pinAsOutput( 13 ); // KI 2
  digitalLow( 13 );
  pinAsOutput( A0 ); // KI 3
  digitalLow( A0 );
  pinAsOutput( A1 ); // KI 4
  digitalLow( A1 );
  pinAsOutput( A2 ); // KI 5
  digitalLow( A2 );
  pinAsOutput( A3 ); // KI 6
  digitalLow( A3 );
  pinAsOutput( A4 ); // KI 7
  digitalLow( A4 );
  pinAsOutput( A5 ); // KI 8
  digitalLow( A5 );

  for ( byte i = 2; i < 12; i++ ) {
    pinAsInput( i ); // KO 0-9
  }
}

void loop () {
  if ( Serial.available() > 0 ) {
    midiIN = Serial.read();

    if ( ( midiIN == midiNOTEon ) || ( midiIN == midiNOTEoff ) ) {
      readDATA = 2;
      midiSTATUS = midiIN;
      //midiIN = 0x00;
    }
    else if ( readDATA >= 1 ) {
      midiDATA[ --readDATA ] = midiIN;
      
      //if ( ( readDATA == 0 ) && ( midiIN >= NOTE_F3 ) && ( midiIN <= NOTE_C6 ) )
    }
  } // serial available

  if ( ( ( midiSTATUS == midiNOTEon ) || ( midiSTATUS == midiNOTEoff ) ) && ( readDATA == 0 ) ) {
    if ( ( midiSTATUS == midiNOTEoff ) || ( ( midiSTATUS == midiNOTEon ) && ( midiDATA[ 0 ] == 0x00 ) ) ) {
      stopNote ( midiDATA[ 1 ] );
    }
    else if ( ( midiSTATUS == midiNOTEon ) && ( ( midiDATA[ 1 ] >= NOTE_F3 ) && ( midiDATA[ 1 ] <= NOTE_C6 ) ) ) {
      startNote( midiDATA[ 1 ] );
    }
    midiSTATUS = 0x00;
  } // if midiSTATUS

  if ( isHigh( KO ) ) {
    digitalHigh( KI );
    delayMicroseconds( 75 );
    digitalLow( KI );
  } // isHigh

} // loop 


void startNote ( byte note ) {
  midiNOTE = note;
  switch ( note ) {
    case NOTE_F3: case NOTE_F3s: case NOTE_G3: case NOTE_G3s:
      KO = 10; // KO 5
      break;
    case NOTE_A3: case NOTE_A3s: case NOTE_B3: case NOTE_C4:
      KO = 9; // KO 6
      break;
    case NOTE_C4s: case NOTE_D4: case NOTE_D4s: case NOTE_E4:
      KO = 8; 
      break;
    case NOTE_F4: case NOTE_F4s: case NOTE_G4: case NOTE_G4s:
      KO = 7; // KO 5
      break;
    case NOTE_A4: case NOTE_A4s: case NOTE_B4: case NOTE_C5:
      KO = 6; // KO 6
      break;
    case NOTE_C5s: case NOTE_D5: case NOTE_D5s: case NOTE_E5:
      KO = 5; 
      break;
    case NOTE_F5: case NOTE_F5s: case NOTE_G5: case NOTE_G5s:
      KO = 4;
      break;
    case NOTE_A5: case NOTE_A5s: case NOTE_B5: case NOTE_C6:
      KO = 3;
      break;
  }

  switch ( note ) {
    case NOTE_F3: case NOTE_A3: case NOTE_C4s: case NOTE_F4: case NOTE_A4: case NOTE_C5s: case NOTE_F5: case NOTE_A5:
      KI = A2; // KI 5
      break;
    case NOTE_F3s: case NOTE_A3s: case NOTE_D4: case NOTE_F4s: case NOTE_A4s: case NOTE_D5: case NOTE_F5s: case NOTE_A5s:
      KI = A3; // KI 6
      break;
    case NOTE_G3: case NOTE_B3: case NOTE_D4s: case NOTE_G4: case NOTE_B4: case NOTE_D5s: case NOTE_G5: case NOTE_B5:
      KI = A4; // KI 7
      break;
    case NOTE_G3s: case NOTE_C4: case NOTE_E4: case NOTE_G4s: case NOTE_C5: case NOTE_E5: case NOTE_G5s: case NOTE_C6:
      KI = A5; // KI 8
      break;
  }
} // startNote


void stopNote ( byte note ) {
  if ( note == midiNOTE ) {
    KO = 0;
    KI = 0;
  }
} // stopNote
