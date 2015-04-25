
/********************************/
/*  Rob Seward 2008-2009        */
/*  v1.0                        */
/*  4/20/2009                   */
//
// -pee 06/13/2014 - update a bit
// -pee 04/21/2015 - fixes, BS
//
//
/********************************/


#include "sha256.h"

#define BINS_SIZE 256
#define CALIBRATION_SIZE 50000

#define NO_BIAS_REMOVAL 0
#define EXCLUSIVE_OR 1
#define VON_NEUMANN 2

#define ASCII_BYTE 0
#define BINARY 1
#define ASCII_BOOL 2

/***  Configure the RNG **************/
int bias_removal = NO_BIAS_REMOVAL;
int output_format = ASCII_BYTE;
/*************************************/


unsigned int bins[BINS_SIZE];
int adc_pin = 0;
int led_pin = 13;

byte threshold = 0;
boolean ledOn = false;

void setup(){

  pinMode(led_pin, OUTPUT);

  Serial.begin(57600);

  for (int i=0; i < BINS_SIZE; i++){
    bins[i] = 0; 
  }  
  
    Sha256.init();


  runCalibration();
}

void loop(){

  // 10 bit adc value
  int adc_value = analogRead(adc_pin);
  
  //byte adc_value = analogRead(adc_pin);
  //Serial.print(adc_value);

  Sha256.print(adc_value);
  printHash(Sha256.result());
  
  //processInput(adc_value, threshold);

}

void runCalibration() {

  boolean initializing = true;
  unsigned int calibration_counter = 0;
  int adc_value;
  byte adc_byte;

  while (initializing == true) {

    adc_value = analogRead(adc_pin);
    adc_byte = adc_value >> 2;

    if(calibration_counter >= CALIBRATION_SIZE){
      threshold = findThreshold();
      initializing = false;
    }

    calibrate(adc_byte);
    calibration_counter++;

    if ((calibration_counter % 1000) == 0 ) {
      blinkLed();
    }
  }

}

void printHash(uint8_t* hash) {
  int i;
  for (i=0; i<32; i++) {
    Serial.print("0123456789abcdef"[hash[i]>>4]);
    Serial.print("0123456789abcdef"[hash[i]&0xf]);
  }
  Serial.println();
}

void processInput(byte adc_byte, byte threshold){

  boolean input_bool;
  input_bool = (adc_byte < threshold) ? 1 : 0;
  switch(bias_removal){
  case VON_NEUMANN:
    vonNeumann(input_bool); 
    break;
  case EXCLUSIVE_OR:
    exclusiveOr(input_bool);
    break;
  case NO_BIAS_REMOVAL:
    buildByte(input_bool);
    break;
  }
}

void exclusiveOr(byte input){
  static boolean flip_flop = 0;
  flip_flop = !flip_flop;
  buildByte(flip_flop ^ input);
}

void vonNeumann(byte input){
  static int count = 1;
  static boolean previous = 0;
  static boolean flip_flop = 0;

  flip_flop = !flip_flop;

  if(flip_flop){
    if(input == 1 && previous == 0){
      buildByte(0);
    }
    else if (input == 0 && previous == 1){
      buildByte(1); 
    }
  }
  previous = input;
}

void buildByte(boolean input){
  static int byte_counter = 0;
  static byte out = 0;

  if (input == 1){
    out = (out << 1) | 0x01;
  }
  else{
    out = (out << 1); 
  }
  byte_counter++;
  byte_counter %= 8;
  if(byte_counter == 0){
    if (output_format == ASCII_BYTE) Serial.println(out, DEC);
    if (output_format == BINARY) Serial.print(out, BIN);
    out = 0;  
  }
  if (output_format == ASCII_BOOL) Serial.print(input, DEC);
}


void calibrate(byte adc_byte){
  bins[adc_byte]++;  
  //printStatus();
}

unsigned int findThreshold(){
  unsigned long half;
  unsigned long total = 0;
  int i;

  for(i=0; i < BINS_SIZE; i++){
    total += bins[i];
  }	

  half = total >> 1;
  total = 0;
  for(i=0; i < BINS_SIZE; i++){
    total += bins[i];
    if(total > half){
      break;
    }	
  }
  return i;
}

//Blinks an LED after each 10th of the calibration completes
//void printStatus(){
//  unsigned int increment = CALIBRATION_SIZE / 10;
//  static unsigned int num_increments = 0; //progress units so far
//  unsigned int threshold;
//
//  threshold = (num_increments + 1) * increment;
//  if(calibration_counter > threshold){
//    num_increments++;
//    //Serial.print("*");
//    blinkLed();
//  }   
//}

void blinkLed(){
  if ( ledOn == false) {
    digitalWrite(led_pin, HIGH);
    ledOn = true;
  } 
  else {
    digitalWrite(led_pin, LOW);
    ledOn = false;
  }
}


