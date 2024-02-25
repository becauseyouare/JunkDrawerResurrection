#include <Arduino.h>

/*  Junk Drawer Resurrection
Chromebook Camera/mic , USB 1-3
PC fan , 12 volt.  (A) port D2
Weather radio, 9 volt.  (B) port D4
Mallory meter, 0-5volt (C) port D3
Big relay , 24 volt. (D) port D8
Phone ringer, 26 volt 20hz 2sec on/4 sec off. T (E) port D5
 +Hook contacts (F) return count from port A3
Microwave fan,110 VAC  (G) port D7
Big motor, 110 VAC. (H) port D9
Mixer motor 24volt. T (I) port D13
Three relays, 24 volt (blk common).  TTT (J,K,L) ports D10,11,12
100 volt meter, 0 - 100v. (M) port 6
Phone dial, contacts (N) return count from port A5
*/
unsigned long previousMillis = millis();
unsigned long previousSecond = millis(); 
int intervalMillis = 0;
bool intervalSecond = 0;
String inString = "";  // string to hold serial input
int ComponentSelect = 0;
//int CompIndex = 0;
int AttCount = 0;
int inNum = 0;
int PhoneDialInterval = 0;
int PhoneDialHis = 0;
int PhoneDialCount = 0;
int PhoneDialComplete = 1;
int PhoneRingInterval = 0;
int PhoneRingPeriod = 0;
bool PhoneRinging = 0;

struct Elements {
  int port;
  bool trigger;
  bool active;
  int attributes[10];
} Component[] = {
  // These are the Arduino ports that each element is connected to
  { 14},// 0 , {}, "null node " , // 
  { 2},//, "PC fan    " , //A
  { 4},//, "Wthr radio" }, //B
  { 3},//, "MalloMeter" }, //C
  { 8},//, "Big Relay " }, //D
  { 5},//, "PhoneRingr" }, //E
  { 17},//, "Hook Contt" }, //F
  { 7},//, "MicoWavFan" }, //G
  { 9},//, "BigMotor  " }, //H
  { 13},//, "MixerMotor" }, //I
  { 10},//, "Relay 1   " }, //J
  { 11},//, "Relay 2   " }, //K
  { 12},//, "Relay 3   " }, //L
  { 6},//, "100v Meter" }, //M
  { 19},//, "PhoneDial Contac" }, //N
  { 0},//, "element..." }, //
};

void triggerActivate(int componentNum) {
  if(Component[componentNum].trigger){
    digitalWrite(Component[componentNum].port,1);
    Component[componentNum].active = 1;
    Component[componentNum].trigger = 0;
  }
}

void dataDump() { // for testing
  for(int x=0 ;x < 15; x++){
    Serial.print("Element ");
    Serial.print(x,DEC);
    Serial.print("\tport ");
    Serial.print(Component[x].port);
    Serial.print("\ttrigger ");
    Serial.print(Component[x].trigger);
    Serial.print("\t active ");
    Serial.print(Component[x].active);
    Serial.print("\t attributes ");
    for(int y=0 ;y < 10; y++){
      Serial.print(",");
       int z = Component[x].attributes[y];
       Serial.print(z,DEC);
      }
    Serial.println(".");    
  }
}

void PhoneDialMonitor() { //--if the phone is dialed the number is stored 
  // monitors the contacts of the Phone Dial 
  // and decodes the number.
  // sends the rrsult to the Serial port and saves it in attributes
  if(digitalRead(19)){
    if(PhoneDialHis == 0){
      PhoneDialHis = 1;
      PhoneDialInterval = 0;  //begin a dial interval count
      Component[14].active = 0;
      PhoneDialComplete = 0;
    } else {
      PhoneDialInterval += intervalMillis;
    }
  }else{
    if(PhoneDialHis){
      PhoneDialHis = 0;
      if(PhoneDialInterval > 40){PhoneDialCount++;}
//      Serial.println(PhoneDialCount,DEC);
      PhoneDialInterval = 0;  //begin a dial interval count
    }else{
      if(PhoneDialComplete == 0){
        PhoneDialInterval += intervalMillis;
//        Serial.println(PhoneDialInterval,DEC); //save the results
        if(PhoneDialInterval > 200){
            PhoneDialComplete = 1;
            Component[14].active = 1;
            Component[14].attributes[0] = PhoneDialCount;
            Serial.println(PhoneDialCount,DEC); //save the results
            PhoneDialCount = 0;
        }
      }
    }
  }
}

    // Read serial input:
void serialEvent() {    
  while (Serial.available() > 0) {
    int inChar = Serial.read();
    if (isDigit(inChar)) {
      // convert the incoming byte to a char and add it to the string:
      inString += (char)inChar;
    }
    // if you get a letter start an command line for that component
    if (isAlpha(inChar)){
      if(inChar == 'X') dataDump();
      ComponentSelect = inChar - 'A' + 1; // A-P prepare to input a string for a Component
      ComponentSelect = constrain(ComponentSelect,1,15);
//      CompIndex = 0;
      AttCount = 0;
      Serial.print("Component: ");
      Serial.print(ComponentSelect,DEC);
    }
    // if you get a comma index to the next attribute of the component 
    if (inChar == ','){
      inNum = inString.toInt();
      inString = "";
      Component[ComponentSelect].attributes[AttCount] = inNum;
      AttCount++;
      Serial.print(",");
      Serial.print(inNum,DEC);
    }
    // if you get a newline, Save the data, clear the indexes
    // , and activate the Component
    if (inChar == '\n') {
      Serial.write(inChar);
      inNum = inString.toInt();
      inString = "";
      Component[ComponentSelect].attributes[AttCount] = inNum;
      AttCount = 0;
      Component[ComponentSelect].trigger = 1;  // trigger the component
      // Serial.print(",");
      // Serial.print(Component[ComponentSelect].port,DEC);
      // Serial.print(",");
      // Serial.print(inNum,DEC);
      // Serial.println("");
      // clear the string for new input:
    }
  }
}

void PhoneRing() {
  //component[5].attributes[]:
  // 0 number of ring patterns
  // 1 bell on milliseconds
  // 2 bell off milliseconds
  // 3 ring period
  // 4 ring space
  if(Component[5].attributes[0]){  // number of rings is not 0
    PhoneRingPeriod += intervalMillis;
    if(PhoneRinging){
      if(PhoneRingPeriod > Component[5].attributes[3]){
        PhoneRingPeriod = 0;
        PhoneRinging = 0;   // this bit enables ringer
        Component[5].attributes[0] -= 1;  //decrement the number of rings
        // Serial.println(Component[5].attributes[0],DEC); //-------
        if((Component[5].attributes[0]) < 1){ // number of ring patterns
          Component[5].active = 0;    // stop ringing
          digitalWrite(Component[5].port,0);
          // Serial.print("End "); //-------
        }  // if num of rings = 0 deactivate
      }
    }else{
      if(PhoneRingPeriod > Component[5].attributes[4]){
        PhoneRingPeriod = 0;
        PhoneRinging = 1;
      }
    }
    PhoneRingInterval += intervalMillis;
    if(digitalRead(Component[5].port)){
      if(PhoneRingInterval > Component[5].attributes[1]){
        PhoneRingInterval = 0;
        // Serial.print("x"); //--------------------
        digitalWrite(Component[5].port,0);
      }
    }else{
      if(PhoneRingInterval > Component[5].attributes[2]){
        PhoneRingInterval = 0;
        if(PhoneRinging) {digitalWrite(Component[5].port,1);} // turn on the output if ringing enabled
      }
    }
  }
}


void setup() {
  Serial.begin(9600);
  Serial.println("hello");
  for(int x = 1;x < 15;x++){
    pinMode((Component[x].port),OUTPUT);//,A start by making everything an output
  }
  // pinMode(14,OUTPUT);// 0 , {}, "null mode " , // 
  // pinMode(2,OUTPUT);//,A "PC fan    " , //
  // pinMode(4,OUTPUT);//,B "Wthr radio" }, //
  // pinMode(3,OUTPUT);//,C "MalloMeter" }, //
  // pinMode(8,OUTPUT);//,D "Big Relay " }, //
  // pinMode(5,OUTPUT);//,E "PhoneRingr" }, //
  pinMode(17,INPUT_PULLUP);//,F "Hook Contt" }, //
  // pinMode(7,OUTPUT);//,G "MicoWavFan" }, //
  // pinMode(9,OUTPUT);//,H "BigMotor  " }, //
  // pinMode(13,OUTPUT);//,I "MixerMotor" }, //
  // pinMode(10,OUTPUT);//,J "Relay 1   " }, //
  // pinMode(11,OUTPUT);//,K "Relay 2   " }, //
  // pinMode(12,OUTPUT);//,L "Relay 3   " }, //
  // pinMode(6,OUTPUT);//,M "100v Meter" }, //
  pinMode(19,INPUT_PULLUP);//, "DialContac" }, //
  // pinMode(14,OUTPUT);//, "null element..." }, //
  // pinMode(14,OUTPUT);// 0 , {}, "null mode " , // 

  Component[5].attributes [1] = 35; //ms   Set up the phone ringer so you just have to enter the number of rings
  Component[5].attributes [2] = 35; //ms
  Component[5].attributes [3] = 2000; //ms ring time
  Component[5].attributes [4] = 4000; //ms ring space
}

void loop() {
  // this is the clock. Each time the program loops the interval is recorded. It's usually 0,1, or 2 milliseconds.
  intervalMillis = int(millis() - previousMillis);
  previousMillis = millis();
  if(intervalMillis == 3)Serial.println(intervalMillis,DEC);
  delay(1);
  if(millis() > (previousSecond + 1000)){  //set intervalSecond for one loop every second
    previousSecond = millis();
    intervalSecond = 1;
  }else{
    intervalSecond = 0;
  }

  triggerActivate(1);  ////A PC fan
  if(Component[1].active){
    if(intervalSecond) Component[1].attributes[0] -= 1;
    if(Component[1].attributes[0] < 1){  // number of seconds
      Component[1].active = 0;
      digitalWrite(Component[1].port,0);
    }
  }
  triggerActivate(2);  ////B weather radio
  if(Component[2].active){
    if(intervalSecond) Component[2].attributes[0] -= 1;
    if(Component[2].attributes[0] < 1){  // number of seconds
      Component[2].active = 0;
      digitalWrite(Component[2].port,0);
    }
  }
  if(Component[3].trigger){////C, "MalloryMeter 
    analogWrite(Component[3].port,Component[3].attributes[0]); // write analog to the meter
    Component[3].trigger = 0;
  }
  triggerActivate(4);  ////D BIG Relay
  if(Component[4].active){
    Component[4].attributes[0] -= 1;
    if(Component[4].attributes[0] < 1){  
      Component[4].active = 0;
      digitalWrite(Component[4].port,0);
    }
  }
  triggerActivate(5);
  if(Component[5].active){  ////E, "PhoneRinger
      PhoneRing();  
  }
  if(digitalRead(Component[6].port) == 0){////, "HookContact  input
    Component[6].active = 1;
  }
    triggerActivate(7);  ////G Microwave Fan
  if(Component[7].active){
    if(intervalSecond) Component[7].attributes[0] -= 1;
    if(Component[7].attributes[0] < 1){  // number of seconds
      Component[7].active = 0;
      digitalWrite(Component[7].port,0);
    }
  }
  triggerActivate(8);  ////H  Big Motor
  if(Component[8].active){
    if(intervalSecond) Component[8].attributes[0] -= 1;
    if(Component[8].attributes[0] < 1){  // number of seconds
      Component[8].active = 0;
      digitalWrite(Component[8].port,0);
    }
  }
  triggerActivate(9);  ////I Mixer Motor
  if(Component[9].active){
    if(intervalSecond) Component[9].attributes[0] -= 1;
    if(Component[9].attributes[0] < 1){  // number of seconds
      Component[9].active = 0;
      digitalWrite(Component[9].port,0);
    }
  }
  triggerActivate(10);  ////J relay 1
  if(Component[10].active){
    Component[10].attributes[0] -= 1;
    if(Component[10].attributes[0] < 1){  // number of milliseconds
      Component[10].active = 0;
      digitalWrite(Component[10].port,0);
    }
  }
  triggerActivate(11);  ////K relay 2
  if(Component[11].active){
    Component[11].attributes[0] -= 1;
    if(Component[11].attributes[0] < 1){  // number of milliseconds
      Component[11].active = 0;
      digitalWrite(Component[11].port,0);
    }
  }
  triggerActivate(12);  ////L relay 3
  if(Component[12].active){
    Component[12].attributes[0] -= 1;
    if(Component[12].attributes[0] < 1){  // number of milliseconds
      Component[12].active = 0;
      digitalWrite(Component[12].port,0);
    }
  }
  if(Component[13].trigger){////M, "100V Meter 
    analogWrite(Component[13].port,Component[13].attributes[0]); // write analog to the meter
    Component[13].trigger = 0;
  }
    PhoneDialMonitor();  // look for any phone dialling activity
}


