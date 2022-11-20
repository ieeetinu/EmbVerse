/*

Project : Simulation of Body Control Unit of Car

Developed by : EmbVerse (www.embverse.com)

Description : An Arduino Uno driven Body Control Unit is simulated that controls the

switching of Head Light, Fog Light and Parking Light of a car. The three lights are 

powered on 12V DC supply and relays are used to get them connnected with Arduino.

Inputs to Arduino :

1. Head Light Switch to ADC through a Voltage Divider Circuit

2. Fog Light Switch to ADC through a Voltage Divider Circuit

3. Parking Light Switch to ADC through a Voltage Divider Circuit

4. Auto Mode Switch to ADC through a Voltage Divider Circuit

- All these four inputs are connected in such a manner that they go to the same ADC

5. A potentiometer connected to ADC replacing a light sensor for simulation ease

Outputs from ADC :

1. Auto Mode LED

2. Relay driving Head Light

3. Relay driving Fog Light

4. Relay driving Parking Light

*/

/************************************************************************************/

/*The below four defined constants are the relevant ADC counts generated when the 

corresponding switch is pressed.*/

#define PARKING_LIGHT_SWITCH 682		

#define FOG_LIGHT_SWITCH 728

#define HEAD_LIGHT_SWITCH 775

#define AUTO_SWITCH 857

/*************************************************************************************/

// The below four defined constants set the Arduino Pins as a label

#define PARKING_LIGHT 2

#define FOG_LIGHT 3

#define HEAD_LIGHT 4

#define AUTO_LED 5

// The ADC count read can vary in actual scenario, so there is an OFFSET taken here.

// For Example, the AUTO_SWITCH is at 857, so all the values between 852 and 862 are valid.

#define OFFSET 5

//The below four variables are for tracking the button state

int buttonState1 = LOW;

int buttonState2 = LOW;

int buttonState3 = LOW;

int buttonState4 = LOW;



//The below variable is for checking if the head light switch is pressed or not.

int pressedhl = 0;



//The below variable is to get the current state of the switch

int stateNow;



//The below variable reads the ADC corresponded to the light sensor.

int lightsensor_reading;



void setup() 

{

  //The below six lines set input and output pin

  pinMode(PARKING_LIGHT, OUTPUT);

  pinMode(FOG_LIGHT, OUTPUT);

  pinMode(HEAD_LIGHT, OUTPUT);

  pinMode(AUTO_LED, OUTPUT);

  pinMode(A0, INPUT);

  pinMode(A1, INPUT);

  

  //The below four lines initialize output pin levels

  digitalWrite(PARKING_LIGHT, LOW);

  digitalWrite(FOG_LIGHT, LOW);

  digitalWrite(HEAD_LIGHT, LOW);

  digitalWrite(AUTO_LED, LOW);

  //The below line initializes the UART with baud rate 9600

  Serial.begin(9600);

}

/*

Function : boolean debounceButton(int state)

Description : This function uses switch debouncing technique to recognize the switch press.

It takes the Defined ADC Counts of switches as argument, takes the ADC Reading, compares it 

with defined ADC Counts of switches considering the Offset. If is within the limits, it holds the 

code for 10 ms and takes the ADC Reading again. If it is still within limits, it returns High,

else it returns a Low.

*/

boolean debounceButton(int state)

{

  stateNow = analogRead(A0);

  if((state + OFFSET) >= stateNow && stateNow >= (state - OFFSET))

  {

    delay(10); //waiting 10ms before rechecking state of button press

    stateNow = analogRead(A0);

  }

  if((state + OFFSET) >= stateNow && stateNow >= (state - OFFSET))

  {

      return HIGH;

  }

  else

  {

      return LOW;

  }

}



//The main code

void loop() 

{  

  // Check if Parking Light ADC value is in the range

  if(debounceButton(PARKING_LIGHT_SWITCH) == HIGH && buttonState1 == LOW)

  {

	digitalWrite(PARKING_LIGHT, !digitalRead(PARKING_LIGHT)); //Invert the light state

    buttonState1 = HIGH;

  }

  else if(debounceButton(PARKING_LIGHT_SWITCH) == LOW && buttonState1 == HIGH)

  {

       buttonState1 = LOW;

  }

  

  // Check if Fog Light ADC value is in the range

  if(debounceButton(FOG_LIGHT_SWITCH) == HIGH && buttonState2 == LOW)

  {

    digitalWrite(FOG_LIGHT, !digitalRead(FOG_LIGHT));  //Toggle the light state

    buttonState2 = HIGH;

  }

  else if(debounceButton(FOG_LIGHT_SWITCH) == LOW && buttonState2 == HIGH)

  {

       buttonState2 = LOW;

  }

  

  // Check if Head Light ADC value is in the range

  if(debounceButton(HEAD_LIGHT_SWITCH) == HIGH && buttonState3 == LOW)

  {

    pressedhl++;  //Set the variable value

    buttonState3 = HIGH;

  }

  else if(debounceButton(HEAD_LIGHT_SWITCH) == LOW && buttonState3 == HIGH)

  {

       buttonState3 = LOW;

  }

  

  // Check if Auto LED ADC value is in the range

  if(debounceButton(AUTO_SWITCH) == HIGH && buttonState4 == LOW)

  {

	digitalWrite(AUTO_LED, !digitalRead(AUTO_LED));  //Toggle the LED state

    buttonState4 = HIGH;

  }

  else if(debounceButton(AUTO_SWITCH) == LOW && buttonState4 == HIGH)

  {

       buttonState4 = LOW;

  }

  

/*---------The Auto mode of operation----------------------*/

  lightsensor_reading = analogRead(A1);

  

  //Checking if the sensor reading is in range while the Auto LED is turned ON

  if(lightsensor_reading >= 256 && digitalRead(AUTO_LED) == HIGH)

  {

      digitalWrite(HEAD_LIGHT, HIGH);

  }

  else if(lightsensor_reading < 256 && digitalRead(AUTO_LED) == HIGH)

  {

      digitalWrite(HEAD_LIGHT, LOW);

  }

  

  //If variable is SET and Auto mode is OFF, toggle the Head light

  if(pressedhl == 1 && digitalRead(AUTO_LED) == LOW)

  {

      pressedhl = 0; //RESET the variable

      digitalWrite(HEAD_LIGHT, !digitalRead(HEAD_LIGHT)); //Toggle light state

  }

  

  /*If variable is SET and Auto mode is ON, DO NOT toggle the Head light and let

  it be in its present state*/

  else if(pressedhl == 1 && digitalRead(AUTO_LED) == HIGH)

  {

      pressedhl = 0;

  }

  

  //Checking Analog input readings through Serial monitor

  Serial.print("Switch Reading : ");

  Serial.print(analogRead(A0));

  Serial.print("  Sensor Reading : ");

  Serial.println(lightsensor_reading);

  delay(100);

}