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
const int sw_mode = 2; // pin D2 to switch between LFO and smooth random voltages
const int sw_tempo = 3; // pin D3 to switch between a regular, unvarying tempo and a variable one
const int pot_tempo = A0; // pin A0 connceted to the wiper of the tempo pot. ccw connected to +5v, cw connected to ground.
const char LFO = 'l';
const char SRV = 's';
char mode = SRV;
const char REG = 'r';
const char VAR = 'v';
char tempo = REG;
const int tempo_vary_length = 128;
float tempo_vary[tempo_vary_length] = {0.50, 0.50, 0.50, 0.50, 0.50, 0.50, 0.50, 0.50, 0.50, 0.50, 0.51, 0.51, 0.51, 0.51, 0.52, 0.52, 0.52, 0.53, 0.53, 0.54, 0.54, 0.55, 0.56, 0.57, 0.57, 0.58, 0.59, 0.60, 0.61, 0.62, 0.63, 0.64, 0.66, 0.67, 0.68, 0.69, 0.71, 0.72, 0.74, 0.75, 0.77, 0.79, 0.80, 0.82, 0.84, 0.86, 0.87, 0.89, 0.91, 0.93, 0.95, 0.97, 0.99, 1.01, 1.03, 1.05, 1.08, 1.10, 1.12, 1.14, 1.16, 1.18, 1.21, 1.23, 1.25, 1.27, 1.29, 1.32, 1.34, 1.36, 1.38, 1.40, 1.42, 1.45, 1.47, 1.49, 1.51, 1.53, 1.55, 1.57, 1.59, 1.61, 1.63, 1.64, 1.66, 1.68, 1.70, 1.71, 1.73, 1.75, 1.76, 1.78, 1.79, 1.81, 1.82, 1.83, 1.84, 1.86, 1.87, 1.88, 1.89, 1.90, 1.91, 1.92, 1.93, 1.93, 1.94, 1.95, 1.96, 1.96, 1.97, 1.97, 1.98, 1.98, 1.98, 1.99, 1.99, 1.99, 1.99, 2.00, 2.00, 2.00, 2.00, 2.00, 2.00, 2.00, 2.00, 2.00}; // a table containing values for varying the tempo of changes
float dest_val; // The CV value we're heading toward (smoothstep's edge1)
float curr_val; // The current CV value (smoothstep's x)
float prev_val; // The CV at the start of this transition (smoothstep's edge0)
unsigned long dest_time; // when to arrive at the destination CV
unsigned long curr_time; // now
unsigned long prev_time; // time at the beginning of the current transition
unsigned int duration = 1000; // how long the transition should take, in whichever time units we're using
float sstep;
int gap; // how far from previous destination to the next one


// precompute smootherstep and store in this lookup table
//const int table_length = 10; // number of samples in lookup table
//float table[table_length]; // a precomputed smoothstep lookup table containing y values

void setup() {
  Serial.begin(9600);
  pinMode(sw_mode, INPUT_PULLUP);
  pinMode(sw_tempo, INPUT_PULLUP);

  dest_val = 0;
  curr_val = 0;
  prev_val = 0;
  curr_time = millis();
  prev_time = curr_time;
  dest_time = curr_time + duration;

  if (digitalRead(sw_mode) == HIGH) {
    mode = LFO;
  } else {
    mode = SRV;
  }
  if (digitalRead(sw_tempo) == HIGH) {
    tempo = REG;
  } else {
    tempo = VAR;
  }

  for (int i = 0; i < tempo_vary_length; i++) {
    tempo_vary[i] = smootherstep( 0.0, 1.0, float(i) / tempo_vary_length) * 1.5 + 0.5;
    // Serial.print(tempo_vary[i]); Serial.print(", ");
  }
  //Serial.println();
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
  // - regular or random tempo

  if (digitalRead(sw_mode) == HIGH) {
    mode = LFO;
  } else {
    mode = SRV;
  }
  if (digitalRead(sw_tempo) == HIGH) {
    tempo = REG;
  } else {
    tempo = VAR;
  }

  curr_time = millis();

  if (curr_val == dest_val) {
    // we're done with a transition. store the destination and set a new one
    prev_val = dest_val;
    prev_time = dest_time;
    if (mode == SRV) {
      dest_val = random(256);
    } else {
      if (dest_val == 0) {
        dest_val = 255;
      } else {
        dest_val = 0;
      }
    }
    dest_time = curr_time + duration;
  }

  duration = map(analogRead(pot_tempo), 0, 1023, 33, 10000);
  if (tempo == VAR) {
    duration *= tempo_vary[random(128)];
  }
  dest_time = prev_time + duration;

  sstep = smootherstep(prev_time, dest_time, curr_time);
  gap = dest_val - prev_val;
  curr_val = prev_val + (sstep * gap);

  analogWrite(CVout, curr_val);

  //Serial.print("dest_val:"); Serial.print(dest_val); Serial.print(", ");
  //Serial.print("curr_val:"); Serial.print(curr_val); Serial.print(", ");
  //Serial.print("duration:"); Serial.print(duration); Serial.print(", ");
  //Serial.println();
}
