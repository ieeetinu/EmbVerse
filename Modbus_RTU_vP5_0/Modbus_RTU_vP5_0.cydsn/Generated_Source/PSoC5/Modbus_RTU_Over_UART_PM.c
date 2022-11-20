/*******************************************************************************
* File Name: Modbus_RTU_Over_UART_PM.c
* Version 2.50
*
* Description:
*  This file provides Sleep/WakeUp APIs functionality.
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


/***************************************
* Local data allocation
***************************************/

static Modbus_RTU_Over_UART_BACKUP_STRUCT  Modbus_RTU_Over_UART_backup =
{
    /* enableState - disabled */
    0u,
};



/*******************************************************************************
* Function Name: Modbus_RTU_Over_UART_SaveConfig
********************************************************************************
*
* Summary:
*  This function saves the component nonretention control register.
*  Does not save the FIFO which is a set of nonretention registers.
*  This function is called by the Modbus_RTU_Over_UART_Sleep() function.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
* Global Variables:
*  Modbus_RTU_Over_UART_backup - modified when non-retention registers are saved.
*
* Reentrant:
*  No.
*
*******************************************************************************/
void Modbus_RTU_Over_UART_SaveConfig(void)
{
    #if(Modbus_RTU_Over_UART_CONTROL_REG_REMOVED == 0u)
        Modbus_RTU_Over_UART_backup.cr = Modbus_RTU_Over_UART_CONTROL_REG;
    #endif /* End Modbus_RTU_Over_UART_CONTROL_REG_REMOVED */
}


/*******************************************************************************
* Function Name: Modbus_RTU_Over_UART_RestoreConfig
********************************************************************************
*
* Summary:
*  Restores the nonretention control register except FIFO.
*  Does not restore the FIFO which is a set of nonretention registers.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
* Global Variables:
*  Modbus_RTU_Over_UART_backup - used when non-retention registers are restored.
*
* Reentrant:
*  No.
*
* Notes:
*  If this function is called without calling Modbus_RTU_Over_UART_SaveConfig() 
*  first, the data loaded may be incorrect.
*
*******************************************************************************/
void Modbus_RTU_Over_UART_RestoreConfig(void)
{
    #if(Modbus_RTU_Over_UART_CONTROL_REG_REMOVED == 0u)
        Modbus_RTU_Over_UART_CONTROL_REG = Modbus_RTU_Over_UART_backup.cr;
    #endif /* End Modbus_RTU_Over_UART_CONTROL_REG_REMOVED */
}


/*******************************************************************************
* Function Name: Modbus_RTU_Over_UART_Sleep
********************************************************************************
*
* Summary:
*  This is the preferred API to prepare the component for sleep. 
*  The Modbus_RTU_Over_UART_Sleep() API saves the current component state. Then it
*  calls the Modbus_RTU_Over_UART_Stop() function and calls 
*  Modbus_RTU_Over_UART_SaveConfig() to save the hardware configuration.
*  Call the Modbus_RTU_Over_UART_Sleep() function before calling the CyPmSleep() 
*  or the CyPmHibernate() function. 
*
* Parameters:
*  None.
*
* Return:
*  None.
*
* Global Variables:
*  Modbus_RTU_Over_UART_backup - modified when non-retention registers are saved.
*
* Reentrant:
*  No.
*
*******************************************************************************/
void Modbus_RTU_Over_UART_Sleep(void)
{
    #if(Modbus_RTU_Over_UART_RX_ENABLED || Modbus_RTU_Over_UART_HD_ENABLED)
        if((Modbus_RTU_Over_UART_RXSTATUS_ACTL_REG  & Modbus_RTU_Over_UART_INT_ENABLE) != 0u)
        {
            Modbus_RTU_Over_UART_backup.enableState = 1u;
        }
        else
        {
            Modbus_RTU_Over_UART_backup.enableState = 0u;
        }
    #else
        if((Modbus_RTU_Over_UART_TXSTATUS_ACTL_REG  & Modbus_RTU_Over_UART_INT_ENABLE) !=0u)
        {
            Modbus_RTU_Over_UART_backup.enableState = 1u;
        }
        else
        {
            Modbus_RTU_Over_UART_backup.enableState = 0u;
        }
    #endif /* End Modbus_RTU_Over_UART_RX_ENABLED || Modbus_RTU_Over_UART_HD_ENABLED*/

    Modbus_RTU_Over_UART_Stop();
    Modbus_RTU_Over_UART_SaveConfig();
}


/*******************************************************************************
* Function Name: Modbus_RTU_Over_UART_Wakeup
********************************************************************************
*
* Summary:
*  This is the preferred API to restore the component to the state when 
*  Modbus_RTU_Over_UART_Sleep() was called. The Modbus_RTU_Over_UART_Wakeup() function
*  calls the Modbus_RTU_Over_UART_RestoreConfig() function to restore the 
*  configuration. If the component was enabled before the 
*  Modbus_RTU_Over_UART_Sleep() function was called, the Modbus_RTU_Over_UART_Wakeup()
*  function will also re-enable the component.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
* Global Variables:
*  Modbus_RTU_Over_UART_backup - used when non-retention registers are restored.
*
* Reentrant:
*  No.
*
*******************************************************************************/
void Modbus_RTU_Over_UART_Wakeup(void)
{
    Modbus_RTU_Over_UART_RestoreConfig();
    #if( (Modbus_RTU_Over_UART_RX_ENABLED) || (Modbus_RTU_Over_UART_HD_ENABLED) )
        Modbus_RTU_Over_UART_ClearRxBuffer();
    #endif /* End (Modbus_RTU_Over_UART_RX_ENABLED) || (Modbus_RTU_Over_UART_HD_ENABLED) */
    #if(Modbus_RTU_Over_UART_TX_ENABLED || Modbus_RTU_Over_UART_HD_ENABLED)
        Modbus_RTU_Over_UART_ClearTxBuffer();
    #endif /* End Modbus_RTU_Over_UART_TX_ENABLED || Modbus_RTU_Over_UART_HD_ENABLED */

    if(Modbus_RTU_Over_UART_backup.enableState != 0u)
    {
        Modbus_RTU_Over_UART_Enable();
    }
}


/* [] END OF FILE */
