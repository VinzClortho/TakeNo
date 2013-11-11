#include <Tone.h>
#include <Servo.h>
#include <avr/pgmspace.h>

#include "takeno.h"

Tone bass;
Tone treble;
Tone treble2;

Servo tomA, tomB;
Servo cymA, cymB;

unsigned int counter;
int array2Index, array2Max;
int array3Index, array3Max;
int array4Index, array4Max;
int array5Index, array5Max;

int pinI1=8;      //define I1 interface
int pinI2=11;    //define I2 interface 
int speedpinA=9;  //enable motor A

int pinI3=12;    //define I3 interface
int pinI4=13;    //define I4 interface 
int speedpinB=10;  //enable motor B

int queue [8] = {-1, -1, -1, -1, -1};

const int snarePin = 22;

boolean hold;
int tomHold = 0;
int cymHold = 0;

int value;

void setup(){
  bass.begin(24);
  treble.begin(2);
  treble2.begin(23);
  
  tomA.attach(5);
  tomB.attach(3);
  cymA.attach(6);
  cymB.attach(7);
  
  tomRelease();
  cymRelease();

  pinMode(pinI1,OUTPUT);
  pinMode(pinI2,OUTPUT);
  pinMode(speedpinA,OUTPUT);
  
  pinMode(pinI3,OUTPUT);
  pinMode(pinI4,OUTPUT);
  pinMode(speedpinB,OUTPUT);

  pinMode(snarePin, OUTPUT);
  
  counter = 0;
  array2Index = 0;
  array3Index = 0;
  array4Index = 0;
  array5Index = 0;
  array2Max = sizeof(array2) / sizeof(int);
  array3Max = sizeof(array3) / sizeof(int);  
  array4Max = sizeof(array4) / sizeof(int);  
  array5Max = sizeof(array5) / sizeof(int);  
  
}


void loop(){
  hold = false;

  // play and unload the delayed parts  
  if(queue[2] > -1){
    if(queue[2] > 0)
      treble.play(queue[2]);
    else 
      treble.stop();
    queue[2] = -1;
  }
  
  if(queue[3] > -1){
    if(queue[3] > 0)
      treble2.play(queue[3]);
    else 
      treble2.stop();
    queue[3] = -1;
  }
  
  if(queue[4] > -1){
    if(queue[4] > 0)
      bass.play(queue[4]);
    else 
      bass.stop();
    queue[4] = -1;
  }
  
  if(queue[5] == 1){
    kickHit();
    hold = true;
    queue[5] = -1;
  }
  
  if(queue[6] == 1){
    hatHit();
    hold = true;
    queue[6] = -1;    
  }
  
  if(queue[7] == 1){
    snareHit();
    hold = true;
    queue[7] = -1;    
  }
    
  if(array2Index < array2Max && pgm_read_word_near(array2 + array2Index) <= counter){
    array2Index++;
    value = pgm_read_word_near(array2 + array2Index);    
    if(value > 0)
      queue[2] = value * 2;
    else 
      queue[2] = 0;
    array2Index++;
  }
  
  if(array3Index < array3Max && pgm_read_word_near(array3 + array3Index) <= counter){
    array3Index++;
    value = pgm_read_word_near(array3 + array3Index);    
    if(value > 0)
      queue[3] = value * 2;
    else 
      queue[3] = 0;
    array3Index++;
  }
  
  if(array4Index < array4Max && pgm_read_word_near(array4 + array4Index) <= counter){
    array4Index++;
    value = pgm_read_word_near(array4 + array4Index);    
    if(value > 0)
      queue[4] = value;
    else 
      queue[4] = 0;
    array4Index++;
  }

  while(array5Index < array5Max && pgm_read_word_near(array5 + array5Index) <= counter){
    array5Index++;

    // filter MIDI drum notes to the appropriate action
    switch(pgm_read_word_near(array5 + array5Index)){
      case 35:    // kick
      case 36:
      case 41:
        queue[5] = 1;
        break;  
      case 43:   // tom
      case 45:
      case 48:
      case 50:
        tomHit();
        tomHold = 2; // servo hold duration
        break;
      case 42:    // hi-hat
      case 44:
      case 46:
      case 53:
      case 54:
        queue[6] = 1;
        break;
      case 49:    // cymbal
      case 52:
      case 55:
      case 57:
      case 59:
        cymHit();
        cymHold = 2;  // servo hold duration
        break;
      case 37:
      case 38:     // snare
      case 39:
      case 40:
      
        queue[7] = 1;
        break;
      default:
        break;
    }
    
    array5Index++;
  }

  if(counter < 51000) counter += 8;    // if not at end of song yet, increment timing counter
  else {
    treble.stop();
    treble2.stop();
    bass.stop();
  }

  delay(30);      // timing delay
  if(hold){      // if a drum has just been played, release it
    kickRelease();
    snareRelease();
    hatRelease();
  }
  
  // decrement servo hold counters and release when zeroed
  if(tomHold > 0){
    tomHold--;
    if(tomHold == 0) tomRelease();
  }
  
  if(cymHold > 0){
    cymHold--;
    if(cymHold == 0) cymRelease();
  }
  
}

inline
void kickHit(){
  analogWrite(speedpinA, 255);    // set speed to full
  digitalWrite(pinI2,LOW);        // push solenoid
  digitalWrite(pinI1,HIGH);
}

inline
void kickRelease(){
  analogWrite(speedpinA, 0);    // set speed to 0
  digitalWrite(pinI2,HIGH);    // release solenoid
  digitalWrite(pinI1,LOW);
  digitalWrite(pinI1,HIGH);
}  

inline
void hatHit(){
  analogWrite(speedpinB, 255);   // set speed to full
  digitalWrite(pinI4,LOW);      // pull solenoid
  digitalWrite(pinI3,HIGH);
}

inline
void hatRelease(){
  analogWrite(speedpinB, 0);     // set speed to 0
  digitalWrite(pinI4,HIGH);     // release solenoid
  digitalWrite(pinI3,LOW);
  digitalWrite(pinI3,HIGH);
}  

inline 
void snareHit(){
  digitalWrite(snarePin, HIGH);
}

inline
void snareRelease(){
  digitalWrite(snarePin, LOW);
}  

inline
void tomHit(){
   tomA.write(80); 
   tomB.write(140);

}

inline
void tomRelease(){
   tomA.write(60);
   tomB.write(120);
}

inline
void cymHit(){
   cymA.write(20); 
   cymB.write(150);

}

inline
void cymRelease(){
   cymA.write(40);
   cymB.write(180);
}


