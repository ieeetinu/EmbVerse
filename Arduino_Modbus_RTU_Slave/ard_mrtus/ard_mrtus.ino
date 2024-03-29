/*==============================================================================
* Project : Implementation of Modbus RTU Function Subset on Arduino
* Author : EmbVerse, LLP (www.embverse.com)
* Date : March 29, 2024
* Description : The project implements 8 Modbus RTU Functions with a dedicated 
* C library that can be used in general. The 8 Modbus Functions are as below :
* 1. 0x01 : Read Coils
* 2. 0x02 : Read Discrete Inputs
* 3. 0x03 : Read Holding Registers
* 4. 0x04 : Read Input Registers
* 5. 0x05 : Write Single Coil
* 6. 0x06 : Write Single Register
* 7. 0x0F : Write Multiple Coils
* 8. 0x10 : Write Multiple Registers
* The working of all the functions has been tested on a Modbus Master Tool called qModbus.
* Download qModbus from here : https://sourceforge.net/projects/qmodbus/
*===============================================================================
*/
#include <avr/interrupt.h>
#include <avr/io.h>
#include <EEPROM.h>
#include "modbus_rtu.h"
#define USED_EEPROM_SECTOR 1
#define FIRST_POWERUP_EEPROM_ADDRESS 0
#define COIL_BEGIN_EEPROM_ADDRESS FIRST_POWERUP_EEPROM_ADDRESS + 1
#define HOLDING_REGISTER_BEGIN_EEPROM_ADDRESS COIL_BEGIN_EEPROM_ADDRESS + COIL_BYTES
#define DISCRETE_INPUT_BEGIN_EEPROM_ADDRESS HOLDING_REGISTER_BEGIN_EEPROM_ADDRESS + (NUMBER_OF_HOLDING_REGISTERS*2)
#define INPUT_REGISTER_BEGIN_EEPROM_ADDRESS DISCRETE_INPUT_BEGIN_EEPROM_ADDRESS + DISCRETE_INPUT_BYTES
#define COIL 0x00
#define DISCRETE_INPUT COIL + 1
#define INPUT_REGISTER DISCRETE_INPUT + 1
#define HOLDING_REGISTER INPUT_REGISTER + 1
#define SET 1u
#define CLEAR 0u
#define STATUS_LED_ON digitalWrite(LED_BUILTIN, HIGH)
#define STATUS_LED_OFF digitalWrite(LED_BUILTIN, LOW)
#define STATUS_LED_TOGGLE digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN))
#define MODBUS_SLAVE_ADDRESS 0x01
#define RESET 0
#define SET !RESET

uint8_t ch_count = 0;
uint16_t timer_count = 0;
uint8_t modbus_received_data[(NUMBER_OF_HOLDING_REGISTERS*2)+7];
uint8_t uart_data_rx[(NUMBER_OF_HOLDING_REGISTERS*2)+7];
typedef enum {MODBUS_IDLE, MODBUS_Rx_DATA_COLLECTION, MODBUS_Rx_DATA_RECEIVED} UART_fsm;
UART_fsm fsm = MODBUS_IDLE; // fsm - finite state machine
void initialize_eeprom(void);
void read_eeprom(void);
void update_eeprom(uint8_t parameter);

/*******************************************************************************************
*Function Name : initialize_eeprom
*Description : 
  Initialises EEPROM by setting default values for Modbus parameters decided by developer.
  Arguments :
    None
  Return : None
  Note : 
    This function is invoked once the fresh piece of chip with EEPROM not written, is
    being programmed the very first time.
*********************************************************************************************/
void initialize_eeprom(void)
{    
    for(uint8_t i = 0; i < COIL_BYTES; i++)
    {
        modbus_coil_value[i] = 0xFF;
        modbus_response_coil_temp[i] = modbus_coil_value[i];
    }    
    update_eeprom(COIL);   
    for(uint8_t i = 0; i < DISCRETE_INPUT_BYTES; i++)
    {
        modbus_discrete_input_value[i] = 0xFF;                
        modbus_response_discrete_input_temp[i] = modbus_discrete_input_value[i];
    }
    update_eeprom(DISCRETE_INPUT);
    for(uint8_t i = 0; i < NUMBER_OF_HOLDING_REGISTERS; i++)
    {   
        modbus_holding_registers_value[i] = 0xFFFF;
        modbus_response_holding_registers_temp[i] = modbus_holding_registers_value[i];
    }
    update_eeprom(HOLDING_REGISTER);
    for(uint8_t i = 0; i < NUMBER_OF_INPUT_REGISTERS; i++)
    {
        modbus_input_registers_value[i] = 0xAA55;
        modbus_response_input_registers_temp[i] =  modbus_input_registers_value[i];
    }
    update_eeprom(INPUT_REGISTER);
}

/*******************************************************************************************
*Function Name : read_eeprom
*Description : 
  Reads EEPROM with the values for Modbus parameters and saves them in variables.
  Arguments :
    None
  Return : None
*********************************************************************************************/
void read_eeprom(void)
{
    for(uint8_t i = 0; i < COIL_BYTES; i++)
    {
        modbus_coil_value[i] = EEPROM.read(COIL_BEGIN_EEPROM_ADDRESS + i);
        modbus_response_coil_temp[i] = modbus_coil_value[i];
    }
    for(uint8_t i = 0; i < DISCRETE_INPUT_BYTES; i++)
    {
        modbus_discrete_input_value[i] = EEPROM.read(DISCRETE_INPUT_BEGIN_EEPROM_ADDRESS + i);
        modbus_response_discrete_input_temp[i] = modbus_discrete_input_value[i];
    }
    for(uint8_t i = 0; i < NUMBER_OF_HOLDING_REGISTERS; i++)
    {
        modbus_holding_registers_value[i] = (uint16_t)(EEPROM.read(HOLDING_REGISTER_BEGIN_EEPROM_ADDRESS + (2*i)) << 8)
                                          | (uint16_t)(EEPROM.read(HOLDING_REGISTER_BEGIN_EEPROM_ADDRESS + (2*i + 1)) & 0xFF);
        modbus_response_holding_registers_temp[i] = modbus_holding_registers_value[i];
    }
    for(uint8_t i = 0; i < NUMBER_OF_INPUT_REGISTERS; i++)
    {
        modbus_input_registers_value[i] = (uint16_t)(EEPROM.read(INPUT_REGISTER_BEGIN_EEPROM_ADDRESS + (2*i)) << 8)
                                          | (uint16_t)(EEPROM.read(INPUT_REGISTER_BEGIN_EEPROM_ADDRESS + (2*i + 1)) & 0xFF);
        modbus_response_input_registers_temp[i] = modbus_input_registers_value[i];
    }
}

/*******************************************************************************************
*Function Name : update_eeprom
*Description : 
  Updates EEPROM with the values set by Modbus Master for Modbus parameters.
  Arguments :
    uint8_t - Function Index of Modbus
  Return : None
*********************************************************************************************/
void update_eeprom(uint8_t parameter)
{
    switch(parameter)
    {
        case COIL:
            for(uint8_t i = 0; i < COIL_BYTES; i++)
            {
                modbus_coil_value[i] = modbus_response_coil_temp[i];
                EEPROM.update(COIL_BEGIN_EEPROM_ADDRESS + i, modbus_coil_value[i]);
            }
            break;
        case DISCRETE_INPUT:
            for(uint8_t i = 0; i < DISCRETE_INPUT_BYTES; i++)
            {
                modbus_discrete_input_value[i] = modbus_response_discrete_input_temp[i];
                EEPROM.update(DISCRETE_INPUT_BEGIN_EEPROM_ADDRESS + i, modbus_discrete_input_value[i]);
            }
            break;    
        case HOLDING_REGISTER:
            for(uint8_t i = 0; i < NUMBER_OF_HOLDING_REGISTERS; i++)
            {
                modbus_holding_registers_value[i] = modbus_response_holding_registers_temp[i];
                EEPROM.update(HOLDING_REGISTER_BEGIN_EEPROM_ADDRESS + (2*i), (uint8_t)(modbus_holding_registers_value[i] >> 8));
                EEPROM.update(HOLDING_REGISTER_BEGIN_EEPROM_ADDRESS + (2*i + 1), (uint8_t)(modbus_holding_registers_value[i] & 0xFF));
            }
            break;
        case INPUT_REGISTER:
            for(uint8_t i = 0; i < NUMBER_OF_INPUT_REGISTERS; i++)
            {
                modbus_input_registers_value[i] = modbus_response_input_registers_temp[i];
                EEPROM.update(INPUT_REGISTER_BEGIN_EEPROM_ADDRESS + (2*i), (uint8_t)(modbus_input_registers_value[i] >> 8));
                EEPROM.update(INPUT_REGISTER_BEGIN_EEPROM_ADDRESS + (2*i + 1), (uint8_t)(modbus_input_registers_value[i] & 0xFF));
            }
            break;    
        default:    
            break;            
    }
}

/*******************************************************************************************
*Function Name : Timer0_initialize
*Description : 
  Sets Timer0 to trigger every 1 ms if enabled.
  Arguments :
    None
  Return : None
  Note :
    This timer is needed to decide whether a valid Modbus Query Data Frame is received from
    Master. The minimum time it takes between consecutive Data Frames to receive is 4 ms. So,
    this timer checks that interval and sets a data reception flag if the bus is IDLE for 4
    or more ms.
*********************************************************************************************/
void Timer0_initialize(uint32_t time_period_us)
{
    //set timer0 interrupt at 2kHz
    TCCR0A = 0;// set entire TCCR0A register to 0
    TCCR0B = 0;// same for TCCR0B
    TCNT0  = 0;//initialize counter value to 0
    // set compare match register for 2khz increments
    OCR0A = (16000000.0) / ((1000000.0/time_period_us)*64.0) - 1.0; //(must be <256)
    // turn on CTC mode
    TCCR0A |= (1 << WGM01);
    // Set CS01 and CS00 bits for 64 prescaler
    TCCR0B |= (1 << CS01) | (1 << CS00);   
    // enable timer compare interrupt
    TIMSK0 |= (1 << OCIE0A);//set timer0 interrupt at 2kHz
}  

/********************************************************************************************************************
  Function Name : USART_init
  Description : 
  Initialises USART by setting Baudrate, Tx Rx Enable, Frame Format and enabling USART 
    Receive Complete Interupt.
  Arguments :
    - unsigned int ubrr : Baudrate Setting. Refer Technical Reference Manual for getting this value.
  Return : Nothing
  Note : 
    The value of ubrr in this code is set to "103" for 9600 baudrate. Refer Technical Reference 
    Manual of AVR for better understanding.
********************************************************************************************************************/
void USART_Init(unsigned int ubrr)
{
    //Set baud rate
    UBRR0H = (unsigned char)(ubrr>>8);
    UBRR0L = (unsigned char)ubrr & 0xFF;
    //The Baudrate is 9600 so no need to set the Double Speed Bit
    UCSR0A = (0<<U2X0);
    //Enable receiver and transmitter
    UCSR0B = (1<<RXEN0) | (1<<TXEN0);
    // Set frame format: 8data, 1stop bit
    UCSR0C = (1<<UCSZ00) | (1<<UCSZ01);
    // Enable the USART Receive Complete interrupt (USART_RXC)
    UCSR0B |= (1 << RXCIE0); 
}

/******************************************************************************
  Function Name : USART_Transmit_Str
  Description : 
  Transmits the input character array over USART.
  Arguments :
    - unsigned char *data : Pointer to the start address of Data array
  Return : Nothing
 ******************************************************************************/
void Modbus_RTU_Over_UART_PutChar(char ch_data)  //
{
        while ( !( UCSR0A & (1<<UDRE0)) );
        // Put data into buffer, sends the data
        UDR0 = ch_data;
}

ISR(TIMER0_COMPA_vect)
{   
    timer_count++;
    if(timer_count >= 4)
    {
        timer_count = 0;
        fsm =  MODBUS_Rx_DATA_RECEIVED;
        TIMSK0 &= (0 << OCIE0A); // Disable Timer0
        STATUS_LED_TOGGLE; // Indicative LED Toggling on Data Frame Reception 
    }
}

void setup()
{
    pinMode(LED_BUILTIN, OUTPUT); // STATUS LED that toggles on every Query Reception 
    digitalWrite(LED_BUILTIN, LOW);
    uint8_t EEPROM_PowerUp_flag; // EEPROM initialization (Once only)
    EEPROM_PowerUp_flag = EEPROM.read(FIRST_POWERUP_EEPROM_ADDRESS);
    if(EEPROM_PowerUp_flag == 0xFF) // Indicates to write EEPROM with default modbus parameter values (set by programmer) 
    {
        initialize_eeprom();
        EEPROM_PowerUp_flag = 0x01; // Next Power Cycles won't initialize EEPROM
        EEPROM.write(FIRST_POWERUP_EEPROM_ADDRESS, EEPROM_PowerUp_flag);
    }    
    read_eeprom(); // Reads the EEPROM and stores values in variables  
    USART_Init(103); // Initializing Serial UART
    Timer0_initialize(1000);
    TIMSK0 &= (0 << OCIE0A);// Disable Timer0
    sei(); // Enable Interrupts
}

void loop()
{
    if(fsm == MODBUS_Rx_DATA_RECEIVED) // Query from Modbus master is received
    {            
        for(uint8_t i = 0; i <= ch_count; i++)
        {
            modbus_received_data[i] = uart_data_rx[i]; // Store the dataframe in an array
        }
        if(modbus_received_data[0] == MODBUS_SLAVE_ADDRESS) // Check Modbus Slave Address
        {
            switch(modbus_received_data[1]) // Check the Function Index
            {
                case 0x01: //Read Coils
                    query_response_read_coil(&modbus_received_data[0], ch_count + 1);
                    for(uint8_t i = 0; i < modbus_response_data_length; i++)
                    {
                        Modbus_RTU_Over_UART_PutChar(modbus_response_read_coil[i]);
                    }
                    break;
                case 0x02: //Read Discrete Inputs
                    query_response_read_discrete_input(&modbus_received_data[0], ch_count + 1);
            
                    for(uint8_t i = 0; i < modbus_response_data_length; i++)
                    {
                        Modbus_RTU_Over_UART_PutChar(modbus_response_read_discrete_input[i]);
                    }
                    break;
                case 0x03 : // Read Holding Registers
                    query_response_read_holding_registers(&modbus_received_data[0], ch_count + 1);
                    for(uint8_t i = 0; i < modbus_response_data_length; i++)
                    {
                        Modbus_RTU_Over_UART_PutChar(modbus_response_read_holding_registers[i]);
                    }
                    break;
                case 0x04 : //Read Input Registers
                    query_response_read_input_registers(&modbus_received_data[0], ch_count + 1);
                    for(uint8_t i = 0; i < modbus_response_data_length; i++)
                    {
                        Modbus_RTU_Over_UART_PutChar(modbus_response_read_input_registers[i]);
                    }
                    break;
                case 0x05 : // Write Single Coil
                    query_response_force_single_coil(&modbus_received_data[0], ch_count + 1);
                    for(uint8_t i = 0; i < modbus_response_data_length; i++)
                    {
                        Modbus_RTU_Over_UART_PutChar(modbus_response_force_single_coil[i]);
                    }
                    update_eeprom(COIL);
                    break;
                case 0x06 : // Write Single Register
                    query_response_preset_single_register(&modbus_received_data[0], ch_count + 1);
                    for(uint8_t i = 0; i < modbus_response_data_length; i++)
                    {
                        Modbus_RTU_Over_UART_PutChar(modbus_response_preset_single_register[i]);
                    }
                    update_eeprom(HOLDING_REGISTER);
                    break;
                case 0x0F : // Write Multiple Coils
                    query_response_force_multiple_coils(&modbus_received_data[0], ch_count + 1);
                    for(uint8_t i = 0; i < modbus_response_data_length; i++)
                    {
                        Modbus_RTU_Over_UART_PutChar(modbus_response_force_multiple_coils[i]);
                    }
                    update_eeprom(COIL);
                    break;
                case 0x10 : // Write Multiple Registers
                    query_response_preset_multiple_registers(&modbus_received_data[0], ch_count + 1);
                    for(uint8_t i = 0; i < modbus_response_data_length; i++)
                    {
                        Modbus_RTU_Over_UART_PutChar(modbus_response_preset_multiple_registers[i]);
                    }
                    update_eeprom(HOLDING_REGISTER);
                    break;
                default :
                    break;                        
            }
        }
        fsm = MODBUS_IDLE;
        ch_count = 0;
    }
}

ISR(USART0_RX_vect) // Uncomment For Arduino Mega
//ISR(USART_RX_vect) // Uncomment for Arduino Uno
{  
    char incoming_Char = UDR0;
    timer_count = 0;
    TIMSK0 |= (1 << OCIE0A); // Enable Timer0
    switch(fsm)
    {
        case MODBUS_IDLE:
        {
            uart_data_rx[0] = incoming_Char;
            if(uart_data_rx[0] == MODBUS_SLAVE_ADDRESS)
            {
                fsm = MODBUS_Rx_DATA_COLLECTION;
                timer_count = 0;
                TIMSK0 |= (1 << OCIE0A); // Enable Timer0
                break;
            }
            else
            {
                break;
            }
        }
        case MODBUS_Rx_DATA_COLLECTION:
        {
            ch_count++;
            uart_data_rx[ch_count] = incoming_Char;        
            timer_count = 0;
            TIMSK0 |= (1 << OCIE0A); // Enable Timer0
            break;
        }
    }
    UCSR0B |= (1 << RXCIE0); // Re-enabling the UART receiving interrupt
}    