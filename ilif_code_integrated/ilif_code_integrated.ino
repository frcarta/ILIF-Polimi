#include <AccelStepper.h>  // by Mike McCauley https://github.com/waspinator/AccelStepper
#include <MultiStepper.h>  // inside AccelStepper, more info: https://www.airspayce.com/mikem/arduino/AccelStepper/classMultiStepper.html#details

// Variables for serial communication 
byte msg[7]; // read message coming from serial port (7 bytes for touch events)
// Example:
// 0x65 touch event
// 0x00 page
// 0x03 component ID
// 0x00 release 
// 0xFF 0xFF 0FF end of message
#define MAXRANGE 99999 

// initialization of variables correspondig to elements of Nextion screen
int p1n0 = 0;
int p1n1 = 0;
int p1n2 = 1;
int p1n3 = 0;
// temporary values for the digits in page 1
int p1n0tmp = 0;
int p1n1tmp = 0;
int p1n2tmp = 1;
int p1n3tmp = 0;

int p2n0 = 0;
int p2n1 = 0;
int p2n2 = 0;
int p2n3 = 0;
// temporary values for the digits in page 2
int p2n0tmp = 0;
int p2n1tmp = 0;
int p2n2tmp = 0;
int p2n3tmp = 0;

int *pDigit = nullptr; // pointer to value of digit
const char * nexCtrl = nullptr; // name of nextion UI control
const char * digits[4] = {"n0", "n1", "n2", "n3"};
int *p1DigitVal[4] = {&p1n0, &p1n1, &p1n2, &p1n3};
int *p1DigitValtmp[4] = {&p1n0tmp, &p1n1tmp, &p1n2tmp, &p1n3tmp};
int *p2DigitVal[4] = {&p2n0, &p2n1, &p2n2, &p2n3};
int *p2DigitValtmp[4] = {&p2n0tmp, &p2n1tmp, &p2n2tmp, &p2n3tmp};

/**Codice Gaia**/
// PARAMETRI PER ILIF:
long goBackSteps = 200;     // number of steps to go back after an endstop is encountered
int pos45steps = -1490;
int endStop = 0;

const int enPin1 = 8;
const int dirPin1 = 5;  //pin for the direction, aka positive or negative sign of the movement command
const int stepPin1 = 2;
const int enPin2 = 8;
const int dirPin2 = 6;
const int stepPin2 = 3;

/*const int enPin3 = 8;
const int dirPin3 = 7;
const int stepPin3 = 4;*/

const int switchUpPin = A0;
const int switchDownPin = A1;
const int switchHomePin = A2;
const int initFindZeroPin = A3;
//const int displayEnable = A3;  //AllDisplayEnable, if HIGH, display enabled, if LOW, display disabled
//A4-A5 available (I2C not used)
const int endStopPin1 = 9;     //->LOW
const int endStopPin2 = 10;    //->HIGH
const int triggerOutPin = 12;
//D13 not available as INPUT_PULLUP (it has a led in parallel)

const int stepFraction = 64;  //working @1/64 step fraction (jumper in the middle)
const int stepManual = 1;
//const int timeButton = 20;  //time for pressing button
long stepTrigOut = 0;  //if !=0, turn pinMode OUTPUT, else, INPUT-> no risk of damages. number of step for generating triggerOut Signal [0,5V]
long startPos1 = 0;
long startPos2 = 0;

long stepFinalPos = 0;

int tau2m1 = 20;
int tau2m2 = 20;

bool flagAknStep1 = false;        //true-> display, false->not display
bool flagAknManual = true;        //true-> manual input-> not display; false->serial input->display Acknowledgement
bool serialDisplayEnable = true;  //true ->display enabled, false->display disabled
//bool flagDisplay = true;          //if true, display enabled, if false, display disabled

String inputStringPC;
String inputStringNextion;  //from nextion to ard
String sendString;          //from ard to nextion

AccelStepper stepper1(AccelStepper::DRIVER, stepPin1, dirPin1);  //(DRIVER, STEP, DIR)
MultiStepper steppers;

//puntatori
const int *pDirPin = &dirPin1;
const int *pStepPin = &stepPin1;
const int *pEnPin = &enPin1;
int *pTau2m = &tau2m1;
AccelStepper *pStepper = &stepper1;
//inizializzati a stepper (1), per corretto funzionamento del primo loop.



//Funzioni Gaia
int GoToEndstop(AccelStepper stepper, int endStop);  //==1, zero found; ==0 zero not found;
char Sign(long number);


// function declarations - Nextion 
void increaseDigit(int* pDigit, const char * nexCtrl );
void decreaseDigit(int* pDigit, const char * nexCtrl );
void executeString(int numMot, char enable, int step);

/**********Varaibles for Francesco's Code***************/
int numMot = 1;
char enable = 'r';
int step = 100;       // MAX 2 147 483 647 STEPS

char cmd[32];
/*******************************************************/





void setup() {
  Serial.begin(9600);

  /* setup Gaia*/
  Serial.println("setTimeout");
  Serial.setTimeout(1000);      //10

   //stepper settings
    Serial.println("settings of stepper motors");
    stepper1.setMaxSpeed(2000);  //5000
    stepper1.setSpeed(1000);  //5000
    stepper1.setAcceleration(300000);
    steppers.addStepper(stepper1);

   //pinMode
    pinMode(endStopPin1, INPUT);
    pinMode(endStopPin2, INPUT);
    pinMode(switchUpPin, INPUT_PULLUP);
    pinMode(switchDownPin, INPUT_PULLUP);
    pinMode(switchHomePin, INPUT_PULLUP);
    //pinMode(displayEnable, INPUT_PULLUP);
    pinMode(triggerOutPin, INPUT_PULLUP);
    pinMode(initFindZeroPin, LOW);
    pinMode(enPin1, OUTPUT);
  //
  // delay(1000);
  // // initZero: procedura di setup per trovare lo zero all'accensione
  // if (digitalRead(initFindZeroPin) == LOW) {
  //   digitalWrite(enPin1, LOW);
  //   if (endStopPin1 == HIGH) {
  //     stepper1.runToNewPosition(-500);
  //   } else if (endStopPin2 == HIGH) {
  //     stepper1.runToNewPosition(500);
  //   } if (GoToEndstop(stepper1, 2) == 0 || GoToEndstop(stepper1, 1) == 0) {
  //       //Serial.println("ERRORE NEL TROVARE LO ZERO. POSSIBILE MALFUNZIONAMENTO DEGLI ENDSTOP. PROCEDERE CON CAUTELA.");
  //     }
  //   delay(300);
  //   // Serial.println("initZero");
  //   // Serial.println(" ");
  //   pinMode(initFindZeroPin, INPUT_PULLUP);   // cambio; "ora hai trovato lo zero iniziale"
  // } else {
  //   digitalWrite(enPin1, HIGH);
  // }  //disable driver

}

void loop() {   // put your main code here, to run repeatedly:
  if (Serial.available() ==  7){ // check repeatedly Serial
    for (int k = 0; k<7; k++){
      msg[k] = Serial.read();
    }
  

    switch(msg[1]) // check page
    {
    /*************************PAGE 0************************************/
      case 0: // page 0
        switch(msg[2]) // components
        {
          case 2: // Set Step button 
          { 
            // set the values of the digits to corresponding values
            for(int i = 0; i<4; i++){
              Serial.print("p[1].");
              Serial.print(digits[i]);
              Serial.print(".val=");
              Serial.print(*p1DigitVal[i]);
              Serial.write(0xFF);
              Serial.write(0xFF);
              Serial.write(0xFF);
            }

            // send step variable to the "Step set to:" numeric text box on page 1 
            Serial.print("p[1].n4.val=");
            Serial.print(step);
            Serial.write(0xFF);
            Serial.write(0xFF);
            Serial.write(0xFF);
    
          }
          break;
          case 3: // arrow up
            //executeStringFlag = 1;
            enable = 'r'; 
            executeString(numMot, enable, -step);
            //snprintf(cmd, sizeof(cmd), "%d%c%d", numMot, enable, step); // cmd contains the variables combined in a string

          break;
          case 4: // arrow down
            //executeStringFlag = 1;
            enable = 'r'; 
            executeString(numMot, enable, step);
            //snprintf(cmd, sizeof(cmd), "%d%c%d", numMot, enable, step); // cmd contains the variables combined in a string
          break;
          
        }
      break; // end page 0
  /*************************PAGE 1************************************/
      case 1: 
        switch(msg[2])
        {
          case 9: // arrow up n3
            pDigit = &p1n3tmp;
            nexCtrl = "n3";
            increaseDigit(pDigit, nexCtrl);
          break;
          case 10: // arrow down 3
            pDigit = &p1n3tmp;
            nexCtrl = "n3";
            decreaseDigit(pDigit, nexCtrl );
          break;
          case 8: // arrow up 2
            pDigit = &p1n2tmp;
            nexCtrl = "n2";
            increaseDigit(pDigit, nexCtrl );
          break;
          case 11: // arrow down 2
            pDigit = &p1n2tmp;
            nexCtrl = "n2";
            decreaseDigit(pDigit, nexCtrl );
          break;   
          case 7: // arrow up 1
            pDigit = &p1n1tmp;
            nexCtrl = "n1";
            increaseDigit(pDigit, nexCtrl );
          break;
          case 12: // arrow down 1
            pDigit = &p1n1tmp;
            nexCtrl = "n1";
            decreaseDigit(pDigit, nexCtrl );
          break;  
          case 6: // arrow up 0
            pDigit = &p1n0tmp;
            nexCtrl = "n0";
            increaseDigit(pDigit, nexCtrl );
          break;
          case 13: // arrow down 0
            pDigit = &p1n0tmp;
            nexCtrl = "n0";
            decreaseDigit(pDigit, nexCtrl );
          break;  
          case 14: // Reset button
            { 
              p1n0tmp = 0;
              p1n1tmp = 0;
              p1n2tmp = 1;
              p1n3tmp = 0;
              
              for (int i=0; i <4; i++){
                Serial.print(digits[i]);
                Serial.print(".val=");
                Serial.print(*p1DigitValtmp[i]);
                Serial.write(0xFF);
                Serial.write(0xFF);
                Serial.write(0xFF);
              }
            }
          break;
          case 15: // ok
          {
            // update true values of digits
            for(int i = 0; i<4; i++){
              *p1DigitVal[i] = *p1DigitValtmp[i];
            }

            step = (p1n3 * 1000) + (p1n2 * 100) + (p1n1 * 10) + (p1n0);
            Serial.print("p[1].n4.val=");
            Serial.print(step);
            Serial.write(0xFF);
            Serial.write(0xFF);
            Serial.write(0xFF);

            Serial.print("p[0].n0.val=");
            Serial.print(step);
            Serial.write(0xFF);
            Serial.write(0xFF);
            Serial.write(0xFF);
          }
          break;
          case 1: // back 
            {
            for(int i=0; i<4; i++){
              *p1DigitValtmp[i]=*p1DigitVal[i];
            }
            Serial.print("p[0].n0.val=");
            Serial.print(step);
            Serial.write(0xFF);
            Serial.write(0xFF);
            Serial.write(0xFF);
            }
          break;
        }
      break; // end page 1
  /*************************PAGE 2************************************/
      case 2:
        switch(msg[2])
        {
          case 9: // arrow up n3
            pDigit = &p2n3tmp;
            nexCtrl = "n3";
            increaseDigit(pDigit, nexCtrl );
          break;
          case 10: // arrow down 3
            pDigit = &p2n3tmp;
            nexCtrl = "n3";
            decreaseDigit(pDigit, nexCtrl );
          break;
          case 8: // arrow up 2
            pDigit = &p2n2tmp;
            nexCtrl = "n2";
            increaseDigit(pDigit, nexCtrl );
          break;
          case 11: // arrow down 2
            pDigit = &p2n2tmp;
            nexCtrl = "n2";
            decreaseDigit(pDigit, nexCtrl );
          break;   
          case 7: // arrow up 1
            pDigit = &p2n1tmp;
            nexCtrl = "n1";
            increaseDigit(pDigit, nexCtrl );
          break;
          case 12: // arrow down 1
            pDigit = &p2n1tmp;
            nexCtrl = "n1";
            decreaseDigit(pDigit, nexCtrl );
          break;  
          case 6: // arrow up 0
            pDigit = &p2n0tmp;
            nexCtrl = "n0";
            increaseDigit(pDigit, nexCtrl );
          break;
          case 13: // arrow down 0
            pDigit = &p2n0tmp;
            nexCtrl = "n0";
            decreaseDigit(pDigit, nexCtrl );
          break;
          case 15: // OK
            enable = 'a'; 
            executeString(numMot, enable, step);

          break;
          case 1:// back

          break;

          //!! aggiungere case "laser" e "lamp"
          // impostano solo il numero (rispettivamente, step=1470 e step=0); il movimento (assoluto) lo dà "ok"
        }
      break; // end page 2

  /*************************PAGE 3************************************/
      case 3:
      //swicth(msg[2])      //!! aggiungere
      //{
      // case 1:
      // break; 
      //} 
      break;     
    } // end switch page
  




  } // end if Serial.availble
} // end void loop


// function definitions
void increaseDigit(int* pDigit, const char * nexCtrl ){
  if(*pDigit == 9) *pDigit = 0;
  else (*pDigit)++;
  Serial.print(nexCtrl);
  Serial.print(".val=");
  Serial.print(*pDigit);
  Serial.write(0xFF);
  Serial.write(0xFF);
  Serial.write(0xFF);
}

void decreaseDigit(int* pDigit, const char * nexCtrl ){
  if(*pDigit == 0) *pDigit = 9;
  else (*pDigit)--;
  Serial.print(nexCtrl);
  Serial.print(".val=");
  Serial.print(*pDigit);
  Serial.write(0xFF);
  Serial.write(0xFF);
  Serial.write(0xFF);
}



void executeString(int numMot, char enable, int step){
  // print corresponding command on Nextion 
  snprintf(cmd, sizeof(cmd), "%d%c%d", numMot, enable, step);

  // print on Nextion
  Serial.print("p[0].t1.txt=\"");
  Serial.print(cmd);
  Serial.print("\"");
  Serial.write(0xFF);
  Serial.write(0xFF);
  Serial.write(0xFF);

  // //execute -- codice Gaia

  // step = -step;   // from mirror holder pov to motor pov

  // switch (enable) {  //sorting serial functions
  //   //FUNZIONI MOVIMENTO
  //   case 'a':  //absolute movement
  //     digitalWrite(*pEnPin, LOW);
  //     if (GoToEndstop(*pStepper, 1) == 1) {
  //       pStepper->setCurrentPosition(0);
  //       pStepper->runToNewPosition(pStepper->currentPosition());
  //       pStepper->moveTo(step);
  //     }
  //   break;
  //   case 'r':  //relative movement
  //     digitalWrite(*pEnPin, LOW);
  //     long currPosTmp = pStepper->currentPosition();
  //     if (GoToEndstop(*pStepper, 1) == 1) {
  //       pStepper->setCurrentPosition(0);
  //       pStepper->runToNewPosition(pStepper->currentPosition());
  //       pStepper->moveTo(currPosTmp + step);
  //     }
  //   break;
  //   case 'p':  // save current position as default "laser" position
  //      pos45steps = pStepper->currentPosition();
  //   break;
  //   // case 's': // setup procedure
  //   //   ...
  //   //   break;
  //   // case 'c':   // endstop check
  //   //   if (digitalRead(endStopPin1)==HIGH )
  //   //     Serial.println("Ostacolo 1");
  //   //   else
  //   //     Serial.println("Open 1");
  //   //   if (digitalRead(endStopPin2)==HIGH)
  //   //     Serial.println("Ostacolo 2");
  //   //   else
  //   //     Serial.println("Open 2");
  //   //   break;
    
  //   // case 'v':  //set motor speed (uStep/second)
  //   //   pStepper->setMaxSpeed(-step);
  //   //   Serial.print("MOT ");
  //   //   Serial.print(numMot);
  //   //   Serial.print(" VEL SET: ");
  //   //   Serial.println(-step);
  //   //   break;
    
  //   // case 'w':  //test connection
  //   //   Serial.println("CONNECTED");
  //   //   break;
  //   // default:  //other letters->do nothing, used 'w' letter
  //   //   step = 0;
  //   // break;
  // } // end of switch (enable)
  

  // //acknowledgements
  // flagAknManual = true;
  // if (digitalRead(switchUpPin) == LOW) {
  //   digitalWrite(*pEnPin, LOW);
  //   pStepper->move(stepManual);
  //   flagAknManual = true;
  // }
  // if (digitalRead(switchDownPin) == LOW) {
  //   digitalWrite(*pEnPin, LOW);
  //   pStepper->move(-stepManual);
  //   flagAknManual = true;
  // }
  // if (digitalRead(switchHomePin) == LOW) {
  //   digitalWrite(*pEnPin, LOW);
  //   pStepper->moveTo(0);
  //   flagAknManual = true;
  // }

  // while (pStepper->distanceToGo()!=0) {    // movement of stepper motor

  //   // EndStops: check if an endstop is encountered
  //   if (pStepper->isRunning()==true) {
  //     if (digitalRead(endStopPin1) == HIGH) {     // Endstop1: minimum angle for mirror holder
  //       digitalWrite(*pEnPin, LOW);  //enable motor driver
  //       pStepper->stop();
  //       // Serial.println("ENDSTOP 1");
  //       pStepper->runToNewPosition(pStepper->currentPosition() - goBackSteps);    // - bc of sign convention; goes clockwise
  //       pStepper->setCurrentPosition(0);
  //       //print currentPos on Nextion
  //       Serial.print("p[0].n2.val=");
  //       Serial.print(pStepper->currentPosition());
  //       Serial.write(0xFF);
  //       Serial.write(0xFF);
  //       Serial.write(0xFF);
  //       //delay(100);
  //       break;
  //     }
  //     if (digitalRead(endStopPin2) == HIGH) {       // Endstop2: max angle for mirror holder
  //       digitalWrite(*pEnPin, LOW);  //enable motor driver
  //       pStepper->stop();
  //       // Serial.println("ENDSTOP 2");
  //       pStepper->runToNewPosition(pStepper->currentPosition() + goBackSteps);
  //       //print currentPos on Nextion
  //       Serial.print("p[0].n2.val=");
  //       Serial.print(pStepper->currentPosition());
  //       Serial.write(0xFF);
  //       Serial.write(0xFF);
  //       Serial.write(0xFF);
  //       break;
  //       // at this point, we do NOT know the position with absolute confidence, due to non-null backlash between the gears!
  //     }
  //   }

  //   pStepper->run();
  // }

  // //print currentPos on Nextion
  // Serial.print("p[0].n2.val=");
  // Serial.print(pStepper->currentPosition());
  // Serial.write(0xFF);
  // Serial.write(0xFF);
  // Serial.write(0xFF);

} //end of function executeString


// // GoToEndstop
// int GoToEndstop(AccelStepper stepper, int endStop) {
//   digitalWrite(enPin1, LOW);                                            //enable motor driver
//   if (endStop == 1)
//     stepper.move(MAXRANGE);                                             //set movement target and distanceToGo attribute
//   else if (endStop == 2)
//     stepper.move(-MAXRANGE);

//   while (stepper.distanceToGo() != 0) {           // while no endstop encountered and maxSteps not reached
//     if (digitalRead(endStopPin1) == HIGH) {  // endstop 1 encountered
//       stepper.stop();
//       stepper.runToNewPosition(stepper.currentPosition() - goBackSteps);
//       break;
//     }
//     if (digitalRead(endStopPin2) == HIGH) {     // endstop 2 encountered
//       stepper.stop();
//       stepper.runToNewPosition(stepper.currentPosition() + goBackSteps);
//       break;
//     }
//   }
//   stepper.run();  //run movement
  
//   if (stepper.distanceToGo() != 0) {
//     return 1;  // endostop 1 or 2 found
//   } else {
//     return 0;  // Error - endstop not found
//   }
// }
