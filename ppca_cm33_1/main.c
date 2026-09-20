/******************************************************************************
* File Name:   main.c
*
* Description: This is the source code for PPCA Core 1 (CM33) in the
*              On-Chip Temperature Measurement application for ModusToolbox.
*              This core is booted by the main secure core (main_cm33_s) and
*              runs as an processor.
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

#include "cy_pdl.h"       /* Peripheral Driver Library */
#include "cycfg.h"        /* Device configurator generated header */
#include <stdio.h>

/*******************************************************************************
* Macros
********************************************************************************/

/* Shared memory address in M4 shared memory space (0x20040000-0x20043FFF).
 * All PPCA cores can access M4 shared memory for inter-core communication.
 * The main secure core reads this address via the PPCA peripheral base at
 * 0x53050800 (M4 window). */
#define PPCA_CPU1_M4_VAR_ADDRESS 0x20040800  /* Shared memory used by PPCA Core 1 */

/*******************************************************************************
* Global Variables
********************************************************************************/


/*******************************************************************************
* Function Prototypes
********************************************************************************/


/*******************************************************************************
* Function Name: main
********************************************************************************
* Summary:
* Entry point for PPCA Core 1 (CM33). This core is booted by the main secure
* core (main_cm33_s). Currently implements an idle loop; application-specific 
* processing can be added here.
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
     /* Infinite loop - add PPCA Core 1 application logic here */
     for(;;)
     {

     }
}
