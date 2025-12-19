/* SPDX-License-Identifier: MIT
 * Copyright (C) 2025 Avnet
 * Authors: Nikola Markovic <nikola.markovic@avnet.com>, Shu Liu <shu.liu@avnet.com> et al.
 */

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include "FreeRTOS.h"
#include "task.h"

#include "iotcl.h"
#include "iotcl_dra_json_config.h"
#include "app_its_config.h"
#include "app_io.h"
#include "app_configurator_task.h"


void app_configurator_task(void * param) {
    (void) param;

    char input_buffer[512];

    printf("App Configurator Task: Started\n");

    app_io_write_str_crlf("Welcome to the /IOTCONNECT Device Configurator.");
    app_io_write_str_crlf("Please enter your your device configuration.");
    app_io_write_str_crlf("To accept defautts [in brackets], just press Enter.");

    if (app_its_config_is_valid()) {
        app_io_write_str_crlf("Device configuration is already stored.");
        app_io_write_str("Do you wish to clear the current configuration? [Y/n]: ");
        while (true) {
            app_io_read_lines(input_buffer, sizeof(input_buffer), false);
            if (strlen(input_buffer) == 0 || input_buffer[0] == 'Y' || input_buffer[0] == 'y') {
                app_its_config_clear();
                app_io_write_str_crlf("Configuration cleared.");
                app_io_write_str_crlf("");
                break;
            } else {
                app_io_write_str_crlf("Keeping the existing configuration.");
                break;
            }
        }
    }

    do {
        
        app_its_config_get_wifi_ssid(input_buffer);

        app_io_write_str("Enter WiFi SSID");
        if (NULL == app_its_config_get_wifi_ssid(NULL)) {
            app_io_write_str_crlf(" :");
        } else {
            app_io_write_str_crlf("[");
            app_io_write_str(app_its_config_get_wifi_ssid(NULL));
            app_io_write_str_crlf("]: ");
        }
        
        app_io_read_lines(input_buffer, sizeof(input_buffer), false);
        if (strlen(input_buffer) == 0 && NULL == app_its_config_get_wifi_ssid(NULL)) {
            app_io_write_str_crlf("WiFi SSID cannot be empty, please try again.");            
        } else {
            if (IOTCL_SUCCESS != app_its_config_set_wifi_ssid(input_buffer)) {
                app_io_write_str_crlf("ERROR: Invalid WiFi SSID, please try again.");
                printf("ERROR: Invalid WiFi SSID input: %s\n", input_buffer);
                continue;
            }
        }
    } while (NULL == app_its_config_get_wifi_ssid(NULL));

    do {        
        app_io_write_str("Enter WiFi Password");
        app_io_write_str_crlf("[");
        const char* current_value = app_its_config_get_wifi_pass(NULL);
        app_io_write_str(current_value ? current_value : "[not set]");
        app_io_write_str_crlf("]: ");
        
        app_io_read_lines(input_buffer, sizeof(input_buffer), false);
        if (strlen(input_buffer) > 0) {
            if (IOTCL_SUCCESS != app_its_config_set_wifi_pass(input_buffer)) {
                app_io_write_str_crlf("ERROR: Invalid WiFi Password, please try again.");
                printf("ERROR: Invalid WiFi Password input: %s\n", input_buffer);
                continue;
            }
        }
    } while (NULL == app_its_config_get_wifi_pass(NULL));
    
    bool keep_existing_config = false;
    if (app_its_config_is_valid() && 
    strlen(app_its_config_get_duid("")) > 0 &&
    strlen(app_its_config_get_cpid("")) > 0 &&
    strlen(app_its_config_get_env("")) > 0 &&
    app_its_config_get_platform_as_string(NULL) != NULL) {
        app_io_write_str_crlf("/IOTCONNECT device settings already exist.");
        app_io_write_str("Do you wish to overwrite /IOTCONNECT device settings? [N/y]: ");
        while (true) {
            input_buffer[0] = '\0';
            app_io_read_lines(input_buffer, sizeof(input_buffer), false);
            if (strlen(input_buffer) == 0 || input_buffer[0] == 'Y' || input_buffer[0] == 'y') {
                app_its_config_set_duid("");
                app_its_config_set_cpid("");
                app_its_config_set_env("");
                app_its_config_set_platform(IOTC_CT_UNDEFINED);
                app_io_write_str_crlf("/IOTCONNECT device settings.");
                break;
            } else {
                keep_existing_config = true; // break out of outer loop as well
                app_io_write_str_crlf("Keeping the existing configuration.");
                break;
            }
        }
    }

    if (!keep_existing_config) {
        int parsing_result;
        app_io_write_str_crlf("Device configuration can be found at your /IOTCONNECT device's Info pannel.");
        do {
            IotclDraJsonConfigResult json_config_result = {0};

            app_io_write_str_crlf("Copy and paste the Device JSON configuration (iotcDeviceConfig.json).");
            app_io_write_str_crlf("When done, press CTRL+D to process the configuration:");
            app_io_read_lines(input_buffer, sizeof(input_buffer), true);
            parsing_result = iotcl_dra_json_config_parse(&json_config_result, input_buffer);
            if (parsing_result != IOTCL_SUCCESS) {
                app_io_write_str_crlf("Configuration parsing failed. Please try again. Ensure to paste the entire JSON.");
                printf("Configuration failed with error: %d.\n", parsing_result);
            } else {

                app_io_write_str("Platform:");
                app_io_write_str("ENV:");
                app_io_write_str_crlf(json_config_result.env);
                app_io_write_str_crlf(json_config_result.pf);
                app_io_write_str("CPID:");
                app_io_write_str_crlf(json_config_result.cpid);
                app_io_write_str("DUID:");
                app_io_write_str_crlf(json_config_result.duid);
                
                app_its_config_set_platform_as_string(json_config_result.pf);
                app_its_config_set_env(json_config_result.env);
                app_its_config_set_cpid(json_config_result.cpid);
                app_its_config_set_duid(json_config_result.duid);

                iotcl_dra_json_config_free(&json_config_result);
                app_io_write_str_crlf("/IOTCONNECT device configuration applied successfully.");
                printf("/IOTCONNECT device configuration applied successfully.");  // const string re-use. \n after
                printf("\n");
            }
        } while (parsing_result != IOTCL_SUCCESS);
    }

    app_io_write_str_crlf("");
    app_io_write_str_crlf("Device configuration complete. Resetting device...");
    printf("\n");
    printf("Device configuration complete. Resetting device..."); // const string re-use \n after
    printf("\n");
    vTaskDelay(pdMS_TO_TICKS(2000));
    NVIC_SystemReset();
    
    // just for correctness. The device should reset before reaching here.
    while (true) {
        taskYIELD();
    }
}