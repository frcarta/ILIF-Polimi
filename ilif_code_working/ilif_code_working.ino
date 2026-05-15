// Variables for serial communication 
byte msg[7]; // read message coming from serial port (7 bytes for touch events)
// Example:
// 0x65 touch event
// 0x00 page
// 0x03 component ID
// 0x00 release 
// 0xFF 0xFF 0FF end of message

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




// function declarations - Nextion 
void increaseDigit(int* pDigit, const char * nexCtrl );
void decreaseDigit(int* pDigit, const char * nexCtrl );
void executeString(int numMot, char enable, int step);

/**********Varaibles for Francesco's Code***************/
int numMot = 1;
char enable = 'r';
int step = 100;

char cmd[32];
/*******************************************************/





void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  //Serial.println("Hello World");
}

void loop() {   // put your main code here, to run repeatedly:
  if (Serial.available() >=7){ // check repeatedly Serial
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
            executeString(numMot, enable, step);
            //snprintf(cmd, sizeof(cmd), "%d%c%d", numMot, enable, step); // cmd contains the variables combined in a string

          break;
          case 4: // arrow down
            //executeStringFlag = 1;
            enable = 'r'; 
            executeString(numMot, enable, -step);
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
            Serial.print("p[0].n0.val=");
            Serial.print(step);
            Serial.write(0xFF);
            Serial.write(0xFF);
            Serial.write(0xFF);
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
            enable = 'q';
            executeString(numMot, enable, step);

          break;
          case 1:// back

          break;
        }
      break; // end page 2

  /*************************PAGE 3************************************/
      case 3:
      //swicth(msg[2])
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
  Serial.print("p[0].t1.txt=\"");
  Serial.print(cmd);
  Serial.print("\"");
  Serial.write(0xFF);
  Serial.write(0xFF);
  Serial.write(0xFF);

}


