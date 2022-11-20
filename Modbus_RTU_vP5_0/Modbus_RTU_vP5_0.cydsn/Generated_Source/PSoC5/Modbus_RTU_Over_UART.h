/*******************************************************************************
* File Name: Modbus_RTU_Over_UART.h
* Version 2.50
*
* Description:
*  Contains the function prototypes and constants available to the UART
*  user module.
*
* Note:
*
********************************************************************************
* Copyright 2008-2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/


#if !defined(CY_UART_Modbus_RTU_Over_UART_H)
#define CY_UART_Modbus_RTU_Over_UART_H

#include "cyfitter.h"
#include "cytypes.h"
#include "CyLib.h" /* For CyEnterCriticalSection() and CyExitCriticalSection() functions */


/***************************************
* Conditional Compilation Parameters
***************************************/

#define Modbus_RTU_Over_UART_RX_ENABLED                     (1u)
#define Modbus_RTU_Over_UART_TX_ENABLED                     (1u)
#define Modbus_RTU_Over_UART_HD_ENABLED                     (0u)
#define Modbus_RTU_Over_UART_RX_INTERRUPT_ENABLED           (0u)
#define Modbus_RTU_Over_UART_TX_INTERRUPT_ENABLED           (0u)
#define Modbus_RTU_Over_UART_INTERNAL_CLOCK_USED            (1u)
#define Modbus_RTU_Over_UART_RXHW_ADDRESS_ENABLED           (0u)
#define Modbus_RTU_Over_UART_OVER_SAMPLE_COUNT              (8u)
#define Modbus_RTU_Over_UART_PARITY_TYPE                    (0u)
#define Modbus_RTU_Over_UART_PARITY_TYPE_SW                 (0u)
#define Modbus_RTU_Over_UART_BREAK_DETECT                   (0u)
#define Modbus_RTU_Over_UART_BREAK_BITS_TX                  (13u)
#define Modbus_RTU_Over_UART_BREAK_BITS_RX                  (13u)
#define Modbus_RTU_Over_UART_TXCLKGEN_DP                    (1u)
#define Modbus_RTU_Over_UART_USE23POLLING                   (1u)
#define Modbus_RTU_Over_UART_FLOW_CONTROL                   (0u)
#define Modbus_RTU_Over_UART_CLK_FREQ                       (0u)
#define Modbus_RTU_Over_UART_TX_BUFFER_SIZE                 (4u)
#define Modbus_RTU_Over_UART_RX_BUFFER_SIZE                 (4u)

/* Check to see if required defines such as CY_PSOC5LP are available */
/* They are defined starting with cy_boot v3.0 */
#if !defined (CY_PSOC5LP)
    #error Component UART_v2_50 requires cy_boot v3.0 or later
#endif /* (CY_PSOC5LP) */

#if defined(Modbus_RTU_Over_UART_BUART_sCR_SyncCtl_CtrlReg__CONTROL_REG)
    #define Modbus_RTU_Over_UART_CONTROL_REG_REMOVED            (0u)
#else
    #define Modbus_RTU_Over_UART_CONTROL_REG_REMOVED            (1u)
#endif /* End Modbus_RTU_Over_UART_BUART_sCR_SyncCtl_CtrlReg__CONTROL_REG */


/***************************************
*      Data Structure Definition
***************************************/

/* Sleep Mode API Support */
typedef struct Modbus_RTU_Over_UART_backupStruct_
{
    uint8 enableState;

    #if(Modbus_RTU_Over_UART_CONTROL_REG_REMOVED == 0u)
        uint8 cr;
    #endif /* End Modbus_RTU_Over_UART_CONTROL_REG_REMOVED */

} Modbus_RTU_Over_UART_BACKUP_STRUCT;


/***************************************
*       Function Prototypes
***************************************/

void Modbus_RTU_Over_UART_Start(void) ;
void Modbus_RTU_Over_UART_Stop(void) ;
uint8 Modbus_RTU_Over_UART_ReadControlRegister(void) ;
void Modbus_RTU_Over_UART_WriteControlRegister(uint8 control) ;

void Modbus_RTU_Over_UART_Init(void) ;
void Modbus_RTU_Over_UART_Enable(void) ;
void Modbus_RTU_Over_UART_SaveConfig(void) ;
void Modbus_RTU_Over_UART_RestoreConfig(void) ;
void Modbus_RTU_Over_UART_Sleep(void) ;
void Modbus_RTU_Over_UART_Wakeup(void) ;

/* Only if RX is enabled */
#if( (Modbus_RTU_Over_UART_RX_ENABLED) || (Modbus_RTU_Over_UART_HD_ENABLED) )

    #if (Modbus_RTU_Over_UART_RX_INTERRUPT_ENABLED)
        #define Modbus_RTU_Over_UART_EnableRxInt()  CyIntEnable (Modbus_RTU_Over_UART_RX_VECT_NUM)
        #define Modbus_RTU_Over_UART_DisableRxInt() CyIntDisable(Modbus_RTU_Over_UART_RX_VECT_NUM)
        CY_ISR_PROTO(Modbus_RTU_Over_UART_RXISR);
    #endif /* Modbus_RTU_Over_UART_RX_INTERRUPT_ENABLED */

    void Modbus_RTU_Over_UART_SetRxAddressMode(uint8 addressMode)
                                                           ;
    void Modbus_RTU_Over_UART_SetRxAddress1(uint8 address) ;
    void Modbus_RTU_Over_UART_SetRxAddress2(uint8 address) ;

    void  Modbus_RTU_Over_UART_SetRxInterruptMode(uint8 intSrc) ;
    uint8 Modbus_RTU_Over_UART_ReadRxData(void) ;
    uint8 Modbus_RTU_Over_UART_ReadRxStatus(void) ;
    uint8 Modbus_RTU_Over_UART_GetChar(void) ;
    uint16 Modbus_RTU_Over_UART_GetByte(void) ;
    uint8 Modbus_RTU_Over_UART_GetRxBufferSize(void)
                                                            ;
    void Modbus_RTU_Over_UART_ClearRxBuffer(void) ;

    /* Obsolete functions, defines for backward compatible */
    #define Modbus_RTU_Over_UART_GetRxInterruptSource   Modbus_RTU_Over_UART_ReadRxStatus

#endif /* End (Modbus_RTU_Over_UART_RX_ENABLED) || (Modbus_RTU_Over_UART_HD_ENABLED) */

/* Only if TX is enabled */
#if(Modbus_RTU_Over_UART_TX_ENABLED || Modbus_RTU_Over_UART_HD_ENABLED)

    #if(Modbus_RTU_Over_UART_TX_INTERRUPT_ENABLED)
        #define Modbus_RTU_Over_UART_EnableTxInt()  CyIntEnable (Modbus_RTU_Over_UART_TX_VECT_NUM)
        #define Modbus_RTU_Over_UART_DisableTxInt() CyIntDisable(Modbus_RTU_Over_UART_TX_VECT_NUM)
        #define Modbus_RTU_Over_UART_SetPendingTxInt() CyIntSetPending(Modbus_RTU_Over_UART_TX_VECT_NUM)
        #define Modbus_RTU_Over_UART_ClearPendingTxInt() CyIntClearPending(Modbus_RTU_Over_UART_TX_VECT_NUM)
        CY_ISR_PROTO(Modbus_RTU_Over_UART_TXISR);
    #endif /* Modbus_RTU_Over_UART_TX_INTERRUPT_ENABLED */

    void Modbus_RTU_Over_UART_SetTxInterruptMode(uint8 intSrc) ;
    void Modbus_RTU_Over_UART_WriteTxData(uint8 txDataByte) ;
    uint8 Modbus_RTU_Over_UART_ReadTxStatus(void) ;
    void Modbus_RTU_Over_UART_PutChar(uint8 txDataByte) ;
    void Modbus_RTU_Over_UART_PutString(const char8 string[]) ;
    void Modbus_RTU_Over_UART_PutArray(const uint8 string[], uint8 byteCount)
                                                            ;
    void Modbus_RTU_Over_UART_PutCRLF(uint8 txDataByte) ;
    void Modbus_RTU_Over_UART_ClearTxBuffer(void) ;
    void Modbus_RTU_Over_UART_SetTxAddressMode(uint8 addressMode) ;
    void Modbus_RTU_Over_UART_SendBreak(uint8 retMode) ;
    uint8 Modbus_RTU_Over_UART_GetTxBufferSize(void)
                                                            ;
    /* Obsolete functions, defines for backward compatible */
    #define Modbus_RTU_Over_UART_PutStringConst         Modbus_RTU_Over_UART_PutString
    #define Modbus_RTU_Over_UART_PutArrayConst          Modbus_RTU_Over_UART_PutArray
    #define Modbus_RTU_Over_UART_GetTxInterruptSource   Modbus_RTU_Over_UART_ReadTxStatus

#endif /* End Modbus_RTU_Over_UART_TX_ENABLED || Modbus_RTU_Over_UART_HD_ENABLED */

#if(Modbus_RTU_Over_UART_HD_ENABLED)
    void Modbus_RTU_Over_UART_LoadRxConfig(void) ;
    void Modbus_RTU_Over_UART_LoadTxConfig(void) ;
#endif /* End Modbus_RTU_Over_UART_HD_ENABLED */


/* Communication bootloader APIs */
#if defined(CYDEV_BOOTLOADER_IO_COMP) && ((CYDEV_BOOTLOADER_IO_COMP == CyBtldr_Modbus_RTU_Over_UART) || \
                                          (CYDEV_BOOTLOADER_IO_COMP == CyBtldr_Custom_Interface))
    /* Physical layer functions */
    void    Modbus_RTU_Over_UART_CyBtldrCommStart(void) CYSMALL ;
    void    Modbus_RTU_Over_UART_CyBtldrCommStop(void) CYSMALL ;
    void    Modbus_RTU_Over_UART_CyBtldrCommReset(void) CYSMALL ;
    cystatus Modbus_RTU_Over_UART_CyBtldrCommWrite(const uint8 pData[], uint16 size, uint16 * count, uint8 timeOut) CYSMALL
             ;
    cystatus Modbus_RTU_Over_UART_CyBtldrCommRead(uint8 pData[], uint16 size, uint16 * count, uint8 timeOut) CYSMALL
             ;

    #if (CYDEV_BOOTLOADER_IO_COMP == CyBtldr_Modbus_RTU_Over_UART)
        #define CyBtldrCommStart    Modbus_RTU_Over_UART_CyBtldrCommStart
        #define CyBtldrCommStop     Modbus_RTU_Over_UART_CyBtldrCommStop
        #define CyBtldrCommReset    Modbus_RTU_Over_UART_CyBtldrCommReset
        #define CyBtldrCommWrite    Modbus_RTU_Over_UART_CyBtldrCommWrite
        #define CyBtldrCommRead     Modbus_RTU_Over_UART_CyBtldrCommRead
    #endif  /* (CYDEV_BOOTLOADER_IO_COMP == CyBtldr_Modbus_RTU_Over_UART) */

    /* Byte to Byte time out for detecting end of block data from host */
    #define Modbus_RTU_Over_UART_BYTE2BYTE_TIME_OUT (25u)
    #define Modbus_RTU_Over_UART_PACKET_EOP         (0x17u) /* End of packet defined by bootloader */
    #define Modbus_RTU_Over_UART_WAIT_EOP_DELAY     (5u)    /* Additional 5ms to wait for End of packet */
    #define Modbus_RTU_Over_UART_BL_CHK_DELAY_MS    (1u)    /* Time Out quantity equal 1mS */

#endif /* CYDEV_BOOTLOADER_IO_COMP */


/***************************************
*          API Constants
***************************************/
/* Parameters for SetTxAddressMode API*/
#define Modbus_RTU_Over_UART_SET_SPACE      (0x00u)
#define Modbus_RTU_Over_UART_SET_MARK       (0x01u)

/* Status Register definitions */
#if( (Modbus_RTU_Over_UART_TX_ENABLED) || (Modbus_RTU_Over_UART_HD_ENABLED) )
    #if(Modbus_RTU_Over_UART_TX_INTERRUPT_ENABLED)
        #define Modbus_RTU_Over_UART_TX_VECT_NUM            (uint8)Modbus_RTU_Over_UART_TXInternalInterrupt__INTC_NUMBER
        #define Modbus_RTU_Over_UART_TX_PRIOR_NUM           (uint8)Modbus_RTU_Over_UART_TXInternalInterrupt__INTC_PRIOR_NUM
    #endif /* Modbus_RTU_Over_UART_TX_INTERRUPT_ENABLED */

    #define Modbus_RTU_Over_UART_TX_STS_COMPLETE_SHIFT          (0x00u)
    #define Modbus_RTU_Over_UART_TX_STS_FIFO_EMPTY_SHIFT        (0x01u)
    #define Modbus_RTU_Over_UART_TX_STS_FIFO_NOT_FULL_SHIFT     (0x03u)
    #if(Modbus_RTU_Over_UART_TX_ENABLED)
        #define Modbus_RTU_Over_UART_TX_STS_FIFO_FULL_SHIFT     (0x02u)
    #else /* (Modbus_RTU_Over_UART_HD_ENABLED) */
        #define Modbus_RTU_Over_UART_TX_STS_FIFO_FULL_SHIFT     (0x05u)  /* Needs MD=0 */
    #endif /* (Modbus_RTU_Over_UART_TX_ENABLED) */

    #define Modbus_RTU_Over_UART_TX_STS_COMPLETE            (uint8)(0x01u << Modbus_RTU_Over_UART_TX_STS_COMPLETE_SHIFT)
    #define Modbus_RTU_Over_UART_TX_STS_FIFO_EMPTY          (uint8)(0x01u << Modbus_RTU_Over_UART_TX_STS_FIFO_EMPTY_SHIFT)
    #define Modbus_RTU_Over_UART_TX_STS_FIFO_FULL           (uint8)(0x01u << Modbus_RTU_Over_UART_TX_STS_FIFO_FULL_SHIFT)
    #define Modbus_RTU_Over_UART_TX_STS_FIFO_NOT_FULL       (uint8)(0x01u << Modbus_RTU_Over_UART_TX_STS_FIFO_NOT_FULL_SHIFT)
#endif /* End (Modbus_RTU_Over_UART_TX_ENABLED) || (Modbus_RTU_Over_UART_HD_ENABLED)*/

#if( (Modbus_RTU_Over_UART_RX_ENABLED) || (Modbus_RTU_Over_UART_HD_ENABLED) )
    #if(Modbus_RTU_Over_UART_RX_INTERRUPT_ENABLED)
        #define Modbus_RTU_Over_UART_RX_VECT_NUM            (uint8)Modbus_RTU_Over_UART_RXInternalInterrupt__INTC_NUMBER
        #define Modbus_RTU_Over_UART_RX_PRIOR_NUM           (uint8)Modbus_RTU_Over_UART_RXInternalInterrupt__INTC_PRIOR_NUM
    #endif /* Modbus_RTU_Over_UART_RX_INTERRUPT_ENABLED */
    #define Modbus_RTU_Over_UART_RX_STS_MRKSPC_SHIFT            (0x00u)
    #define Modbus_RTU_Over_UART_RX_STS_BREAK_SHIFT             (0x01u)
    #define Modbus_RTU_Over_UART_RX_STS_PAR_ERROR_SHIFT         (0x02u)
    #define Modbus_RTU_Over_UART_RX_STS_STOP_ERROR_SHIFT        (0x03u)
    #define Modbus_RTU_Over_UART_RX_STS_OVERRUN_SHIFT           (0x04u)
    #define Modbus_RTU_Over_UART_RX_STS_FIFO_NOTEMPTY_SHIFT     (0x05u)
    #define Modbus_RTU_Over_UART_RX_STS_ADDR_MATCH_SHIFT        (0x06u)
    #define Modbus_RTU_Over_UART_RX_STS_SOFT_BUFF_OVER_SHIFT    (0x07u)

    #define Modbus_RTU_Over_UART_RX_STS_MRKSPC           (uint8)(0x01u << Modbus_RTU_Over_UART_RX_STS_MRKSPC_SHIFT)
    #define Modbus_RTU_Over_UART_RX_STS_BREAK            (uint8)(0x01u << Modbus_RTU_Over_UART_RX_STS_BREAK_SHIFT)
    #define Modbus_RTU_Over_UART_RX_STS_PAR_ERROR        (uint8)(0x01u << Modbus_RTU_Over_UART_RX_STS_PAR_ERROR_SHIFT)
    #define Modbus_RTU_Over_UART_RX_STS_STOP_ERROR       (uint8)(0x01u << Modbus_RTU_Over_UART_RX_STS_STOP_ERROR_SHIFT)
    #define Modbus_RTU_Over_UART_RX_STS_OVERRUN          (uint8)(0x01u << Modbus_RTU_Over_UART_RX_STS_OVERRUN_SHIFT)
    #define Modbus_RTU_Over_UART_RX_STS_FIFO_NOTEMPTY    (uint8)(0x01u << Modbus_RTU_Over_UART_RX_STS_FIFO_NOTEMPTY_SHIFT)
    #define Modbus_RTU_Over_UART_RX_STS_ADDR_MATCH       (uint8)(0x01u << Modbus_RTU_Over_UART_RX_STS_ADDR_MATCH_SHIFT)
    #define Modbus_RTU_Over_UART_RX_STS_SOFT_BUFF_OVER   (uint8)(0x01u << Modbus_RTU_Over_UART_RX_STS_SOFT_BUFF_OVER_SHIFT)
    #define Modbus_RTU_Over_UART_RX_HW_MASK                     (0x7Fu)
#endif /* End (Modbus_RTU_Over_UART_RX_ENABLED) || (Modbus_RTU_Over_UART_HD_ENABLED) */

/* Control Register definitions */
#define Modbus_RTU_Over_UART_CTRL_HD_SEND_SHIFT                 (0x00u) /* 1 enable TX part in Half Duplex mode */
#define Modbus_RTU_Over_UART_CTRL_HD_SEND_BREAK_SHIFT           (0x01u) /* 1 send BREAK signal in Half Duplez mode */
#define Modbus_RTU_Over_UART_CTRL_MARK_SHIFT                    (0x02u) /* 1 sets mark, 0 sets space */
#define Modbus_RTU_Over_UART_CTRL_PARITY_TYPE0_SHIFT            (0x03u) /* Defines the type of parity implemented */
#define Modbus_RTU_Over_UART_CTRL_PARITY_TYPE1_SHIFT            (0x04u) /* Defines the type of parity implemented */
#define Modbus_RTU_Over_UART_CTRL_RXADDR_MODE0_SHIFT            (0x05u)
#define Modbus_RTU_Over_UART_CTRL_RXADDR_MODE1_SHIFT            (0x06u)
#define Modbus_RTU_Over_UART_CTRL_RXADDR_MODE2_SHIFT            (0x07u)

#define Modbus_RTU_Over_UART_CTRL_HD_SEND               (uint8)(0x01u << Modbus_RTU_Over_UART_CTRL_HD_SEND_SHIFT)
#define Modbus_RTU_Over_UART_CTRL_HD_SEND_BREAK         (uint8)(0x01u << Modbus_RTU_Over_UART_CTRL_HD_SEND_BREAK_SHIFT)
#define Modbus_RTU_Over_UART_CTRL_MARK                  (uint8)(0x01u << Modbus_RTU_Over_UART_CTRL_MARK_SHIFT)
#define Modbus_RTU_Over_UART_CTRL_PARITY_TYPE_MASK      (uint8)(0x03u << Modbus_RTU_Over_UART_CTRL_PARITY_TYPE0_SHIFT)
#define Modbus_RTU_Over_UART_CTRL_RXADDR_MODE_MASK      (uint8)(0x07u << Modbus_RTU_Over_UART_CTRL_RXADDR_MODE0_SHIFT)

/* StatusI Register Interrupt Enable Control Bits. As defined by the Register map for the AUX Control Register */
#define Modbus_RTU_Over_UART_INT_ENABLE                         (0x10u)

/* Bit Counter (7-bit) Control Register Bit Definitions. As defined by the Register map for the AUX Control Register */
#define Modbus_RTU_Over_UART_CNTR_ENABLE                        (0x20u)

/*   Constants for SendBreak() "retMode" parameter  */
#define Modbus_RTU_Over_UART_SEND_BREAK                         (0x00u)
#define Modbus_RTU_Over_UART_WAIT_FOR_COMPLETE_REINIT           (0x01u)
#define Modbus_RTU_Over_UART_REINIT                             (0x02u)
#define Modbus_RTU_Over_UART_SEND_WAIT_REINIT                   (0x03u)

#define Modbus_RTU_Over_UART_OVER_SAMPLE_8                      (8u)
#define Modbus_RTU_Over_UART_OVER_SAMPLE_16                     (16u)

#define Modbus_RTU_Over_UART_BIT_CENTER                         (Modbus_RTU_Over_UART_OVER_SAMPLE_COUNT - 2u)

#define Modbus_RTU_Over_UART_FIFO_LENGTH                        (4u)
#define Modbus_RTU_Over_UART_NUMBER_OF_START_BIT                (1u)
#define Modbus_RTU_Over_UART_MAX_BYTE_VALUE                     (0xFFu)

/* 8X always for count7 implementation */
#define Modbus_RTU_Over_UART_TXBITCTR_BREAKBITS8X   ((Modbus_RTU_Over_UART_BREAK_BITS_TX * Modbus_RTU_Over_UART_OVER_SAMPLE_8) - 1u)
/* 8X or 16X for DP implementation */
#define Modbus_RTU_Over_UART_TXBITCTR_BREAKBITS ((Modbus_RTU_Over_UART_BREAK_BITS_TX * Modbus_RTU_Over_UART_OVER_SAMPLE_COUNT) - 1u)

#define Modbus_RTU_Over_UART_HALF_BIT_COUNT   \
                            (((Modbus_RTU_Over_UART_OVER_SAMPLE_COUNT / 2u) + (Modbus_RTU_Over_UART_USE23POLLING * 1u)) - 2u)
#if (Modbus_RTU_Over_UART_OVER_SAMPLE_COUNT == Modbus_RTU_Over_UART_OVER_SAMPLE_8)
    #define Modbus_RTU_Over_UART_HD_TXBITCTR_INIT   (((Modbus_RTU_Over_UART_BREAK_BITS_TX + \
                            Modbus_RTU_Over_UART_NUMBER_OF_START_BIT) * Modbus_RTU_Over_UART_OVER_SAMPLE_COUNT) - 1u)

    /* This parameter is increased on the 2 in 2 out of 3 mode to sample voting in the middle */
    #define Modbus_RTU_Over_UART_RXBITCTR_INIT  ((((Modbus_RTU_Over_UART_BREAK_BITS_RX + Modbus_RTU_Over_UART_NUMBER_OF_START_BIT) \
                            * Modbus_RTU_Over_UART_OVER_SAMPLE_COUNT) + Modbus_RTU_Over_UART_HALF_BIT_COUNT) - 1u)

#else /* Modbus_RTU_Over_UART_OVER_SAMPLE_COUNT == Modbus_RTU_Over_UART_OVER_SAMPLE_16 */
    #define Modbus_RTU_Over_UART_HD_TXBITCTR_INIT   ((8u * Modbus_RTU_Over_UART_OVER_SAMPLE_COUNT) - 1u)
    /* 7bit counter need one more bit for OverSampleCount = 16 */
    #define Modbus_RTU_Over_UART_RXBITCTR_INIT      (((7u * Modbus_RTU_Over_UART_OVER_SAMPLE_COUNT) - 1u) + \
                                                      Modbus_RTU_Over_UART_HALF_BIT_COUNT)
#endif /* End Modbus_RTU_Over_UART_OVER_SAMPLE_COUNT */

#define Modbus_RTU_Over_UART_HD_RXBITCTR_INIT                   Modbus_RTU_Over_UART_RXBITCTR_INIT


/***************************************
* Global variables external identifier
***************************************/

extern uint8 Modbus_RTU_Over_UART_initVar;
#if (Modbus_RTU_Over_UART_TX_INTERRUPT_ENABLED && Modbus_RTU_Over_UART_TX_ENABLED)
    extern volatile uint8 Modbus_RTU_Over_UART_txBuffer[Modbus_RTU_Over_UART_TX_BUFFER_SIZE];
    extern volatile uint8 Modbus_RTU_Over_UART_txBufferRead;
    extern uint8 Modbus_RTU_Over_UART_txBufferWrite;
#endif /* (Modbus_RTU_Over_UART_TX_INTERRUPT_ENABLED && Modbus_RTU_Over_UART_TX_ENABLED) */
#if (Modbus_RTU_Over_UART_RX_INTERRUPT_ENABLED && (Modbus_RTU_Over_UART_RX_ENABLED || Modbus_RTU_Over_UART_HD_ENABLED))
    extern uint8 Modbus_RTU_Over_UART_errorStatus;
    extern volatile uint8 Modbus_RTU_Over_UART_rxBuffer[Modbus_RTU_Over_UART_RX_BUFFER_SIZE];
    extern volatile uint8 Modbus_RTU_Over_UART_rxBufferRead;
    extern volatile uint8 Modbus_RTU_Over_UART_rxBufferWrite;
    extern volatile uint8 Modbus_RTU_Over_UART_rxBufferLoopDetect;
    extern volatile uint8 Modbus_RTU_Over_UART_rxBufferOverflow;
    #if (Modbus_RTU_Over_UART_RXHW_ADDRESS_ENABLED)
        extern volatile uint8 Modbus_RTU_Over_UART_rxAddressMode;
        extern volatile uint8 Modbus_RTU_Over_UART_rxAddressDetected;
    #endif /* (Modbus_RTU_Over_UART_RXHW_ADDRESS_ENABLED) */
#endif /* (Modbus_RTU_Over_UART_RX_INTERRUPT_ENABLED && (Modbus_RTU_Over_UART_RX_ENABLED || Modbus_RTU_Over_UART_HD_ENABLED)) */


/***************************************
* Enumerated Types and Parameters
***************************************/

#define Modbus_RTU_Over_UART__B_UART__AM_SW_BYTE_BYTE 1
#define Modbus_RTU_Over_UART__B_UART__AM_SW_DETECT_TO_BUFFER 2
#define Modbus_RTU_Over_UART__B_UART__AM_HW_BYTE_BY_BYTE 3
#define Modbus_RTU_Over_UART__B_UART__AM_HW_DETECT_TO_BUFFER 4
#define Modbus_RTU_Over_UART__B_UART__AM_NONE 0

#define Modbus_RTU_Over_UART__B_UART__NONE_REVB 0
#define Modbus_RTU_Over_UART__B_UART__EVEN_REVB 1
#define Modbus_RTU_Over_UART__B_UART__ODD_REVB 2
#define Modbus_RTU_Over_UART__B_UART__MARK_SPACE_REVB 3



/***************************************
*    Initial Parameter Constants
***************************************/

/* UART shifts max 8 bits, Mark/Space functionality working if 9 selected */
#define Modbus_RTU_Over_UART_NUMBER_OF_DATA_BITS    ((8u > 8u) ? 8u : 8u)
#define Modbus_RTU_Over_UART_NUMBER_OF_STOP_BITS    (1u)

#if (Modbus_RTU_Over_UART_RXHW_ADDRESS_ENABLED)
    #define Modbus_RTU_Over_UART_RX_ADDRESS_MODE    (0u)
    #define Modbus_RTU_Over_UART_RX_HW_ADDRESS1     (0u)
    #define Modbus_RTU_Over_UART_RX_HW_ADDRESS2     (0u)
#endif /* (Modbus_RTU_Over_UART_RXHW_ADDRESS_ENABLED) */

#define Modbus_RTU_Over_UART_INIT_RX_INTERRUPTS_MASK \
                                  (uint8)((1 << Modbus_RTU_Over_UART_RX_STS_FIFO_NOTEMPTY_SHIFT) \
                                        | (0 << Modbus_RTU_Over_UART_RX_STS_MRKSPC_SHIFT) \
                                        | (0 << Modbus_RTU_Over_UART_RX_STS_ADDR_MATCH_SHIFT) \
                                        | (0 << Modbus_RTU_Over_UART_RX_STS_PAR_ERROR_SHIFT) \
                                        | (0 << Modbus_RTU_Over_UART_RX_STS_STOP_ERROR_SHIFT) \
                                        | (0 << Modbus_RTU_Over_UART_RX_STS_BREAK_SHIFT) \
                                        | (0 << Modbus_RTU_Over_UART_RX_STS_OVERRUN_SHIFT))

#define Modbus_RTU_Over_UART_INIT_TX_INTERRUPTS_MASK \
                                  (uint8)((0 << Modbus_RTU_Over_UART_TX_STS_COMPLETE_SHIFT) \
                                        | (0 << Modbus_RTU_Over_UART_TX_STS_FIFO_EMPTY_SHIFT) \
                                        | (0 << Modbus_RTU_Over_UART_TX_STS_FIFO_FULL_SHIFT) \
                                        | (0 << Modbus_RTU_Over_UART_TX_STS_FIFO_NOT_FULL_SHIFT))


/***************************************
*              Registers
***************************************/

#ifdef Modbus_RTU_Over_UART_BUART_sCR_SyncCtl_CtrlReg__CONTROL_REG
    #define Modbus_RTU_Over_UART_CONTROL_REG \
                            (* (reg8 *) Modbus_RTU_Over_UART_BUART_sCR_SyncCtl_CtrlReg__CONTROL_REG )
    #define Modbus_RTU_Over_UART_CONTROL_PTR \
                            (  (reg8 *) Modbus_RTU_Over_UART_BUART_sCR_SyncCtl_CtrlReg__CONTROL_REG )
#endif /* End Modbus_RTU_Over_UART_BUART_sCR_SyncCtl_CtrlReg__CONTROL_REG */

#if(Modbus_RTU_Over_UART_TX_ENABLED)
    #define Modbus_RTU_Over_UART_TXDATA_REG          (* (reg8 *) Modbus_RTU_Over_UART_BUART_sTX_TxShifter_u0__F0_REG)
    #define Modbus_RTU_Over_UART_TXDATA_PTR          (  (reg8 *) Modbus_RTU_Over_UART_BUART_sTX_TxShifter_u0__F0_REG)
    #define Modbus_RTU_Over_UART_TXDATA_AUX_CTL_REG  (* (reg8 *) Modbus_RTU_Over_UART_BUART_sTX_TxShifter_u0__DP_AUX_CTL_REG)
    #define Modbus_RTU_Over_UART_TXDATA_AUX_CTL_PTR  (  (reg8 *) Modbus_RTU_Over_UART_BUART_sTX_TxShifter_u0__DP_AUX_CTL_REG)
    #define Modbus_RTU_Over_UART_TXSTATUS_REG        (* (reg8 *) Modbus_RTU_Over_UART_BUART_sTX_TxSts__STATUS_REG)
    #define Modbus_RTU_Over_UART_TXSTATUS_PTR        (  (reg8 *) Modbus_RTU_Over_UART_BUART_sTX_TxSts__STATUS_REG)
    #define Modbus_RTU_Over_UART_TXSTATUS_MASK_REG   (* (reg8 *) Modbus_RTU_Over_UART_BUART_sTX_TxSts__MASK_REG)
    #define Modbus_RTU_Over_UART_TXSTATUS_MASK_PTR   (  (reg8 *) Modbus_RTU_Over_UART_BUART_sTX_TxSts__MASK_REG)
    #define Modbus_RTU_Over_UART_TXSTATUS_ACTL_REG   (* (reg8 *) Modbus_RTU_Over_UART_BUART_sTX_TxSts__STATUS_AUX_CTL_REG)
    #define Modbus_RTU_Over_UART_TXSTATUS_ACTL_PTR   (  (reg8 *) Modbus_RTU_Over_UART_BUART_sTX_TxSts__STATUS_AUX_CTL_REG)

    /* DP clock */
    #if(Modbus_RTU_Over_UART_TXCLKGEN_DP)
        #define Modbus_RTU_Over_UART_TXBITCLKGEN_CTR_REG        \
                                        (* (reg8 *) Modbus_RTU_Over_UART_BUART_sTX_sCLOCK_TxBitClkGen__D0_REG)
        #define Modbus_RTU_Over_UART_TXBITCLKGEN_CTR_PTR        \
                                        (  (reg8 *) Modbus_RTU_Over_UART_BUART_sTX_sCLOCK_TxBitClkGen__D0_REG)
        #define Modbus_RTU_Over_UART_TXBITCLKTX_COMPLETE_REG    \
                                        (* (reg8 *) Modbus_RTU_Over_UART_BUART_sTX_sCLOCK_TxBitClkGen__D1_REG)
        #define Modbus_RTU_Over_UART_TXBITCLKTX_COMPLETE_PTR    \
                                        (  (reg8 *) Modbus_RTU_Over_UART_BUART_sTX_sCLOCK_TxBitClkGen__D1_REG)
    #else     /* Count7 clock*/
        #define Modbus_RTU_Over_UART_TXBITCTR_PERIOD_REG    \
                                        (* (reg8 *) Modbus_RTU_Over_UART_BUART_sTX_sCLOCK_TxBitCounter__PERIOD_REG)
        #define Modbus_RTU_Over_UART_TXBITCTR_PERIOD_PTR    \
                                        (  (reg8 *) Modbus_RTU_Over_UART_BUART_sTX_sCLOCK_TxBitCounter__PERIOD_REG)
        #define Modbus_RTU_Over_UART_TXBITCTR_CONTROL_REG   \
                                        (* (reg8 *) Modbus_RTU_Over_UART_BUART_sTX_sCLOCK_TxBitCounter__CONTROL_AUX_CTL_REG)
        #define Modbus_RTU_Over_UART_TXBITCTR_CONTROL_PTR   \
                                        (  (reg8 *) Modbus_RTU_Over_UART_BUART_sTX_sCLOCK_TxBitCounter__CONTROL_AUX_CTL_REG)
        #define Modbus_RTU_Over_UART_TXBITCTR_COUNTER_REG   \
                                        (* (reg8 *) Modbus_RTU_Over_UART_BUART_sTX_sCLOCK_TxBitCounter__COUNT_REG)
        #define Modbus_RTU_Over_UART_TXBITCTR_COUNTER_PTR   \
                                        (  (reg8 *) Modbus_RTU_Over_UART_BUART_sTX_sCLOCK_TxBitCounter__COUNT_REG)
    #endif /* Modbus_RTU_Over_UART_TXCLKGEN_DP */

#endif /* End Modbus_RTU_Over_UART_TX_ENABLED */

#if(Modbus_RTU_Over_UART_HD_ENABLED)

    #define Modbus_RTU_Over_UART_TXDATA_REG             (* (reg8 *) Modbus_RTU_Over_UART_BUART_sRX_RxShifter_u0__F1_REG )
    #define Modbus_RTU_Over_UART_TXDATA_PTR             (  (reg8 *) Modbus_RTU_Over_UART_BUART_sRX_RxShifter_u0__F1_REG )
    #define Modbus_RTU_Over_UART_TXDATA_AUX_CTL_REG     (* (reg8 *) Modbus_RTU_Over_UART_BUART_sRX_RxShifter_u0__DP_AUX_CTL_REG)
    #define Modbus_RTU_Over_UART_TXDATA_AUX_CTL_PTR     (  (reg8 *) Modbus_RTU_Over_UART_BUART_sRX_RxShifter_u0__DP_AUX_CTL_REG)

    #define Modbus_RTU_Over_UART_TXSTATUS_REG           (* (reg8 *) Modbus_RTU_Over_UART_BUART_sRX_RxSts__STATUS_REG )
    #define Modbus_RTU_Over_UART_TXSTATUS_PTR           (  (reg8 *) Modbus_RTU_Over_UART_BUART_sRX_RxSts__STATUS_REG )
    #define Modbus_RTU_Over_UART_TXSTATUS_MASK_REG      (* (reg8 *) Modbus_RTU_Over_UART_BUART_sRX_RxSts__MASK_REG )
    #define Modbus_RTU_Over_UART_TXSTATUS_MASK_PTR      (  (reg8 *) Modbus_RTU_Over_UART_BUART_sRX_RxSts__MASK_REG )
    #define Modbus_RTU_Over_UART_TXSTATUS_ACTL_REG      (* (reg8 *) Modbus_RTU_Over_UART_BUART_sRX_RxSts__STATUS_AUX_CTL_REG )
    #define Modbus_RTU_Over_UART_TXSTATUS_ACTL_PTR      (  (reg8 *) Modbus_RTU_Over_UART_BUART_sRX_RxSts__STATUS_AUX_CTL_REG )
#endif /* End Modbus_RTU_Over_UART_HD_ENABLED */

#if( (Modbus_RTU_Over_UART_RX_ENABLED) || (Modbus_RTU_Over_UART_HD_ENABLED) )
    #define Modbus_RTU_Over_UART_RXDATA_REG             (* (reg8 *) Modbus_RTU_Over_UART_BUART_sRX_RxShifter_u0__F0_REG )
    #define Modbus_RTU_Over_UART_RXDATA_PTR             (  (reg8 *) Modbus_RTU_Over_UART_BUART_sRX_RxShifter_u0__F0_REG )
    #define Modbus_RTU_Over_UART_RXADDRESS1_REG         (* (reg8 *) Modbus_RTU_Over_UART_BUART_sRX_RxShifter_u0__D0_REG )
    #define Modbus_RTU_Over_UART_RXADDRESS1_PTR         (  (reg8 *) Modbus_RTU_Over_UART_BUART_sRX_RxShifter_u0__D0_REG )
    #define Modbus_RTU_Over_UART_RXADDRESS2_REG         (* (reg8 *) Modbus_RTU_Over_UART_BUART_sRX_RxShifter_u0__D1_REG )
    #define Modbus_RTU_Over_UART_RXADDRESS2_PTR         (  (reg8 *) Modbus_RTU_Over_UART_BUART_sRX_RxShifter_u0__D1_REG )
    #define Modbus_RTU_Over_UART_RXDATA_AUX_CTL_REG     (* (reg8 *) Modbus_RTU_Over_UART_BUART_sRX_RxShifter_u0__DP_AUX_CTL_REG)

    #define Modbus_RTU_Over_UART_RXBITCTR_PERIOD_REG    (* (reg8 *) Modbus_RTU_Over_UART_BUART_sRX_RxBitCounter__PERIOD_REG )
    #define Modbus_RTU_Over_UART_RXBITCTR_PERIOD_PTR    (  (reg8 *) Modbus_RTU_Over_UART_BUART_sRX_RxBitCounter__PERIOD_REG )
    #define Modbus_RTU_Over_UART_RXBITCTR_CONTROL_REG   \
                                        (* (reg8 *) Modbus_RTU_Over_UART_BUART_sRX_RxBitCounter__CONTROL_AUX_CTL_REG )
    #define Modbus_RTU_Over_UART_RXBITCTR_CONTROL_PTR   \
                                        (  (reg8 *) Modbus_RTU_Over_UART_BUART_sRX_RxBitCounter__CONTROL_AUX_CTL_REG )
    #define Modbus_RTU_Over_UART_RXBITCTR_COUNTER_REG   (* (reg8 *) Modbus_RTU_Over_UART_BUART_sRX_RxBitCounter__COUNT_REG )
    #define Modbus_RTU_Over_UART_RXBITCTR_COUNTER_PTR   (  (reg8 *) Modbus_RTU_Over_UART_BUART_sRX_RxBitCounter__COUNT_REG )

    #define Modbus_RTU_Over_UART_RXSTATUS_REG           (* (reg8 *) Modbus_RTU_Over_UART_BUART_sRX_RxSts__STATUS_REG )
    #define Modbus_RTU_Over_UART_RXSTATUS_PTR           (  (reg8 *) Modbus_RTU_Over_UART_BUART_sRX_RxSts__STATUS_REG )
    #define Modbus_RTU_Over_UART_RXSTATUS_MASK_REG      (* (reg8 *) Modbus_RTU_Over_UART_BUART_sRX_RxSts__MASK_REG )
    #define Modbus_RTU_Over_UART_RXSTATUS_MASK_PTR      (  (reg8 *) Modbus_RTU_Over_UART_BUART_sRX_RxSts__MASK_REG )
    #define Modbus_RTU_Over_UART_RXSTATUS_ACTL_REG      (* (reg8 *) Modbus_RTU_Over_UART_BUART_sRX_RxSts__STATUS_AUX_CTL_REG )
    #define Modbus_RTU_Over_UART_RXSTATUS_ACTL_PTR      (  (reg8 *) Modbus_RTU_Over_UART_BUART_sRX_RxSts__STATUS_AUX_CTL_REG )
#endif /* End  (Modbus_RTU_Over_UART_RX_ENABLED) || (Modbus_RTU_Over_UART_HD_ENABLED) */

#if(Modbus_RTU_Over_UART_INTERNAL_CLOCK_USED)
    /* Register to enable or disable the digital clocks */
    #define Modbus_RTU_Over_UART_INTCLOCK_CLKEN_REG     (* (reg8 *) Modbus_RTU_Over_UART_IntClock__PM_ACT_CFG)
    #define Modbus_RTU_Over_UART_INTCLOCK_CLKEN_PTR     (  (reg8 *) Modbus_RTU_Over_UART_IntClock__PM_ACT_CFG)

    /* Clock mask for this clock. */
    #define Modbus_RTU_Over_UART_INTCLOCK_CLKEN_MASK    Modbus_RTU_Over_UART_IntClock__PM_ACT_MSK
#endif /* End Modbus_RTU_Over_UART_INTERNAL_CLOCK_USED */


/***************************************
*       Register Constants
***************************************/

#if(Modbus_RTU_Over_UART_TX_ENABLED)
    #define Modbus_RTU_Over_UART_TX_FIFO_CLR            (0x01u) /* FIFO0 CLR */
#endif /* End Modbus_RTU_Over_UART_TX_ENABLED */

#if(Modbus_RTU_Over_UART_HD_ENABLED)
    #define Modbus_RTU_Over_UART_TX_FIFO_CLR            (0x02u) /* FIFO1 CLR */
#endif /* End Modbus_RTU_Over_UART_HD_ENABLED */

#if( (Modbus_RTU_Over_UART_RX_ENABLED) || (Modbus_RTU_Over_UART_HD_ENABLED) )
    #define Modbus_RTU_Over_UART_RX_FIFO_CLR            (0x01u) /* FIFO0 CLR */
#endif /* End  (Modbus_RTU_Over_UART_RX_ENABLED) || (Modbus_RTU_Over_UART_HD_ENABLED) */


/***************************************
* The following code is DEPRECATED and
* should not be used in new projects.
***************************************/

/* UART v2_40 obsolete definitions */
#define Modbus_RTU_Over_UART_WAIT_1_MS      Modbus_RTU_Over_UART_BL_CHK_DELAY_MS   

#define Modbus_RTU_Over_UART_TXBUFFERSIZE   Modbus_RTU_Over_UART_TX_BUFFER_SIZE
#define Modbus_RTU_Over_UART_RXBUFFERSIZE   Modbus_RTU_Over_UART_RX_BUFFER_SIZE

#if (Modbus_RTU_Over_UART_RXHW_ADDRESS_ENABLED)
    #define Modbus_RTU_Over_UART_RXADDRESSMODE  Modbus_RTU_Over_UART_RX_ADDRESS_MODE
    #define Modbus_RTU_Over_UART_RXHWADDRESS1   Modbus_RTU_Over_UART_RX_HW_ADDRESS1
    #define Modbus_RTU_Over_UART_RXHWADDRESS2   Modbus_RTU_Over_UART_RX_HW_ADDRESS2
    /* Backward compatible define */
    #define Modbus_RTU_Over_UART_RXAddressMode  Modbus_RTU_Over_UART_RXADDRESSMODE
#endif /* (Modbus_RTU_Over_UART_RXHW_ADDRESS_ENABLED) */

/* UART v2_30 obsolete definitions */
#define Modbus_RTU_Over_UART_initvar                    Modbus_RTU_Over_UART_initVar

#define Modbus_RTU_Over_UART_RX_Enabled                 Modbus_RTU_Over_UART_RX_ENABLED
#define Modbus_RTU_Over_UART_TX_Enabled                 Modbus_RTU_Over_UART_TX_ENABLED
#define Modbus_RTU_Over_UART_HD_Enabled                 Modbus_RTU_Over_UART_HD_ENABLED
#define Modbus_RTU_Over_UART_RX_IntInterruptEnabled     Modbus_RTU_Over_UART_RX_INTERRUPT_ENABLED
#define Modbus_RTU_Over_UART_TX_IntInterruptEnabled     Modbus_RTU_Over_UART_TX_INTERRUPT_ENABLED
#define Modbus_RTU_Over_UART_InternalClockUsed          Modbus_RTU_Over_UART_INTERNAL_CLOCK_USED
#define Modbus_RTU_Over_UART_RXHW_Address_Enabled       Modbus_RTU_Over_UART_RXHW_ADDRESS_ENABLED
#define Modbus_RTU_Over_UART_OverSampleCount            Modbus_RTU_Over_UART_OVER_SAMPLE_COUNT
#define Modbus_RTU_Over_UART_ParityType                 Modbus_RTU_Over_UART_PARITY_TYPE

#if( Modbus_RTU_Over_UART_TX_ENABLED && (Modbus_RTU_Over_UART_TXBUFFERSIZE > Modbus_RTU_Over_UART_FIFO_LENGTH))
    #define Modbus_RTU_Over_UART_TXBUFFER               Modbus_RTU_Over_UART_txBuffer
    #define Modbus_RTU_Over_UART_TXBUFFERREAD           Modbus_RTU_Over_UART_txBufferRead
    #define Modbus_RTU_Over_UART_TXBUFFERWRITE          Modbus_RTU_Over_UART_txBufferWrite
#endif /* End Modbus_RTU_Over_UART_TX_ENABLED */
#if( ( Modbus_RTU_Over_UART_RX_ENABLED || Modbus_RTU_Over_UART_HD_ENABLED ) && \
     (Modbus_RTU_Over_UART_RXBUFFERSIZE > Modbus_RTU_Over_UART_FIFO_LENGTH) )
    #define Modbus_RTU_Over_UART_RXBUFFER               Modbus_RTU_Over_UART_rxBuffer
    #define Modbus_RTU_Over_UART_RXBUFFERREAD           Modbus_RTU_Over_UART_rxBufferRead
    #define Modbus_RTU_Over_UART_RXBUFFERWRITE          Modbus_RTU_Over_UART_rxBufferWrite
    #define Modbus_RTU_Over_UART_RXBUFFERLOOPDETECT     Modbus_RTU_Over_UART_rxBufferLoopDetect
    #define Modbus_RTU_Over_UART_RXBUFFER_OVERFLOW      Modbus_RTU_Over_UART_rxBufferOverflow
#endif /* End Modbus_RTU_Over_UART_RX_ENABLED */

#ifdef Modbus_RTU_Over_UART_BUART_sCR_SyncCtl_CtrlReg__CONTROL_REG
    #define Modbus_RTU_Over_UART_CONTROL                Modbus_RTU_Over_UART_CONTROL_REG
#endif /* End Modbus_RTU_Over_UART_BUART_sCR_SyncCtl_CtrlReg__CONTROL_REG */

#if(Modbus_RTU_Over_UART_TX_ENABLED)
    #define Modbus_RTU_Over_UART_TXDATA                 Modbus_RTU_Over_UART_TXDATA_REG
    #define Modbus_RTU_Over_UART_TXSTATUS               Modbus_RTU_Over_UART_TXSTATUS_REG
    #define Modbus_RTU_Over_UART_TXSTATUS_MASK          Modbus_RTU_Over_UART_TXSTATUS_MASK_REG
    #define Modbus_RTU_Over_UART_TXSTATUS_ACTL          Modbus_RTU_Over_UART_TXSTATUS_ACTL_REG
    /* DP clock */
    #if(Modbus_RTU_Over_UART_TXCLKGEN_DP)
        #define Modbus_RTU_Over_UART_TXBITCLKGEN_CTR        Modbus_RTU_Over_UART_TXBITCLKGEN_CTR_REG
        #define Modbus_RTU_Over_UART_TXBITCLKTX_COMPLETE    Modbus_RTU_Over_UART_TXBITCLKTX_COMPLETE_REG
    #else     /* Count7 clock*/
        #define Modbus_RTU_Over_UART_TXBITCTR_PERIOD        Modbus_RTU_Over_UART_TXBITCTR_PERIOD_REG
        #define Modbus_RTU_Over_UART_TXBITCTR_CONTROL       Modbus_RTU_Over_UART_TXBITCTR_CONTROL_REG
        #define Modbus_RTU_Over_UART_TXBITCTR_COUNTER       Modbus_RTU_Over_UART_TXBITCTR_COUNTER_REG
    #endif /* Modbus_RTU_Over_UART_TXCLKGEN_DP */
#endif /* End Modbus_RTU_Over_UART_TX_ENABLED */

#if(Modbus_RTU_Over_UART_HD_ENABLED)
    #define Modbus_RTU_Over_UART_TXDATA                 Modbus_RTU_Over_UART_TXDATA_REG
    #define Modbus_RTU_Over_UART_TXSTATUS               Modbus_RTU_Over_UART_TXSTATUS_REG
    #define Modbus_RTU_Over_UART_TXSTATUS_MASK          Modbus_RTU_Over_UART_TXSTATUS_MASK_REG
    #define Modbus_RTU_Over_UART_TXSTATUS_ACTL          Modbus_RTU_Over_UART_TXSTATUS_ACTL_REG
#endif /* End Modbus_RTU_Over_UART_HD_ENABLED */

#if( (Modbus_RTU_Over_UART_RX_ENABLED) || (Modbus_RTU_Over_UART_HD_ENABLED) )
    #define Modbus_RTU_Over_UART_RXDATA                 Modbus_RTU_Over_UART_RXDATA_REG
    #define Modbus_RTU_Over_UART_RXADDRESS1             Modbus_RTU_Over_UART_RXADDRESS1_REG
    #define Modbus_RTU_Over_UART_RXADDRESS2             Modbus_RTU_Over_UART_RXADDRESS2_REG
    #define Modbus_RTU_Over_UART_RXBITCTR_PERIOD        Modbus_RTU_Over_UART_RXBITCTR_PERIOD_REG
    #define Modbus_RTU_Over_UART_RXBITCTR_CONTROL       Modbus_RTU_Over_UART_RXBITCTR_CONTROL_REG
    #define Modbus_RTU_Over_UART_RXBITCTR_COUNTER       Modbus_RTU_Over_UART_RXBITCTR_COUNTER_REG
    #define Modbus_RTU_Over_UART_RXSTATUS               Modbus_RTU_Over_UART_RXSTATUS_REG
    #define Modbus_RTU_Over_UART_RXSTATUS_MASK          Modbus_RTU_Over_UART_RXSTATUS_MASK_REG
    #define Modbus_RTU_Over_UART_RXSTATUS_ACTL          Modbus_RTU_Over_UART_RXSTATUS_ACTL_REG
#endif /* End  (Modbus_RTU_Over_UART_RX_ENABLED) || (Modbus_RTU_Over_UART_HD_ENABLED) */

#if(Modbus_RTU_Over_UART_INTERNAL_CLOCK_USED)
    #define Modbus_RTU_Over_UART_INTCLOCK_CLKEN         Modbus_RTU_Over_UART_INTCLOCK_CLKEN_REG
#endif /* End Modbus_RTU_Over_UART_INTERNAL_CLOCK_USED */

#define Modbus_RTU_Over_UART_WAIT_FOR_COMLETE_REINIT    Modbus_RTU_Over_UART_WAIT_FOR_COMPLETE_REINIT

#endif  /* CY_UART_Modbus_RTU_Over_UART_H */


/* [] END OF FILE */
