/****************************************************************************
* File Name        : main.c
*
* Description      : This source file contains the main routine for CM55 CPU
*
* Related Document : See README.md
*
*****************************************************************************
* (c) 2025, Infineon Technologies AG, or an affiliate of Infineon
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
*****************************************************************************/

/*******************************************************************************
* Header Files
*******************************************************************************/

#include "cybsp.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"

// for TFM
#include "tfm_ns_interface.h"
#include "os_wrapper/common.h"



/*****************************************************************************
 * Macros
 *****************************************************************************/
/* The timeout value in microsecond used to wait for core to be booted
    Use value 0U for infinite wait till the core is booted successfully.*/
#define CM55_BOOT_WAIT_TIME_USEC            (10U)

/*****************************************************************************
 * Function Prototypes
 *****************************************************************************/

/*******************************************************************************
* Global Variables
********************************************************************************/


void test_task(void * arg) {
    (void) arg;
    while(true) {
        // This works:
        // printf("cm55-hello\n");
        vTaskDelay(pdMS_TO_TICKS(10000));
    }

}

int main(void)
{
    cy_rslt_t result;

    /* Initialize the device and board peripherals */
    result = cybsp_init() ;
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* Enable global interrupts */
    __enable_irq();

    /* Initialize TF-M interface */
    result = tfm_ns_interface_init();
    if(result != OS_WRAPPER_SUCCESS)
    {
        CY_ASSERT(0);
    }

    Cy_SysLib_Delay(50);

    result = xTaskCreate(test_task, "MQTT Client task", 4 * 1024, NULL, 3, NULL);
    
    /* Start the FreeRTOS scheduler. */
    vTaskStartScheduler();
}

/* Fix for undefined reference to tfm_ns_interface_init */
uint32_t tfm_ns_interface_init(void)
{
    /* For CM55 with Mailbox, initialization is handled by BSP/IPC or is not required via this API */
    return OS_WRAPPER_SUCCESS;
}

/* [] END OF FILE */