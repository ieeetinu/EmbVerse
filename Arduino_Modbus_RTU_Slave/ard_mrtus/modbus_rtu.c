/* ========================================
 * File : modbus_rtu.c
 * Author : EmbVerse (www.embverse.com)
 * Date : August 25, 2021
 * ========================================
*/

#include "modbus_rtu.h"
#include <stdint.h>

//#define PSoC5 // For the code segments that should get active only if it's PSoC5

#ifdef PSoC5
#define STATUS_LED_ON Status_LED_Write(SET)
#define STATUS_LED_OFF Status_LED_Write(CLEAR)
#define STATUS_LED_TOGGLE Status_LED_Write(!Status_LED_Read())
#endif
uint8_t reverse_data(uint8_t number);
uint8_t modbus_response_data_length;
uint16_t modbus_holding_registers_value[NUMBER_OF_HOLDING_REGISTERS];
uint8_t modbus_holding_registers_value_H[NUMBER_OF_HOLDING_REGISTERS];
uint8_t modbus_holding_registers_value_L[NUMBER_OF_HOLDING_REGISTERS];
uint16_t modbus_input_registers_value[NUMBER_OF_INPUT_REGISTERS];
uint8_t modbus_input_registers_value_H[NUMBER_OF_INPUT_REGISTERS];
uint8_t modbus_input_registers_value_L[NUMBER_OF_INPUT_REGISTERS];
uint8_t modbus_discrete_input_value[DISCRETE_INPUT_BYTES + 1];
uint8_t modbus_coil_value[COIL_BYTES + 1];
uint8_t modbus_response_read_coil[COIL_BYTES + 5];
uint8_t modbus_response_read_discrete_input[DISCRETE_INPUT_BYTES + 5];
uint8_t modbus_response_read_holding_registers[(2*NUMBER_OF_HOLDING_REGISTERS) + 5];
uint8_t modbus_response_preset_single_register[8];
uint8_t modbus_response_preset_multiple_registers[8];
uint8_t modbus_response_read_input_registers[(2*NUMBER_OF_INPUT_REGISTERS) + 5];
uint8_t modbus_response_force_single_coil[8];
uint8_t modbus_response_force_multiple_coils[8];
uint8_t modbus_response_coil_temp[COIL_BYTES + 1];
uint8_t modbus_response_discrete_input_temp[DISCRETE_INPUT_BYTES + 1];
uint16_t modbus_response_holding_registers_temp[NUMBER_OF_HOLDING_REGISTERS];
uint16_t modbus_response_input_registers_temp[NUMBER_OF_INPUT_REGISTERS];
void query_response_read_coil(uint8_t *modbus_received_data, uint8_t number_of_bytes);
void query_response_read_input_registers(uint8_t *modbus_received_data, uint8_t number_of_bytes);
void query_response_read_discrete_input(uint8_t *modbus_received_data, uint8_t number_of_bytes);
void query_response_read_holding_registers(uint8_t *modbus_received_data, uint8_t number_of_bytes);
void query_response_force_single_coil(uint8_t *modbus_received_data, uint8_t number_of_bytes);
void query_response_preset_single_register(uint8_t *modbus_received_data, uint8_t number_of_bytes);
void query_response_preset_multiple_registers(uint8_t *modbus_received_data, uint8_t number_of_bytes);
void query_response_force_multiple_coils(uint8_t *modbus_received_data, uint8_t number_of_bytes);
unsigned short CRC16(unsigned char *puchMsg, unsigned short usDataLen);
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
uint8_t reverse_data(uint8_t number)
{
    uint8_t data_array[8] = {number & 0x80, number & 0x40, number & 0x20, number & 0x10, number & 0x08, number & 0x04, number & 0x02, number & 0x01};
    number = (data_array[0] >> 7) | (data_array[1] >> 5) | (data_array[2] >> 3) | (data_array[3] >> 1) | (data_array[4] << 1) | (data_array[5] << 3) | (data_array[6] << 5) | (data_array[7] << 7);
    return number;
}
void query_response_read_coil(uint8_t *modbus_received_data, uint8_t number_of_bytes)
{
    uint8_t modulo_coil_count_array[8] = {0x00, 0x80, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC, 0xFE};
    crc16 = CRC16(&(*(modbus_received_data + 0)), number_of_bytes - 2);
    if(((uint16_t)(*(modbus_received_data + number_of_bytes - 2)) == (crc16 >> 8)) && ((uint16_t)(*(modbus_received_data + number_of_bytes - 1) == (crc16 & 0x00FF))))
    {
        if((*(modbus_received_data + 0)) == MODBUS_SLAVE_ADDRESS)
        {
            uint16_t coil_count;
            coil_count = ((uint16_t)(*(modbus_received_data + 4)) << 8) | ((uint16_t)(*(modbus_received_data + 5)) & 0xFF);
            uint16_t coil_start_address;
            coil_start_address = (((uint16_t)(*(modbus_received_data + 2)) << 8) | ((uint16_t)(*(modbus_received_data + 3)) & 0xFF));
            if(coil_start_address >= NUMBER_OF_COILS)
            {
                modbus_response_data_length = 5;
                modbus_response_read_coil[0] = *(modbus_received_data + 0);
                modbus_response_read_coil[1] = *(modbus_received_data + 1) + 128;
                modbus_response_read_coil[2] = 0x02;
                crc16 = CRC16(&modbus_response_read_coil[0], 3); 
                modbus_response_read_coil[3] = (uint8_t)(crc16 >> 8);
                modbus_response_read_coil[4] = (uint8_t)(crc16 & 0x00FF);
            }   
            else if((coil_start_address + coil_count) >= (NUMBER_OF_COILS + 1))
            {
                modbus_response_data_length = 5;
                modbus_response_read_coil[0] = *(modbus_received_data + 0);
                modbus_response_read_coil[1] = *(modbus_received_data + 1) + 128;
                modbus_response_read_coil[2] = 0x03;
                crc16 = CRC16(&modbus_response_read_coil[0], 3); 
                modbus_response_read_coil[3] = (uint8_t)(crc16 >> 8);
                modbus_response_read_coil[4] = (uint8_t)(crc16 & 0x00FF);
            }
            else
            {
                //STATUS_LED_TOGGLE;
                uint8_t modulo_coil_count = (uint8_t)coil_count % 8; 
                uint8_t modulo_start_address = (uint8_t)(coil_start_address % 8);
				uint8_t coil_index = coil_count / 8;
                uint8_t byte_index = (uint8_t)(coil_start_address / 8);
                modbus_response_read_coil[0] = *(modbus_received_data + 0);
                modbus_response_read_coil[1] = *(modbus_received_data + 1);
                uint8_t u = 0;
				if(modulo_start_address == 0 && modulo_coil_count == 0)
				{
					for(u = 0; u < coil_index; u++)
					{
						modbus_response_read_coil[u + 3] = modbus_coil_value[u + byte_index];
					}	
				}
				else if(modulo_start_address == 0 && modulo_coil_count < 8)
				{
					for(u = 0; u < coil_index; u++)
					{
						modbus_response_read_coil[u + 3] = modbus_coil_value[u + byte_index];// & modulo_coil_count_array[byte_index]);
					}	
				}
				else if(modulo_start_address != 0)
			    {
					for(u = 0; u < coil_index; u++)
					{
						modbus_response_read_coil[u + 3] = (modbus_coil_value[u + byte_index] << modulo_start_address) | (modbus_coil_value[u + byte_index + 1] >> (8 - modulo_start_address));
					}	
			    }
				if(modulo_coil_count != 0 )
                {
                    modbus_response_data_length = 5 + (((coil_count - modulo_coil_count + 8) / 8));
                    modbus_response_read_coil[u + 3] = reverse_data(((modbus_coil_value[u + byte_index] << modulo_start_address)) | ((modbus_coil_value[u + byte_index + 1]) >> (8 - modulo_start_address))) & modulo_coil_count_array[modulo_coil_count];   
                    modbus_response_read_coil[2] = (uint8_t)((coil_count + 8 - modulo_coil_count)/ 8);
                }
                else
                {
                    modbus_response_read_coil[2] = (uint8_t)((coil_count + 8 - modulo_coil_count)/ 8) - 1;
                    modbus_response_data_length = 5 + (((coil_count - modulo_coil_count + 8) / 8)) - 1;
                }
                crc16 = CRC16(&modbus_response_read_coil[0], modbus_response_data_length - 2);
                modbus_response_read_coil[modbus_response_data_length - 2] = (uint8_t)(crc16 >> 8);
                modbus_response_read_coil[modbus_response_data_length - 1] = (uint8_t)(crc16 & 0x00FF);
            }
        }       
    }
}
void query_response_read_discrete_input(uint8_t *modbus_received_data, uint8_t number_of_bytes)
{
    uint8_t modulo_discrete_input_count_array[8] = {0x00, 0x80, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC, 0xFE};
    crc16 = CRC16(&(*(modbus_received_data + 0)), number_of_bytes - 2);
    if(((uint16_t)(*(modbus_received_data + number_of_bytes - 2)) == (crc16 >> 8)) && ((uint16_t)(*(modbus_received_data + number_of_bytes - 1) == (crc16 & 0x00FF))))
    {
        if((*(modbus_received_data + 0)) == MODBUS_SLAVE_ADDRESS)
        {
            uint16_t discrete_input_count;
            discrete_input_count = ((uint16_t)(*(modbus_received_data + 4)) << 8) | ((uint16_t)(*(modbus_received_data + 5)) & 0xFF);
            uint16_t discrete_input_start_address;
            discrete_input_start_address = (((uint16_t)(*(modbus_received_data + 2)) << 8) | ((uint16_t)(*(modbus_received_data + 3)) & 0xFF));
            if(discrete_input_start_address >= NUMBER_OF_DISCRETE_INPUTS)
            {
                modbus_response_data_length = 5;
                modbus_response_read_discrete_input[0] = *(modbus_received_data + 0);
                modbus_response_read_discrete_input[1] = *(modbus_received_data + 1) + 128;
                modbus_response_read_discrete_input[2] = 0x02;
                crc16 = CRC16(&modbus_response_read_coil[0], 3); 
                modbus_response_read_discrete_input[3] = (uint8_t)(crc16 >> 8);
                modbus_response_read_discrete_input[4] = (uint8_t)(crc16 & 0x00FF);
            }
            else if((discrete_input_start_address + discrete_input_count) >= (NUMBER_OF_DISCRETE_INPUTS + 1))
            {
                modbus_response_data_length = 5;
                modbus_response_read_discrete_input[0] = *(modbus_received_data + 0);
                modbus_response_read_discrete_input[1] = *(modbus_received_data + 1) + 128;
                modbus_response_read_discrete_input[2] = 0x03;
                crc16 = CRC16(&modbus_response_read_coil[0], 3); 
                modbus_response_read_discrete_input[3] = (uint8_t)(crc16 >> 8);
                modbus_response_read_discrete_input[4] = (uint8_t)(crc16 & 0x00FF);
            }
            else
            {
                //STATUS_LED_TOGGLE;
                uint8_t modulo_discrete_input_count = (uint8_t)discrete_input_count % 8;
                uint8_t modulo_start_address = (uint8_t)(discrete_input_start_address % 8);
                uint8_t byte_index = (uint8_t)(discrete_input_start_address / 8);
                modbus_response_read_discrete_input[0] = *(modbus_received_data + 0);
                modbus_response_read_discrete_input[1] = *(modbus_received_data + 1);
                uint8_t u = 0;
                for(u = 0; u < ((discrete_input_count + 8 - modulo_discrete_input_count)/ 8); u++)
                {
                    modbus_response_read_discrete_input[u + 3] = ((modbus_discrete_input_value[byte_index] >> modulo_start_address)) | (modbus_discrete_input_value[byte_index + 1] << (8 - modulo_start_address));
                    byte_index++;
                }
                if(modulo_discrete_input_count != 0)
                {
                    modbus_response_data_length = 5 + (((discrete_input_count - modulo_discrete_input_count + 8) / 8));
                    modbus_response_read_discrete_input[u + 2] = (((modbus_response_discrete_input_temp[byte_index] >> modulo_start_address)) | ((modbus_response_discrete_input_temp[byte_index + 1]) << (8 - modulo_start_address))) & modulo_discrete_input_count_array[modulo_discrete_input_count];
                    modbus_response_read_discrete_input[2] = (uint8_t)((discrete_input_count + 8 - modulo_discrete_input_count)/ 8);
                }
                else
                {
                    modbus_response_read_discrete_input[2] = (uint8_t)((discrete_input_count + 8 - modulo_discrete_input_count)/ 8) - 1;
                    modbus_response_data_length = 5 + (((discrete_input_count - modulo_discrete_input_count + 8) / 8)) - 1;
                }
                crc16 = CRC16(&modbus_response_read_discrete_input[0], modbus_response_data_length - 2);
                modbus_response_read_discrete_input[modbus_response_data_length - 2] = (uint8_t)(crc16 >> 8);
                modbus_response_read_discrete_input[modbus_response_data_length - 1] = (uint8_t)(crc16 & 0x00FF);
            }
        }       
    }
}
void query_response_force_single_coil(uint8_t *modbus_received_data, uint8_t number_of_bytes)
{
    crc16 = CRC16(&(*(modbus_received_data + 0)), number_of_bytes - 2);
    if(((uint16_t)(*(modbus_received_data + number_of_bytes - 2)) == (crc16 >> 8)) && ((uint16_t)(*(modbus_received_data + number_of_bytes - 1) == (crc16 & 0x00FF))))
    {
        if((*modbus_received_data + 0) == MODBUS_SLAVE_ADDRESS)
        {
            uint16_t coil_address = ((uint16_t)(*(modbus_received_data + 2)) << 8) | (uint16_t)(*(modbus_received_data + 3));
            uint16_t forced_coil_value = ((uint16_t)(*(modbus_received_data + 4)) << 8) | (uint16_t)(*(modbus_received_data + 5));
            if(coil_address >= NUMBER_OF_COILS)
            {
                modbus_response_data_length = 5;
                modbus_response_force_single_coil[0] = *(modbus_received_data + 0);
                modbus_response_force_single_coil[1] = *(modbus_received_data + 1) + 128;
                modbus_response_force_single_coil[2] = 0x02;
                crc16 = CRC16(&modbus_response_read_coil[0], 3); 
                modbus_response_force_single_coil[3] = (uint8_t)(crc16 >> 8);
                modbus_response_force_single_coil[4] = (uint8_t)(crc16 & 0x00FF);
            }
            else if(forced_coil_value != 0x0000 && forced_coil_value != 0xFF00)
            {
                modbus_response_data_length = 5;
                modbus_response_force_single_coil[0] = *(modbus_received_data + 0);
                modbus_response_force_single_coil[1] = *(modbus_received_data + 1) + 128;
                modbus_response_force_single_coil[2] = 0x03;
                crc16 = CRC16(&modbus_response_read_coil[0], 3); 
                modbus_response_force_single_coil[3] = (uint8_t)(crc16 >> 8);
                modbus_response_force_single_coil[4] = (uint8_t)(crc16 & 0x00FF);
            }
            else
            {
                //STATUS_LED_TOGGLE;
                uint8_t modulo_coil_address = coil_address % 8;
                uint8_t coil_address_index = coil_address / 8;
                uint8_t force_single_coil_array_for_zero_pad[8] = {0x7F, 0xBF, 0xDF, 0xEF, 0xF7, 0xFB, 0xFD, 0xFE};
                uint8_t force_single_coil_array_for_writing_one_pad[8] = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};
                switch(forced_coil_value)
                {
                    case 0xFF00:
                        modbus_response_coil_temp[coil_address_index] = (modbus_response_coil_temp[coil_address_index] & force_single_coil_array_for_zero_pad[modulo_coil_address]) | force_single_coil_array_for_writing_one_pad[modulo_coil_address];
                        break;
                    case 0x0000:
                        modbus_response_coil_temp[coil_address_index] = (modbus_response_coil_temp[coil_address_index] & force_single_coil_array_for_zero_pad[modulo_coil_address]);
                        break;
                    default:
                        break;
                }
                modbus_response_force_single_coil[0] = *(modbus_received_data + 0);
                modbus_response_force_single_coil[1] = *(modbus_received_data + 1);
                modbus_response_force_single_coil[2] = *(modbus_received_data + 2);
                modbus_response_force_single_coil[3] = *(modbus_received_data + 3);
                modbus_response_force_single_coil[4] = *(modbus_received_data + 4);
                modbus_response_force_single_coil[5] = *(modbus_received_data + 5);
                modbus_response_data_length = 8;
                crc16 = CRC16(& modbus_response_force_single_coil[0], modbus_response_data_length - 2);
                modbus_response_force_single_coil[modbus_response_data_length - 2] = (uint8_t)(crc16 >> 8);
                modbus_response_force_single_coil[modbus_response_data_length - 1] = (uint8_t)(crc16 & 0x00FF);
            }
        }
    }   
}
void query_response_force_multiple_coils(uint8_t *modbus_received_data, uint8_t number_of_bytes)
{
    crc16 = CRC16(&(*(modbus_received_data + 0)), number_of_bytes - 2);
    if(((uint16_t)(*(modbus_received_data + number_of_bytes - 2)) == (crc16 >> 8)) && ((uint16_t)(*(modbus_received_data + number_of_bytes - 1) == (crc16 & 0x00FF))))
    {
        uint16_t quantity_of_coils = ((uint16_t)(*(modbus_received_data + 4)) << 8) | (uint16_t)(*(modbus_received_data + 5));
        uint16_t coil_start_address = ((uint16_t)(*(modbus_received_data + 2)) << 8) | (uint16_t)(*(modbus_received_data + 3));
        if(coil_start_address >= NUMBER_OF_COILS)
        {
            modbus_response_data_length = 5;
            modbus_response_force_multiple_coils[0] = *(modbus_received_data + 0);
            modbus_response_force_multiple_coils[1] = *(modbus_received_data + 1) + 128;
            modbus_response_force_multiple_coils[2] = 0x02;
            crc16 = CRC16(&modbus_response_read_coil[0], 3); 
            modbus_response_force_multiple_coils[3] = (uint8_t)(crc16 >> 8);
            modbus_response_force_multiple_coils[4] = (uint8_t)(crc16 & 0x00FF);
        }
        else if((coil_start_address + quantity_of_coils) >= (NUMBER_OF_COILS + 1))
        {
            modbus_response_data_length = 5;
            modbus_response_force_multiple_coils[0] = *(modbus_received_data + 0);
            modbus_response_force_multiple_coils[1] = *(modbus_received_data + 1) + 128;
            modbus_response_force_multiple_coils[2] = 0x03;
            crc16 = CRC16(&modbus_response_read_coil[0], 3); 
            modbus_response_force_multiple_coils[3] = (uint8_t)(crc16 >> 8);
            modbus_response_force_multiple_coils[4] = (uint8_t)(crc16 & 0x00FF);
        }
        else
        {
            //STATUS_LED_TOGGLE;
            uint8_t modulo_quantity_of_coils = quantity_of_coils % 8; 
            uint8_t modulo_coil_start_address = coil_start_address % 8; 
            uint8_t coil_address_index = (uint8_t)(coil_start_address / 8); 
            modbus_response_force_multiple_coils[0] = *(modbus_received_data + 0);
            modbus_response_force_multiple_coils[1] = *(modbus_received_data + 1);
            uint8_t coil_bit_padding_with_one[8] = {0x80, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC, 0xFE, 0xFF};
            uint8_t coil_bit_padding_with_zero[8] = {0x7F, 0x3F, 0x1F, 0x0F, 0x07, 0x03, 0x01, 0x00};
			uint8_t u = 0;
            if(modulo_coil_start_address == 0 && modulo_quantity_of_coils == 0)
            {
                for(u = 0; u < (*(modbus_received_data + 6)); u++)
                {
                    modbus_response_coil_temp[u + coil_address_index] = reverse_data(*(modbus_received_data + 7 + u));
                }
            }
            else if(modulo_coil_start_address == 0 && modulo_quantity_of_coils != 0)
            {
                for(u = 0; u < (*(modbus_received_data + 6) - 1); u++)
                {
                    modbus_response_coil_temp[u + coil_address_index] = reverse_data(*(modbus_received_data + 7 + u));
                }
                modbus_response_coil_temp[u + coil_address_index] = (modbus_response_coil_temp[coil_address_index] & coil_bit_padding_with_zero[modulo_quantity_of_coils - 1]) | ((reverse_data(*(modbus_received_data + 7 + u)) /*<< (8 - modulo_quantity_of_coils)*/) & coil_bit_padding_with_one[modulo_quantity_of_coils - 1]);
            }
            else if(modulo_coil_start_address != 0 && modulo_quantity_of_coils != 0)
            {
                if((modulo_coil_start_address + quantity_of_coils) <= 8)
                {
                    for(u = 0; u < *(modbus_received_data + 6); u++)
                    {
                        if((modulo_coil_start_address + quantity_of_coils) <= 7)
                        {
                            modbus_response_coil_temp[coil_address_index] = 
                            reverse_data((modbus_response_coil_temp[coil_address_index] 
                            & coil_bit_padding_with_one[modulo_coil_start_address - 1])
                            | (modbus_response_coil_temp[coil_address_index] 
                            & coil_bit_padding_with_zero[modulo_coil_start_address + modulo_quantity_of_coils - 1]) 
                            | (*(modbus_received_data + 7)) << (8 - modulo_coil_start_address - modulo_quantity_of_coils));
                            break;
                        }
                        if(u == 0)
                        {
                            modbus_response_coil_temp[coil_address_index] = 
                            (modbus_response_coil_temp[coil_address_index] 
                            & coil_bit_padding_with_one[modulo_coil_start_address - 1]) 
                            | (reverse_data(*(modbus_received_data + 7 + u)) >> (modulo_coil_start_address) 
                            & coil_bit_padding_with_zero[modulo_coil_start_address - 1]);
                            coil_address_index++;
                        }
                        else if((u == *(modbus_received_data + 6) - 1) && u >= 1)
                        {
                            if(((modulo_coil_start_address + modulo_quantity_of_coils) % 8) != 0)
                            {
                                if(*(modbus_received_data + 7 + u) == 0x05)
                                    modbus_response_coil_temp[coil_address_index] = 
                                    (modbus_response_coil_temp[coil_address_index] & coil_bit_padding_with_zero[((modulo_coil_start_address + modulo_quantity_of_coils) % 8) - 1]) 
                                    | (((reverse_data(*(modbus_received_data + 7 + u - 1)) << (8 - modulo_coil_start_address))
                                    | (reverse_data(*(modbus_received_data + 7 + u)) >> (modulo_coil_start_address)))
                                    & coil_bit_padding_with_one[((modulo_coil_start_address + modulo_quantity_of_coils) % 8) - 1]);
                            }
                            else
                            {
                                modbus_response_coil_temp[coil_address_index] = 
                                (modbus_response_coil_temp[coil_address_index] & 0x00) 
                                | ((reverse_data(*(modbus_received_data + 7 + u - 1)) << (8 - modulo_coil_start_address))
                                | ((reverse_data(*(modbus_received_data + 7 + u)) >> (modulo_coil_start_address))));
                            }
                        }
                        else
                        {
                            modbus_response_coil_temp[coil_address_index] = 
                            (modbus_response_coil_temp[coil_address_index] & 0x00) 
                            | ((reverse_data(*(modbus_received_data + 7 + u - 1)) << (8 - modulo_coil_start_address))
                            | ((reverse_data(*(modbus_received_data + 7 + u)) >> (modulo_coil_start_address))));
                            coil_address_index++;
                        }
                    }    
                }
                else if((modulo_coil_start_address + quantity_of_coils) > 8)
                {
                    for(u = 0; u < (*(modbus_received_data + 6) + 1); u++)
                    {
                        if(u == 0)
                        {
                            modbus_response_coil_temp[coil_address_index] = 
                            (modbus_response_coil_temp[coil_address_index] 
                            & coil_bit_padding_with_one[modulo_coil_start_address - 1]) 
                            | (reverse_data(*(modbus_received_data + 7 + u)) >> (modulo_coil_start_address) 
                            & coil_bit_padding_with_zero[modulo_coil_start_address - 1]);
							modbus_response_coil_temp[coil_address_index] = reverse_data(modbus_response_coil_temp[coil_address_index]);
                            coil_address_index++;
                        }
                        else if((u == *(modbus_received_data + 6)) && u >= 1)
                        {
                            modbus_response_coil_temp[coil_address_index] = 
                            (modbus_response_coil_temp[coil_address_index] & coil_bit_padding_with_zero[((modulo_coil_start_address + modulo_quantity_of_coils) % 8) - 1]) 
                            | ((reverse_data(*(modbus_received_data + 7 + u - 1)) << (8 - modulo_coil_start_address))
                            & coil_bit_padding_with_one[((modulo_coil_start_address + modulo_quantity_of_coils) % 8) - 1]);                        
                        }
                        else
                        {
                            modbus_response_coil_temp[coil_address_index] = 
                            (modbus_response_coil_temp[coil_address_index] & 0x00) 
                            | ((reverse_data(*(modbus_received_data + 7 + u - 1)) << (8 - modulo_coil_start_address))
                            | ((reverse_data(*(modbus_received_data + 7 + u)) >> (modulo_coil_start_address))));
							coil_address_index++;
                        }
                    }    
                }
            }    
            else if(modulo_coil_start_address != 0 && modulo_quantity_of_coils == 0)
            {
                for(u = 0; u < *(modbus_received_data + 6) + 1; u++)
                {
                    if(u == 0)
                    {
                        modbus_response_coil_temp[coil_address_index] = 
                        (modbus_response_coil_temp[coil_address_index] 
                        & coil_bit_padding_with_one[modulo_coil_start_address - 1]) 
                        | ((reverse_data(*(modbus_received_data + 7 + u)) >> (modulo_coil_start_address)) 
                        & coil_bit_padding_with_zero[modulo_coil_start_address - 1]);
                        coil_address_index++;
                    }    
                    else if(u == *(modbus_received_data + 6))
                    {
                        modbus_response_coil_temp[coil_address_index] = 
                        (modbus_response_coil_temp[coil_address_index] 
                        & coil_bit_padding_with_zero[modulo_coil_start_address - 1]) 
                        | ((reverse_data(*(modbus_received_data + 7 + u - 1)) << (8 - modulo_coil_start_address)) 
                        & coil_bit_padding_with_one[modulo_coil_start_address - 1]);
                    }
                    else
                    {
                        modbus_response_coil_temp[coil_address_index] = 
                        (modbus_response_coil_temp[coil_address_index] & 0x00) 
                        | (reverse_data(*(modbus_received_data + 7 + u - 1)) << (8 - modulo_coil_start_address)) 
                        | (reverse_data(*(modbus_received_data + 7 + u)) >> (modulo_coil_start_address));
                        coil_address_index++;
                    }
                }    
            }
            uint8_t v;
            for(v = 0; v < 6; v++)
            {
                modbus_response_force_multiple_coils[v] = (*(modbus_received_data + v));
            }
            modbus_response_data_length = 8;
            crc16 = CRC16(& modbus_response_force_multiple_coils[0], modbus_response_data_length - 2);
            modbus_response_force_multiple_coils[modbus_response_data_length - 2] = (uint8_t)(crc16 >> 8);
            modbus_response_force_multiple_coils[modbus_response_data_length - 1] = (uint8_t)(crc16 & 0x00FF);
        }
    }    
}
void query_response_read_holding_registers(uint8_t *modbus_received_data, uint8_t number_of_bytes)
{
    crc16 = CRC16(&(*(modbus_received_data + 0)), number_of_bytes - 2);
    if(((uint16_t)(*(modbus_received_data + number_of_bytes - 2)) == (crc16 >> 8)) && ((uint16_t)(*(modbus_received_data + number_of_bytes - 1) == (crc16 & 0x00FF))))
    {
        if((*(modbus_received_data + 0)) == MODBUS_SLAVE_ADDRESS)
        {
            uint16_t holding_registers_count;
            holding_registers_count = ((uint16_t)(*(modbus_received_data + 4)) << 8) | ((uint16_t)(*(modbus_received_data + 5)) & 0xFF);
            uint16_t holding_registers_start_address;
            holding_registers_start_address = (((uint16_t)(*(modbus_received_data + 2)) << 8) | ((uint16_t)(*(modbus_received_data + 3)) & 0xFF));
            if(holding_registers_start_address >= NUMBER_OF_HOLDING_REGISTERS)
            {
                modbus_response_data_length = 5;
                modbus_response_read_holding_registers[0] = *(modbus_received_data + 0);
                modbus_response_read_holding_registers[1] = *(modbus_received_data + 1) + 128;
                modbus_response_read_holding_registers[2] = 0x02;
                crc16 = CRC16(&modbus_response_read_coil[0], 3); 
                modbus_response_read_holding_registers[3] = (uint8_t)(crc16 >> 8);
                modbus_response_read_holding_registers[4] = (uint8_t)(crc16 & 0x00FF);
            }
            else if((holding_registers_start_address + holding_registers_count) >= (NUMBER_OF_HOLDING_REGISTERS + 1))
            {
                modbus_response_data_length = 5;
                modbus_response_read_holding_registers[0] = *(modbus_received_data + 0);
                modbus_response_read_holding_registers[1] = *(modbus_received_data + 1) + 128;
                modbus_response_read_holding_registers[2] = 0x03;
                crc16 = CRC16(&modbus_response_read_coil[0], 3); 
                modbus_response_read_holding_registers[3] = (uint8_t)(crc16 >> 8);
                modbus_response_read_holding_registers[4] = (uint8_t)(crc16 & 0x00FF);
            }
            else
            {
                //STATUS_LED_TOGGLE;
                modbus_response_read_holding_registers[0] = *(modbus_received_data + 0);
                modbus_response_read_holding_registers[1] = *(modbus_received_data + 1);
                modbus_response_read_holding_registers[2] = holding_registers_count * 2; 
                uint8_t u = 0;
                for(u = 0; u < (holding_registers_count * 2); u++)
                {
                    if(u % 2 == 0)
                    {
                        modbus_response_read_holding_registers[u + 3] = (uint8_t)((modbus_holding_registers_value[holding_registers_start_address + (uint8_t)(u/2)] & 0xFF00) >> 8);
                    }
                    if(u % 2 != 0)
                    {
                        modbus_response_read_holding_registers[u + 3] = (uint8_t)((modbus_holding_registers_value[holding_registers_start_address + (uint8_t)(u/2)] & 0x00FF));
                    }                
               }
               modbus_response_data_length = 5 + (holding_registers_count * 2);
               crc16 = CRC16(&modbus_response_read_holding_registers[0], modbus_response_data_length - 2);
               modbus_response_read_holding_registers[modbus_response_data_length - 2] = (uint8_t)(crc16 >> 8);
               modbus_response_read_holding_registers[modbus_response_data_length - 1] = (uint8_t)(crc16 & 0x00FF);
            } 
        }       
    }
}
void query_response_read_input_registers(uint8_t *modbus_received_data, uint8_t number_of_bytes)
{
    crc16 = CRC16(&(*(modbus_received_data + 0)), number_of_bytes - 2);
    if(((uint16_t)(*(modbus_received_data + number_of_bytes - 2)) == (crc16 >> 8)) && ((uint16_t)(*(modbus_received_data + number_of_bytes - 1) == (crc16 & 0x00FF))))
    {
        if((*(modbus_received_data + 0)) == MODBUS_SLAVE_ADDRESS)
        {
            uint16_t input_registers_count;
            input_registers_count = ((uint16_t)(*(modbus_received_data + 4)) << 8) | ((uint16_t)(*(modbus_received_data + 5)) & 0xFF);
            uint16_t input_registers_start_address;
            input_registers_start_address = (((uint16_t)(*(modbus_received_data + 2)) << 8) | ((uint16_t)(*(modbus_received_data + 3)) & 0xFF));
            if(input_registers_start_address >= NUMBER_OF_INPUT_REGISTERS)
            {
                modbus_response_data_length = 5;
                modbus_response_read_input_registers[0] = *(modbus_received_data + 0);
                modbus_response_read_input_registers[1] = *(modbus_received_data + 1) + 128;
                modbus_response_read_input_registers[2] = 0x02;
                crc16 = CRC16(&modbus_response_read_coil[0], 3); 
                modbus_response_read_input_registers[3] = (uint8_t)(crc16 >> 8);
                modbus_response_read_input_registers[4] = (uint8_t)(crc16 & 0x00FF);
            }
            else if((input_registers_start_address + input_registers_count) >= (NUMBER_OF_INPUT_REGISTERS + 1))
            {
                modbus_response_data_length = 5;
                modbus_response_read_input_registers[0] = *(modbus_received_data + 0);
                modbus_response_read_input_registers[1] = *(modbus_received_data + 1) + 128;
                modbus_response_read_input_registers[2] = 0x03;
                crc16 = CRC16(&modbus_response_read_coil[0], 3); 
                modbus_response_read_input_registers[3] = (uint8_t)(crc16 >> 8);
                modbus_response_read_input_registers[4] = (uint8_t)(crc16 & 0x00FF);
            }
            else
            {
                //STATUS_LED_TOGGLE;
                modbus_response_read_input_registers[0] = *(modbus_received_data + 0);
                modbus_response_read_input_registers[1] = *(modbus_received_data + 1);
                modbus_response_read_input_registers[2] = input_registers_count * 2; 
                uint8_t u = 0;
                for(u = 0; u < (input_registers_count * 2); u++)
                {
                    if(u % 2 == 0)
                    {
                        modbus_response_read_input_registers[u + 3] = (uint8_t)((modbus_input_registers_value[input_registers_start_address + (uint8_t)(u/2)] & 0xFF00) >> 8);
                    }
                    if(u % 2 != 0)
                    {
                        modbus_response_read_input_registers[u + 3] = (uint8_t)((modbus_input_registers_value[input_registers_start_address + (uint8_t)(u/2)] & 0x00FF));
                    }                
               }
               modbus_response_data_length = 5 + (input_registers_count * 2);
               crc16 = CRC16(&modbus_response_read_input_registers[0], modbus_response_data_length - 2);
               modbus_response_read_input_registers[modbus_response_data_length - 2] = (uint8_t)(crc16 >> 8);
               modbus_response_read_input_registers[modbus_response_data_length - 1] = (uint8_t)(crc16 & 0x00FF);
            }
        }
    }
}
void query_response_preset_single_register(uint8_t *modbus_received_data, uint8_t number_of_bytes)
{
    crc16 = CRC16(&(*(modbus_received_data + 0)), number_of_bytes - 2);
    if(((uint16_t)(*(modbus_received_data + number_of_bytes - 2)) == (crc16 >> 8)) && ((uint16_t)(*(modbus_received_data + number_of_bytes - 1) == (crc16 & 0x00FF))))
    {
        if((*modbus_received_data + 0) == MODBUS_SLAVE_ADDRESS)
        {
            uint16_t holding_register_address = ((uint16_t)(*(modbus_received_data + 2)) << 8) | (uint16_t)(*(modbus_received_data + 3));
            if(holding_register_address >= NUMBER_OF_HOLDING_REGISTERS)
            {
                modbus_response_data_length = 5;
                modbus_response_preset_single_register[0] = *(modbus_received_data + 0);
                modbus_response_preset_single_register[1] = *(modbus_received_data + 1) + 128;
                modbus_response_preset_single_register[2] = 0x02;
                crc16 = CRC16(&modbus_response_read_coil[0], 3); 
                modbus_response_preset_single_register[3] = (uint8_t)(crc16 >> 8);
                modbus_response_preset_single_register[4] = (uint8_t)(crc16 & 0x00FF);
            } 
            else
            {
                //STATUS_LED_TOGGLE;
                uint16_t preset_holding_register_value = ((uint16_t)(*(modbus_received_data + 4)) << 8) | (uint16_t)(*(modbus_received_data + 5));
                modbus_response_holding_registers_temp[holding_register_address] = preset_holding_register_value;
                modbus_response_preset_single_register[0] = *(modbus_received_data + 0);
                modbus_response_preset_single_register[1] = *(modbus_received_data + 1);
                modbus_response_preset_single_register[2] = *(modbus_received_data + 2);
                modbus_response_preset_single_register[3] = *(modbus_received_data + 3);
                modbus_response_preset_single_register[4] = *(modbus_received_data + 4);
                modbus_response_preset_single_register[5] = *(modbus_received_data + 5);
                modbus_response_data_length = 8;
                crc16 = CRC16(& modbus_response_preset_single_register[0], modbus_response_data_length - 2);
                modbus_response_preset_single_register[modbus_response_data_length - 2] = (uint8_t)(crc16 >> 8);
                modbus_response_preset_single_register[modbus_response_data_length - 1] = (uint8_t)(crc16 & 0x00FF);
            }
        }
    }   
}
void query_response_preset_multiple_registers(uint8_t *modbus_received_data, uint8_t number_of_bytes)
{
    crc16 = CRC16(&(*(modbus_received_data + 0)), number_of_bytes - 2);
    if(((uint16_t)(*(modbus_received_data + number_of_bytes - 2)) == (crc16 >> 8)) && ((uint16_t)(*(modbus_received_data + number_of_bytes - 1) == (crc16 & 0x00FF))))
    {
        if((*modbus_received_data + 0) == MODBUS_SLAVE_ADDRESS)
        {
            uint16_t holding_register_start_address = ((uint16_t)(*(modbus_received_data + 2)) << 8) | (uint16_t)(*(modbus_received_data + 3));
            uint16_t holding_register_count = ((uint16_t)(*(modbus_received_data + 4)) << 8) | (uint16_t)(*(modbus_received_data + 5));
            if(holding_register_start_address >= NUMBER_OF_HOLDING_REGISTERS)
            {
                modbus_response_data_length = 5;
                modbus_response_preset_multiple_registers[0] = (*modbus_received_data + 0);
                modbus_response_preset_multiple_registers[1] = *(modbus_received_data + 1) + 128;
                modbus_response_preset_multiple_registers[2] = 0x02;
                crc16 = CRC16(&modbus_response_read_coil[0], 3); 
                modbus_response_preset_multiple_registers[3] = (uint8_t)(crc16 >> 8);
                modbus_response_preset_multiple_registers[4] = (uint8_t)(crc16 & 0x00FF);
            }   
            else if((holding_register_start_address + holding_register_count) >= (NUMBER_OF_HOLDING_REGISTERS + 1))
            {
                modbus_response_data_length = 5;
                modbus_response_preset_multiple_registers[0] = *(modbus_received_data + 0);
                modbus_response_preset_multiple_registers[1] = *(modbus_received_data + 1) + 128;
                modbus_response_preset_multiple_registers[2] = 0x03;
                crc16 = CRC16(&modbus_response_read_coil[0], 3); 
                modbus_response_preset_multiple_registers[3] = (uint8_t)(crc16 >> 8);
                modbus_response_preset_multiple_registers[4] = (uint8_t)(crc16 & 0x00FF);
            }
            else
            {
                //STATUS_LED_TOGGLE;
                uint8_t i;
                for(i = 0; i < (holding_register_count * 2); i++)
                {
                    if(i % 2 != 0)
                    {
                        modbus_response_holding_registers_temp[holding_register_start_address + (uint8_t)(i / 2)] = ((uint16_t)*(modbus_received_data + i + 6) << 8) | ((uint16_t)*(modbus_received_data + i + 7) & 0x00FF);
                    }
                }
                modbus_response_preset_multiple_registers[0] = *(modbus_received_data + 0);
                modbus_response_preset_multiple_registers[1] = *(modbus_received_data + 1);
                modbus_response_preset_multiple_registers[2] = *(modbus_received_data + 2);
                modbus_response_preset_multiple_registers[3] = *(modbus_received_data + 3);
                modbus_response_preset_multiple_registers[4] = *(modbus_received_data + 4);
                modbus_response_preset_multiple_registers[5] = *(modbus_received_data + 5);
                modbus_response_data_length = 8;
                crc16 = CRC16(& modbus_response_preset_multiple_registers[0], modbus_response_data_length - 2);
                modbus_response_preset_multiple_registers[modbus_response_data_length - 2] = (uint8_t)(crc16 >> 8);
                modbus_response_preset_multiple_registers[modbus_response_data_length - 1] = (uint8_t)(crc16 & 0x00FF);
            }    
        }
    }
}
/* [] END OF FILE */
