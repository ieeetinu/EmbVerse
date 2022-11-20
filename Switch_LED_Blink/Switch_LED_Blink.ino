/*
Q-1 Switch & LED Blinking with Arduino:
There is a tactile switch (only one switch) and LED connected to Arduino. The LED is off initially.
Depending on switch press, the LED blinks in following way.
1st Switch press: LED blinks at frequency of 0.5 Hz.
2nd Switch press: LED blinks at frequency of 1 Hz.
3rd Switch press: LED blinks at frequency of 2 Hz.
4th Switch press: LED turns off.
Create circuit and code for Arduino which can achieve following. Assume proper pins for connecting
LED and Switch.
*/
#include "TimerOne.h"
#define RESET 0 // Constant for flags
#define SET !RESET // Constant for flags
#define SWITCH_PIN 3 // Defining switch input pin as pin 3 of Arduino Nano because it has interrupt available
#define LED_PIN 13 // Defining output for LED blinking as pin 13 of Arduino Nano because LED is at pin 13 on board
#define REFERENCE_TIME 1000000 // 1000000 us that is 1 second of reference time is set based on which different constants are set
#define SWITCH_DEBOUNCE_FREQUENCY 20 // 50 ms time that is 20 Hz frequency is set for testing valid switch input
#define TIMER_ONE_INTERRUPT_TIME 10 // Timer 1 interrupt occurance time in us
#define SWITCH_DEBOUNCE_TIME_LIMIT (REFERENCE_TIME / SWITCH_DEBOUNCE_FREQUENCY) / TIMER_ONE_INTERRUPT_TIME // Switch debounce time limit count for correct switch press detection
#define LED_PIN_TOGGLE_TIME_TWO_Hz (REFERENCE_TIME / 2) / TIMER_ONE_INTERRUPT_TIME // Output LED toggle time count for 2 Hz of blinking
#define LED_PIN_TOGGLE_TIME_ONE_Hz REFERENCE_TIME / TIMER_ONE_INTERRUPT_TIME // Output LED toggle time count for 1 Hz of blinking
#define LED_PIN_TOGGLE_TIME_HALF_Hz (REFERENCE_TIME * 2) / TIMER_ONE_INTERRUPT_TIME // Output LED toggle time count for 0.5 Hz of blinking
#define LED_PIN_OFF digitalWrite(LED_PIN, LOW) // Switching LED Off
#define LED_PIN_TOGGLE digitalWrite(LED_PIN, !digitalRead(LED_PIN)) // Toggling LED 
uint16_t switch_pressed_counter = 0;
uint32_t led_pin_toggle_counter = 0; // Counter for LED blinking which, when reaches to specific limit based on operation, resets 
uint8_t led_blink_frequency_counter = 0; // Counter for deciding the LED Blinking frequency, which resets after being greater than 3
bool switch_current_state = HIGH; // Variable that stores the current state of the digital input pin 3 by reading its value, set to HIGH because HIGH indicates the switch is open 
bool switch_previous_state = HIGH; // Variable used for switch debouncing correction, set to HIGH because HIGH indicates the switch is open
bool switch_interrupt_flag = RESET; // Flag that is polled in the Timer ISR to decide whether the switch is pressed
bool switch_pressed_flag = RESET; // Once the switch_interrupt_flag becomes TRUE, this flag becomes TRUE and gets reset to FALSE after SWITCH_DEBOUNCE_TIME_LIMIT
void timer_one_isr(void)
{
    if(switch_interrupt_flag == SET) // Falling Edge detected at Pin 3
    {
        switch_interrupt_flag = RESET; // Resetting the switch interrupt flag
        switch_current_state = digitalRead(SWITCH_PIN); // Reading Switch Pin 3 and storing  
        if(switch_current_state == LOW) // The state of the Pin 3 has to be LOW to proceed further 
        {
            switch_pressed_flag = SET; // Flag is set to correct switch debounce
            switch_previous_state = switch_current_state; // Reference is stored to check switch debounce later 
        }     
    }
    if(switch_pressed_flag == SET) // Polling the flag to re-enter for cofirming correct switch input entry everytime till SWITCH_DEBOUNCE_TIME_LIMIT is reached
    {
        switch_pressed_counter++; // Increment the counter till SWITCH_DEBOUNCE_TIME_LIMIT    
        if(switch_pressed_counter >= SWITCH_DEBOUNCE_TIME_LIMIT) // SWITCH_DEBOUNCE_TIME_LIMIT is achieved
        {
            switch_current_state = digitalRead(SWITCH_PIN); // Reading Switch Pin 3 and storing
            if(switch_current_state == switch_previous_state) // Comparing the reading with the stored reference value
            {
                led_blink_frequency_counter++; // Switch press is valid and hence increment a counter the value of which decides the frequency of LED Blinking
            }
            switch_pressed_flag = RESET; // Reset the flag
            switch_previous_state = HIGH; // Reset the reference value to HIGH because HIGH indicates the switch is open
            switch_current_state = HIGH; // Reset the current reading value to HIGH because HIGH indicates the switch is open
            switch_pressed_counter = 0; // Reset the counter
        }
    }
    switch(led_blink_frequency_counter) // Taking the counter in switch statement - it has values 0, 1, 2 and 3
    {
        case 1:
            led_pin_toggle_counter++; // Incrementing the counter till LED_PIN_TOGGLE_TIME_HALF_Hz that makes LED Blinking Time 2000 ms
            if(led_pin_toggle_counter >= LED_PIN_TOGGLE_TIME_HALF_Hz) // Counter has reached the value LED_PIN_TOGGLE_TIME_HALF_Hz
            {
                led_pin_toggle_counter = 0; // Resetting the counter
                LED_PIN_TOGGLE; // Toggling the LED
            }
            break;
        case 2:
            led_pin_toggle_counter++; // Incrementing the counter till LED_PIN_TOGGLE_TIME_ONE_Hz that makes LED Blinking Time 1000 ms
            if(led_pin_toggle_counter >= LED_PIN_TOGGLE_TIME_ONE_Hz) // Counter has reached the value LED_PIN_TOGGLE_TIME_ONE_Hz
            {
                led_pin_toggle_counter = 0; // Resetting the counter
                LED_PIN_TOGGLE; // Toggling the LED
            }
            break;
        case 3:
            led_pin_toggle_counter++; // Incrementing the counter till LED_PIN_TOGGLE_TIME_TWO_Hz that makes LED Blinking Time 500 ms
            if(led_pin_toggle_counter >= LED_PIN_TOGGLE_TIME_TWO_Hz) // Counter has reached the value LED_PIN_TOGGLE_TIME_TWO_Hz
            {
                led_pin_toggle_counter = 0; // Resetting the counter
                LED_PIN_TOGGLE; // Toggling the LED
            }
            break;
        default:
            led_pin_toggle_counter = 0; // Resetting the counter
            led_blink_frequency_counter = 0; // Resetting the counter
            LED_PIN_OFF; // Switching the LED Off
            break;
    }        
}
void switch_input_isr(void)
{
    switch_interrupt_flag = SET; // flag is set as FALLING Edge is detected
}
void setup() 
{
    pinMode (SWITCH_PIN , INPUT_PULLUP); // Pin 3 as Digital Input with internal pull up because the switch is placed in such a way that when pressed, Pin 3 connects to ground 
    pinMode(LED_PIN, OUTPUT); // Pin 13 which connected to the On-Board LED is set as output pin
    LED_PIN_OFF; // Switching LED Off as initialization
    Timer1.initialize(TIMER_ONE_INTERRUPT_TIME); // Initializing Timer 1 with Timer overflow time as TIMER_ONE_INTERRUPT_TIME
    Timer1.attachInterrupt(timer_one_isr); // Taking 'timer_one_isr' as ISR
    attachInterrupt(digitalPinToInterrupt(SWITCH_PIN),switch_input_isr,FALLING); // Setting the digital input interrupt at Pin 3 with the mode set for FALLING Edge
    TIMSK1 = _BV(TOIE1); // sets the timer 1 overflow interrupt enable bit
}
void loop() 
{
}
