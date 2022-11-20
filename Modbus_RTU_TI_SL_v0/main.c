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
* The TI Stellaris Launchpad Development Kit used in the project is found here :
* https://www.ubuy.co.in/catalog/product/view/id/27359233/s/texas-instruments-ek-lm4f120xl-eval-board-stellaris-lm4f120-launchpad?utm_source=gad&utm_medium=cpc&utm_campaign=inshop&loc=9061769&gclid=CjwKCAjwvuGJBhB1EiwACU1AiR9MwUALlrNoOZxt5SP_fVnhyqKnFxhnR8at2itEOaSbfTEfByxIGhoC538QAvD_BwE
* The working of all the functions has been tested on a Modbus Master Tool called qModbus.
* Download qModbus from here : https://sourceforge.net/projects/qmodbus/
*===============================================================================
*/
#include "inc/lm4f120h5qr.h"
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "driverlib/fpu.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "driverlib/systick.h"
#include "driverlib/eeprom.h"
#include "utils/uartstdio.h"
#include "modbus_rtu.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#ifdef DEBUG
void
__error__(char *pcFilename, unsigned long ulLine)
{
}
#endif
#define LED 0x02
#define MODBUS_IDLE 0u
#define MODBUS_Rx_DATA_COLLECTION MODBUS_IDLE + 1
#define MODBUS_Rx_DATA_RECEIVED MODBUS_Rx_DATA_COLLECTION + 1
#define FIRST_POWERUP_EEPROM_ADDRESS 0
#define COIL_BEGIN_EEPROM_ADDRESS FIRST_POWERUP_EEPROM_ADDRESS + 4
#define HOLDING_REGISTER_BEGIN_EEPROM_ADDRESS COIL_BEGIN_EEPROM_ADDRESS + 52
#define DISCRETE_INPUT_BEGIN_EEPROM_ADDRESS HOLDING_REGISTER_BEGIN_EEPROM_ADDRESS + 532
#define INPUT_REGISTER_BEGIN_EEPROM_ADDRESS DISCRETE_INPUT_BEGIN_EEPROM_ADDRESS + 52
#define COIL 0x00
#define DISCRETE_INPUT COIL + 1
#define INPUT_REGISTER DISCRETE_INPUT + 1
#define HOLDING_REGISTER INPUT_REGISTER + 1
// Variable Declaration - Start -
uint8_t byte_count = 0;
uint8_t modbus_received_data[64];
uint8_t fsm = MODBUS_IDLE;
uint8_t uart_data_rx[150];
uint32_t  SysTick_value = 0;
char rx_character;
uint8_t fsm_uart = MODBUS_IDLE;
char received_character = '\0';
char received_string[20];
// Variable Declaration - End -
void UARTSend(uint8_t *pucBuffer, uint8_t ulCount);
void systick_int(void)
{
	SysTick_value++;
	if(SysTick_value >= 4u)
	{
	    SysTick_value = 0;
	    fsm =  MODBUS_Rx_DATA_RECEIVED;
	    SysTickDisable();
	}
}
void UARTIntHandler(void)
{
    unsigned long ulStatus;
    ulStatus = ROM_UARTIntStatus(UART0_BASE, true);
    ROM_UARTIntClear(UART0_BASE, ulStatus);
    if(ROM_UARTCharsAvail(UART0_BASE))
    {
    	rx_character =  ROM_UARTCharGetNonBlocking(UART0_BASE);
    	SysTickEnable();
   	    switch(fsm)
   	    {
   	        case MODBUS_IDLE:
   	        {
   	            uart_data_rx[0] = rx_character;
   	            if(uart_data_rx[0] == MODBUS_SLAVE_ADDRESS)
   	            {
   	                fsm = MODBUS_Rx_DATA_COLLECTION;
   	                SysTick_value = 0;
   	                SysTickEnable();
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
   	            SysTick_value = 0;
   	            SysTickEnable();
   	            break;
   	        }
   	        default:
   	        	break;
   	    }
    }
}
void UARTSend(uint8_t *pucBuffer, uint8_t ulCount)
{
	while(ulCount)
	{
		ROM_UARTCharPut(UART0_BASE, *pucBuffer);
		pucBuffer++;
		ulCount--;
	}
}
void update_eeprom(uint8_t parameter)
{
	uint8_t i;
    switch(parameter)
    {
        case COIL:
            for(i = 0; i < COIL_BYTES; i++)
            {
                modbus_coil_value[i] = modbus_response_coil_temp[i];
                EEPROMProgram((unsigned long *)&modbus_coil_value[i], COIL_BEGIN_EEPROM_ADDRESS + (4*i), sizeof(uint8_t));
            }
            break;
        case DISCRETE_INPUT:
            for(i = 0; i < DISCRETE_INPUT_BYTES; i++)
            {
                modbus_discrete_input_value[i] = modbus_response_discrete_input_temp[i];
                EEPROMProgram((unsigned long *)&modbus_discrete_input_value[i], DISCRETE_INPUT_BEGIN_EEPROM_ADDRESS + (4*i), sizeof(uint8_t));
            }
            break;
        case HOLDING_REGISTER:

            for(i = 0; i < NUMBER_OF_HOLDING_REGISTERS; i++)
            {
            	modbus_holding_registers_value[i] = modbus_response_holding_registers_temp[i];
            	modbus_holding_registers_value_H[i] = (uint8_t)(modbus_holding_registers_value[i] >> 8);
            	modbus_holding_registers_value_L[i] = (uint8_t)(modbus_holding_registers_value[i] & 0xFF);
                EEPROMProgram((unsigned long *)&modbus_holding_registers_value_H[i], HOLDING_REGISTER_BEGIN_EEPROM_ADDRESS + (8*i), sizeof(uint8_t));
                EEPROMProgram((unsigned long *)&modbus_holding_registers_value_L[i], HOLDING_REGISTER_BEGIN_EEPROM_ADDRESS + (8*i) + 4, sizeof(uint8_t));
            }
            break;
        case INPUT_REGISTER:
            for(i = 0; i < NUMBER_OF_INPUT_REGISTERS; i++)
            {
                modbus_input_registers_value[i] = modbus_response_input_registers_temp[i];
                modbus_input_registers_value_H[i] = (uint8_t)(modbus_input_registers_value[i] >> 8);
                modbus_input_registers_value_L[i] = (uint8_t)(modbus_input_registers_value[i] & 0xFF);
                EEPROMProgram((unsigned long *)&modbus_input_registers_value_H[i], INPUT_REGISTER_BEGIN_EEPROM_ADDRESS + (8*i), sizeof(uint8_t));
                EEPROMProgram((unsigned long *)&modbus_input_registers_value_L[i], INPUT_REGISTER_BEGIN_EEPROM_ADDRESS + (8*i) + 4, sizeof(uint8_t));
            }
            break;
        default:
            break;
    }
}

void initialize_eeprom(void)
{
	uint8_t i;
    for(i = 0; i < COIL_BYTES; i++)
    {
        modbus_coil_value[i] = 0xFF;
        modbus_response_coil_temp[i] = modbus_coil_value[i];
    }
    update_eeprom(COIL);
    for(i = 0; i < DISCRETE_INPUT_BYTES; i++)
    {
        modbus_discrete_input_value[i] = 0xFF;
        modbus_response_discrete_input_temp[i] = modbus_discrete_input_value[i];
    }
    update_eeprom(DISCRETE_INPUT);
    for(i = 0; i < NUMBER_OF_HOLDING_REGISTERS; i++)
    {
        modbus_holding_registers_value[i] = 0xBBEE;
        modbus_response_holding_registers_temp[i] = modbus_holding_registers_value[i];
    }
    update_eeprom(HOLDING_REGISTER);
    for(i = 0; i < NUMBER_OF_INPUT_REGISTERS; i++)
    {
        modbus_input_registers_value[i] = 0xAA55;
        modbus_response_input_registers_temp[i] =  modbus_input_registers_value[i];
    }
    update_eeprom(INPUT_REGISTER);
}
void read_eeprom(void)
{
	uint8_t i;
    for(i = 0; i < COIL_BYTES; i++)
    {
        EEPROMRead((unsigned long *)&modbus_coil_value[i], COIL_BEGIN_EEPROM_ADDRESS + (4*i), sizeof(uint8_t));
        modbus_response_coil_temp[i] = modbus_coil_value[i];
    }
    for(i = 0; i < DISCRETE_INPUT_BYTES; i++)
    {
        EEPROMRead((unsigned long *)&modbus_discrete_input_value[i], DISCRETE_INPUT_BEGIN_EEPROM_ADDRESS + (4*i), sizeof(uint8_t));
        modbus_response_discrete_input_temp[i] = modbus_discrete_input_value[i];
    }
    for(i = 0; i < NUMBER_OF_HOLDING_REGISTERS; i++)
    {
    	modbus_holding_registers_value_H[i] = 0;
    	modbus_holding_registers_value_L[i] = 0;
    	EEPROMRead((unsigned long *)&modbus_holding_registers_value_H[i], HOLDING_REGISTER_BEGIN_EEPROM_ADDRESS + (8*i), sizeof(uint8_t));
    	EEPROMRead((unsigned long *)&modbus_holding_registers_value_L[i], HOLDING_REGISTER_BEGIN_EEPROM_ADDRESS + (8*i) + 4, sizeof(uint8_t));
    	modbus_holding_registers_value[i] = ((uint16_t)((uint16_t)modbus_holding_registers_value_H[i]) << 8) | ((uint16_t)((uint16_t)modbus_holding_registers_value_L[i]) & 0xFF);
        modbus_response_holding_registers_temp[i] = modbus_holding_registers_value[i];
    }

    for(i = 0; i < NUMBER_OF_INPUT_REGISTERS; i++)
    {
        EEPROMRead((unsigned long *)&modbus_input_registers_value_H[i], INPUT_REGISTER_BEGIN_EEPROM_ADDRESS + (8*i), sizeof(uint8_t));
        EEPROMRead((unsigned long *)&modbus_input_registers_value_L[i], INPUT_REGISTER_BEGIN_EEPROM_ADDRESS + (8*i) + 4, sizeof(uint8_t));
        modbus_input_registers_value[i] = ((uint16_t)((uint16_t)modbus_input_registers_value_H[i]) << 8) | ((uint16_t)((uint16_t)modbus_input_registers_value_L[i]) & 0xFF);
        modbus_response_input_registers_temp[i] = modbus_input_registers_value[i];
    }
}
int main(void)
{
    ROM_FPUEnable();
    ROM_FPULazyStackingEnable();
    ROM_SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN |
                       SYSCTL_XTAL_16MHZ);
    SYSCTL_RCGC2_R = SYSCTL_RCGC2_GPIOF;
    uint32_t ulLoop = SYSCTL_RCGC2_R;
    GPIO_PORTF_DIR_R = LED;
    GPIO_PORTF_DEN_R = LED;
    GPIO_PORTF_DATA_R &= 0xFD;
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    ROM_IntMasterEnable();
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    ROM_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    UARTStdioInit(0);
    ROM_IntEnable(INT_UART0);
    ROM_UARTIntEnable(UART0_BASE, UART_INT_RX | UART_INT_RT);
    SysTickIntRegister(&systick_int);
    SysTickPeriodSet(SysCtlClockGet()/1000);
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_EEPROM0);
    EEPROMInit();
    uint8_t EEPROM_PowerUp_flag;
    //EEPROMMassErase(); // In Case, EEPROM Erase is required, uncomment this line
    EEPROMRead((unsigned long *)&EEPROM_PowerUp_flag, FIRST_POWERUP_EEPROM_ADDRESS, sizeof(uint8_t));
    if(EEPROM_PowerUp_flag == 0xFF)
    {
    	GPIO_PORTF_DATA_R ^= LED;
    	initialize_eeprom();
    	EEPROM_PowerUp_flag = 0x00;
    	EEPROMProgram((unsigned long *)&EEPROM_PowerUp_flag, FIRST_POWERUP_EEPROM_ADDRESS, sizeof(uint8_t));
    }
    else
    	GPIO_PORTF_DATA_R &= 0xF1;
    read_eeprom();
    while(1)
    {
    	if(fsm == MODBUS_Rx_DATA_RECEIVED)
    	{
    		uint8_t i;
    	    for(i = 0; i <= byte_count; i++)
    	    {
    	        modbus_received_data[i] = uart_data_rx[i];
    	    }
    	    if(modbus_received_data[0] == MODBUS_SLAVE_ADDRESS)
    	    {
    	        switch(modbus_received_data[1])
    	        {
    	        	case 0x01:
    	                query_response_read_coil(&modbus_received_data[0], byte_count + 1);
    	                UARTSend(&modbus_response_read_coil[0], modbus_response_data_length);
    	                break;
    	            case 0x02:
    	                query_response_read_discrete_input(&modbus_received_data[0], byte_count + 1);
    	                UARTSend(&modbus_response_read_discrete_input[0], modbus_response_data_length);
    	                break;
    	            case 0x03 :
    	                query_response_read_holding_registers(&modbus_received_data[0], byte_count + 1);
    	                UARTSend(&modbus_response_read_holding_registers[0], modbus_response_data_length);
    	                break;
    	            case 0x04 :
    	                query_response_read_input_registers(&modbus_received_data[0], byte_count + 1);
    	                UARTSend(&modbus_response_read_input_registers[0], modbus_response_data_length);
    	                break;
    	            case 0x05 :
    	                query_response_force_single_coil(&modbus_received_data[0], byte_count + 1);
    	                UARTSend(&modbus_response_force_single_coil[0], modbus_response_data_length);
    	                update_eeprom(COIL);
    	                break;
    	            case 0x06 :
    	                query_response_preset_single_register(&modbus_received_data[0], byte_count + 1);
    	                UARTSend(&modbus_response_preset_single_register[0], modbus_response_data_length);
    	                update_eeprom(HOLDING_REGISTER);
    	                break;
    	            case 0x0F :
    	                query_response_force_multiple_coils(&modbus_received_data[0], byte_count + 1);
    	                UARTSend(&modbus_response_force_multiple_coils[0], modbus_response_data_length);
    	                update_eeprom(COIL);
    	                break;
    	            case 0x10 :
    	                query_response_preset_multiple_registers(&modbus_received_data[0], byte_count + 1);
    	                UARTSend(&modbus_response_preset_multiple_registers[0], modbus_response_data_length);
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
