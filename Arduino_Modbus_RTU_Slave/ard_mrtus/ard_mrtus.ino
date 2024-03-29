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

uint16_t tcount = 0;
uint8_t ch_count = 0;
uint16_t timer_count = 0;
bool one_ms_timer_flag = RESET, data_received_flag = RESET, blank_data_received_flag = RESET, timeout_flag = RESET, data_length_overflow_flag = RESET;
uint8_t modbus_received_data[64];
uint8_t uart_data_rx[64];
typedef enum {MODBUS_IDLE, MODBUS_Rx_DATA_COLLECTION, MODBUS_Rx_DATA_RECEIVED} UART_fsm;
UART_fsm fsm = MODBUS_IDLE;
void initialize_eeprom(void);
void read_eeprom(void);
void update_eeprom(uint8_t parameter);

void initialize_eeprom(void)
{    
    for(uint8_t i = 0; i < COIL_BYTES; i++)
    {
        modbus_coil_value[i] = 0xFF;
        modbus_response_coil_temp[i] = modbus_coil_value[i];
        //EEPROM.write(COIL_BEGIN_EEPROM_ADDRESS + i, modbus_coil_value[i]);
    }    
    update_eeprom(COIL);   
    for(uint8_t i = 0; i < DISCRETE_INPUT_BYTES; i++)
    {
        modbus_discrete_input_value[i] = 0xFF;                
        modbus_response_discrete_input_temp[i] = modbus_discrete_input_value[i];
        //EEPROM.write(COIL_BEGIN_EEPROM_ADDRESS + i, modbus_discrete_input_value[i]);
    }
    update_eeprom(DISCRETE_INPUT);
    for(uint8_t i = 0; i < NUMBER_OF_HOLDING_REGISTERS; i++)
    {   
        modbus_holding_registers_value[i] = 0xFFFF;
        modbus_response_holding_registers_temp[i] = modbus_holding_registers_value[i];
        //EEPROM.write(COIL_BEGIN_EEPROM_ADDRESS + i, modbus_holding_registers_value[i]);
    }
    update_eeprom(HOLDING_REGISTER);
    for(uint8_t i = 0; i < NUMBER_OF_INPUT_REGISTERS; i++)
    {
        modbus_input_registers_value[i] = 0xAA55;
        modbus_response_input_registers_temp[i] =  modbus_input_registers_value[i];
        //EEPROM.write(COIL_BEGIN_EEPROM_ADDRESS + i, modbus_coil_value[i]);
    }
    update_eeprom(INPUT_REGISTER);
}
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
    The value of ubrr in this code is set to "16" for 115200 baudrate. Refer Technical Reference 
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
void Modbus_RTU_Over_UART_PutChar(char dat_a)  //
{
    //while(c > 0)
    //{  
        // Wait for empty transmit buffer
        while ( !( UCSR0A & (1<<UDRE0)) );
        // Put data into buffer, sends the data
        UDR0 = dat_a;
        
    //}
}
/*End of Function USART_Transmit_Str*/
ISR(TIMER0_COMPA_vect)
{   
    tcount++;
    if(tcount >= 4)
    {
        tcount = 0;
        fsm =  MODBUS_Rx_DATA_RECEIVED;
        TIMSK0 &= (0 << OCIE0A);
        STATUS_LED_TOGGLE;
    }    
  //TIMSK0 |= (1 << OCIE0A);
}
void setup()
{
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);
    uint8_t EEPROM_PowerUp_flag;
    EEPROM_PowerUp_flag = EEPROM.read(FIRST_POWERUP_EEPROM_ADDRESS);
    if(EEPROM_PowerUp_flag == 0xFF) 
    {
        initialize_eeprom();
        EEPROM_PowerUp_flag = 0x01;
        EEPROM.write(FIRST_POWERUP_EEPROM_ADDRESS, EEPROM_PowerUp_flag);
    }    
    read_eeprom();  
  /* Call Tlc.init() to setup the tlc.
     You can optionally pass an initial PWM value (0 - 4095) for all channels.*/
    //Serial.begin(9600);
    USART_Init(103); // Initializing Serial UART
    
    Timer0_initialize(1000);
    TIMSK0 &= (0 << OCIE0A);//set timer0 interrupt at 2kHz
    sei();
}
void loop()
{
    if(fsm == MODBUS_Rx_DATA_RECEIVED)
    {            
        for(uint8_t i = 0; i <= ch_count; i++)
        {
            modbus_received_data[i] = uart_data_rx[i];
        }
        if(modbus_received_data[0] == MODBUS_SLAVE_ADDRESS)
        {
            switch(modbus_received_data[1])
            {
                case 0x01:
                    query_response_read_coil(&modbus_received_data[0], ch_count + 1);
                    for(uint8_t i = 0; i < modbus_response_data_length; i++)
                    {
                        Modbus_RTU_Over_UART_PutChar(modbus_response_read_coil[i]);
                    }
                    break;
                case 0x02:
                    query_response_read_discrete_input(&modbus_received_data[0], ch_count + 1);
            
                    for(uint8_t i = 0; i < modbus_response_data_length; i++)
                    {
                        Modbus_RTU_Over_UART_PutChar(modbus_response_read_discrete_input[i]);
                    }
                    break;
                case 0x03 :
                    query_response_read_holding_registers(&modbus_received_data[0], ch_count + 1);
                    for(uint8_t i = 0; i < modbus_response_data_length; i++)
                    {
                        Modbus_RTU_Over_UART_PutChar(modbus_response_read_holding_registers[i]);
                    }
                    break;
                case 0x04 :
                    query_response_read_input_registers(&modbus_received_data[0], ch_count + 1);
                    for(uint8_t i = 0; i < modbus_response_data_length; i++)
                    {
                        Modbus_RTU_Over_UART_PutChar(modbus_response_read_input_registers[i]);
                    }
                    break;
                case 0x05 :
                    query_response_force_single_coil(&modbus_received_data[0], ch_count + 1);
                    for(uint8_t i = 0; i < modbus_response_data_length; i++)
                    {
                        Modbus_RTU_Over_UART_PutChar(modbus_response_force_single_coil[i]);
                    }
                    update_eeprom(COIL);
                    break;
                case 0x06 :
                    query_response_preset_single_register(&modbus_received_data[0], ch_count + 1);
                    for(uint8_t i = 0; i < modbus_response_data_length; i++)
                    {
                        Modbus_RTU_Over_UART_PutChar(modbus_response_preset_single_register[i]);
                    }
                    update_eeprom(HOLDING_REGISTER);
                    break;
                case 0x0F :
                    query_response_force_multiple_coils(&modbus_received_data[0], ch_count + 1);
                    for(uint8_t i = 0; i < modbus_response_data_length; i++)
                    {
                        Modbus_RTU_Over_UART_PutChar(modbus_response_force_multiple_coils[i]);
                    }
                    update_eeprom(COIL);
                    break;
                case 0x10 :
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
// ISR(USART0_RX_vect) //Arduino Mega
ISR(USART_RX_vect) //Arduino Uno
{  
    
    char incoming_Char = UDR0;
    tcount = 0;
    TIMSK0 |= (1 << OCIE0A);
    switch(fsm)
    {
        case MODBUS_IDLE:
        {
            uart_data_rx[0] = incoming_Char;
            if(uart_data_rx[0] == MODBUS_SLAVE_ADDRESS)
            {
                fsm = MODBUS_Rx_DATA_COLLECTION;
                tcount = 0;
                TIMSK0 |= (1 << OCIE0A);
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
            
            tcount = 0;
            TIMSK0 |= (1 << OCIE0A);
            break;
        }
    }
    UCSR0B |= (1 << RXCIE0); // Re-enabling the UART receiving interrupt
}    
