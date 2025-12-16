/*******************************************************************************
* File Name:   main.c
*
* Description: This is the source code for the USB Device CDC echo Example
*              for non-secure application in the CM33 CPU.
*
* Related Document: See README.md
*
*
********************************************************************************
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

#include <stdio.h>
#include "cybsp.h"
#include "mtb_hal.h"

#include "USB.h"
#include "USB_CDC.h"

#include "FreeRTOS.h"
#include "task.h"

#include "app_io.h"


/*******************************************************************************
* Macros
********************************************************************************/
#define USB_CONFIG_DELAY_MS       (50u)    /* In milliseconds */
#define VENDOR_ID                 (0x058B)
#define PRODUCT_ID                (0x0282)
#define RESET_VAL                 (0u)
#define USB_ENABLE_FLAG           (0u)
#define USB_BULK_IN_INTERVAL      (0u)
#define USB_BULK_OUT_INTERVAL     (0u)
#define USB_INT_INTERVAL          (64u) 

#define GPIO_LOW                  (0u)
#define GPIO_HIGH                 (1u)

/* The timeout value in microsecond used to wait for core to be booted */
#define CM55_BOOT_WAIT_TIME_USEC  (10u)

/* App boot address for CM55 project */
#define CM55_APP_BOOT_ADDR          (CYMEM_CM33_0_m55_nvm_START + \
                                        CYBSP_MCUBOOT_HEADER_SIZE)

#ifndef USB_CDC_PRODUCT_NAME
#define USB_CDC_PRODUCT_NAME       "Avnet CDC Configurator"
#endif

static const USB_DEVICE_INFO usb_device_info = {
    VENDOR_ID,                    /* VendorId    */
    PRODUCT_ID,                   /* ProductId    */
    "Infineon Technologies",      /* VendorName   */
    USB_CDC_PRODUCT_NAME,         /* ProductName  */
    "12839847"                    /* SerialNumber */
};

// Handle for the CDC class isntance that is used in all calls
static USB_CDC_HANDLE usb_cdc_handle;

/* ****************************************************************************** */

static void usb_add_cdc(void) 
{
    static U8             out_buffer[USB_HS_BULK_MAX_PACKET_SIZE];
    USB_CDC_INIT_DATA     init_data;
    USB_ADD_EP_INFO       ep_bulk_in;
    USB_ADD_EP_INFO       ep_bulk_out;
    USB_ADD_EP_INFO       ep_int_in;

    memset(&init_data, 0, sizeof(init_data));
    ep_bulk_in.Flags          = USB_ENABLE_FLAG;               /* Flags not used */
    ep_bulk_in.InDir          = USB_DIR_IN;                    /* IN direction (Device to Host) */
    ep_bulk_in.Interval       = USB_BULK_IN_INTERVAL;          /* Interval not used for Bulk endpoints */
    ep_bulk_in.MaxPacketSize  = USB_HS_BULK_MAX_PACKET_SIZE;   /* Maximum packet size (512B for Bulk in High-Speed) */
    ep_bulk_in.TransferType   = USB_TRANSFER_TYPE_BULK;        /* Endpoint type - Bulk */
    init_data.EPIn  = USBD_AddEPEx(&ep_bulk_in, NULL, 0);

    ep_bulk_out.Flags         = USB_ENABLE_FLAG;               /* Flags not used */
    ep_bulk_out.InDir         = USB_DIR_OUT;                   /* OUT direction (Host to Device) */
    ep_bulk_out.Interval      = USB_BULK_OUT_INTERVAL;         /* Interval not used for Bulk endpoints */
    ep_bulk_out.MaxPacketSize = USB_HS_BULK_MAX_PACKET_SIZE;   /* Maximum packet size (512B for Bulk in High-Speed) */
    ep_bulk_out.TransferType  = USB_TRANSFER_TYPE_BULK;        /* Endpoint type - Bulk */
    init_data.EPOut = USBD_AddEPEx(&ep_bulk_out, out_buffer, sizeof(out_buffer));

    ep_int_in.Flags           = USB_ENABLE_FLAG;               /* Flags not used */
    ep_int_in.InDir           = USB_DIR_IN;                    /* IN direction (Device to Host) */
    ep_int_in.Interval        = USB_INT_INTERVAL;              /* Interval of 8 ms (64 * 125us) */
    ep_int_in.MaxPacketSize   = USB_HS_INT_MAX_PACKET_SIZE ;   /* Maximum packet size (64 for Interrupt) */
    ep_int_in.TransferType    = USB_TRANSFER_TYPE_INT;         /* Endpoint type - Interrupt */
    init_data.EPInt = USBD_AddEPEx(&ep_int_in, NULL, 0);

    usb_cdc_handle = USBD_CDC_Add(&init_data);
}

void app_io_write_data(const char* data, size_t data_len) {
    size_t total_written = 0;
    int result;

    while (total_written < data_len) {
        result = USBD_CDC_Write(usb_cdc_handle, data + total_written, data_len - total_written, 0);
            printf("Chunk result=%d\n", result);
        if (result < 0) {
            // Handle error (e.g., log or break)
            break;
        }
        total_written += result;
    }

    // Wait for all queued data to be transmitted
    USBD_CDC_WaitForTX(usb_cdc_handle, 0);

}

void app_io_write_data_crlf(const char* data, size_t data_len) {
    if (data_len >= 2 && data[data_len - 2] == '\r' && data[data_len - 1] == '\n') {
        data_len = data_len - 2;
    } else if (data_len >= 1 && (data[data_len - 1] == '\n' || data[data_len - 1] == '\r')) {
        data_len = data_len - 1;        
    }
    app_io_write_data(data, data_len);
    app_io_write_data("\r\n", 2);
}

void app_io_write_str_crlf(const char* data) {
    app_io_write_data_crlf(data, strlen(data));
}

void app_io_write_str(const char* data) {
    app_io_write_data(data, strlen(data));
}

int app_io_read_lines(char * buffer, size_t buffer_len, bool until_eod) {
    size_t total_received = 0;
    if (buffer_len < 1) {
        printf("usb_read_lines: buffer_len must be at least 1 to accommodate null terminator\n");
    }
    
    buffer[0] = '\0'; // clear the buffer
    
    vTaskDelay(pdMS_TO_TICKS(10));
    
    while (USB_STAT_CONFIGURED != (USBD_GetState() & (USB_STAT_CONFIGURED | USB_STAT_SUSPENDED)))
    {
        Cy_GPIO_Inv(CYBSP_USER_LED_PORT, CYBSP_USER_LED_PIN);
        vTaskDelay(pdMS_TO_TICKS(200));
    }

    Cy_GPIO_Write(CYBSP_USER_LED_PORT, CYBSP_USER_LED_PIN, GPIO_HIGH);

    while (true) {
        int num_bytes_received = USBD_CDC_Receive(usb_cdc_handle, &buffer[total_received], buffer_len - total_received - 1 /* for terminator*/, 0);
        if (num_bytes_received > 0) {
            /* host side echo */            
            app_io_write_data(&buffer[total_received], num_bytes_received);
            // hopefully this is right... At least in TeraTerm it seems to work correctly in CR mode:
            if (buffer[total_received + num_bytes_received - 1] == '\r') {
                app_io_write_data("\n", 1);
            }
        } else {
            vTaskDelay(pdMS_TO_TICKS(10)); // wait a bit before trying again
            continue;
        }

        bool found_terminator = false;
        for (int i = 0; i < num_bytes_received; i++) {
            char ch = buffer[total_received + i];
            if (!until_eod) {
                if (ch == '\n' || ch == '\r') {
                    num_bytes_received = i; // truncate to the newline
                    found_terminator = true;
                    break;
                }
            } else {
                if (ch == 0x04) { // EOD character
                    num_bytes_received = i; // truncate to the EOD character
                    found_terminator = true;
                    break;
                }
            }
        }
        total_received += num_bytes_received;
        if (found_terminator) {
            break;
        }
    } 

    buffer[total_received] = '\0';

    Cy_GPIO_Write(CYBSP_USER_LED_PORT, CYBSP_USER_LED_PIN, GPIO_LOW);

    return (int) total_received;

}

int app_io_init(void) {
    printf("Initializing the Configurator\n");

    /* Initializes the USB stack */
    USBD_Init();

    /* Endpoint Initialization for CDC class */
    usb_add_cdc();

    /* Set device info used in enumeration */
    USBD_SetDeviceInfo(&usb_device_info);

    /* Start the USB stack */
    USBD_Start();

    return 0;
}

void usb_cdc_test(void * param) {
    (void) param;

    app_io_init();

    vTaskDelay(pdMS_TO_TICKS(3000));

    app_io_write_str_crlf("Hello from USB CDC!");
    app_io_write_str("0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789!\r\n");

    char read_buffer[1024];
    app_io_write_str_crlf("Please type something and press Enter:");
    app_io_read_lines(read_buffer, sizeof(read_buffer), false);
    app_io_write_str_crlf(""); 
    printf("Received from USB CDC: %s\n", read_buffer);

    app_io_write_str_crlf("Please type something and press CTRL+D:");
    app_io_read_lines(read_buffer, sizeof(read_buffer), true);    
    app_io_write_str_crlf(""); // most terminals will just have CR in the input, so add a newline
    printf("Received from USB CDC until EOD: %s\n", read_buffer);
    for(;;) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

/* [] END OF FILE */
