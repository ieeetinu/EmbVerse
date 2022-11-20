/* ========================================
 * File : modbus_rtu.h
 * Author : EmbVerse (www.embverse.com)
 * Date : August 25, 2021
 * ========================================
*/
#ifndef __MODBUS_RTU_H__
#define __MODBUS_RTU_H__

//*****************************************************************************
//
// If building with a C++ compiler, make all of the definitions in this header
// have a C binding.
//
//*****************************************************************************
#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#define MODBUS_SLAVE_ADDRESS 0x01
#define NUMBER_OF_COILS 64
#define NUMBER_OF_DISCRETE_INPUTS 64
#define NUMBER_OF_INPUT_REGISTERS 64
#define NUMBER_OF_HOLDING_REGISTERS 64
#define COIL_BYTES NUMBER_OF_COILS / 8
#define DISCRETE_INPUT_BYTES NUMBER_OF_DISCRETE_INPUTS / 8
extern uint8_t reverse_data(uint8_t number);
extern uint8_t modbus_response_data_length;
extern uint16_t modbus_holding_registers_value[NUMBER_OF_HOLDING_REGISTERS];
extern uint8_t modbus_holding_registers_value_H[NUMBER_OF_HOLDING_REGISTERS];
extern uint8_t modbus_holding_registers_value_L[NUMBER_OF_HOLDING_REGISTERS];
extern uint16_t modbus_input_registers_value[NUMBER_OF_INPUT_REGISTERS];
extern uint8_t modbus_input_registers_value_H[NUMBER_OF_INPUT_REGISTERS];
extern uint8_t modbus_input_registers_value_L[NUMBER_OF_INPUT_REGISTERS];
extern uint8_t modbus_discrete_input_value[DISCRETE_INPUT_BYTES + 1];
extern uint8_t modbus_coil_value[COIL_BYTES + 1];
extern uint8_t modbus_response_read_coil[COIL_BYTES + 5];
extern uint8_t modbus_response_read_discrete_input[DISCRETE_INPUT_BYTES + 5];
extern uint8_t modbus_response_read_holding_registers[(2*NUMBER_OF_HOLDING_REGISTERS) + 5];
extern uint8_t modbus_response_preset_single_register[8];
extern uint8_t modbus_response_preset_multiple_registers[8];
extern uint8_t modbus_response_read_input_registers[(2*NUMBER_OF_INPUT_REGISTERS) + 5];
extern uint8_t modbus_response_force_single_coil[8];
extern uint8_t modbus_response_force_multiple_coils[8];
extern uint8_t modbus_response_coil_temp[COIL_BYTES + 1];
extern uint8_t modbus_response_discrete_input_temp[DISCRETE_INPUT_BYTES + 1];
extern uint16_t modbus_response_holding_registers_temp[NUMBER_OF_HOLDING_REGISTERS];
extern uint16_t modbus_response_input_registers_temp[NUMBER_OF_INPUT_REGISTERS];
extern void query_response_read_coil(uint8_t *modbus_received_data, uint8_t number_of_bytes);
extern void query_response_read_input_registers(uint8_t *modbus_received_data, uint8_t number_of_bytes);
extern void query_response_read_discrete_input(uint8_t *modbus_received_data, uint8_t number_of_bytes);
extern void query_response_read_holding_registers(uint8_t *modbus_received_data, uint8_t number_of_bytes);
extern void query_response_force_single_coil(uint8_t *modbus_received_data, uint8_t number_of_bytes);
extern void query_response_preset_single_register(uint8_t *modbus_received_data, uint8_t number_of_bytes);
extern void query_response_preset_multiple_registers(uint8_t *modbus_received_data, uint8_t number_of_bytes);
extern void query_response_force_multiple_coils(uint8_t *modbus_received_data, uint8_t number_of_bytes);
extern unsigned short CRC16(unsigned char *puchMsg, unsigned short usDataLen);
extern unsigned short crc16;
#ifdef __cplusplus
}
#endif

#endif // __MODBUS_RTU_H__

/* [] END OF FILE */
