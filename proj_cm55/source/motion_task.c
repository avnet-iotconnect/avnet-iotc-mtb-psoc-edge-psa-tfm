/******************************************************************************
 * File Name:   motion_task.c
 *
 * Description: This file contains the task that initializes and configures the
 *              BMI270 Motion Sensor and displays the sensor orientation.
 *
 * Related Document: See README.md
 *
 *
 *******************************************************************************
 * Copyright 2023-2025, Cypress Semiconductor Corporation (an Infineon company) or
 * an affiliate of Cypress Semiconductor Corporation.  All rights reserved.
 *
 * This software, including source code, documentation and related
 * materials ("Software") is owned by Cypress Semiconductor Corporation
 * or one of its affiliates ("Cypress") and is protected by and subject to
 * worldwide patent protection (United States and foreign),
 * United States copyright laws and international treaty provisions.
 * Therefore, you may use this Software only as provided in the license
 * agreement accompanying the software package from which you
 * obtained this Software ("EULA").
 * If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
 * non-transferable license to copy, modify, and compile the Software
 * source code solely for use in connection with Cypress's
 * integrated circuit products.  Any reproduction, modification, translation,
 * compilation, or representation of this Software except as specified
 * above is prohibited without the express written permission of Cypress.
 *
 * Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
 * reserves the right to make changes to the Software without notice. Cypress
 * does not assume any liability arising out of the application or use of the
 * Software or any product or circuit described in the Software. Cypress does
 * not authorize its products for use in any products where a malfunction or
 * failure of the Cypress product may reasonably be expected to result in
 * significant property damage, injury or death ("High Risk Product"). By
 * including Cypress's product in a High Risk Product, the manufacturer
 * of such system or application assumes all risk of such use and in doing
 * so agrees to indemnify Cypress against all liability.
 *******************************************************************************/

/*******************************************************************************
 * Header Files
 *******************************************************************************/
#include "motion_task.h"
#include "retarget_io_init.h"
#include "ipc_communication.h"

// The task spams console output, so disable prints (by default)
#ifndef ENABLE_MOTION_TASK_PRINTS
#undef printf
#define printf(...)
#endif

/*******************************************************************************
 * Global Variables
 ********************************************************************************/

static mtb_hal_i2c_t CYBSP_I2C_CONTROLLER_hal_obj;
static cy_stc_scb_i2c_context_t CYBSP_I2C_CONTROLLER_context;

/* Motion sensor task handle */
static TaskHandle_t motion_sensor_task_handle;

/* Instance of BMI270 sensor structure */
static mtb_bmi270_t bmi270;
static mtb_bmi270_data_t bmi270_data;

/******************************************************************************
 * Macros
 ******************************************************************************/
#define DELAY_MS (1000U)
/* Task priority and stack size for the Motion sensor task */
#define TASK_MOTION_SENSOR_PRIORITY     (configMAX_PRIORITIES - 1)
#define TASK_MOTION_SENSOR_STACK_SIZE   (1024U)
/* I2C Clock frequency in Hz */
#define I2C_CLK_FREQ_HZ                 (400000U)

/*******************************************************************************
 * Function Name: motion_sensor_init
 ********************************************************************************
 * Summary:
 *  Function that configures the I2C controller interface and then initializes
 *  the motion sensor.
 *
 * Parameters:
 *  None
 *
 * Return:
 * result
 *
 *******************************************************************************/
static cy_rslt_t motion_sensor_init(void)
{
    cy_rslt_t result;

    /* Initialize the I2C controller interface for BMI270 motion sensor */
    result = Cy_SCB_I2C_Init(CYBSP_I2C_CONTROLLER_HW,
            &CYBSP_I2C_CONTROLLER_config,
            &CYBSP_I2C_CONTROLLER_context);

    if(CY_RSLT_SUCCESS != result)
    {
        printf(" Error : I2C initialization failed !!\r\n");
        handle_app_error();
    }
    Cy_SCB_I2C_Enable(CYBSP_I2C_CONTROLLER_HW);

    /* Configure the I2C controller interface with the desired clock frequency */
    result = mtb_hal_i2c_setup(&CYBSP_I2C_CONTROLLER_hal_obj,
            &CYBSP_I2C_CONTROLLER_hal_config,
            &CYBSP_I2C_CONTROLLER_context,
            NULL);

    if(CY_RSLT_SUCCESS != result)
    {
        printf(" Error : I2C setup failed !!\r\n");
        handle_app_error();
    }

    /* Initialize the BMI270 motion sensor */
    result = mtb_bmi270_init_i2c(&bmi270, 
                                &CYBSP_I2C_CONTROLLER_hal_obj, 
                                MTB_BMI270_ADDRESS_DEFAULT);
    if(CY_RSLT_SUCCESS != result)
    {
        printf(" Error : IMU sensor init failed !!\r\n");
        handle_app_error();
    }

    result = mtb_bmi270_config_default(&bmi270);
    if(CY_RSLT_SUCCESS != result)
    {
        printf(" Error : IMU sensor config failed !!\r\n");
        handle_app_error();
    }

    return result;
}

/*******************************************************************************
 * Function Name: motion_sensor_update_orientation
 ********************************************************************************
 * Summary:
 *  Function that updates the orientation status to one of the 6 types,
 *  'ORIENTATION_UP, ORIENTATION_DOWN, TOP_EDGE, BOTTOM_EDGE,
 *  LEFT_EDGE, and RIGHT_EDGE'. This functions detects the axis that is most perpendicular
 *  to the ground based on the absolute value of acceleration in that axis.
 *  The sign of the acceleration signifies whether the axis is facing the ground
 *  or the opposite.
 *
 * Return:
 *  CY_RSLT_SUCCESS upon successful orientation update, else a non-zero value
 *  that indicates the error.
 *
 *******************************************************************************/
static cy_rslt_t motion_sensor_update_orientation(void)
{
    ipc_payload_t* payload = cm55_ipc_get_payload_ptr();
    /* Status variable */
    cy_rslt_t result = CY_RSLT_SUCCESS;
    int16_t abs_x;
    int16_t abs_y;
    int16_t abs_z;

    /* Read x, y, z components of acceleration */
    result = mtb_bmi270_read(&bmi270, &bmi270_data);
    if (CY_RSLT_SUCCESS != result)
    {
        printf("read data failed\r\n");
    }
    abs_x = abs(bmi270_data.sensor_data.acc.x);
    abs_y = abs(bmi270_data.sensor_data.acc.y);
    abs_z = abs(bmi270_data.sensor_data.acc.z);

    if ((abs_z > abs_x) && (abs_z > abs_y))
    {
        if (bmi270_data.sensor_data.acc.z < 0)
        {
            /* Kit faces down (towards the ground) */
            printf("Orientation = ORIENTATION_DOWN\r\n");
            payload->label_id = 1;
            strcpy(payload->label, "down");
        }
        else
        {
            /* Kit faces up (towards the sky/ceiling) */
            printf("Orientation = ORIENTATION_UP\r\n");
            payload->label_id = 0;
            strcpy(payload->label, "up");
        }
    }
    /* Y axis (parallel with shorter edge of board) is most aligned with
     * gravity.
     */
    else if ((abs_y > abs_x) && (abs_y > abs_z))
    {
        if (bmi270_data.sensor_data.acc.y > 0)
        {
            /* Kit has an inverted landscape orientation */
            printf("Orientation = ORIENTATION_BOTTOM_EDGE\r\n");
            payload->label_id = 3;
            strcpy(payload->label, "bottom");
            
        }
        else
        {
            /* Kit has landscape orientation */
            printf("Orientation = ORIENTATION_TOP_EDGE\r\n");
            payload->label_id = 2;
            strcpy(payload->label, "top");
        }
    }
    /* X axis (parallel with longer edge of board) is most aligned with
     * gravity.
     */
    else
    {
        if (bmi270_data.sensor_data.acc.x < 0)
        {
            /* Kit has an inverted portrait orientation */
            printf("Orientation = ORIENTATION_RIGHT_EDGE\r\n");
            payload->label_id = 5;
            strcpy(payload->label, "right_edge");
        }
        else
        {
            /* Kit has portrait orientation */
            printf("Orientation = ORIENTATION_LEFT_EDGE\r\n");
            payload->label_id = 4;
            strcpy(payload->label, "left_edge");
        }
    }
    cm55_ipc_send_to_cm33();
    return result;
}

/*******************************************************************************
 * Function Name: task_motion
 ********************************************************************************
 * Summary:
 *  Task that configures the Motion Sensor and processes the sensor data to
 *  display the sensor orientation.
 *
 * Parameters:
 *  void *pvParameters : Task parameter defined during task creation (unused)
 *
 * Return:
 *  None
 *
 *******************************************************************************/
static void task_motion(void* pvParameters)
{
    /* Status variable to indicate the result of various operations */
    cy_rslt_t result;

    /* Remove warning for unused parameter */
    CY_UNUSED_PARAMETER(pvParameters);

    /* \x1b[2J\x1b[;H - ANSI ESC sequence for clear screen */
    printf("\x1b[2J\x1b[;H");

    printf("***************************************************************************\n");
    printf("    PSOC Edge MCU: Interfacing IMU Sensor Through I2C (FreeRTOS)    \n");
    printf("***************************************************************************\r\n");

    /* Initialize BMI270 motion sensor and suspend the task upon failure */
    result = motion_sensor_init();
    if(CY_RSLT_SUCCESS != result)
    {
        printf(" Error : Motion Sensor initialization failed !!\n Check hardware connection\r\n");
        handle_app_error();
    }
    printf("BMI270 Motion Sensor successfully initialized.\r\n");

    printf("Change the orientation of the kit to observe different orientation values.\r\n\n");

    for(;;)
    {
        /* Get current orientation */
        motion_sensor_update_orientation();
        vTaskDelay( DELAY_MS/portTICK_PERIOD_MS );
    }
}

/*******************************************************************************
 * Function Name: create_motion_sensor_task
 ********************************************************************************
 * Summary:
 *  Function that creates the motion sensor task.
 *
 * Parameters:
 *  None
 *
 * Return:
 *  CY_RSLT_SUCCESS upon successful creation of the motion sensor task, else a
 *  non-zero value that indicates the error.
 *
 *******************************************************************************/
cy_rslt_t create_motion_sensor_oritentation_task(void)
{
    BaseType_t status;

    status = xTaskCreate(task_motion, "Motion Sensor Task", TASK_MOTION_SENSOR_STACK_SIZE,
            NULL, TASK_MOTION_SENSOR_PRIORITY, &motion_sensor_task_handle);

    return (pdPASS == status) ? CY_RSLT_SUCCESS : (cy_rslt_t) status;
}

/* [] END OF FILE */