/*******************************************************************************
* File Name: Modbus_RTU_Over_UARTINT.c
* Version 2.50
*
* Description:
*  This file provides all Interrupt Service functionality of the UART component
*
********************************************************************************
* Copyright 2008-2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "Modbus_RTU_Over_UART.h"
#include "cyapicallbacks.h"


/***************************************
* Custom Declarations
***************************************/
/* `#START CUSTOM_DECLARATIONS` Place your declaration here */

/* `#END` */

#if (Modbus_RTU_Over_UART_RX_INTERRUPT_ENABLED && (Modbus_RTU_Over_UART_RX_ENABLED || Modbus_RTU_Over_UART_HD_ENABLED))
    /*******************************************************************************
    * Function Name: Modbus_RTU_Over_UART_RXISR
    ********************************************************************************
    *
    * Summary:
    *  Interrupt Service Routine for RX portion of the UART
    *
    * Parameters:
    *  None.
    *
    * Return:
    *  None.
    *
    * Global Variables:
    *  Modbus_RTU_Over_UART_rxBuffer - RAM buffer pointer for save received data.
    *  Modbus_RTU_Over_UART_rxBufferWrite - cyclic index for write to rxBuffer,
    *     increments after each byte saved to buffer.
    *  Modbus_RTU_Over_UART_rxBufferRead - cyclic index for read from rxBuffer,
    *     checked to detect overflow condition.
    *  Modbus_RTU_Over_UART_rxBufferOverflow - software overflow flag. Set to one
    *     when Modbus_RTU_Over_UART_rxBufferWrite index overtakes
    *     Modbus_RTU_Over_UART_rxBufferRead index.
    *  Modbus_RTU_Over_UART_rxBufferLoopDetect - additional variable to detect overflow.
    *     Set to one when Modbus_RTU_Over_UART_rxBufferWrite is equal to
    *    Modbus_RTU_Over_UART_rxBufferRead
    *  Modbus_RTU_Over_UART_rxAddressMode - this variable contains the Address mode,
    *     selected in customizer or set by UART_SetRxAddressMode() API.
    *  Modbus_RTU_Over_UART_rxAddressDetected - set to 1 when correct address received,
    *     and analysed to store following addressed data bytes to the buffer.
    *     When not correct address received, set to 0 to skip following data bytes.
    *
    *******************************************************************************/
    CY_ISR(Modbus_RTU_Over_UART_RXISR)
    {
        uint8 readData;
        uint8 readStatus;
        uint8 increment_pointer = 0u;

    #if(CY_PSOC3)
        uint8 int_en;
    #endif /* (CY_PSOC3) */

    #ifdef Modbus_RTU_Over_UART_RXISR_ENTRY_CALLBACK
        Modbus_RTU_Over_UART_RXISR_EntryCallback();
    #endif /* Modbus_RTU_Over_UART_RXISR_ENTRY_CALLBACK */

        /* User code required at start of ISR */
        /* `#START Modbus_RTU_Over_UART_RXISR_START` */

        /* `#END` */

    #if(CY_PSOC3)   /* Make sure nested interrupt is enabled */
        int_en = EA;
        CyGlobalIntEnable;
    #endif /* (CY_PSOC3) */

        do
        {
            /* Read receiver status register */
            readStatus = Modbus_RTU_Over_UART_RXSTATUS_REG;
            /* Copy the same status to readData variable for backward compatibility support 
            *  of the user code in Modbus_RTU_Over_UART_RXISR_ERROR` section. 
            */
            readData = readStatus;

            if((readStatus & (Modbus_RTU_Over_UART_RX_STS_BREAK | 
                            Modbus_RTU_Over_UART_RX_STS_PAR_ERROR |
                            Modbus_RTU_Over_UART_RX_STS_STOP_ERROR | 
                            Modbus_RTU_Over_UART_RX_STS_OVERRUN)) != 0u)
            {
                /* ERROR handling. */
                Modbus_RTU_Over_UART_errorStatus |= readStatus & ( Modbus_RTU_Over_UART_RX_STS_BREAK | 
                                                            Modbus_RTU_Over_UART_RX_STS_PAR_ERROR | 
                                                            Modbus_RTU_Over_UART_RX_STS_STOP_ERROR | 
                                                            Modbus_RTU_Over_UART_RX_STS_OVERRUN);
                /* `#START Modbus_RTU_Over_UART_RXISR_ERROR` */

                /* `#END` */
                
            #ifdef Modbus_RTU_Over_UART_RXISR_ERROR_CALLBACK
                Modbus_RTU_Over_UART_RXISR_ERROR_Callback();
            #endif /* Modbus_RTU_Over_UART_RXISR_ERROR_CALLBACK */
            }
            
            if((readStatus & Modbus_RTU_Over_UART_RX_STS_FIFO_NOTEMPTY) != 0u)
            {
                /* Read data from the RX data register */
                readData = Modbus_RTU_Over_UART_RXDATA_REG;
            #if (Modbus_RTU_Over_UART_RXHW_ADDRESS_ENABLED)
                if(Modbus_RTU_Over_UART_rxAddressMode == (uint8)Modbus_RTU_Over_UART__B_UART__AM_SW_DETECT_TO_BUFFER)
                {
                    if((readStatus & Modbus_RTU_Over_UART_RX_STS_MRKSPC) != 0u)
                    {
                        if ((readStatus & Modbus_RTU_Over_UART_RX_STS_ADDR_MATCH) != 0u)
                        {
                            Modbus_RTU_Over_UART_rxAddressDetected = 1u;
                        }
                        else
                        {
                            Modbus_RTU_Over_UART_rxAddressDetected = 0u;
                        }
                    }
                    if(Modbus_RTU_Over_UART_rxAddressDetected != 0u)
                    {   /* Store only addressed data */
                        Modbus_RTU_Over_UART_rxBuffer[Modbus_RTU_Over_UART_rxBufferWrite] = readData;
                        increment_pointer = 1u;
                    }
                }
                else /* Without software addressing */
                {
                    Modbus_RTU_Over_UART_rxBuffer[Modbus_RTU_Over_UART_rxBufferWrite] = readData;
                    increment_pointer = 1u;
                }
            #else  /* Without addressing */
                Modbus_RTU_Over_UART_rxBuffer[Modbus_RTU_Over_UART_rxBufferWrite] = readData;
                increment_pointer = 1u;
            #endif /* (Modbus_RTU_Over_UART_RXHW_ADDRESS_ENABLED) */

                /* Do not increment buffer pointer when skip not addressed data */
                if(increment_pointer != 0u)
                {
                    if(Modbus_RTU_Over_UART_rxBufferLoopDetect != 0u)
                    {   /* Set Software Buffer status Overflow */
                        Modbus_RTU_Over_UART_rxBufferOverflow = 1u;
                    }
                    /* Set next pointer. */
                    Modbus_RTU_Over_UART_rxBufferWrite++;

                    /* Check pointer for a loop condition */
                    if(Modbus_RTU_Over_UART_rxBufferWrite >= Modbus_RTU_Over_UART_RX_BUFFER_SIZE)
                    {
                        Modbus_RTU_Over_UART_rxBufferWrite = 0u;
                    }

                    /* Detect pre-overload condition and set flag */
                    if(Modbus_RTU_Over_UART_rxBufferWrite == Modbus_RTU_Over_UART_rxBufferRead)
                    {
                        Modbus_RTU_Over_UART_rxBufferLoopDetect = 1u;
                        /* When Hardware Flow Control selected */
                        #if (Modbus_RTU_Over_UART_FLOW_CONTROL != 0u)
                            /* Disable RX interrupt mask, it is enabled when user read data from the buffer using APIs */
                            Modbus_RTU_Over_UART_RXSTATUS_MASK_REG  &= (uint8)~Modbus_RTU_Over_UART_RX_STS_FIFO_NOTEMPTY;
                            CyIntClearPending(Modbus_RTU_Over_UART_RX_VECT_NUM);
                            break; /* Break the reading of the FIFO loop, leave the data there for generating RTS signal */
                        #endif /* (Modbus_RTU_Over_UART_FLOW_CONTROL != 0u) */
                    }
                }
            }
        }while((readStatus & Modbus_RTU_Over_UART_RX_STS_FIFO_NOTEMPTY) != 0u);

        /* User code required at end of ISR (Optional) */
        /* `#START Modbus_RTU_Over_UART_RXISR_END` */

        /* `#END` */

    #ifdef Modbus_RTU_Over_UART_RXISR_EXIT_CALLBACK
        Modbus_RTU_Over_UART_RXISR_ExitCallback();
    #endif /* Modbus_RTU_Over_UART_RXISR_EXIT_CALLBACK */

    #if(CY_PSOC3)
        EA = int_en;
    #endif /* (CY_PSOC3) */
    }
    
#endif /* (Modbus_RTU_Over_UART_RX_INTERRUPT_ENABLED && (Modbus_RTU_Over_UART_RX_ENABLED || Modbus_RTU_Over_UART_HD_ENABLED)) */


#if (Modbus_RTU_Over_UART_TX_INTERRUPT_ENABLED && Modbus_RTU_Over_UART_TX_ENABLED)
    /*******************************************************************************
    * Function Name: Modbus_RTU_Over_UART_TXISR
    ********************************************************************************
    *
    * Summary:
    * Interrupt Service Routine for the TX portion of the UART
    *
    * Parameters:
    *  None.
    *
    * Return:
    *  None.
    *
    * Global Variables:
    *  Modbus_RTU_Over_UART_txBuffer - RAM buffer pointer for transmit data from.
    *  Modbus_RTU_Over_UART_txBufferRead - cyclic index for read and transmit data
    *     from txBuffer, increments after each transmitted byte.
    *  Modbus_RTU_Over_UART_rxBufferWrite - cyclic index for write to txBuffer,
    *     checked to detect available for transmission bytes.
    *
    *******************************************************************************/
    CY_ISR(Modbus_RTU_Over_UART_TXISR)
    {
    #if(CY_PSOC3)
        uint8 int_en;
    #endif /* (CY_PSOC3) */

    #ifdef Modbus_RTU_Over_UART_TXISR_ENTRY_CALLBACK
        Modbus_RTU_Over_UART_TXISR_EntryCallback();
    #endif /* Modbus_RTU_Over_UART_TXISR_ENTRY_CALLBACK */

        /* User code required at start of ISR */
        /* `#START Modbus_RTU_Over_UART_TXISR_START` */

        /* `#END` */

    #if(CY_PSOC3)   /* Make sure nested interrupt is enabled */
        int_en = EA;
        CyGlobalIntEnable;
    #endif /* (CY_PSOC3) */

        while((Modbus_RTU_Over_UART_txBufferRead != Modbus_RTU_Over_UART_txBufferWrite) &&
             ((Modbus_RTU_Over_UART_TXSTATUS_REG & Modbus_RTU_Over_UART_TX_STS_FIFO_FULL) == 0u))
        {
            /* Check pointer wrap around */
            if(Modbus_RTU_Over_UART_txBufferRead >= Modbus_RTU_Over_UART_TX_BUFFER_SIZE)
            {
                Modbus_RTU_Over_UART_txBufferRead = 0u;
            }

            Modbus_RTU_Over_UART_TXDATA_REG = Modbus_RTU_Over_UART_txBuffer[Modbus_RTU_Over_UART_txBufferRead];

            /* Set next pointer */
            Modbus_RTU_Over_UART_txBufferRead++;
        }

        /* User code required at end of ISR (Optional) */
        /* `#START Modbus_RTU_Over_UART_TXISR_END` */

        /* `#END` */

    #ifdef Modbus_RTU_Over_UART_TXISR_EXIT_CALLBACK
        Modbus_RTU_Over_UART_TXISR_ExitCallback();
    #endif /* Modbus_RTU_Over_UART_TXISR_EXIT_CALLBACK */

    #if(CY_PSOC3)
        EA = int_en;
    #endif /* (CY_PSOC3) */
   }
#endif /* (Modbus_RTU_Over_UART_TX_INTERRUPT_ENABLED && Modbus_RTU_Over_UART_TX_ENABLED) */


/* [] END OF FILE */
