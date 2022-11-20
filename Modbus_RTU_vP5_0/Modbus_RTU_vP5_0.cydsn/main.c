/*==============================================================================
* Project : Implementation of Modbus RTU Function Subset on Cypress PSoC5
* Author : EmbVerse (www.embverse.com)
* Date : August 25, 2021
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
* The Development Kit used in the project is found here :
* https://www.cypress.com/documentation/development-kitsboards/cy8ckit-059-psoc-5lp-prototyping-kit-onboard-programmer-and
* The working of all the functions has been tested on a Modbus Master Tool called qModbus.
* Download qModbus from here : https://sourceforge.net/projects/qmodbus/
*===============================================================================
*/
#include <project.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "modbus_rtu.h"

// Defined Constants - Start -
#define USED_EEPROM_SECTOR (1u)
#define FIRST_POWERUP_EEPROM_ADDRESS ((USED_EEPROM_SECTOR * CYDEV_EEPROM_SECTOR_SIZE) + 0x00)
#define COIL_BEGIN_EEPROM_ADDRESS FIRST_POWERUP_EEPROM_ADDRESS + 1
#define HOLDING_REGISTER_BEGIN_EEPROM_ADDRESS COIL_BEGIN_EEPROM_ADDRESS + 8
#define DISCRETE_INPUT_BEGIN_EEPROM_ADDRESS HOLDING_REGISTER_BEGIN_EEPROM_ADDRESS + 128
#define INPUT_REGISTER_BEGIN_EEPROM_ADDRESS DISCRETE_INPUT_BEGIN_EEPROM_ADDRESS + 8
#define COIL 0x00
#define DISCRETE_INPUT COIL + 1
#define INPUT_REGISTER DISCRETE_INPUT + 1
#define HOLDING_REGISTER INPUT_REGISTER + 1
#define MODBUS_IDLE 0u
#define MODBUS_Rx_DATA_COLLECTION MODBUS_IDLE + 1
#define MODBUS_Rx_DATA_RECEIVED MODBUS_Rx_DATA_COLLECTION + 1
#define SET 1u
#define CLEAR 0u
#define NO_OF_TICKS 24000u
#define STATUS_LED_ON Status_LED_Write(SET)
#define STATUS_LED_OFF Status_LED_Write(CLEAR)
#define STATUS_LED_TOGGLE Status_LED_Write(!Status_LED_Read())
// Defined Constants - End -

// Variable Declaration - Start -
uint8 byte_count = 0;
uint8 modbus_received_data[64];
uint8 fsm = MODBUS_IDLE;
uint8 uart_data_rx[64];
uint32  SysTick_value;
volatile uint32 source = 0;
char rx_character;
// Variable Declaration - End -

// Interrupt Service Routine Declaration - Start - 
CY_ISR_PROTO(MySysTickHandler);
CY_ISR_PROTO (ISRModbusRTUOverUARTRxHandler);
// Interrupt Service Routine Declaration - End

//Function Declaration - Start -
void initialize_eeprom(void);
void read_eeprom(void);
void update_eeprom(uint8_t parameter);
void system_initialize(void);
//Function Declaration - End -

void initialize_eeprom(void)
{    
    for(uint8 i = 0; i < COIL_BYTES; i++)
    {
        modbus_coil_value[i] = 0xFF;
        modbus_response_coil_temp[i] = modbus_coil_value[i];
    }    
    update_eeprom(COIL);   
    for(uint8 i = 0; i < DISCRETE_INPUT_BYTES; i++)
    {
        modbus_discrete_input_value[i] = 0xFF;                
        modbus_response_discrete_input_temp[i] = modbus_discrete_input_value[i];
    }
    update_eeprom(DISCRETE_INPUT);
    for(uint8 i = 0; i < NUMBER_OF_HOLDING_REGISTERS; i++)
    {   
        modbus_holding_registers_value[i] = 0xFFFF;
        modbus_response_holding_registers_temp[i] = modbus_holding_registers_value[i];
    }
    update_eeprom(HOLDING_REGISTER);
    for(uint8 i = 0; i < NUMBER_OF_INPUT_REGISTERS; i++)
    {
        modbus_input_registers_value[i] = 0xAA55;
        modbus_response_input_registers_temp[i] =  modbus_input_registers_value[i];
    }
    update_eeprom(INPUT_REGISTER);
}
void read_eeprom(void)
{
    for(uint8 i = 0; i < COIL_BYTES; i++)
    {
        modbus_coil_value[i] = EEPROM_ReadByte(COIL_BEGIN_EEPROM_ADDRESS + i);
        modbus_response_coil_temp[i] = modbus_coil_value[i];
    }
    for(uint8 i = 0; i < DISCRETE_INPUT_BYTES; i++)
    {
        modbus_discrete_input_value[i] = EEPROM_ReadByte(DISCRETE_INPUT_BEGIN_EEPROM_ADDRESS + i);
        modbus_response_discrete_input_temp[i] = modbus_discrete_input_value[i];
    }
    for(uint8 i = 0; i < NUMBER_OF_HOLDING_REGISTERS; i++)
    {
        modbus_holding_registers_value[i] = (uint16)(EEPROM_ReadByte(HOLDING_REGISTER_BEGIN_EEPROM_ADDRESS + (2*i)) << 8)
                                          | (uint16)(EEPROM_ReadByte(HOLDING_REGISTER_BEGIN_EEPROM_ADDRESS + (2*i + 1)) & 0xFF);
        modbus_response_holding_registers_temp[i] = modbus_holding_registers_value[i];
    }
    for(uint8 i = 0; i < NUMBER_OF_INPUT_REGISTERS; i++)
    {
        modbus_input_registers_value[i] = (uint16)(EEPROM_ReadByte(INPUT_REGISTER_BEGIN_EEPROM_ADDRESS + (2*i)) << 8)
                                          | (uint16)(EEPROM_ReadByte(INPUT_REGISTER_BEGIN_EEPROM_ADDRESS + (2*i + 1)) & 0xFF);
        modbus_response_input_registers_temp[i] = modbus_input_registers_value[i];
    }
}
void update_eeprom(uint8_t parameter)
{
    switch(parameter)
    {
        case COIL:
            for(uint8 i = 0; i < COIL_BYTES; i++)
            {
                modbus_coil_value[i] = modbus_response_coil_temp[i];
                EEPROM_WriteByte(modbus_coil_value[i], COIL_BEGIN_EEPROM_ADDRESS + i);
            }
            break;
        case DISCRETE_INPUT:
            for(uint8 i = 0; i < DISCRETE_INPUT_BYTES; i++)
            {
                modbus_discrete_input_value[i] = modbus_response_discrete_input_temp[i];
                EEPROM_WriteByte(modbus_discrete_input_value[i], DISCRETE_INPUT_BEGIN_EEPROM_ADDRESS + i);
            }
            break;    
        case HOLDING_REGISTER:
            for(uint8 i = 0; i < NUMBER_OF_HOLDING_REGISTERS; i++)
            {
                modbus_holding_registers_value[i] = modbus_response_holding_registers_temp[i];
                EEPROM_WriteByte((uint8_t)(modbus_holding_registers_value[i] >> 8), HOLDING_REGISTER_BEGIN_EEPROM_ADDRESS + (2*i));
                EEPROM_WriteByte((uint8_t)(modbus_holding_registers_value[i]) & 0xFF, HOLDING_REGISTER_BEGIN_EEPROM_ADDRESS + (2*i + 1));
            }
            break;
        case INPUT_REGISTER:
            for(uint8 i = 0; i < NUMBER_OF_INPUT_REGISTERS; i++)
            {
                modbus_input_registers_value[i] = modbus_response_input_registers_temp[i];
                EEPROM_WriteByte((uint8_t)(modbus_input_registers_value[i] >> 8), INPUT_REGISTER_BEGIN_EEPROM_ADDRESS + (2*i));
                EEPROM_WriteByte((uint8_t)(modbus_input_registers_value[i] & 0xFF), INPUT_REGISTER_BEGIN_EEPROM_ADDRESS + (2*i + 1));
            }
            break;    
        default:    
            break;            
    }
}

void system_initialize(void)
{
    CyGlobalIntEnable; /* Enable global interrupts. */
    EEPROM_Start();
    CySysTickStart();  /* Start the systick */
    CySysTickSetCallback(0, MySysTickHandler);  /* Add the Systick callback */
    SysTick_value = 0;
    uint8 EEPROM_PowerUp_flag;   
    EEPROM_PowerUp_flag = EEPROM_ReadByte(FIRST_POWERUP_EEPROM_ADDRESS);    
    if(EEPROM_PowerUp_flag == 0)
    {
        STATUS_LED_ON;
        initialize_eeprom();
        EEPROM_PowerUp_flag = 1;
        EEPROM_WriteByte(EEPROM_PowerUp_flag, FIRST_POWERUP_EEPROM_ADDRESS);
    }
    else
        STATUS_LED_OFF;
    read_eeprom();   
    Modbus_RTU_Over_UART_Start();
    ISR_MBRTU_UART_Rx_StartEx(ISRModbusRTUOverUARTRxHandler);                                    // start the isr of uart
        
}
int main(void)
{
    system_initialize();
    while(1)
    {
        if(fsm == MODBUS_Rx_DATA_RECEIVED)
        {            
            for(uint8 i = 0; i <= byte_count; i++)
            {
                modbus_received_data[i] = uart_data_rx[i];
            }
            if(modbus_received_data[0] == MODBUS_SLAVE_ADDRESS)
            {
                switch(modbus_received_data[1])
                {
                    case 0x01:
                        query_response_read_coil(&modbus_received_data[0], byte_count + 1);
                        for(uint8 i = 0; i < modbus_response_data_length; i++)
                        {
                            Modbus_RTU_Over_UART_PutChar(modbus_response_read_coil[i]);
                        }
                        break;
                    case 0x02:
                        query_response_read_discrete_input(&modbus_received_data[0], byte_count + 1);
                
                        for(uint8 i = 0; i < modbus_response_data_length; i++)
                        {
                            Modbus_RTU_Over_UART_PutChar(modbus_response_read_discrete_input[i]);
                        }
                        break;
                    case 0x03 :
                        query_response_read_holding_registers(&modbus_received_data[0], byte_count + 1);
                        for(uint8 i = 0; i < modbus_response_data_length; i++)
                        {
                            Modbus_RTU_Over_UART_PutChar(modbus_response_read_holding_registers[i]);
                        }
                        break;
                    case 0x04 :
                        query_response_read_input_registers(&modbus_received_data[0], byte_count + 1);
                        for(uint8 i = 0; i < modbus_response_data_length; i++)
                        {
                            Modbus_RTU_Over_UART_PutChar(modbus_response_read_input_registers[i]);
                        }
                        break;
                    case 0x05 :
                        query_response_force_single_coil(&modbus_received_data[0], byte_count + 1);
                        for(uint8 i = 0; i < modbus_response_data_length; i++)
                        {
                            Modbus_RTU_Over_UART_PutChar(modbus_response_force_single_coil[i]);
                        }
                        update_eeprom(COIL);
                        break;
                    case 0x06 :
                        query_response_preset_single_register(&modbus_received_data[0], byte_count + 1);
                        for(uint8 i = 0; i < modbus_response_data_length; i++)
                        {
                            Modbus_RTU_Over_UART_PutChar(modbus_response_preset_single_register[i]);
                        }
                        update_eeprom(HOLDING_REGISTER);
                        break;
                    case 0x0F :
                        query_response_force_multiple_coils(&modbus_received_data[0], byte_count + 1);
                        for(uint8 i = 0; i < modbus_response_data_length; i++)
                        {
                            Modbus_RTU_Over_UART_PutChar(modbus_response_force_multiple_coils[i]);
                        }
                        update_eeprom(COIL);
                        break;
                    case 0x10 :
                        query_response_preset_multiple_registers(&modbus_received_data[0], byte_count + 1);
                        for(uint8 i = 0; i < modbus_response_data_length; i++)
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
            byte_count = 0;
        }
    }
}

CY_ISR(ISRModbusRTUOverUARTRxHandler)
{
    rx_character =  Modbus_RTU_Over_UART_GetChar();
    CySysTickStart();
    switch(fsm)
    {
        case MODBUS_IDLE:
        {
            uart_data_rx[0] = rx_character;
            if(uart_data_rx[0] == MODBUS_SLAVE_ADDRESS)
            {
                fsm = MODBUS_Rx_DATA_COLLECTION;
                CySysTickClear();
                SysTick_value = 0;
                CySysTickStart();
                break;
            }
            else
            {
                break;
            }
        }
        case MODBUS_Rx_DATA_COLLECTION:
        {
            byte_count++;
            uart_data_rx[byte_count] = rx_character;
            CySysTickClear();
            SysTick_value = 0;
            CySysTickStart();
            break;
        }
    }
    source =  Modbus_RTU_Over_UART_ReadRxStatus();
    Modbus_RTU_Over_UART_ClearRxBuffer();
    source = 0;
}
CY_ISR(MySysTickHandler)
{
    SysTick_value++;
    if(SysTick_value >= 4u)
    {
        SysTick_value = 0;
        fsm =  MODBUS_Rx_DATA_RECEIVED;
        CySysTickStop();
    }    
}
/* [] END OF FILE */