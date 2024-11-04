/*

  LFOptions

  A synthesizer module that generates smoothly changing control voltages at a user-defined pace.

  Design goals:
  - Modes: LFO, stepped random voltages (SRV)
  - Tempo can be regular or normally distributed around a specified pace (range: 0.5x to 2x interval
  - unipolar (0-5v) or bipolar (5vpp) out
  - (optional) a second CV out one octave up, or some other interesting interval

  Controls:
  - Pace: how long it takes to get to the new voltage (knob & CV in)
  - Variablility: how unsteady is the pace (knob and CV in)
  - LFO/SRV (switch)
  - unipolar/bipolar

  Uses Ken Perlin's smootherstep() to move from one value to the next.
  https://www.shadertoy.com/view/MlyBWK
  https://en.wikipedia.org/wiki/Smoothstep#Variations

  This project is inspired by Hagiwo's Bezier curve random CV generator and borrows input and output schematics from it.

  This code is written for an Arduino Nano v3.
*/


const int CVout = 9; // pins 3, 9, 10, and 11 operate at 490Hz PWM frequency
const int tempoin = 0;
const char LFO = 'l';
const char SRV = 's';
char mode = SRV;
float dest_val; // The CV value we're heading toward (smoothstep's edge1)
float curr_val; // The current CV value (smoothstep's x)
float prev_val; // The CV at the start of this transition (smoothstep's edge0)
unsigned long dest_time; // when to arrive at the destination CV
unsigned long curr_time; // now
unsigned long prev_time; // time at the beginning of the current transition
unsigned int duration = 3000; // how long the transition should take, in whichever time units we're using
float sstep;
int gap; // how far from previous destination to the next one
int tempo; 

// precompute smootherstep and store in this lookup table
//const int table_length = 10; // number of samples in lookup table
//float table[table_length]; // a precomputed smoothstep lookup table containing y values

void setup() {
  Serial.begin(9600);

  dest_val = 0;
  curr_val = 0;
  prev_val = 0;
  curr_time = millis();
  prev_time = curr_time;
  dest_time = curr_time + duration;
  

}

// Ken Perlin suggests an improved version of the smoothstep() function,
// which has zero 1st- and 2nd-order derivatives at x = 0 and x = 1.
// Source: https://www.shadertoy.com/view/MlyBWK
float smootherstep(float edge0, float edge1, float x)
{
  x = constrain((x - edge0) / (edge1 - edge0), 0., 1.);
  return x * x * x * (x * (x * 6. - 15.) + 10.);
}


void loop() {

  // TODO: 
  // - option to reset random seed after 4 values
  // - LFO or SRV
  // - regular or random tempo

  curr_time = millis();

  if (curr_val == dest_val) {
    // store destination and set a new one
    prev_val = dest_val;
    prev_time = dest_time;
    dest_val = random(256);
    dest_time = curr_time + duration;
  }

  duration = map(analogRead(tempoin), 0, 1023, 33, 2000);
  dest_time = prev_time + duration;

  sstep = smootherstep(prev_time, dest_time, curr_time);
  gap = dest_val - prev_val;
  curr_val = prev_val + (sstep * gap);

  analogWrite(CVout, curr_val);

  Serial.print("dest_val:");
  Serial.print(dest_val);
  Serial.print(", ");
  Serial.print("curr_val:");
  Serial.print(curr_val);
  Serial.print(", ");
//  Serial.print("gap:");
//  Serial.print(gap);
//  Serial.print(", ");
//  Serial.print("x:");
//  Serial.print(x*100);
//  Serial.print(", ");

  Serial.println();

  //
  //  curr_time = millis();
  //  tail = time & B011;
  //  Serial.print("tail:");
  //  Serial.print(tail);
  //  Serial.println();
}
