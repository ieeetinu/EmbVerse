#include <avr/interrupt.h> 
#include <avr/io.h> 

/* Defined Constants */

/* USART Baud Rate Configurations */

#define FOSC 16000000 // Clock Speed
#define BAUD 115200
#define MYUBRR FOSC/16/BAUD

/* USART Receive Complete Interrupt Vector */

#define USART_RXC_vect _VECTOR(18)

/* fsm_usart */

#define WAIT 0 
#define FILL !WAIT

/* Slave Addresses */
#define SLAVE_ONE 0x01
#define SLAVE_TWO 0x03
#define SLAVE_THREE 0x05

/* Constants for easy readability. */
#define ONE_REGISTER 0x0001
#define MOTOR_START 0x0000

/* Slave Register Addresses */

#define VELOCITY_COMMAND_ADDRESS 0x2000
#define ACCELERATION_COMMAND_ADDRESS VELOCITY_COMMAND_ADDRESS + 1
#define MOTION_COMMAND_ADDRESS_LOW ACCELERATION_COMMAND_ADDRESS + 1
#define MOTION_COMMAND_ADDRESS_HIGH MOTION_COMMAND_ADDRESS_LOW + 1
#define MOTION_STATUS_ADDRESS MOTION_COMMAND_ADDRESS_HIGH + 1

/* Modbus RTU Functions */

#define READ_REGISTERS 0x03
#define PRESET_SINGLE_REGISTER 0x06

/* For Query to Slave */

#define MODBUS_DATA_TRANSACTION_PIN 13

/* For Slave Response */

#define SLAVE_RESPONSE_ERROR_CNT 3
#define RESPONSE_OK 0
#define RESPONSE_CRC_ERROR RESPONSE_OK + 1
#define RESPONSE_ERROR RESPONSE_CRC_ERROR + 1
#define MASTER_RECEIVE_IDLE RESPONSE_ERROR + 1

/* Flags */

#define TRUE 1
#define FALSE !TRUE

/* End of Defined Constants */

/* Global Variable Declaration */

bool fsm_usart, motion_status_read;
unsigned char Master_Query[8], Slave_Response[10];
unsigned char serial_inChar, serial_valid_received_char_cnt, modbus_char_length;
unsigned short crc16;

/* Table of CRC values for highâ€“order byte */

static unsigned char auchCRCHi[] = {
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
    0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01,
    0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81,
    0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01,
    0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
    0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01,
    0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
    0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01,
    0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
    0x40
};

/* Table of CRC values for lowâ€“order byte */

static unsigned char auchCRCLo[] = {
    0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06, 0x07, 0xC7, 0x05, 0xC5, 0xC4,
    0x04, 0xCC, 0x0C, 0x0D, 0xCD, 0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09,
    0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A, 0x1E, 0xDE, 0xDF, 0x1F, 0xDD,
    0x1D, 0x1C, 0xDC, 0x14, 0xD4, 0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,
    0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3, 0xF2, 0x32, 0x36, 0xF6, 0xF7,
    0x37, 0xF5, 0x35, 0x34, 0xF4, 0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A,
    0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28, 0xE8, 0xE9, 0x29, 0xEB, 0x2B, 0x2A, 0xEA, 0xEE,
    0x2E, 0x2F, 0xEF, 0x2D, 0xED, 0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
    0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60, 0x61, 0xA1, 0x63, 0xA3, 0xA2,
    0x62, 0x66, 0xA6, 0xA7, 0x67, 0xA5, 0x65, 0x64, 0xA4, 0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F,
    0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68, 0x78, 0xB8, 0xB9, 0x79, 0xBB,
    0x7B, 0x7A, 0xBA, 0xBE, 0x7E, 0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,
    0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71, 0x70, 0xB0, 0x50, 0x90, 0x91,
    0x51, 0x93, 0x53, 0x52, 0x92, 0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C,
    0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B, 0x99, 0x59, 0x58, 0x98, 0x88,
    0x48, 0x49, 0x89, 0x4B, 0x8B, 0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
    0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42, 0x43, 0x83, 0x41, 0x81, 0x80,
    0x40
};

/* End of Global Variable Declaration */

/* Function Declaration */

void msdelay(unsigned long ms);
void USART_Init(unsigned int ubrr);
void USART_Transmit(unsigned char *data, unsigned char charLength);
unsigned short CRC16(unsigned char *puchMsg, unsigned short usDataLen);
unsigned char readModBusRegister(unsigned int register_read_start_address, unsigned int no_of_registers, unsigned char slave_address, unsigned char rtu_function);
unsigned char writeModBusRegister(unsigned int register_read_start_address, unsigned int value_of_register, unsigned char slave_address, unsigned char rtu_function);
void motor_control(unsigned char slave_drive_id, unsigned int velocity_command, unsigned int acceleration_command, unsigned long motion_command);


/* End of Function Declaration */

/* Function Definitions */

/********************************************************************************************************************
  Function Name : msdelay
  Description : 
  Provides delay in code based on input argument. The argument value entered is the time
    delay in Milliseconds.
  Arguments :
    - unsigned long ms : Delay time value in Milliseconds
  Return : Nothing
********************************************************************************************************************/

void msdelay(unsigned long ms)
{
    if (ms > 20)
    {
        unsigned long start = millis(); 
        while (millis() - start <= ms) ;
    } 
    else
    { 
        delayMicroseconds((unsigned int)ms * 1000);
    } 
}

/* End of Function msdelay */

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
    //The Baudrate is 115200 so set the Double Speed Bit
    UCSR0A = (1<<U2X0);
    //Enable receiver and transmitter
    UCSR0B = (1<<RXEN0) | (1<<TXEN0);
    // Set frame format : 8data, 1stop bit
    UCSR0C = (1<<UCSZ00) | (1<<UCSZ01);
    // Enable the USART Receive Complete interrupt (USART_RXC)
    UCSR0B |= (1 << RXCIE0); 
}

/* End of Function USART_init */

/********************************************************************************************************************
  Function Name : USART_Transmit
  Description : 
  Transmits the input array over USART.
  Arguments :
    - unsigned char *data : Pointer to the start address of Data array
    - unsigned char charLength : Number of characters of the input array that are to be transmitted
  Return : Nothing
********************************************************************************************************************/

void USART_Transmit(unsigned char *data, unsigned char charLength)
{
    uint8_t index = 0;
    while(index < charLength)
    {  
        /* Wait for empty transmit buffer */
        while (!(UCSR0A & (1<<UDRE0)));
        /* Put data into buffer, sends the data */
        UDR0 = *data;
        data++;
        index++;
    }
    index = 0;
}

/* End of Function USART_Transmit */

/********************************************************************************************************************
  Function Name : CRC16
  Description : 
  Calculates and returns the 16-bit CRC Checksum based on Modbus RTU.
  Arguments :
    - unsigned char *puchMsg : Input array start address for which 16-bit CRC has to be calculated
    - unsigned short usDataLen : Number of elements of array for which 16-bit CRC has to be calculated
  Return : unsigned short
    - 16 -bit CRC Checksum
  Note : 
    The Input array address and the Number of elements have to be entered within their boundary.
********************************************************************************************************************/

unsigned short CRC16(unsigned char *puchMsg, unsigned short usDataLen)
{
    unsigned char uchCRCHi = 0xFF ; /* high byte of CRC initialized */
    unsigned char uchCRCLo = 0xFF ; /* low byte of CRC initialized */
    unsigned uIndex ; /* will index into CRC lookup table */
    while(usDataLen--) /* pass through message buffer */
    {
        uIndex = uchCRCHi ^ *puchMsg++ ; /* calculate the CRC */
        uchCRCHi = uchCRCLo ^ auchCRCHi[uIndex] ;
        uchCRCLo = auchCRCLo[uIndex] ;
    }
    return (uchCRCHi << 8 | uchCRCLo) ;
}

/* End of Function CRC16 */

/********************************************************************************************************************
  Function Name : readModBusRegister
  Description : 
  Queries the slave to read one/multiple registers and verifies the response from slave.
    It also processes the response when the Master is reading the register address 8196.     
  Arguments :
    - unsigned int register_read_start_address : The start address from which one/multiple registers are to be read.
    - unsigned int no_of_registers : Number of registers to be read. 
    - unsigned char slave_address : Slave Address 
    - unsigned char rtu_function : Modbus RTU Function
  Return : unsigned char
    - RESPONSE_OK : Valid Response
    - RESPONSE_CRC_ERROR : Error in CRC Checksum
    - RESPONSE_ERROR : Not Valid Response
  Note :  
    For this code, a valid call for this function is with arguments :
      - register_read_start_address : 0x2004
      - no_of_registers : 0x01
      - slave_address : 0x01 Or 0x03 Or 0x05
      - rtu_function : 0x03
********************************************************************************************************************/

unsigned char readModBusRegister(unsigned int register_read_start_address, unsigned int no_of_registers, unsigned char slave_address, unsigned char rtu_function)
{
    unsigned char slave_preset_response_error_count;
    unsigned char modbus_rtu_preset_slave_response;
    Master_Query[0] = slave_address;
    Master_Query[1] = rtu_function;
    Master_Query[2] = register_read_start_address >> 8;
    Master_Query[3] = register_read_start_address & 0x00FF;
    Master_Query[4] = no_of_registers >> 8;
    Master_Query[5] = no_of_registers & 0x00FF;
    crc16 = CRC16(&Master_Query[0], 6);
    Master_Query[6] = (unsigned char)(crc16 >> 8);
    Master_Query[7] = (unsigned char)(crc16 & 0x00FF);
    digitalWrite(MODBUS_DATA_TRANSACTION_PIN, HIGH); // Pin 13 Set for Putting Query by Transmitting Query Data
    msdelay(4); // Delay to let the Pin Output be stable
    USART_Transmit(&Master_Query[0], 8); // Putting Query by Transmitting Query Data
    delayMicroseconds(200); // Delay for letting the data transfer complete
    digitalWrite(MODBUS_DATA_TRANSACTION_PIN, LOW); // Pin 13 Reset for Activating the Modbus Master to Receive Data from Slave
    msdelay(2); // Timeout delay for receiving slave response
    modbus_char_length = serial_valid_received_char_cnt;
    fsm_usart = WAIT; // USART FSM gets a reset to WAIT state
    serial_valid_received_char_cnt = 0; // USART received character count gets reset
    /*Slave Response Verification */
    if((Slave_Response[0] == slave_address) && (Slave_Response[1] == rtu_function) && (Slave_Response[2] == (unsigned char)(no_of_registers << 1)))//) && (Slave_Response[3] == ((unsigned char)(no_of_registers << 1) & 0x00FF)))// && (Slave_Response[3] == 0x00) && (Slave_Response[4] == 0x00) && (Slave_Response[5] == 0x00) && (Slave_Response[6] == 0x00) && (Slave_Response[7] == 0xC4) && (Slave_Response[8] == 0xCE))
    {
        crc16 = CRC16(&Slave_Response[0], modbus_char_length - 2);
        /* CRC16 Checksum Verification */
        if((Slave_Response[modbus_char_length - 1] == ((unsigned char)(crc16 & 0x00FF))) && (Slave_Response[modbus_char_length - 2] == ((unsigned char)(crc16 >> 8))))
        {
            if(motion_status_read == TRUE) // It indicates that Modbus Master has Queried for reading Motion Status Register
            {
                motion_status_read = FALSE;
                if(Slave_Response[4] == 0x04) // Motor Status is IDLE
                {
                    /* Set Motor Parameters */
                    motor_control(slave_address, 200, 500, 4000);
                    slave_preset_response_error_count = 0;
                    modbus_rtu_preset_slave_response = MASTER_RECEIVE_IDLE;
                    while(modbus_rtu_preset_slave_response != RESPONSE_OK)   
                    {
                        /* Start the Motor */
                        modbus_rtu_preset_slave_response = writeModBusRegister(MOTION_STATUS_ADDRESS, MOTOR_START, slave_address, PRESET_SINGLE_REGISTER);
                        if(++slave_preset_response_error_count > SLAVE_RESPONSE_ERROR_CNT)
                        {
                            slave_preset_response_error_count = 0;
                            break;
                        }   
                    }    
                }    
            }    
            for(int u = 0; u < 10; u++)
                Slave_Response[u] = 0;
            return(RESPONSE_OK);
        }    
        else
        { 
            for(int u = 0; u < 10; u++)
                Slave_Response[u] = 0;
            return(RESPONSE_CRC_ERROR);
        }     
    }
    else
    {
        for(int u = 0; u < 10; u++)
            Slave_Response[u] = 0;    
        return(RESPONSE_ERROR);
    }    
}

/* End of Function readModbusRegister */

/********************************************************************************************************************
  Function Name : writeModBusRegister
  Description : 
  Queries the slave to preset single register and verifies the response from slave.
  Arguments :
    - unsigned int register_read_start_address : The start address at which a single register is to be written.
    - unsigned int value_of_register : Value to be writen to the register. 
    - unsigned char slave_address : Slave Address 
    - unsigned char rtu_function : Modbus RTU Function
  Return : unsigned char
    - RESPONSE_OK : Valid Response
    - RESPONSE_CRC_ERROR : Error in CRC Checksum
    - RESPONSE_ERROR : Not Valid Response
  Note :  
    For this code, a valid call for this function is with arguments :
      - register_read_start_address : 0x2000 Or 0x2001 Or 0x2002 Or 0x2004
      - no_of_registers : 0x01
      - slave_address : 0x01 Or 0x03 Or 0x05
      - rtu_function : 0x06
********************************************************************************************************************/

unsigned char writeModBusRegister(unsigned int register_read_start_address, unsigned int value_of_register, unsigned char slave_address, unsigned char rtu_function)
{
    Master_Query[0] = slave_address;
    Master_Query[1] = rtu_function;
    Master_Query[2] = register_read_start_address >> 8;
    Master_Query[3] = register_read_start_address & 0x00FF;
    Master_Query[4] = value_of_register >> 8;
    Master_Query[5] = value_of_register & 0x00FF;
    crc16 = CRC16(&Master_Query[0], 6);
    Master_Query[6] = (unsigned char)(crc16 >> 8);
    Master_Query[7] = (unsigned char)(crc16 & 0x00FF);
    digitalWrite(MODBUS_DATA_TRANSACTION_PIN, HIGH); // Pin 13 Set for Putting Query by Transmitting Query Data
    msdelay(4); // Delay to let the Pin Output be stable
    USART_Transmit(&Master_Query[0], 8); // Putting Query by Transmitting Query Data
    delayMicroseconds(200); // Delay for letting the data transfer complete
    digitalWrite(MODBUS_DATA_TRANSACTION_PIN, LOW); // Pin 13 Reset for Activating the Modbus Master to Receive Data from Slave
    msdelay(2); // Timeout delay for receiving slave response
    modbus_char_length = serial_valid_received_char_cnt;
    fsm_usart = WAIT; // USART FSM gets a reset to WAIT state
    serial_valid_received_char_cnt = 0; // USART received character count gets reset
    /*Slave Response Verification */
    if((Slave_Response[0] == slave_address) && (Slave_Response[1] == rtu_function) && (Slave_Response[2] == ((unsigned char)(register_read_start_address >> 8))) && (Slave_Response[3] == ((unsigned char)(register_read_start_address) & 0x00FF)))// && (Slave_Response[3] == 0x00) && (Slave_Response[4] == 0x00) && (Slave_Response[5] == 0x00) && (Slave_Response[6] == 0x00) && (Slave_Response[7] == 0xC4) && (Slave_Response[8] == 0xCE))
    {
        crc16 = CRC16(&Slave_Response[0], modbus_char_length - 2);
        /* CRC Checksum Verification */ 
        if((Slave_Response[modbus_char_length - 1] == ((unsigned char)(crc16 & 0x00FF))) && (Slave_Response[modbus_char_length - 2] == ((unsigned char)(crc16 >> 8))))
        {
            for(int u = 0; u < 10; u++)
                Slave_Response[u] = 0;
            return(RESPONSE_OK);
        }    
        else
        { 
            for(int u = 0; u < 10; u++)
                Slave_Response[u] = 0;
            return(RESPONSE_CRC_ERROR);
        }     
    }
    else
    {
        for(int u = 0; u < 10; u++)
            Slave_Response[u] = 0;
        return(RESPONSE_ERROR);
    }    
}

/* End of Function writeModbusRegister */

/********************************************************************************************************************
  Function Name : motor_control
  Description : 
  Controls the motion of motor connected with the slave drive.
  Arguments :
    - unsigned char slave_drive_id : Slave Drive's Modbus Address
    - unsigned int velocity_command : Input value of Velocity Command
    - unsigned int acceleration_command : Input value of Acceleration Command
    - unsigned long motion_command : Input value of Motion Command
  Return : Nothing
  Note : 
    The Motion Command is a 32-bit Value that gets divided between two Registers Addressed at 0x2002 and 0x2003
********************************************************************************************************************/

void motor_control(unsigned char slave_drive_id, unsigned int velocity_command, unsigned int acceleration_command, unsigned long motion_command)
{
    unsigned char slave_preset_response_error_count;
    unsigned char modbus_rtu_preset_slave_response;
    modbus_rtu_preset_slave_response = MASTER_RECEIVE_IDLE;
    slave_preset_response_error_count = 0;
    while(modbus_rtu_preset_slave_response != RESPONSE_OK)
    {
        modbus_rtu_preset_slave_response = writeModBusRegister(VELOCITY_COMMAND_ADDRESS, velocity_command, slave_drive_id, PRESET_SINGLE_REGISTER);
        if(++slave_preset_response_error_count > SLAVE_RESPONSE_ERROR_CNT)
        {
            slave_preset_response_error_count = 0;
            break;
        }      
    }
    if(modbus_rtu_preset_slave_response != RESPONSE_OK)
    {
        digitalWrite(slave_drive_id + 1, HIGH);
    }
    else 
    {
        digitalWrite(slave_drive_id + 1, LOW);
    }
    slave_preset_response_error_count = 0;    
    modbus_rtu_preset_slave_response = MASTER_RECEIVE_IDLE;
    while(modbus_rtu_preset_slave_response != RESPONSE_OK)
    {
        modbus_rtu_preset_slave_response = writeModBusRegister(ACCELERATION_COMMAND_ADDRESS, acceleration_command, slave_drive_id, PRESET_SINGLE_REGISTER);
        if(++slave_preset_response_error_count > SLAVE_RESPONSE_ERROR_CNT)
        {
            slave_preset_response_error_count = 0;
            break;
        }
    }
    if(modbus_rtu_preset_slave_response != RESPONSE_OK)
    {
        digitalWrite(slave_drive_id + 1, HIGH);
    }
    else 
    {
        digitalWrite(slave_drive_id + 1, LOW);
    }
    slave_preset_response_error_count = 0;    
    modbus_rtu_preset_slave_response = MASTER_RECEIVE_IDLE;
    while(modbus_rtu_preset_slave_response != RESPONSE_OK)
    {
        modbus_rtu_preset_slave_response = writeModBusRegister(MOTION_COMMAND_ADDRESS_LOW, (unsigned int)(motion_command & 0xFFFF), slave_drive_id, PRESET_SINGLE_REGISTER);
        if(++slave_preset_response_error_count > SLAVE_RESPONSE_ERROR_CNT)
        {
            slave_preset_response_error_count = 0;
            break;
        }
    }
    if(modbus_rtu_preset_slave_response != RESPONSE_OK)
    {
        digitalWrite(slave_drive_id + 1, HIGH);
    }
    else 
    {
        digitalWrite(slave_drive_id + 1, LOW);
    }
    slave_preset_response_error_count = 0;    
    modbus_rtu_preset_slave_response = MASTER_RECEIVE_IDLE;
    while(modbus_rtu_preset_slave_response != RESPONSE_OK)
    {
        modbus_rtu_preset_slave_response = writeModBusRegister(MOTION_COMMAND_ADDRESS_HIGH, (unsigned int)(motion_command >> 16), slave_drive_id, PRESET_SINGLE_REGISTER);
        if(++slave_preset_response_error_count > SLAVE_RESPONSE_ERROR_CNT)
        {
            slave_preset_response_error_count = 0;
            break;
        }
    }
    if(modbus_rtu_preset_slave_response != RESPONSE_OK)
    {
        digitalWrite(slave_drive_id + 1, HIGH);
    }
    else 
    {
        digitalWrite(slave_drive_id + 1, LOW);
    }
}

/* End of Function motor_control */

void setup() {
    fsm_usart = WAIT;
    serial_valid_received_char_cnt = 0;
    serial_inChar = 0;
    crc16 = 0;
    motion_status_read = FALSE;
    pinMode(MODBUS_DATA_TRANSACTION_PIN, OUTPUT);
    digitalWrite(MODBUS_DATA_TRANSACTION_PIN, HIGH);
    pinMode(2, OUTPUT);
    digitalWrite(2, LOW);
    pinMode(4, OUTPUT);
    digitalWrite(4, LOW);
    pinMode(6, OUTPUT);
    digitalWrite(6, LOW);
    digitalWrite(A3, INPUT_PULLUP);
    pinMode(A3, OUTPUT);
    digitalWrite(A3, LOW);
    for(int u = 0; u < 10; u++)
        Slave_Response[u] = 0;
    if((unsigned long)BAUD > 57600)
        USART_Init((unsigned int)((unsigned int)(MYUBRR) << 1));
    else
        USART_Init((unsigned int)MYUBRR);       
}

/* End of Function setup */

void loop() 
{
    unsigned char modbus_rtu_read_slave_response, slave_read_response_error_count;
    unsigned char slave_id;
    slave_id = 0x01;
    for(unsigned char counter = 0; counter < 3; counter++)
    {
        motion_status_read = TRUE;
        slave_read_response_error_count = 0;
        modbus_rtu_read_slave_response = MASTER_RECEIVE_IDLE;
        while(modbus_rtu_read_slave_response != RESPONSE_OK)
        {    
            modbus_rtu_read_slave_response = readModBusRegister(MOTION_STATUS_ADDRESS, ONE_REGISTER, slave_id, READ_REGISTERS);
            if(++slave_read_response_error_count > SLAVE_RESPONSE_ERROR_CNT)
            {
                slave_read_response_error_count = 0;
                break;
            }
        } 
        if(modbus_rtu_read_slave_response == RESPONSE_OK)
        {
            digitalWrite(slave_id+1, LOW);
        }  
        else if(modbus_rtu_read_slave_response != RESPONSE_OK)
        {
            digitalWrite(slave_id+1, HIGH);
        }
        slave_id += 2;
    }    
    digitalWrite(A3, !digitalRead(A3));
}

/* End of Function loop */

/********************************************************************************************************************
  Function Name :  ISR(USART_RXC_vect)
  Description : 
    Interrupt Service Routine for USART Receive Complete Interrupt.
  Arguments : Nothing
  Return : Nothing
********************************************************************************************************************/

ISR(USART_RXC_vect)
{  
    serial_inChar = UDR0;
    switch(fsm_usart)
    {
        case WAIT :
            if(serial_inChar == SLAVE_ONE || serial_inChar == SLAVE_TWO || serial_inChar == SLAVE_THREE)
            {
                serial_valid_received_char_cnt = 0;
                Slave_Response[serial_valid_received_char_cnt] = serial_inChar;
                serial_valid_received_char_cnt++;
                fsm_usart = FILL;
            }
            break;
        case FILL :
            Slave_Response[serial_valid_received_char_cnt] = serial_inChar;
            serial_valid_received_char_cnt++;     
            break;  
    }  
    UCSR0B |= (1 << RXCIE0); 
}

/* End of ISR USART_RXC_vect */

/* End of Code */
