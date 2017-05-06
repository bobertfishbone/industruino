/*
* Industruino Demo Code - Default code loaded onto Industruino REV3
*
* Copyright (c) 2013 Loic De Buck <contact.industruino@gmail.com>
*
* Industruino is a DIN-rail mountable Arduino Leonardo compatible product
* Please visit www.industruino.com for further information and code examples.
* Standard peripherals connected to Industruino are:
* UC1701 compatible LCD; rst:D19 dc:D20 dn:D21 sclk:D22 (set pin configuration in UC1701 library header) 
* 3-button membrane panel; analog pin A5
*/

#include <Wire.h>
#include <UC1701.h>
#include <EEPROM.h>

int minutes[9];
int temp1[9];
int temp2[9];
int temp3[9];
int currentStage = 1;
int totalStages = 1;
unsigned long currentMillis;
unsigned long previousMillis;
bool started = false;

static UC1701 lcd;

String thingtochange = "";
String numtochange = "";

//menu defines

//- initial cursor parameters
int coll = 0; //column counter for cursor - always kept at 0 in this demo (left side of the screen)
int channel = 0; //Counter is controlled by the up&down buttons on the membrane panel. Has double use; 1. As row controller for the cursor (screen displays 6 rows of text, counting from 0 to 5). 2. As editor for numerical values shown on screen 
int lastChannel = 0; //keeps track of previous 'channel'. Is used to detect change in state.

//- initial menu level parameters
int MenuLevel = 0; //Defines the depth of the menu tree
int MenuID = 0; //Defines the unique identifier of each menu that resides on the same menu level
int channelUpLimit = 5; //Defines the upper limit of the button counter: 1. To limit cursor's downward row movement 2. To set the upper limit of value that is beeing edited.
int channelLowLimit = 0; //Defines the lower limit of the button counter: 1. To limit cursor's upward row movement 2. To set the lower limit of value that is beeing edited.

//- initial parameters for 'value editing mode'
int valueEditing = 0; //Flag to indicate if the interface is in 'value editing mode', thus disabling cursor movement.
int row = 0; //Temporary location to store the current cursor position whilst in 'value editing mode'.
int constrainEnc = 1; //Enable/disable constraining the button panel's counter to a lower and upper limit.
float valueEditingInc = 0; //Increments of each button press when using 'value editing mode'. 
float TargetValue = 0; // Target value to be edited in 'value editing mode'

//Membrane panel button defines

int buttonUpState = 0; //status of "Up" button input
int buttonEnterState = 0; //status of "Enter" button input
int buttonDownState = 0; //status of "Down" button input

int prevBtnUp = 0; //previous state of "Up" button
int prevBtnEnt = 0; //previous state of "Enter" button
int prevBtnDown = 0; //previous state of "Down" button

int lastBtnUp = 0; //time since last "Up" pressed event
int lastBtnEnt = 0; //time since last "Enter" pressed event
int lastBtnDown = 0; //time since last "Down" pressed event

int enterPressed = 0; //status of "Enter" button after debounce filtering : 1 = pressed 0 = unpressed

int transEntInt = 250; //debounce treshold for "Enter" button
int transInt = 100; //debounce for other buttons
unsigned long lastAdminActionTime = 0; //keeps track of last button activity

// These constants won't change.  They're used to give names
// to the pins used:
const int analogInPin = A5;  // Analog input pin that the button panel is attached to
const int backlightPin = 26; // PWM output pin that the LED backlight is attached to
const int buttonEnterPin = 24;
const int buttonUpPin = 25;
const int buttonDownPin = 23;
const int D0 = 0;
const int D1 = 1;
const int D2 = 2;
const int D3 = 3;
const int D4 = 4;
const int D5 = 5;
const int D6 = 6;
const int D7 = 7;
const int D8 = 8;
const int D9 = 9;
const int D10 = 10;
const int D11 = 11;
const int D12 = 12;
const int D14 = 14;
const int D15 = 15;
const int D16 = 16;
const int D17 = 17;
const int tempPin1 = 0;
const int tempPin2 = 1;
const int tempPin3 = 2;
const int dampercontrol1 = 3;
const int dampercontrol2 = 4;
const int dampercontrol3 = 5;


int ButtonsAnalogValue = 0;        // value read from mebrane panel buttons.
int backlightIntensity = 0;        // LCD backlight intesity
int backlightIntensityDef = 0;     // Default LCD backlight intesity
unsigned long lastLCDredraw = 0;   // keeps track of last time the screen was redrawn

void setup() {
  
  SetInput(); //Sets all general pins to input
  backlightIntensity = 0; //loads the backlight intensity from EEPROM
  pinMode(buttonEnterPin, INPUT);
  pinMode(buttonUpPin, INPUT);
  pinMode(buttonDownPin, INPUT);
  pinMode(backlightPin, OUTPUT); //set backlight pin to output 
  digitalWrite (backlightPin, backlightIntensity); //convert backlight intesity from a value of 0-5 to a value of 0-255 for PWM.

//LCD init
  lcd.begin();  //sets the resolution of the LCD screen
  
  for (int y=0; y <= 7; y++){
for (int x=0; x <= 128; x++){
        lcd.setCursor(x, y);
        lcd.print(" ");
   } 
}
  
//debug  
 Serial.begin(9600); //enables Serial port for debugging messages

//Menu init  
 ReadEEPROM();
 MenuSplash(); //load first menu
 previousMillis = millis();
}

/*
* 1. The loop function calls a function to check the buttons (this could also be driven by timer interrupt) and updates the button counter (variable called 'channel'), which increases when 'Down' button is pressed and decreases when "Up" buttons is pressed.
* 2. Next, the loop function calls the 'Navigate' function which draws the cursor in a position based on the button counter, and when the "Enter" button is pressed checks which new menu should be  loaded or what other action to perform.
* 3. Each menu's content and scope is defined in a separate function. Each menu should have a defined 'MenuLevel' (depth of the menu tree, starting from 0) and unique MenuID so that the Navigate function can discern which menu is active.

*To make your own menus you should take 2 steps:

*1. make a new menu function, edit the parameters such MenuLevel and MenuID, scope of the cursor (number of rows, constraints etc).
*2. Edit the 'Navigate' function to reflect the menu function that you just made and assigning an action to it. 
*/

void loop() { 
	getInput();
	if (started) {  

  currentMillis = millis();  

  

  if (currentMillis - previousMillis >= (minutes[currentStage] * 60000)) {
      previousMillis = currentMillis;
     if (currentStage < totalStages){
      currentStage++;
      }
      else Stop();
  }
  if (getTemp(tempPin1) > temp1[currentStage]) {
    moveDamper(dampercontrol1);
  }
  
  }
  
  ReadButtons(); //check buttons
  Navigate(); //update menus and perform actions
  delay(50);
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
//UI menu content - edit, add or remove these functions to make your own menu structure
//These functions only generate the content that is printed to the screen, please also edit the "Navigate" function further below to add actions to each menu.
//------------------------------------------------------------------------------------------------------------------------------------------------------------

void MenuSplash() { //this function draws the first menu - splash screen
//menu inintialisers
  channel = 0; //starting row position of the cursor (top row) - controlled by the button panel counter
  channelUpLimit = 0; //upper row limit
  channelLowLimit = 0; //lower row limit
  MenuLevel = 0; //menu tree depth -> first level
  MenuID = 0; //unique menu id -> has to be unique for each menu on the same menu level.
  enterPressed = 0; //clears any possible accidental "Enter" presses that could have been caried over from the previous menu
  lcd.clear(); //clear the screen
//actual user content on the screen
  lcd.setCursor(5, 2); //set the cursor to the fifth pixel from the left edge, third row.
  lcd.print("Malt Gargler 5000"); //print text on screen
}

void MenuMain() { //second menu - choice of submenu's
//menu inintialisers
  channel = 0; //starting row position of the cursor (top row) - controlled by the button panel counter
  channelUpLimit = 3; //upper row limit
  MenuLevel = 1; //menu tree depth -> second level
  MenuID = 1; //unique menu id -> has to be unique for each menu on the same menu level.
  enterPressed = 0; //clears any possible accidental "Enter" presses that could have been caried over from the previous menu
  lcd.clear(); //clear the screen
  ScrollCursor(); //enable the moving cursor (note that this function is not called in the splash screen, thus disabling the cursor)
//actual user content on the screen
  lcd.setCursor(6, 0); //set the cursor to the sixth pixel from the left edge, first row.
  lcd.print("Setup"); //print text on screen
  lcd.setCursor(6, 1); //set the cursor to the sixth pixel from the left edge, second row.
  lcd.print("View Current Status"); //print text on screen
  lcd.setCursor(6, 2); //set the cursor to the sixth pixel from the left edge, third row.
  if (started) {
    lcd.print("Stop"); //print text on screen
  }
  else lcd.print("Start");
  lcd.setCursor(6, 3);
  lcd.print("Back");  
}

void MenuSetup() { //submenu of Main menu - setup screen for Industruino
  channel = 0;
  channelUpLimit = 2;
  channelLowLimit = 0;
  MenuID = 1;
  MenuLevel = 2;
  enterPressed = 0;
  lcd.clear();
  ScrollCursor();
  lcd.setCursor(6, 0);
  lcd.print("Stages     ");
  lcd.setCursor(100, 0);
  lcd.print(totalStages,1);
  lcd.setCursor(6, 1);
  lcd.print("Config Stages");
  lcd.setCursor(6, 2);
  lcd.print("Back");
}

void StageSetup(int stage){
  channel = 0;
  channelUpLimit = 6;
  channelLowLimit = 0;
  MenuID = 1;
  MenuLevel = 3;
  enterPressed = 0;
  lcd.clear();
  ScrollCursor();
  lcd.setCursor(6, 0);
  lcd.print("Time (mins)    ");
  lcd.setCursor(100, 0);
  lcd.print(minutes[stage]);
  lcd.setCursor(6, 1);
  lcd.print("Temp (F)    ");
  lcd.setCursor(100, 1);
  lcd.print(temp1[stage]);
  lcd.setCursor(6, 2);
  lcd.print("Temp (F)    ");
  lcd.setCursor(100, 2);
  lcd.print(temp2[stage]);
  lcd.setCursor(6, 3);
  lcd.print("Temp (F)    ");
  lcd.setCursor(100, 3);
  lcd.print(temp3[stage]);  
  lcd.setCursor(6, 4);
  lcd.print("Next");
  lcd.setCursor(6, 5);
  lcd.print("Back");
  lcd.setCursor(6, 6);
  lcd.print("Exit");
  lcd.setCursor(6, 7);
  lcd.print("Stage ");
  lcd.print(stage);
  lcd.print("/");
  lcd.print(totalStages);
}

void CurrentState(){
   channel = 0;
  channelUpLimit = 6;
  channelLowLimit = 6;
  MenuID = 2;
  MenuLevel = 2;
  enterPressed = 0;
  lcd.clear();
  ScrollCursor();
  lcd.setCursor(6, 0);
  lcd.print("Time (mins)    ");
  lcd.setCursor(100, 0);
  lcd.print((currentMillis / 60000));
  lcd.print("/");
  lcd.print(minutes[currentStage]);
  lcd.setCursor(6, 1);
  lcd.print("Temp (F)    ");
  lcd.setCursor(100, 1);
  lcd.print(getTemp(tempPin1), 2);
  lcd.setCursor(6, 2);
  lcd.print("Temp (F)    ");
  lcd.setCursor(100, 2);
  lcd.print(getTemp(tempPin2), 2);
  lcd.setCursor(6, 3);
  lcd.print("Temp (F)    ");
  lcd.setCursor(100, 3);
  lcd.print(getTemp(tempPin3), 2);  
  lcd.setCursor(6, 6);
  lcd.print("Exit");
  lcd.setCursor(6, 7);
  lcd.print("Stage ");
  lcd.print(currentStage);
  lcd.print("/");
  lcd.print(totalStages);
}


//---------------------------------------------------------------------------------------------------------------------------------------------------
//UI control logic, please edit this function to reflect the specific menus that your created above and your desired actions for each cursor position
//---------------------------------------------------------------------------------------------------------------------------------------------------



void Navigate()
{ 

if (valueEditing != 1){
  
  if (MenuLevel == 0) //check if current activated menu is the 'splash screen' (first level)
  {
      {
      if (enterPressed == 1) MenuMain(); //if enter is pressed load the 'Main menu'
      }
  }
  
   if (MenuLevel == 1){ //check if current activated menu is the 'Main menu' (first level)
    
    if (channel == 0 && enterPressed == 1) MenuSetup(); //if cursor is on the first row and enter is pressed load the 'Setup' menu
    if (channel == 1 && enterPressed == 1) CurrentState(); //if cursor is on the second row and enter is pressed load the 'Demo' menu
	if (channel == 2 && enterPressed == 1 && !started) {
		WriteEEPROM();
		Start(); //if cursor is on the third row and enter is pressed load the 'splash screen'
	}
    if (channel == 2 && enterPressed == 1 && started) Stop(); //if cursor is on the third row and enter is pressed load the 'splash screen'
    if (channel == 3 && enterPressed == 1) MenuSplash();
     }
  
    
    
    if (MenuLevel == 2){ 
        if (MenuID == 1){ // MenuSetup
          if (channel == 0 && enterPressed == 1) {
            totalStages = EditValue(9, 1, 1);
          }
          if (channel == 1 && enterPressed == 1) StageSetup(currentStage);
          if (channel == 2 && enterPressed == 1) MenuMain();
        }
        if (MenuID == 2){
          if (channel == 6 && enterPressed == 1) MenuMain();
        }
    }

    if (MenuLevel == 3){
      if (MenuID == 1){ //StageSetup
        if (channel == 0 && enterPressed == 1) minutes[currentStage] = EditValue(600, 5, 90);
        if (channel == 1 && enterPressed == 1) temp1[currentStage] = EditValue(400, 10, 200);
        if (channel == 2 && enterPressed == 1) temp2[currentStage] = EditValue(400, 10, 200);
        if (channel == 3 && enterPressed == 1) temp3[currentStage] = EditValue(400, 10, 200);
        if (channel == 4 && enterPressed == 1) {
          if (currentStage < totalStages) currentStage++;
		  else currentStage = 1;
          StageSetup(currentStage);
        }
        
        if (channel == 5 && enterPressed == 1) {
          if (currentStage > 0) currentStage--;
		  else currentStage = totalStages;
          StageSetup(currentStage);
        }
        if (channel == 6 && enterPressed == 1) MenuSetup();
      }
    }
      
      
 //dont remove this part
 if (channel != lastChannel && valueEditing != 1 && MenuID != 0){ //updates the cursor position if button counter changed and 'value editing mode' is not running
  ScrollCursor();
  }
}
}


//---------------------------------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------------------------------
  
  
  float EditValue(int maxval, int incBy, int localStart) //a function to edit a variable using the UI - function is called by the main 'Navigate' UI control function and is loaded with a variable to be edited
{
  row = channel; //save the current cursor position so that after using the buttons for 'value editing mode' the cursor position can be reinstated.
  channel = 0; //reset the button counter so to avoid carrying over a value from the cursor.
  TargetValue = localStart;
  constrainEnc = 0; //disable constrainment of button counter's range
  valueEditingInc = incBy; //increment for each button press
  valueEditing = 1; //flag to indicate that we are going into 'value editing mode'.
  enterPressed = 0; //clears any possible accidental "Enter" presses that could have been caried over
  while (enterPressed != 1){ //stays in 'value editing mode' until enter is pressed
  ReadButtons(); //check the buttons for any change
  if (channel != lastChannel){ //when up or down button is pressed
       if (channel < lastChannel && TargetValue < maxval){ //if 'Up' button is pressed, and is within constraint range.
         TargetValue+= valueEditingInc; //increment target variable with pre-defined increment value
       }
       if (channel > lastChannel && TargetValue > 1){ //if 'Down' button is pressed, and is within constraint range.
         TargetValue-= valueEditingInc ; //decrement target variable with pre-defined increment value
       }
      //clear a section of a row to make space for updated value
      for (int i=95; i <= 105; i++){ 
      lcd.setCursor(i, row);
      lcd.print("   ");
       }
       //print updated value
       lcd.setCursor(100, row);
       Serial.println(TargetValue);
       lcd.print(TargetValue, 0);
       lastChannel = channel;
       }  
  delay(50);
  }  
  channel = row; //load back the previous row position to the button counter so that the cursor stays in the same position as it was left before switching to 'value editing mode'
  constrainEnc = 0; //enable constrainment of button counter's range so to stay within the menu's range
  channelUpLimit = 2; //upper row limit
  valueEditing = 0; //flag to indicate that we are leaving 'value editing mode'
  enterPressed = 0; //clears any possible accidental "Enter" presses that could have been caried over
  return TargetValue; //return the edited value to the main 'Navigate' UI control function for further processing
}
  
  
//--------------------------------------------------------------------------------------------------------------------------------------------- 
// Peripheral functions
//--------------------------------------------------------------------------------------------------------------------------------------------- 
 void ReadButtons(){ 
 
  buttonEnterState = digitalRead(buttonEnterPin);
  buttonUpState = digitalRead(buttonUpPin);
  buttonDownState = digitalRead(buttonDownPin);
  
      if (buttonEnterState == HIGH && prevBtnEnt == LOW)
      {
         if ((millis() - lastBtnEnt) > transEntInt) 
         {
         enterPressed = 1;
         }
         lastBtnEnt = millis();
         lastAdminActionTime = millis();
         Serial.println(enterPressed);
       }
      prevBtnEnt = buttonEnterState;
      
      
           if (buttonUpState == HIGH && prevBtnUp == LOW)
      {
         if ((millis() - lastBtnUp) > transInt) 
         {
         channel--;
         }
         lastBtnUp = millis();
         lastAdminActionTime = millis();
         //Serial.println("UpPressed");
       }
      prevBtnUp = buttonUpState;
      
      
           if (buttonDownState == HIGH && prevBtnDown == LOW)
      {
         if ((millis() - lastBtnDown) > transInt) 
         {
         channel++;
         }
         lastBtnDown = millis();
         lastAdminActionTime = millis();
         //Serial.println("DownPressed");
       }
      prevBtnDown = buttonDownState;
      
       if (constrainEnc == 1){
   channel = constrain(channel, channelLowLimit, channelUpLimit);
   }
      
 }

 
 
 void SetOutput(){  // a simple function called to set a group of pins as outputs
  pinMode(D0, OUTPUT);
  pinMode(D1, OUTPUT); 
  pinMode(D2, OUTPUT);
  pinMode(D3, OUTPUT);
  pinMode(D4, OUTPUT);
  pinMode(D5, OUTPUT); 
  pinMode(D6, OUTPUT); 
  pinMode(D7, OUTPUT);
  pinMode(D8, OUTPUT);   
  pinMode(D9, OUTPUT); 
  pinMode(D10, OUTPUT);
  pinMode(D11, OUTPUT);
  pinMode(D12, OUTPUT);
  pinMode(D14, OUTPUT); 
  pinMode(D15, OUTPUT);  
  pinMode(D16, OUTPUT);
  pinMode(D17, OUTPUT); 
 }
 
void SetInput(){  // a simple function called to set a group of pins as inputs
  pinMode(D0, INPUT);
  pinMode(D1, INPUT);
  pinMode(D2, INPUT);  
  pinMode(D3, INPUT);
  pinMode(D4, INPUT);
  pinMode(D5, INPUT); 
  pinMode(D6, INPUT); 
  pinMode(D7, INPUT);
  pinMode(D8, INPUT);  
  pinMode(D9, INPUT); 
  pinMode(D10, INPUT);
  pinMode(D11, INPUT);
  pinMode(D12, INPUT);
  pinMode(D14, INPUT); 
  pinMode(D15, INPUT);  
  pinMode(D16, INPUT);
  pinMode(D17, INPUT); 
 }
 
 void ResetParameters(){ //resets the setup parameters of Industruino and saves the settings to EEPROM
   
   backlightIntensity = backlightIntensityDef; //load the default backlight intensity value
   digitalWrite(backlightPin, backlightIntensity); //map the value (from 0-5) to a corresponding PWM value (0-255) and update the output 
   EEPROM.write(0, backlightIntensity); //save the current backlight intensity to EEPROM
   MenuSetup(); //return to the setup menu
 }
 
 

//---------------------------------------------------------------------------------------------------------------------------------------------
// UI core functions
//--------------------------------------------------------------------------------------------------------------------------------------------- 
 


void ScrollCursor() //makes the cursor move
{
  lastChannel = channel; //keep track button counter changes
  for (int i=0; i <= 6; i++){ //clear the whole column when redrawing a new cursor
      lcd.setCursor(coll, i);
      lcd.print(" ");
   } 
  lcd.setCursor(coll, channel); //set new cursor position
  lcd.print(">"); //draw cursor
 
  }
  
  
//---------------------------------------------------------------------------------------------------------------------------------------------
// Malter Functions
//---------------------------------------------------------------------------------------------------------------------------------------------   

void moveDamper(int damper) {
  
}

double getTemp(int thermPin) {
  double mAVal = analogRead(thermPin);
  double temp = mAVal * 2;
  return temp;
}

void Start(){
  currentStage = 1;
started = true;
Serial.println("Cycle started!");
MenuMain();
}

void Stop(){
  started = false;
  Serial.println("Cycle stopped!");
  MenuMain();
}

void help() {
	Serial.println("Thermometer 1: T1");
	Serial.println("Thermometer 2: T2");
	Serial.println("Thermometer 3: T3");
	Serial.println("Stage Time: ST");
}

void SerialControl(char pin, int newVal) {
	switch (pin) {
	case 'A':
		temp1[currentStage] = newVal;
		break;
	case 'B':
		temp2[currentStage] = newVal;
		break;
	case 'C':
		temp3[currentStage] = newVal;
		break;
	case 'T':
		minutes[currentStage] = newVal;
		break;
	}
	
}

void ReadEEPROM() {
	for (int i = 1; i <= totalStages; i++) {
		minutes[i] = EEPROM.read(i);
		temp1[i] = EEPROM.read(i + 20);
		temp2[i] = EEPROM.read(i + 40);
		temp3[i] = EEPROM.read(i + 60);
	}
}

void WriteEEPROM() {
	for (int i = 1; i <= totalStages; i++) {
		EEPROM.update(i, minutes[i]);
		EEPROM.update(i + 20, temp1[i]);
		EEPROM.update(i + 40, temp2[i]);
		EEPROM.update(i + 60, temp3[i]);
	}
}

void getInput() {
	thingtochange = "";
	numtochange = "";
	char newchar;
	int word = 1;
	String stage = "";
	while (Serial.available() > 0) {
		newchar = Serial.read();
		if (newchar == '?') {
			help();
			Serial.flush();
		}
		if (newchar == ' ') word++;
		if (word == 1 && newchar != ' ' && newchar != '\n') {
			thingtochange += newchar;
			
		}
		
		if (word == 2 && newchar != ' ' && newchar != '\n') {
			stage += newchar;
			
		}
		if (word == 3 && newchar != ' ' && newchar != '\n') {
			numtochange += newchar;
			
		}
		if (newchar == '\n') {
			//Serial.println(thingtochange);
			//Serial.println(stage);
			//Serial.println(numtochange);
			ParseInput(thingtochange, stage, numtochange);
			break;
		}
	}
	
}

void ParseInput(String thing, String localstage, String localamount) {
	int amount = localamount.toInt();
	int stage = localstage.toInt();

	if (thing == "time") {
		minutes[stage] = amount;
		Serial.print("Stage ");
		Serial.print(stage);
		Serial.print(" time changed to ");
		Serial.print(amount); 
		Serial.println(" minutes");
	}
	else if (thing == "set1") {
		temp1[stage] = amount;
		Serial.print("Setpoint 1 for stage ");
		Serial.print(stage);
		Serial.print(" changed to ");
		Serial.print(amount);
		Serial.println(" degrees");
	}
	else if (thing == "set2") {
		temp2[stage] = amount;
		Serial.print("Setpoint 2 for stage ");
		Serial.print(stage);
		Serial.print(" changed to ");
		Serial.print(amount);
		Serial.println(" degrees");
	}
	else if (thing == "set3") {
		temp3[stage] = amount;
		Serial.print("Setpoint 3 for stage ");
		Serial.print(stage);
		Serial.print(" changed to ");
		Serial.print(amount);
		Serial.println(" degrees");
	}
	else if (thing == "status") {
		Serial.print("Stage ");
		Serial.print(currentStage);
		Serial.print(" of ");
		Serial.println(totalStages);
		Serial.print("Elapsed Stage Time: ");
		Serial.println(previousMillis / 60000);
		Serial.print("Total Stage Time: ");
		Serial.println(minutes[currentStage]);
		Serial.print("Temp 1 ");
		Serial.print(getTemp(tempPin1));
		Serial.print(", Setpoint ");
		Serial.println(temp1[currentStage]);
		Serial.print("Temp 2 ");
		Serial.print(getTemp(tempPin2));
		Serial.print(", Setpoint ");
		Serial.println(temp2[currentStage]);
		Serial.print("Temp 3 ");
		Serial.print(getTemp(tempPin3));
		Serial.print(", Setpoint ");
		Serial.println(temp3[currentStage]);
	}
	else if (thing == "stop") Stop();
	else if (thing == "start") Start();
	else if (thing == "list") {
		for (int i = 1; i <= totalStages; i++) {
			Serial.print("Stage ");
			Serial.print(i);
			Serial.println(": ");
			Serial.print("Time: ");
			Serial.println(minutes[i]);
			Serial.print("Setpoint 1: ");
			Serial.println(temp1[i]);
			Serial.print("Setpoint 2: ");
			Serial.println(temp2[i]);
			Serial.print("Setpoint 3: ");
			Serial.println(temp3[i]);
		}
	}
	else Serial.println("Invalid input!");
	}
