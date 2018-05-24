// Cognex-Linear-Scanner
// Copyright (c) 2018, Peter Walecki, Hongyi Fan and Gabriel Taubin
// All rights reserved.

#include <SoftwareSerial.h>

// Cognex InSight 7802 Camera

const int COGNEX_HSOUT_0 = A3;
const int COGNEX_HSOUT_1 = A2;
const int COGNEX_INOUT_3 = A1;
const int COGNEX_INOUT_2 = A0;
const int COGNEX_RX       = 8;
const int COGNEX_TX       = 9;

int nFrame = 0;
//
// A4988 stepper driver pin assignments
//
const int MS1            = 7;
const int MS2            = 6;
const int MS3            = 5;
const int RESET_         = 4;
const int STEP           = A4;
const int DIR            = A5;

// unused
// D11,D10,A7,A6,D12

const int LIMIT_SWITCH_POS = 2; // positive step direction
const int LIMIT_SWITCH_NEG = 3; // negative step direction

//
// LED+LASER PIN ASSIGNMENTS
//
const int LED    = 13;
const int LASER  = 13;

String motorDriver       = "A4988";
String motorType         = "39SHD0611-15/12V/0.4A";
int    halfDelayStep     =   1000; // HDS 
int    limit_prev        =      0;
int    limit_curr        =      0;
float  mmPerFrame        =  0.25f; // mm in LINEAR mode; degrees in ROTARY mode
float  stepsPerMM        =    100; // MS16 steps
bool   verbose           =   false;

volatile bool limitPositive     = false;
volatile bool limitNegative     = false;
volatile int  direction_ = 1;

SoftwareSerial cognexSerial(COGNEX_RX,COGNEX_TX);

//////////////////////////////////////////////////////////////////////
void setup() {

  // Serial.begin(9600); //USB serial comms
  Serial.begin(115200); //USB serial comms

  // cognexSerial.begin(9600);
  cognexSerial.begin(9600);

  // limit switches

  pinMode(LIMIT_SWITCH_POS, INPUT_PULLUP); 
  delay(10);
  attachInterrupt(digitalPinToInterrupt(LIMIT_SWITCH_POS),
                  limitSwitchPositiveDirection,CHANGE);

  pinMode(LIMIT_SWITCH_NEG, INPUT_PULLUP); 
  delay(10);
  attachInterrupt(digitalPinToInterrupt(LIMIT_SWITCH_NEG),
                  limitSwitchNagativeDirection,CHANGE);

  // Stepper motor controller pin setup
  pinMode(MS1, OUTPUT);
  pinMode(MS2, OUTPUT);
  pinMode(MS3, OUTPUT);
  pinMode(STEP, OUTPUT);
  pinMode(DIR, OUTPUT);
  pinMode(RESET_, OUTPUT);
  
  digitalWrite(MS1,LOW);
  digitalWrite(MS2,LOW);
  digitalWrite(MS3,LOW);
  digitalWrite(STEP,LOW);
  digitalWrite(DIR,LOW);
  digitalWrite(RESET_,LOW); // stepper motor disabled

  // LASER controller pin setup
  pinMode(LASER, OUTPUT);
  digitalWrite(LASER, LOW);

  // CognexInSight 7802 camera Discret I/O pins
  pinMode(COGNEX_HSOUT_0,INPUT);
  pinMode(COGNEX_HSOUT_1,INPUT);
  pinMode(COGNEX_INOUT_2,OUTPUT);
  pinMode(COGNEX_INOUT_3,OUTPUT);
  digitalWrite(COGNEX_INOUT_2, LOW);
  digitalWrite(COGNEX_INOUT_3, LOW);
}

void printVariable(String name, int value) {
  Serial.print(name);
  Serial.print(" = ");
  Serial.println(value,DEC);
}

void printVariable(String name, float value) {
  Serial.print(name);
  Serial.print(" = ");
  Serial.println(value,2);
}

void printVariable(String name, String value) {
  Serial.print(name);
  Serial.print(" = \"");
  Serial.print(value);
  Serial.println("\"");  
}

//
// INTERRUPT SERVICE ROUTINES (ISR)
//

void limitSwitchPositiveDirection() { // positive direction
  limitPositive = digitalRead(LIMIT_SWITCH_POS);
}

void limitSwitchNagativeDirection() { // negative direction
  limitNegative = digitalRead(LIMIT_SWITCH_NEG);
}

void statusLimitSwitches() {
  Serial.print("limitPositive = ");
  Serial.println(limitPositive);
  Serial.print("limitNegative = ");
  Serial.println(limitNegative);
}

//////////////////////////////////////////////////////////////////////
void enableMotor(bool value) {
  if(value)
    digitalWrite(RESET_,HIGH); // stepper motor enabled
  else
    digitalWrite(RESET_,LOW); // stepper motor disabled
}

//////////////////////////////////////////////////////////////////////
void setMicrostepping(int MS) { // A4988

  if(verbose) {
    Serial.print("MS=");
    Serial.println(MS);
  }

  if(MS == 1) {
    digitalWrite(MS1, LOW);
    digitalWrite(MS2, LOW);     
    digitalWrite(MS3, LOW);     
  } else if(MS == 2) {
    digitalWrite(MS1, HIGH);
    digitalWrite(MS2, LOW);   
    digitalWrite(MS3, LOW);     
  } else if(MS == 4) {
    digitalWrite(MS1, LOW);
    digitalWrite(MS2, HIGH);  
    digitalWrite(MS3, LOW);     
  } else if(MS == 8) {
    digitalWrite(MS1, HIGH);
    digitalWrite(MS2, HIGH);  
    digitalWrite(MS3, LOW);     
  } else if(MS == 16) {
    digitalWrite(MS1, HIGH);
    digitalWrite(MS2, HIGH);    
    digitalWrite(MS3, HIGH);     
  }
}

//////////////////////////////////////////////////////////////////////
void stepCognex(int cognex_input, int nSteps) {

  if(cognex_input==COGNEX_INOUT_2) {
    if(verbose) Serial.print("IN2 ");
  } else if(cognex_input==COGNEX_INOUT_3) {
    if(verbose) Serial.print("IN3 ");
  } else {
    if(verbose) Serial.println("INPUT ERROR");
    return;
  }
  if(verbose) Serial.println(nSteps);

  if(nSteps<0) nSteps = -nSteps;

  for(int i = 0; i < nSteps; i++) {
    digitalWrite(cognex_input, HIGH);
    if(i%50==0) digitalWrite(LED, HIGH);
    delayMicroseconds(5000000);
    digitalWrite(cognex_input, LOW);
    if(i%50==25) digitalWrite(LED, LOW);
    delayMicroseconds(5000000);    
  }

  // int out0 = digitalRead(COGNEX_HSOUT_0);
  // int out1 = digitalRead(COGNEX_HSOUT_1);
  // Serial.print("OUT0 = ");
  // Serial.println(out0);
  // Serial.print("OUT1 = ");
  // Serial.println(out1);
}

//////////////////////////////////////////////////////////////////////
int stepMotor(int nSteps) {

  int steps = 0;

  // if(verbose) {
  //   Serial.print("S ");
  //   Serial.println(nSteps);
  // }

  if(nSteps > 0) {
    digitalWrite(DIR, HIGH);
    direction_ = 1;  // ?
  }

  if(nSteps < 0) {
    digitalWrite(DIR, LOW);
    nSteps *= -1;   
    direction_ = -1;  // ?
  }
  
  // delay(10);
  delay(1);

  volatile bool& limitSwitch = (direction_==1)?limitPositive:limitNegative;

  if(limitSwitch==false) {
    for(; steps < nSteps; steps++) {
      if(limitSwitch) break;
      digitalWrite(STEP, LOW);
      // if(i%50==0) digitalWrite(LED, HIGH);
      delayMicroseconds(halfDelayStep);
      digitalWrite(STEP, HIGH);
      // if(i%50==25) digitalWrite(LED, LOW);
      delayMicroseconds(halfDelayStep);
    }

    // if(verbose) {
    //   Serial.print("L ");
    //   Serial.println(i);
    // }

  }

  return steps;
}

//////////////////////////////////////////////////////////////////////
void goHome(){
  int stepDir = -1;
    
  // touch the negative limit switch fast
  setMicrostepping(1);
  while(!limitNegative) {
    digitalWrite(DIR, LOW); // move in the negative direction
    digitalWrite(STEP, LOW);    
    delayMicroseconds(halfDelayStep);
    digitalWrite(STEP, HIGH);
    delayMicroseconds(halfDelayStep);    
  }

  delay(100);
    
  // move forward until the Negative limit switch is off
  setMicrostepping(16);
  while(limitNegative) {
    digitalWrite(DIR, HIGH); // move in the positive direction
    digitalWrite(STEP, LOW);    
    delayMicroseconds(halfDelayStep);
    digitalWrite(STEP, HIGH);
    delayMicroseconds(halfDelayStep);    
  }

  delay(100);

  // then touch the limit switch again, but more slowly
  setMicrostepping(16);
  while(!limitNegative) {
    digitalWrite(DIR, LOW); // move in the negative direction
    digitalWrite(STEP, LOW);    
    delayMicroseconds(halfDelayStep);
    digitalWrite(STEP, HIGH);
    delayMicroseconds(halfDelayStep);    
  }
  // setMicrostepping(1);
}

//////////////////////////////////////////////////////////////////////
void executeCommand(String str) {
  String cmd         = "";
  String args        = "";
  String arg1        = "";
  String arg2        = "";
  int    steps       = 0;
  int    frames      = 0;
  int    laser       = 0;

  int space = -1;
  if((space=str.indexOf(' '))>=0) {
    cmd        = str.substring(0, space);
    args       = str.substring(space+1);
  }
  if((space=args.indexOf(' '))>=0) {
    arg1       = args.substring(0, space);
    arg2       = args.substring(space+1);
  }

  if(cmd=="I" || cmd=="INFO" ) {
    if(args=="") {
      Serial.print("INFO\n");
      Serial.print("COGNEX LINEAR LASER SCANNER 2018\n");
      printVariable("motorDriver           ",motorDriver);
      printVariable("motorType             ",motorType);
      printVariable("halfDelayStep   (HDS) ",halfDelayStep);
      printVariable("mmPerFrame      (MMF) ",mmPerFrame);
      printVariable("stepsPerMM      (SMM) ",stepsPerMM);
    } else {
      if(args=="MD" || args=="motorDriver" || args=="motorDriver ") {
        printVariable("motorDriver",motorDriver);
      } else if(args=="MT" || args=="motorType" || args=="motorType ") {
        printVariable("motorType",motorType);
      } else if(args=="HDS" || args=="halfDelayStep" || args=="halfDelayStep ") {
        printVariable("halfDelayStep",halfDelayStep);
      } else if(args=="MMF" || args=="mmPerFrame" || args=="mmPerFrame ") {
        printVariable("mmPerFrame",mmPerFrame);
      } else if(args=="SMM" || args=="stepsPerMM" || args=="stepsPerMM ") {
        printVariable("stepsPerMM",stepsPerMM);
      } else {
        Serial.print("ERROR");
      }
      Serial.println("");
    }

  } else if(cmd=="EM" || cmd=="ENABLEMOTOR" ) {
    
    enableMotor(true);
    if(verbose) Serial.println("EM");
    
  } else if(cmd=="DM" || cmd=="DISABLEMOTOR" ) {
    
    enableMotor(false);
    if(verbose) Serial.println("DM");
    
  } else if(cmd=="S" || cmd=="STEP" ) {

    steps = arg1.toInt();
    if(true/*verbose*/) {
      Serial.print("S ");
      Serial.print(steps);
    }
    steps = stepMotor(steps);
    if(true/*verbose*/) {
      Serial.print(" -> ");
      Serial.println(steps);
    }
  } else if(cmd == "MS" || cmd == "MICROSTEPPING") {

    setMicrostepping(arg1.toInt());  // in {1, 2, 4, 8, 16}

  } else if(cmd=="IN2") {

    steps = arg1.toInt();
    stepCognex(COGNEX_INOUT_2,steps);

  } else if(cmd=="IN3") {
    
    steps = arg1.toInt();
    stepCognex(COGNEX_INOUT_3,steps);

  } else if(cmd=="IS") {

    if(verbose) {
      Serial.print("IS ");
      Serial.println(arg1);
    }

    cognexSerial.println(arg1); // has to be terminated with NL
    
  } else if(cmd=="F" || cmd=="FORWARD_FRAMES" ) {

    frames = arg1.toInt();
    if(frames<=0) frames = 1;
    steps = (int)(stepsPerMM*mmPerFrame);

    if(verbose) {
      Serial.print("FRAMES FORWARD : ");
      Serial.print(frames);
    }

    int f=0;
    for(;f<frames;f++) {
      if(limitPositive) break;
      stepMotor(steps);
    }

    if(verbose) {
      Serial.print(" -> ");
      Serial.println(f);
    }

  } else if(cmd=="B" || cmd=="BACKWARD_FRAMES") {

    frames = arg1.toInt();
    if(frames<=0) frames = 1;
    steps = (int)(stepsPerMM*mmPerFrame);

    if(verbose) {
      Serial.print("FRAMES BACKWARD : ");
      Serial.print(frames);
    }
    
    int f=0;
    for(;f<frames;f++) {
      if(limitNegative) break;
      stepMotor(-steps);
    }

    if(verbose) {
      Serial.print(" -> ");
      Serial.println(f);
    }

  } else if(cmd == "SLS" || cmd == "STATUS_LIMIT_SWITCHES") {

    statusLimitSwitches();

  } else if(cmd == "H" || cmd == "HOME") {

    enableMotor(true);
    if(verbose) Serial.println("EM");
    goHome(); // MS=16 upon return

  } else if(cmd=="+L") {

    digitalWrite(LASER, HIGH);

  } else if(cmd=="-L") {

    digitalWrite(LASER, LOW);

  } else if(cmd=="L" || cmd=="LASER") {

    if(arg1.toInt()==0)
      digitalWrite(LASER, LOW);
    else
      digitalWrite(LASER, HIGH);

  } else if(cmd=="HDS" || cmd=="HALF_DELAY_STEP") {

    halfDelayStep   = arg1.toInt();

    if(verbose) {
      Serial.print("HDS=");
      Serial.println(halfDelayStep);
    }

  } else if(cmd=="MMF" || cmd=="MM_PER_FRAME") {

    mmPerFrame = arg1.toFloat();

    if(verbose) {
      Serial.print("MMF=");
      Serial.println(mmPerFrame);
    }

  } else if(cmd=="+V" || cmd=="+VERBOSE") {

    verbose = true;
    Serial.println("VERBOSE=ON");

  } else if(cmd=="-V" || cmd=="-VERBOSE") {

    verbose = false;
    Serial.println("VERBOSE=OFF");

  } else if(cmd=="MLOOP") {

    nFrame = arg1.toInt();
    Serial.println(nFrame);
    Serial.print("Start Loop");
    nFrame--;
    //cognexSerial.println("1");
    delayMicroseconds(100);
    digitalWrite(COGNEX_INOUT_2,LOW);
    delayMicroseconds(1000);
    digitalWrite(COGNEX_INOUT_2, HIGH);
    
     //stepCognex(COGNEX_INOUT_2,steps);
  } else if(cmd=="SNS" || cmd=="STEPANDSCAN" ) {
    Serial.print("SNS ");
    delay(500);
    steps = arg1.toInt();
    //if(true/*verbose*/) {
      //Serial.print("S ");
      //Serial.print(steps);
    //}
    steps = stepMotor(steps);
    if(true/*verbose*/) {
      Serial.print(" -> ");
      Serial.println(steps);
    } 
    Serial.println(nFrame);

    delayMicroseconds(1000);
    digitalWrite(LASER, LOW);
    delay(300);
    cognexSerial.println(nFrame);
    delay(300);
    digitalWrite(LASER,HIGH);
    delay(300);
    Serial.print(" Laser Change ");
    digitalWrite(COGNEX_INOUT_3, LOW);
    delayMicroseconds(2000);
    digitalWrite(COGNEX_INOUT_3, HIGH);
    Serial.print("Send Message");
    //digitalWrite(COGNEX_INOUT_3, LOW);
    //delayMicroseconds(10000);
    //digitalWrite(COGNEX_INOUT_3, HIGH);
    
    //delay(100);
    if (nFrame > 0) {
      nFrame--;
      //cognexSerial.println(nFrame);
     digitalWrite(COGNEX_INOUT_2,LOW);
     delayMicroseconds(1000);
     digitalWrite(COGNEX_INOUT_2, HIGH);
    }
    else
    {
      enableMotor(false);
      if(verbose) Serial.println("DM");
    }
  } else {

    if(verbose) {
      Serial.print("ERROR : ");
      Serial.println(cmd);
    }
  }
}

//////////////////////////////////////////////////////////////////////
void loop() {
  
  if (cognexSerial.available() > 0) {
    String str = cognexSerial.readString();
    Serial.print("cmd (cognex)=\"");
    Serial.print(str);
    Serial.println("\"");
    executeCommand(str);
    while(cognexSerial.available()>0 && cognexSerial.read()!='\n');
  }
  if (Serial.available() > 0) {
    String str = Serial.readString();
    executeCommand(str); 
    while(Serial.available()>0 && Serial.read()!='\n');
  }

}
