/*******************************************************************************
* File Name: Modbus_RTU_Over_UART_BOOT.c
* Version 2.50
*
* Description:
*  This file provides the source code of bootloader communication APIs for the
*  UART component.
*
* Note:
*
********************************************************************************
* Copyright 2008-2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "Modbus_RTU_Over_UART.h"

#if defined(CYDEV_BOOTLOADER_IO_COMP) && (0u != ((CYDEV_BOOTLOADER_IO_COMP == CyBtldr_Modbus_RTU_Over_UART) || \
                                          (CYDEV_BOOTLOADER_IO_COMP == CyBtldr_Custom_Interface)))


/*******************************************************************************
* Function Name: Modbus_RTU_Over_UART_CyBtldrCommStart
********************************************************************************
*
* Summary:
*  Starts the UART communication component.
*
* Parameters:
*  None
*
* Return:
*  None
*
* Side Effects:
*  This component automatically enables global interrupt.
*
*******************************************************************************/
void Modbus_RTU_Over_UART_CyBtldrCommStart(void) CYSMALL 
{
    /* Start UART component and clear the Tx,Rx buffers */
    Modbus_RTU_Over_UART_Start();
    Modbus_RTU_Over_UART_ClearRxBuffer();
    Modbus_RTU_Over_UART_ClearTxBuffer();
}


/*******************************************************************************
* Function Name: Modbus_RTU_Over_UART_CyBtldrCommStop
********************************************************************************
*
* Summary:
*  Disables the communication component and disables the interrupt.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void Modbus_RTU_Over_UART_CyBtldrCommStop(void) CYSMALL 
{
    /* Stop UART component */
    Modbus_RTU_Over_UART_Stop();
}


/*******************************************************************************
* Function Name: Modbus_RTU_Over_UART_CyBtldrCommReset
********************************************************************************
*
* Summary:
*  Resets the receive and transmit communication Buffers.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void Modbus_RTU_Over_UART_CyBtldrCommReset(void) CYSMALL 
{
    /* Clear RX and TX buffers */
    Modbus_RTU_Over_UART_ClearRxBuffer();
    Modbus_RTU_Over_UART_ClearTxBuffer();
}


/*******************************************************************************
* Function Name: Modbus_RTU_Over_UART_CyBtldrCommWrite
********************************************************************************
*
* Summary:
*  Allows the caller to write data to the boot loader host. This function uses
* a blocking write function for writing data using UART communication component.
*
* Parameters:
*  pData:    A pointer to the block of data to send to the device
*  size:     The number of bytes to write.
*  count:    Pointer to an unsigned short variable to write the number of
*             bytes actually written.
*  timeOut:  Number of units to wait before returning because of a timeout.
*
* Return:
*   cystatus: This function will return CYRET_SUCCESS if data is sent
*             successfully.
*
* Side Effects:
*  This function should be called after command was received .
*
*******************************************************************************/
cystatus Modbus_RTU_Over_UART_CyBtldrCommWrite(const uint8 pData[], uint16 size, uint16 * count, uint8 timeOut) CYSMALL
         
{
    uint16 bufIndex = 0u;

    if(0u != timeOut)
    {
        /* Suppress compiler warning */
    }

    /* Clear receive buffers */
    Modbus_RTU_Over_UART_ClearRxBuffer();

    /* Write TX data using blocking function */
    while(bufIndex < size)
    {
        Modbus_RTU_Over_UART_PutChar(pData[bufIndex]);
        bufIndex++;
    }

    /* Return success code */
    *count = size;

    return (CYRET_SUCCESS);
}


/*******************************************************************************
* Function Name: Modbus_RTU_Over_UART_CyBtldrCommRead
********************************************************************************
*
* Summary:
*  Receives the command.
*
* Parameters:
*  pData:    A pointer to the area to store the block of data received
*             from the device.
*  size:     Maximum size of the read buffer
*  count:    Pointer to an unsigned short variable to write the number
*             of bytes actually read.
*  timeOut:  Number of units to wait before returning because of a timeOut.
*            Time out is measured in 10s of ms.
*
* Return:
*  cystatus: This function will return CYRET_SUCCESS if at least one byte is
*            received successfully within the time out interval. If no data is
*            received  this function will return CYRET_EMPTY.
*
*  BYTE2BYTE_TIME_OUT is used for detecting time out marking end of block data
*  from host. This has to be set to a value which is greater than the expected
*  maximum delay between two bytes during a block/packet transmission from the
*  host. You have to account for the delay in hardware converters while
*  calculating this value, if you are using any USB-UART bridges.
*******************************************************************************/
cystatus Modbus_RTU_Over_UART_CyBtldrCommRead(uint8 pData[], uint16 size, uint16 * count, uint8 timeOut) CYSMALL
         
{
    uint16 iCntr;
    uint16 dataIndexCntr;
    uint16 tempCount;
    uint16 oldDataCount;

    cystatus status = CYRET_EMPTY;

    /* Check whether data is received within the time out period.
    *  Time out period is in units of 10ms.
    *  If at least one byte is received within the time out interval, wait for more data */
    for (iCntr = 0u; iCntr < ((uint16)10u * timeOut); iCntr++)
    {
        /* If at least one byte is received within the timeout interval
        *  enter the next loop waiting for more data reception
        */
        if(0u != Modbus_RTU_Over_UART_GetRxBufferSize())
        {
            /* Wait for more data until 25ms byte to byte time out interval.
            * If no data is received during the last 25 ms(BYTE2BYTE_TIME_OUT)
            * then it is considered as end of transmitted data block(packet)
            * from the host and the program execution will break from the
            * data awaiting loop with status=CYRET_SUCCESS
            */
            do
            {
                oldDataCount = Modbus_RTU_Over_UART_GetRxBufferSize();
                CyDelay(Modbus_RTU_Over_UART_BYTE2BYTE_TIME_OUT);
            }
            while(Modbus_RTU_Over_UART_GetRxBufferSize() > oldDataCount);

            status = CYRET_SUCCESS;
            break;
        }
        /* If the data is not received, give a delay of 
        *  Modbus_RTU_Over_UART_BL_CHK_DELAY_MS and check again until the timeOut specified.
        */
        else
        {
            CyDelay(Modbus_RTU_Over_UART_BL_CHK_DELAY_MS);
        }
    }

    /* Initialize the data read indexes and Count value */
    *count = 0u;
    dataIndexCntr = 0u;

    /* If GetRxBufferSize()>0 , move the received data to the pData buffer */
    while(Modbus_RTU_Over_UART_GetRxBufferSize() > 0u)
    {
        tempCount = Modbus_RTU_Over_UART_GetRxBufferSize();
        *count  =(*count) + tempCount;

        /* Check if buffer overflow will occur before moving the data */
        if(*count < size)
        {
            for (iCntr = 0u; iCntr < tempCount; iCntr++)
            {
                /* Read the data and move it to the pData buffer */
                pData[dataIndexCntr] = Modbus_RTU_Over_UART_ReadRxData();
                dataIndexCntr++;
            }

            /* Check if the last received byte is end of packet defined by bootloader
            *  If not wait for additional Modbus_RTU_Over_UART_WAIT_EOP_DELAY ms.
            */
            if(pData[dataIndexCntr - 1u] != Modbus_RTU_Over_UART_PACKET_EOP)
            {
                CyDelay(Modbus_RTU_Over_UART_WAIT_EOP_DELAY);
            }
        }
        /* If there is no space to move data, break from the loop */
        else
        {
            *count = (*count) - tempCount;
            break;
        }
    }

    return (status);
}

#endif /* end CYDEV_BOOTLOADER_IO_COMP */


/* [] END OF FILE */
