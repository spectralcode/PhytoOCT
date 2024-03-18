// This is the Arduino code for the control board of the PhytoOCT System.
// The control board generates the scan signals, synchronizes the scanner
// with the line sensor, enables/disables the SLD, and registers button
// presses that are used to start the OCT application's recording process.
// Authors: Mohamand Bsata, Miroslav Zabic
// Date: 2023


#include <SPI.h>
#include <OneButton.h>

#define MAXDELAY 16383   // You can´t delay a signal more than 16.383 ms
#define LIMIT 4095       // The maximal output of the MCP4822 in gain mode 2 is 4096 mV. You can´t exceed that
#define STEPSLIMIT 320  

const int PIN_CS = 10;      //Use PIN 10 on the Arduino uno for chip select
const int PIN_ldac = 9;     //Use PIN 9 on the Arduino uno for for LDAC. Must be low all the time
const int PIN_trigger = 8;  //Use PIN 8 to trigger the USB Line Camera
const int PIN_sld = 7;  //Use PIN 7 to turn on/off SLD

const int buttonGround = 2; //Use PIN 2 to act as aground for the red button
const int buttonPin = 3; //Use Pin 3 to trigger the red Button
OneButton button(buttonPin, true); //create a OneButton object

// You write commands to the MCP with 16-Bit commands
// Bits 0-11 are the input data Bits
// Bit 12 can turn the DAC oputput on/off
// Bit 13 decides which gain mode to use
// Bit 14 doesn´t care
// Bit 15 decides which Chanal to write to

const int GAIN_1 = 0x1;  // To use gain mode 1 (2048 mV max Output) set Bit 13 to 1
const int GAIN_2 = 0x0;  // To use gain mode 2 (4096 mV max Output) set Bit 13 to 0

const int A = 0x0;      // To write data to channal A set Bit 15 to 0
const int B = 0x1;      // To write data to channal B set Bit 15 to 1

const int activemode = 0x1;  // To turn the output on set Bit 12 to 1

const int xSteps = 256;  // We want to have 320 A-Scans per B-Scan
const int ySteps = 256;  // we want to have 320 B-Scans

int xAmplitude = 380;  // This is the Amplitude of the x-Scanner. The Scanner-Driverboard will translate this value in mV to an angle in degrees
int yAmplitude = 380;  // This is the Amplitude of the y-Scanner. The Scanner-Driverboard will translate this value in mV to an angle in degrees

int xOffset = 2600;  // This value decides at what angle the x-scanner will start the scanning process. After a B-scan is complete the x-scanner will return to this position 
int yOffset = 1500;  // This value decides at what angle the y-scanner will start the scanning process. After all B-scans are complete the y-scanner will return to this position 

int xPhase = 4; 
int yPhase = 0;

int xStepDelay = 110;  // The spectrometer need time to collect data so we dalay after each x scanner step to give the spectrometer enough time for data acquisition. This value is also the duration of high state of the acquisition trigger signal. The value here is in µs.
int yStepDelay = 212;  // We also wait a little bit after a B-scan is done. The value here is in µs.

int yStepCounter = 0;

int xWaveform[xSteps];  // We store all the voltage values for the x-Scanner in an array. The array is as big as the numbers of A-scans we want to have.
int yWaveform[ySteps];  // We store all the voltage values for the y-Scanner in an array. The array is as big as the numbers of B-scans we want to have.

bool evenFrameNr = false;
bool bidirectionalScan = false;

bool xActivated = true;  // In case we need to deactivate the x-scanner
bool yActivated = false; // In case we need to deactivate the y-scanner
bool run = false; // start or stop x- and y- scanning

bool newCommandReceived = false;

const byte numChars = 32;
char receivedChars[numChars];
char receivedCommand[numChars] = { 0 };

int receivedValue = 0;

const char* terminatingChar = '\n';
const char* delimiter = "\=";


//-------------------------------------------------------------------------------------------------------------------------------//

void setup()
{
  pinMode(PIN_trigger, OUTPUT);
  pinMode(PIN_sld, OUTPUT);
  pinMode(PIN_ldac, OUTPUT);
  digitalWrite(PIN_ldac, LOW);
  pinMode(PIN_CS, OUTPUT);

  pinMode(buttonGround, OUTPUT);
  digitalWrite(buttonGround, LOW);
  button.attachClick(buttonClicked);

  SPI.begin();  
  SPI.setClockDivider(SPI_CLOCK_DIV2);

  generateSawtoothWaveform(xWaveform, xSteps, xAmplitude, xOffset);
  generateSawtoothWaveform(yWaveform, ySteps, yAmplitude, yOffset);
  
  moveScannersToZeroPosition();

  Serial.begin(9600);
  Serial.println("Everything ready!");
  Serial.println();
}

//-------------------------------------------------------------------------------------------------------------------------------//

void buttonClicked() {
  Serial.println("startrecording");
}

//-------------------------------------------------------------------------------------------------------------------------------//


//this function enables the control of a MCP4822 DAC
//original author is Kerry Wong; see: http://www.kerrywong.com/2012/07/25/code-for-mcp4821-mcp4822/ 
void setOutput(byte channel, byte gain, byte shutdown, unsigned int val) {
  byte lowByte = val & 0xff;
  byte highByte = ((val >> 8) & 0xff) | channel << 7 | gain << 5 | shutdown << 4;
   
  PORTB &= 0xfb;
  SPI.transfer(highByte);
  SPI.transfer(lowByte);
  PORTB |= 0x4;
}

//-------------------------------------------------------------------------------------------------------------------------------//

void generateSawtoothWaveform(int* waveformArray, int steps, int amplitude, int offset){
  for(int i = 0; i < steps; i++){
    waveformArray[i] = min(max((offset + (float)amplitude*(((float)i/((float)steps))-0.5)), 0), LIMIT);
  }
}

//-------------------------------------------------------------------------------------------------------------------------------//

void loop() {
  if(run == true){
    if(xActivated == true){
      for(int i = 0; i < xSteps; i++){
        int xIndexPosition = (i+xPhase)%xSteps;
        if(evenFrameNr && bidirectionalScan){
          xIndexPosition = (xSteps-1)-xIndexPosition;
        }
        setOutput(A, GAIN_2, activemode, xWaveform[xIndexPosition]);
        
        //emit acquisition trigger
        digitalWrite(PIN_trigger, HIGH);
        delayMicroseconds(xStepDelay);
        digitalWrite(PIN_trigger, LOW);
      }
      evenFrameNr = !evenFrameNr;
    }

    if(yActivated == true) {
        yStepCounter = (yStepCounter+1)%ySteps;
        int yIndexPosition = (yStepCounter+yPhase)%ySteps;
        setOutput(B, GAIN_2, activemode, yWaveform[yIndexPosition]);
        if(yIndexPosition == 0){
          delayMicroseconds(yStepDelay);
        }
    }
  } else {
    //reset y scan step counter
    yStepCounter = 0;
  }

  button.tick();

  receiveData();
  if (newCommandReceived == true) {
    parseAndProcessData();
    newCommandReceived = false;
  }
}








//-------------------------------------------------------------------------------------------------------------------------------//
//-------------------------------------------------------------------------------------------------------------------------------//
//-------------------------------------------------------------------------------------------------------------------------------//
//             Here begins the Part of the code that´s responsible for the communication through Serial Monitor                  //
//-------------------------------------------------------------------------------------------------------------------------------//
//-------------------------------------------------------------------------------------------------------------------------------//
//-------------------------------------------------------------------------------------------------------------------------------//



void receiveData() {
  static byte ndx = 0;
  char rc;

  while (Serial.available() > 0 && newCommandReceived == false) { 
    rc = Serial.read();
    if (rc != terminatingChar) {
      receivedChars[ndx] = rc;
      ndx++;
      if (ndx >= numChars) {
        ndx = numChars - 1;
      }
    } else {
      receivedChars[ndx] = '\0';  // terminate the string
      ndx = 0;
      newCommandReceived = true;
    }
  }
}

//-------------------------------------------------------------------------------------------------------------------------------//
//                      Here we deal with Serial Monitor commands that contain both words and numbers                            //
//-------------------------------------------------------------------------------------------------------------------------------//

void processCommand(char* command, int value){
   if(strcmp(command, "run") == 0){
    if(value == 0){
      run = false;
      yStepCounter = 0;
      Serial.println("scanning off");  
      moveScannersToZeroPosition();
      delayMicroseconds(200);
    } else {
      run = true;
      Serial.println("scanning on");  
    }
    return;
  } 

  else if(strcmp(command, "sld") == 0){
    if(value == 0){
      digitalWrite(PIN_sld, LOW);
      Serial.println("SLD off");  
    } else {
      digitalWrite(PIN_sld, HIGH);
      Serial.println("SLD on");  
    }
    return;
  } 
 
  else if(strcmp(command, "xactivate") == 0){
    if(value == 0){
      xActivated = false;
      Serial.println("x scanner off");  
      setOutput(A, GAIN_2, activemode, xOffset);
      delayMicroseconds(200);
    } else {
      xActivated = true;
      Serial.println("x scanner on");  
    }
    return;
  } 
 
  else if(strcmp(command, "yactivate") == 0){
    if(value == 0){
      yActivated = false;
      Serial.println("y scanner off");  
      setOutput(B, GAIN_2, activemode, yOffset);
      delayMicroseconds(200);
    } else {
      yActivated = true;
      Serial.println("y scanner on");  
    }
    return;
  }
  
  else if(strcmp(command, "xoffset") == 0){
    xOffset = max(min(value, LIMIT), 0);

    moveScannersToZeroPosition();
    delayMicroseconds(200);
    generateSawtoothWaveform(xWaveform, xSteps, xAmplitude, xOffset);

    Serial.print("xOffset set to ");
    Serial.println(xOffset);

    return;
  } 
 
  else if(strcmp(command, "yoffset") == 0){
    yOffset = max(min(value, LIMIT), 0);

    moveScannersToZeroPosition();
    delayMicroseconds(200);
    generateSawtoothWaveform(yWaveform, ySteps, yAmplitude, yOffset);

    Serial.print("yOffset set to ");
    Serial.println(yOffset);

    return;
  }

  else if(strcmp(command, "xamplitude") == 0){
    xAmplitude = max(min(value, LIMIT), 0);

    moveScannersToZeroPosition();
    delayMicroseconds(200);
    generateSawtoothWaveform(xWaveform, xSteps, xAmplitude, xOffset);

    Serial.print("xAmplitude set to ");
    Serial.println(xAmplitude);
    return;
  } 

  else if(strcmp(command, "yamplitude") == 0){
    yAmplitude = max(min(value, LIMIT), 0);

    moveScannersToZeroPosition();
    delayMicroseconds(200);
    generateSawtoothWaveform(yWaveform, ySteps, yAmplitude, yOffset);

    Serial.print("yAmplitude set to ");
    Serial.println(yAmplitude);
    return;
  }

  else if(strcmp(command, "xphase") == 0){
    xPhase = value;
    delayMicroseconds(200);
    Serial.print("xPhase set to ");
    Serial.println(xPhase);
    return;
  }

  else if(strcmp(command, "yphase") == 0){
    yPhase = value;
    delayMicroseconds(200);
    Serial.print("yPhase set to ");
    Serial.println(yPhase);
    return;
  }

  else if(strcmp(command, "xstepdelay") == 0){
    xStepDelay = max(min(value, MAXDELAY), 0);
    Serial.print("xStepDelay set to ");
    Serial.println(xStepDelay);
    return;
  } 

  else if(strcmp(command, "ystepdelay") == 0){
    yStepDelay = max(min(value, MAXDELAY), 0);
    Serial.print("yStepDelay set to ");
    Serial.println(yStepDelay);
    return;
  } 

  else if(strcmp(command, "delay") == 0){
    xStepDelay = max(min(value, MAXDELAY), 0);
    Serial.print("xStepDelay set to ");
    Serial.println(xStepDelay);
    return;
  } 

  else if(strcmp(command, "bidirectionalscan") == 0){
    if(value == 0){
      bidirectionalScan = false;
      Serial.println("Bi-directional scan disabled");  
    } else {
      bidirectionalScan = true;
      Serial.println("Bi-directional scan enabled");   
    }
    return;
  }

  else {
    Serial.print("Unknown command with number value: ");
    Serial.println(value);
  }
}

//-------------------------------------------------------------------------------------------------------------------------------//
//                           Here we deal with Serial Monitor commands that contain only words                                   //
//-------------------------------------------------------------------------------------------------------------------------------//

void processCommand(char* command){
  if(strcmp(command, "help") == 0){
    Serial.println("No help for you! :'(");
  } else if(strcmp(command, "save") == 0){
    Serial.println("Saving to EEPROM... failed. nobody implementes this feature yet");
  } else if(strcmp(command, "showxcurve") == 0){
    for(int i = 0; i < xSteps; i++){
      Serial.print("x:");
      Serial.println(xWaveform[i]);
      delay(10);
    }
  } else {
    Serial.println("Unknown command.");
  }
}

//-------------------------------------------------------------------------------------------------------------------------------//

void parseAndProcessData() {  // split the data into its parts
  char* token = strtok(receivedChars, delimiter); 
  strcpy(receivedCommand, token);    //copy command
  token = strtok(NULL, delimiter); //get pointer to second token if there is one
  if(token != NULL) { //check if there was a delimiter char and extract integer value
    receivedValue = atoi(token);
    processCommand(receivedCommand, receivedValue);
  } else {
    processCommand(receivedCommand);
  }
}


void moveScannersToZeroPosition(){
  setOutput(A, GAIN_2, activemode, xOffset);
  setOutput(B, GAIN_2, activemode, yOffset);
}
