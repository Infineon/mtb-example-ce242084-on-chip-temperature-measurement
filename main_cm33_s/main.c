/******************************************************************************
* File Name:   main.c
*
* Description: This is the source code for the main secure core (CM33_S) in
*              the On-Chip Temperature Measurement application for ModusToolbox.
*              It initializes peripherals (UART, ADC, AREF, PPCA), boots the
*              PPCA cores, and reads the on-chip temperature sensor on user
*              button press.
*
* Related Document: See README.md
*
*
********************************************************************************
* (c) 2026, Infineon Technologies AG, or an affiliate of Infineon
* Technologies AG. All rights reserved.
* This software, associated documentation and materials ("Software") is
* owned by Infineon Technologies AG or one of its affiliates ("Infineon")
* and is protected by and subject to worldwide patent protection, worldwide
* copyright laws, and international treaty provisions. Therefore, you may use
* this Software only as provided in the license agreement accompanying the
* software package from which you obtained this Software. If no license
* agreement applies, then any use, reproduction, modification, translation, or
* compilation of this Software is prohibited without the express written
* permission of Infineon.
*
* Disclaimer: UNLESS OTHERWISE EXPRESSLY AGREED WITH INFINEON, THIS SOFTWARE
* IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
* INCLUDING, BUT NOT LIMITED TO, ALL WARRANTIES OF NON-INFRINGEMENT OF
* THIRD-PARTY RIGHTS AND IMPLIED WARRANTIES SUCH AS WARRANTIES OF FITNESS FOR A
* SPECIFIC USE/PURPOSE OR MERCHANTABILITY.
* Infineon reserves the right to make changes to the Software without notice.
* You are responsible for properly designing, programming, and testing the
* functionality and safety of your intended application of the Software, as
* well as complying with any legal requirements related to its use. Infineon
* does not guarantee that the Software will be free from intrusion, data theft
* or loss, or other breaches ("Security Breaches"), and Infineon shall have
* no liability arising out of any Security Breaches. Unless otherwise
* explicitly approved by Infineon, the Software may not be used in any
* application where a failure of the Product or any consequences of the use
* thereof can reasonably be expected to result in personal injury.
*******************************************************************************/

/*******************************************************************************
* Header Files
********************************************************************************/

#include "cy_pdl.h"
#include "cybsp.h"
#include "cy_retarget_io.h"

/*******************************************************************************
* Macros
********************************************************************************/

/* These are the addresses where the core0 and core1 images are located. */
#define CORE0_IMAGE_ADDRESS    CYMEM_CM33_0_S_m33s_ppca0_nvm_C_S_START    //  0x12030000
#define CORE1_IMAGE_ADDRESS    CYMEM_CM33_0_S_m33s_ppca1_nvm_C_S_START    //  0x12038000

#define PPCA0_IMAGE_SIZE       CYMEM_CM33_0_S_ppca0_code_SIZE
#define PPCA1_IMAGE_SIZE       CYMEM_CM33_0_S_ppca1_code_SIZE

/* Shared memory addresses for inter-core communication */
/* PPCA cores write to these locations, main core reads from them */
/* Variables located in M4 shared memory space (16KB at 0x20040000-0x20043FFF from PPCA view) */
/* Main core accesses PPCA memory through PPCA peripheral base with memory windows: */
/* M1 (CPU0 data): 0x53020000, M3 (CPU1 data): 0x53040000, M4 (shared): 0x53050000 */
#define PPCA_CPU0_M4_VAR_ADDRESS   0x53050400  /* Written by PPCA Core 0 */
#define PPCA_CPU1_M4_VAR_ADDRESS   0x53050800  /* Written by PPCA Core 1 */

/*******************************************************************************
* Global Variables
********************************************************************************/

/* Debug UART variables */
static cy_stc_scb_uart_context_t    DEBUG_UART_context; /* DEBUG_UART context */
static mtb_hal_uart_t               DEBUG_UART_hal_obj; /* Debug DEBUG_UART HAL object */

/* Variable for average ADC temperature data of MCU */
float temperature_avg = 0;

/* Variable for ADC temperature data of MCU */
int16_t temperature = 0;

/* Delay in microseconds for ADC conversion settling time */
uint16_t delay = 2;

/*******************************************************************************
* Function Prototypes
********************************************************************************/

/* Function to check if the user button is pressed */
bool user_button_is_pressed(void);

/* Function to read the on chip temperature */
void read_on_chip_temperature(float *tempOutput, uint16_t timeoutUs);

/*******************************************************************************
* Function Name: main
********************************************************************************
* Summary:
* Entry point for the main secure CM33 core. Initializes board peripherals
* (UART, ADC, AREF, PPCA), boots the PPCA cores (CPU0 and CPU1),and enters
* a loop that reads the on-chip temperature sensor each time the user button 
* is pressed.
*
* Parameters:
*  void
*
* Return:
*  int
*
*******************************************************************************/
int main(void)
{
    cy_rslt_t result;

    /* Initialize the device and board peripherals */
    result = cybsp_init();

    /* Board init failed. Stop program execution */
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* Initialize and enable the SCB-based debug UART */
    Cy_SCB_UART_Init(DEBUG_UART_HW, &DEBUG_UART_config, &DEBUG_UART_context);
    Cy_SCB_UART_Enable(DEBUG_UART_HW);

    /* Setup the HAL DEBUG_UART */
    result = mtb_hal_uart_setup(&DEBUG_UART_hal_obj, &DEBUG_UART_hal_config,
                                &DEBUG_UART_context, NULL);

    /* HAL UART initialization failed. Stop program execution */
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* Initialize retarget-io to redirect printf output to the debug UART */
    result = cy_retarget_io_init(&DEBUG_UART_hal_obj);

    /* Retarget-io initialization failed. Stop program execution */
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* Transmit header to the terminal */
    /* \x1b[2J\x1b[;H - ANSI ESC sequence for clear screen */
    printf("\x1b[2J\x1b[;H");
    printf("************************************************************\r\n");
    printf("PSOC Control C3M/P8: Temperature measurement\r\n");
    printf("************************************************************\r\n\n");
    printf("Press user button 'SW3'\r\n\n");

    /* Initialize and enable the PPCA (Programmable Power Control Accelerator) */
    Cy_PPCA_CNFG_Init(PPCA_CNFG_HW, &PPCA_CNFG_config);
    Cy_PPCA_Enable(PPCA_CNFG_HW);

    /* Enable exclusive access to the EPU resources based
     * on the provided resources allocation configuration. */
    Cy_PPCA_EPU_EnableExclusiveAccess(PPCA_EPU, true);

    /* Enable EPU */
    Cy_PPCA_EPU_Enable(PPCA_EPU);

    /* Initialize and enable the analog reference (AREF) block */
    Cy_PPCA_AREF_Init(AREF_HW, &AREF_config);
    Cy_PPCA_AREF_Enable(AREF_HW);

    /* Enable current source to the on-chip temperature sensor (TSNS) */
    Cy_PPCA_AREF_Current_To_TSNS_Enable(AREF_HW);

    /* Set ADC calibration gain mode for accurate temperature readings */
    Cy_PPCA_ADC_Set_Calib_Gain_mode(ADC0_HW, 1);

    /* Initialize and enable ADC 0 for temperature measurement */
    Cy_PPCA_ADC_Init(ADC0_HW, &ADC0_config);
    Cy_PPCA_ADC_Enable(ADC0_HW);

    /* Boot the PPCA cores from their respective flash images */
    Cy_System_Init_CPU0((void*)CORE0_IMAGE_ADDRESS, PPCA0_IMAGE_SIZE);
    Cy_System_Init_CPU1((void*)CORE1_IMAGE_ADDRESS, PPCA1_IMAGE_SIZE);

    /* Enable global interrupts */
    __enable_irq();

    for (;;)
    {
        /* Check if the user button (SW3) was pressed */
        if(user_button_is_pressed())
        {
            /* Read the on-chip temperature sensor via ADC */
            read_on_chip_temperature(&temperature_avg, delay);

            /* Print the measured die temperature to the debug UART */
            printf("Die Temperature: %f C \r\n\n", (double)temperature_avg);
        }
    }
}

/*******************************************************************************
* Function Name: user_button_is_pressed
****************************************************************************//**
* Summary:
*  Checks whether the user button (SW3) has been pressed and released.
*  Implements software debouncing by counting 10 ms intervals while the
*  button is held. A press shorter than 100 ms is rejected as a glitch.
*
* Return:
*  true  - Valid button press detected (held for at least 100 ms).
*  false - Button not pressed or press was too short.
*
*******************************************************************************/
bool user_button_is_pressed(void)
{
    uint32_t pressCount = 0;

    /* Return immediately if the button is not currently pressed */
    if(Cy_GPIO_Read(CYBSP_USER_BTN_PORT, CYBSP_USER_BTN_PIN) != CYBSP_BTN_PRESSED)
    {
        return false;
    }
    /* Wait while the button is held, counting 10 ms intervals for debouncing */
    while (Cy_GPIO_Read(CYBSP_USER_BTN_PORT, CYBSP_USER_BTN_PIN) == CYBSP_BTN_PRESSED)
    {
        /* Wait for 10 ms */
        Cy_SysLib_Delay(10);
        pressCount++;
    }
    /* Post-release debounce delay */
    Cy_SysLib_Delay(10);

    if(10 < pressCount)
    {
        return true;
    }
    else
    {
        return false;
    }
}

/*******************************************************************************
* Function Name: read_on_chip_temperature
********************************************************************************
* Summary:
* Reads the current die temperature from the on-chip temperature sensor.
* Triggers a manual ADC conversion on the TSNS channel, waits for the result,
* and converts the raw ADC value to degrees Celsius.
*
* Parameters:
* \param tempOutput
* Pointer to a float where the calculated temperature (in °C) is stored.
*
* \param timeoutUs
* Settling time in microseconds to wait after triggering the ADC conversion.
*
* Return:
*  void
*
*******************************************************************************/
void read_on_chip_temperature(float *tempOutput, uint16_t timeoutUs)
{

    /* Trigger a manual ADC conversion on channel 7 (temperature sensor) */
    Cy_PPCA_ADC_Manual_Trigger(ADC0_HW,7);

    /* Wait for the ADC conversion to complete */
    Cy_SysLib_DelayUs(timeoutUs);

    /* Read the raw ADC value from the temperature sensor channel */
    temperature = (int16_t) Cy_PPCA_ADC_Read_AUX_ADC_Data(ADC0_HW,CY_AUX_CHANNEL_TSNS);

    /* Convert the raw ADC value to temperature in degrees Celsius */
    *tempOutput = Cy_PPCA_ADC_TEMP_Calc(temperature);
}
