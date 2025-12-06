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
#include "retarget_io_init.h"
#include "ipc_communication.h"

#if defined(GESTURE_MODEL)
#include "radar.h"
#elif defined(FALLDETECTION_MODEL)
#include "imu.h"
#elif defined(DIRECTIONOFARRIVAL_MODEL)
#include "doa.h"
#elif defined(MOTION_SENSOR)
#include "motion_task.h"
#elif defined(COUGH_MODEL) || defined(ALARM_MODEL) || defined(BABYCRY_MODEL)
#include "audio.h"
#endif

#ifdef COMPONENT_FREERTOS
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#endif


/* The timeout value in microsecond used to wait for core to be booted
    Use value 0U for infinite wait till the core is booted successfully.*/
#define CM55_BOOT_WAIT_TIME_USEC            (10U)

// This can be used for troubleshooting IPC issues
void test_task(void * arg) {
    (void) arg;
    while(true) {
        // This works:
        ipc_payload_t* payload = cm55_ipc_get_payload_ptr();
        payload->label_id = 1;
        strcpy(payload->label, "test");
        printf("Hello from CM55 test\n");
        vTaskDelay(pdMS_TO_TICKS(5000));
        cm55_ipc_send_to_cm33();
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

    /* Initialize retarget-io middleware */
    init_retarget_io();

    /* Setup IPC communication for CM55*/
    cm55_ipc_communication_setup();

    Cy_SysLib_Delay(50);

#if 0
    printf("\x1b[2J\x1b[;H");
#endif

    /* Create the RTOS task */
#if defined(GESTURE_MODEL)
    result = create_radar_task();
#elif defined(FALLDETECTION_MODEL)
    result = create_motion_sensor_task();
#elif defined(DIRECTIONOFARRIVAL_MODEL)
    result = create_doa_task();
#elif defined(MOTION_SENSOR)
    result = create_motion_sensor_oritentation_task();
#elif defined(COUGH_MODEL) || defined(ALARM_MODEL) || defined(BABYCRY_MODEL)
    result = create_audio_task();
#else // IDLE or unknown
    (void) result;
    xTaskCreate(test_task, "TestTask", 4 * 1024, NULL, 3, NULL);
#endif
    /* Start the FreeRTOS scheduler. */
    vTaskStartScheduler();
}

/* [] END OF FILE */
